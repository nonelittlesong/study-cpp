#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/opencv.hpp>
#include "mtcnnAttribute.h"

bool cmpScore(orderScore lsh, orderScore rsh){
    if(lsh.score<rsh.score)
        return true;
    else
        return false;
}

static float getElapse(struct timeval *tv1,struct timeval *tv2)
{
    float t = 0.0f;
    if (tv1->tv_sec == tv2->tv_sec)
        t = (tv2->tv_usec - tv1->tv_usec)/1000.0f;
    else
        t = ((tv2->tv_sec - tv1->tv_sec) * 1000 * 1000 + tv2->tv_usec - tv1->tv_usec)/1000.0f;
    return t;
}

mtcnn::mtcnn(){
    Pnet.load_param("../models/det1.param");
    Pnet.load_model("../models/det1.bin");
    Rnet.load_param("../models/det2.param");
    Rnet.load_model("../models/det2.bin");
    Onet.load_param("../models/det3.param");
    Onet.load_model("../models/det3.bin");
}

void mtcnn::generateBbox(ncnn::Mat score, ncnn::Mat location, std::vector<Bbox>& boundingBox_, std::vector<orderScore>& bboxScore_, float scale){
    int stride = 2;
    int cellsize = 12;
    int count = 0;
    //score p
    float *p = score.channel(1);//score.data + score.cstep;
    float *plocal = location.channel(0);   //***************
    Bbox bbox;
    orderScore order;
    for(int row=0;row<score.h;row++){
        for(int col=0;col<score.w;col++){
            if(*p>threshold[0]){
                bbox.score = *p;
                order.score = *p;
                order.oriOrder = count;
                bbox.x1 = round((stride*col+1)/scale);
                bbox.y1 = round((stride*row+1)/scale);
                bbox.x2 = round((stride*col+1+cellsize)/scale);
                bbox.y2 = round((stride*row+1+cellsize)/scale);
                bbox.exist = true;
                bbox.area = (bbox.x2 - bbox.x1)*(bbox.y2 - bbox.y1);
                for(int channel=0;channel<4;channel++)
                    bbox.regreCoord[channel]=location.channel(channel)[0];
                boundingBox_.push_back(bbox);
                bboxScore_.push_back(order);
                count++;
            }
            p++;
            plocal++;
        }
    }
}
void mtcnn::nms(std::vector<Bbox> &boundingBox_, std::vector<orderScore> &bboxScore_, const float overlap_threshold, string modelname){
    if(boundingBox_.empty()){
        return;
    }
    std::vector<int> heros;
    //sort the score
    sort(bboxScore_.begin(), bboxScore_.end(), cmpScore);

    int order = 0;
    float IOU = 0;
    float maxX = 0;
    float maxY = 0;
    float minX = 0;
    float minY = 0;
    while(bboxScore_.size()>0){
        order = bboxScore_.back().oriOrder;
        bboxScore_.pop_back();
        if(order<0)continue;
        if(boundingBox_.at(order).exist == false) continue;
        heros.push_back(order);
        boundingBox_.at(order).exist = false;//delete it

        for(int num=0;num<boundingBox_.size();num++){
            if(boundingBox_.at(num).exist){
                //the iou
                maxX = (boundingBox_.at(num).x1>boundingBox_.at(order).x1)?boundingBox_.at(num).x1:boundingBox_.at(order).x1;
                maxY = (boundingBox_.at(num).y1>boundingBox_.at(order).y1)?boundingBox_.at(num).y1:boundingBox_.at(order).y1;
                minX = (boundingBox_.at(num).x2<boundingBox_.at(order).x2)?boundingBox_.at(num).x2:boundingBox_.at(order).x2;
                minY = (boundingBox_.at(num).y2<boundingBox_.at(order).y2)?boundingBox_.at(num).y2:boundingBox_.at(order).y2;
                //maxX1 and maxY1 reuse 
                maxX = ((minX-maxX+1)>0)?(minX-maxX+1):0;
                maxY = ((minY-maxY+1)>0)?(minY-maxY+1):0;
                //IOU reuse for the area of two bbox
                IOU = maxX * maxY;
                if(!modelname.compare("Union"))
                    IOU = IOU/(boundingBox_.at(num).area + boundingBox_.at(order).area - IOU);
                else if(!modelname.compare("Min")){
                    IOU = IOU/((boundingBox_.at(num).area<boundingBox_.at(order).area)?boundingBox_.at(num).area:boundingBox_.at(order).area);
                }
                if(IOU>overlap_threshold){
                    boundingBox_.at(num).exist=false;
                    for(vector<orderScore>::iterator it=bboxScore_.begin(); it!=bboxScore_.end();it++){
                        if((*it).oriOrder == num) {
                            (*it).oriOrder = -1;
                            break;
                        }
                    }
                }
            }
        }
    }
    for(int i=0;i<heros.size();i++)
        boundingBox_.at(heros.at(i)).exist = true;
}
void mtcnn::refineAndSquareBbox(vector<Bbox> &vecBbox, const int &height, const int &width){
    if(vecBbox.empty()){
        cout<<"Bbox is empty!!"<<endl;
        return;
    }
    float bbw=0, bbh=0, maxSide=0;
    float h = 0, w = 0;
    float x1=0, y1=0, x2=0, y2=0;
    for(vector<Bbox>::iterator it=vecBbox.begin(); it!=vecBbox.end();it++){
        if((*it).exist){
            bbw = (*it).x2 - (*it).x1 + 1;
            bbh = (*it).y2 - (*it).y1 + 1;
            x1 = (*it).x1 + (*it).regreCoord[0]*bbw;
            y1 = (*it).y1 + (*it).regreCoord[1]*bbh;
            x2 = (*it).x2 + (*it).regreCoord[2]*bbw;
            y2 = (*it).y2 + (*it).regreCoord[3]*bbh;

            w = x2 - x1 + 1;
            h = y2 - y1 + 1;
          
            maxSide = (h>w)?h:w;
            x1 = x1 + w*0.5 - maxSide*0.5;
            y1 = y1 + h*0.5 - maxSide*0.5;
            (*it).x2 = round(x1 + maxSide - 1);
            (*it).y2 = round(y1 + maxSide - 1);
            (*it).x1 = round(x1);
            (*it).y1 = round(y1);

            //boundary check
            if((*it).x1<0)(*it).x1=0;
            if((*it).y1<0)(*it).y1=0;
            if((*it).x2>width)(*it).x2 = width - 1;
            if((*it).y2>height)(*it).y2 = height - 1;

            it->area = (it->x2 - it->x1)*(it->y2 - it->y1);
        }
    }
}
void mtcnn::detect(ncnn::Mat& img_, std::vector<Bbox>& finalBbox_){
    firstBbox_.clear();
    firstOrderScore_.clear();
    secondBbox_.clear();
    secondBboxScore_.clear();
    thirdBbox_.clear();
    thirdBboxScore_.clear();
    
    img = img_;
    img_w = img.w;
    img_h = img.h;
    img.substract_mean_normalize(mean_vals, norm_vals);

    float minl = img_w<img_h?img_w:img_h;
    int MIN_DET_SIZE = 20;
    int minsize = 40;
    float m = (float)MIN_DET_SIZE/minsize;
    minl *= m;
    float factor = 0.709;
    int factor_count = 0;
    vector<float> scales_;
    while(minl>MIN_DET_SIZE){
        if(factor_count>0)m = m*factor;
        scales_.push_back(m);
        minl *= factor;
        factor_count++;
    }
    orderScore order;
    int count = 0;

    for (size_t i = 0; i < scales_.size(); i++) {
        int hs = (int)ceil(img_h*scales_[i]);
        int ws = (int)ceil(img_w*scales_[i]);
        //ncnn::Mat in = ncnn::Mat::from_pixels_resize(image_data, ncnn::Mat::PIXEL_RGB2BGR, img_w, img_h, ws, hs);
        ncnn::Mat in;
        resize_bilinear(img_, in, ws, hs);
        //in.substract_mean_normalize(mean_vals, norm_vals);
        ncnn::Extractor ex = Pnet.create_extractor();
        ex.set_light_mode(true);
        ex.input("data", in);
        ncnn::Mat score_, location_;
        ex.extract("prob1", score_);
        ex.extract("conv4-2", location_);
        std::vector<Bbox> boundingBox_;
        std::vector<orderScore> bboxScore_;
        generateBbox(score_, location_, boundingBox_, bboxScore_, scales_[i]);
        nms(boundingBox_, bboxScore_, nms_threshold[0]);

        for(vector<Bbox>::iterator it=boundingBox_.begin(); it!=boundingBox_.end();it++){
            if((*it).exist){
                firstBbox_.push_back(*it);
                order.score = (*it).score;
                order.oriOrder = count;
                firstOrderScore_.push_back(order);
                count++;
            }
        }
        bboxScore_.clear();
        boundingBox_.clear();
    }
    //the first stage's nms
    if(count<1)return;
    nms(firstBbox_, firstOrderScore_, nms_threshold[0]);
    refineAndSquareBbox(firstBbox_, img_h, img_w);
    std::cout<<"firstBbox_.size()= "<< firstBbox_.size() <<std::endl;

    //second stage
    count = 0;
    for(vector<Bbox>::iterator it=firstBbox_.begin(); it!=firstBbox_.end();it++){
        if((*it).exist){
            ncnn::Mat tempIm;
            copy_cut_border(img, tempIm, (*it).y1, img_h-(*it).y2, (*it).x1, img_w-(*it).x2);
            ncnn::Mat in;
            resize_bilinear(tempIm, in, 24, 24);
            ncnn::Extractor ex = Rnet.create_extractor();
            ex.set_light_mode(true);
            ex.input("data", in);
            ncnn::Mat score, bbox;
            ex.extract("prob1", score);
            ex.extract("conv5-2", bbox);
            if((score[1])>threshold[1]){
                for(int channel=0;channel<4;channel++)
                    it->regreCoord[channel]=bbox[channel];
                it->area = (it->x2 - it->x1)*(it->y2 - it->y1);
                it->score = score[1];
                secondBbox_.push_back(*it);
                order.score = it->score;
                order.oriOrder = count++;
                secondBboxScore_.push_back(order);
		}

            else{
                (*it).exist=false;
            }
        }
    }
    std::cout<<"secondBbox_.size()= "<< secondBbox_.size() <<std::endl;
    if(count<1)return;
    nms(secondBbox_, secondBboxScore_, nms_threshold[1]);
    refineAndSquareBbox(secondBbox_, img_h, img_w);

    //third stage 
    count = 0;
    for(vector<Bbox>::iterator it=secondBbox_.begin(); it!=secondBbox_.end();it++){
        if((*it).exist){
            ncnn::Mat tempIm;
            copy_cut_border(img, tempIm, (*it).y1, img_h-(*it).y2, (*it).x1, img_w-(*it).x2);
            ncnn::Mat in;
            resize_bilinear(tempIm, in, 48, 48);
            ncnn::Extractor ex = Onet.create_extractor();
            ex.set_light_mode(true);
            ex.input("data", in);
            ncnn::Mat score, bbox, keyPoint;
            ex.extract("prob1", score);
            ex.extract("conv6-2", bbox);
            ex.extract("conv6-3", keyPoint);
            if(score[1]>threshold[2]){
                for(int channel=0;channel<4;channel++)
                    it->regreCoord[channel]=bbox[channel];
                it->area = (it->x2 - it->x1)*(it->y2 - it->y1);
                it->score = score[1];
                for(int num=0;num<5;num++){
                    (it->ppoint)[num] = it->x1 + (it->x2 - it->x1)*keyPoint[num];
                    (it->ppoint)[num+5] = it->y1 + (it->y2 - it->y1)*keyPoint[num+5];
		}

                thirdBbox_.push_back(*it);
                order.score = it->score;
                order.oriOrder = count++;
                thirdBboxScore_.push_back(order);
            }
            else
                (*it).exist=false;
            }
        }

    std::cout<<"thirdBbox_.size()= "<< thirdBbox_.size() <<std::endl;
    if(count<1)return;
    refineAndSquareBbox(thirdBbox_, img_h, img_w);
    nms(thirdBbox_, thirdBboxScore_, nms_threshold[2], "Min");
    finalBbox_ = thirdBbox_;
}

//===================================================================================


void Attribute::setup(){
    squeezenet.load_param("../models/resnet.param");
    squeezenet.load_model("../models/resnet.bin");
}
/*
void Attribute::classify(const cv::Mat& bgr, std::vector<float>& cls_scores){
    //ncnn::Mat in = ncnn::Mat::from_pixels_resize(bgr.data, ncnn::Mat::PIXEL_BGR, bgr.cols, bgr.rows, 224, 224);
    AtbIn = ncnn::Mat::from_pixels_resize(bgr.data, ncnn::Mat::PIXEL_BGR, bgr.cols, bgr.rows, 224, 224);
    const float mean_vals[3] = {104.f, 117.f, 123.f};
    AtbIn.substract_mean_normalize(mean_vals, 0);
    ncnn::Extractor ex = squeezenet.create_extractor();
    ex.set_light_mode(true);
    //ex.set_num_threads(4);
    int64 t00 = cv::getTickCount();
    ex.input("data", AtbIn);
    //ncnn::Mat out;
    ex.extract("prob", AtbOut);
    int64 t11 = cv::getTickCount();
    double secs1 = (t11-t00)/cv::getTickFrequency();
    std::cout<<"attribute inference secs time:"<< secs1 <<" s "<<std::endl;

    cls_scores.resize(AtbOut.w);
    for (int j=0; j<AtbOut.w; j++)
    {
        cls_scores[j] = AtbOut[j];
    }
}
*/
void Attribute::classify(const cv::Mat& bgr, std::vector<float>& gender_scores, std::vector<float>& age_scores) {
    AtbIn = ncnn::Mat::from_pixels_resize(bgr.data, ncnn::Mat::PIXEL_BGR, bgr.cols, bgr.rows, 224, 224);
    ncnn::Extractor ex = squeezenet.create_extractor();
    ex.set_light_mode(true);
    //ex.set_num_threads(4);
    int64 t00 = cv::getTickCount();
    //ex.input("data", AtbIn);
    ex.input("Data1", AtbIn);
    // gender
    ex.extract("prob_gender", AtbOut_gender);
    gender_scores.resize(AtbOut_gender.w);
    for (int j=0; j<AtbOut_gender.w; j++)
    {
        gender_scores[j] = AtbOut_gender[j];
    }
    // age
    ex.extract("prob_age", AtbOut_age);
    age_scores.resize(AtbOut_age.w);
    for (int j = 0; j < AtbOut_age.w; ++j)
    {
        age_scores[j] = AtbOut_age[j];
    }

    int64 t11 = cv::getTickCount();
    double secs1 = (t11-t00)/cv::getTickFrequency();
    cout << "attributes detect time: " << secs1 << endl;
}
static int getMaxId(std::vector<float>& arr, float &confidence){
  //assert(arr != NULL);
    int num = arr.size();
    float max_val = arr[0];
    int id = 0;
    for (int i = 1; i < num; i++) {
      if (arr[i] > max_val) {
          std::cout<<"arr[i] : "<< arr[i] <<std::endl;
          max_val = arr[i];
          id = i;
      }
    }
    std::cout<<"confidence : "<< max_val <<std::endl;
    confidence = max_val;   
    return id;
}

cv::Mat cropFaceForAttr(cv::Mat& image, int x1, int y1, int x2, int y2){
    cv::Size crop_size = cv::Size(224, 224);
    cv::Rect roi;
    cv::Mat crop;
    roi.x = x1;
    roi.y = y1;
    roi.width = x2 - x1 + 1;
    roi.height = y2 - y1 + 1;
    if(0 <= roi.x && 0 <= roi.width && roi.x + roi.width <= image.cols && 0 <= roi.y && 0 <= roi.height && roi.y + roi.height <= image.rows )
    {
        crop = image(roi);
        cv::resize(crop, crop, crop_size, 0, 0, CV_INTER_LINEAR);
    }
    return crop;
}

//====================================================================
void test_video() {
    mtcnn mm;
    Attribute attribute;
    attribute.setup();

    /*
    cv::VideoCapture mVideoCapture(0);
    if (!mVideoCapture.isOpened()) {
        return;
    }
    */
    // 读取图片
    std::ifstream infile;
    infile.open("../newdata.txt", std::ios::in);
    std::string path;
    int age, gender;

    // 计算准确率
    int picture_num, face_exist, face_num, acc_gender, acc_age = 0;

    cv::Mat frame;
    while (infile.peek()!=EOF) {
        /*
        mVideoCapture >> frame;
	if (frame.empty()) {
            break;
        }
        */
        // 读取图片
        infile >> path >> age >> gender;
        frame = cv::imread("../"+path);
        if (frame.empty() == true) continue;
        picture_num++;

        std::cout << "read jpg: " << path << std::endl;
        std::cout << "age: " << age << std::endl;
        std::cout << "gender: " << gender << std::endl;
        int64 t1 = cv::getTickCount();

        ncnn::Mat ncnn_img = ncnn::Mat::from_pixels(frame.data, ncnn::Mat::PIXEL_BGR2RGB, frame.cols, frame.rows);
        std::vector<Bbox> finalBbox;
        mm.detect(ncnn_img, finalBbox);
        const int num_box = finalBbox.size();
        
        // 统计数据
        if (num_box > 0) face_exist++;
        if (num_box != 1) continue;
        face_num++;

        std::vector<cv::Rect> bbox;
        bbox.resize(num_box);

        //std::vector<float> cls_scores;
        std::vector<float> gender_scores;
        std::vector<float> age_scores;

        float confidence = 0;

        int total = 0;
        for(vector<Bbox>::iterator it=finalBbox.begin(); it!=finalBbox.end();it++){
            if((*it).exist){
                total++;
                cv::Mat attCropImg = cropFaceForAttr(frame, (*it).x1, (*it).y1, (*it).x2, (*it).y2 );
                attribute.classify(attCropImg, gender_scores, age_scores);
                // gender
                int genderID = getMaxId(gender_scores, confidence);
                if(genderID == 0){
                    cv::rectangle(frame, Point((*it).x1, (*it).y1), Point((*it).x2, (*it).y2), Scalar(255, 0, 255), 2,8,0);
                }
                else{
                    cv::rectangle(frame, Point((*it).x1, (*it).y1), Point((*it).x2, (*it).y2), Scalar(255, 255, 0), 2,8,0);
                }
                if (genderID == gender) acc_gender++;
		        // age
                confidence = 0;
                int ageID = getMaxId(age_scores, confidence);
                switch (ageID) {
                    case 0:
                        cv::putText(frame, "child", Point((*it).x1, (*it).y1), cv::FONT_HERSHEY_COMPLEX, 1, Scalar(0, 255, 255));
                        break;
                    case 1:
                        cv::putText(frame, "teenager", Point((*it).x1, (*it).y1), cv::FONT_HERSHEY_COMPLEX, 1, Scalar(0, 255, 255));
                        break;
                    case 2:
                        cv::putText(frame, "young", Point((*it).x1, (*it).y1), cv::FONT_HERSHEY_COMPLEX, 1, Scalar(0, 255, 255));
                        break;
                    case 3:
                        cv::putText(frame, "moddle", Point((*it).x1, (*it).y1), cv::FONT_HERSHEY_COMPLEX, 1, Scalar(0, 255, 255));
                        break;
                    case 4:
                        cv::putText(frame, "old", Point((*it).x1, (*it).y1), cv::FONT_HERSHEY_COMPLEX, 1, Scalar(0, 255, 255));
                        break;
                    default:
                        cv::putText(frame, "unknown", Point((*it).x1, (*it).y1), cv::FONT_HERSHEY_COMPLEX, 1, Scalar(0, 255, 255));
                        break;
                }
                if (ageID == age) acc_age++;
            }
        }
        std::cout << "totol detect " << total << " persons" << std::endl;
        imshow("face_detection", frame);
 	
        int64 t2 = cv::getTickCount();
        double secs = (t2-t1)/cv::getTickFrequency();
        std::cout<<"secs time:"<< secs <<" s "<<std::endl;
        int q = cv::waitKey(3);
        if (q == 27) {
            break;
        }
    }

    std::cout << "****************统计结果****************" << std::endl;
    std::cout << "图片总数: " << picture_num << std::endl;
    std::cout << "检测到人脸的图片数: " << face_exist << std::endl;
    std::cout << "检测到一张人脸的图片数: " << face_num << std::endl;
    std::cout << "性别准确率: " << acc_gender*1.0/face_num << std::endl;
    std::cout << "年龄准确率: " << acc_age*1.0/face_num << std::endl;
    
    infile.close();
    return ;
}


int main(int argc, char** argv)
{
    test_video();

    return 0;
}

