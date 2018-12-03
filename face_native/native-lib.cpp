#include <jni.h>
#include <android/log.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <fstream>
#include <algorithm>
#include "mtcnnAttribute.h"

#define TAG "native-lib.cpp"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

static mtcnn *mtcnn_;
Attribute attribute;

// 模型是否已经初始化
bool detection_init_ok = false;
bool gender_init_ok = false;

string tFaceModelDir;

// 记录耗时
//fstream fout;

// 人脸计数
jobject mCallbackObj;
jmethodID displayCount; // typedef struct _jmethodID* jmethodID
std::vector<Bbox> lastBox;
int a[8] = {0};

struct mProperties {
    int flag;
    int gender[2];
    int age[5];
};
std::vector<mProperties> mmp;

// 两个相交的部分
struct rect {
    int minx;
    int miny;
    int maxx;
    int maxy;
    float area;
}mOverlap;

///////////////////////////////////////////////////////////////////////////////////////////////
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

static int getMaxId(std::vector<float>& arr, float &confidence){
    //assert(arr != NULL);
    int num = arr.size();
    float max_val = arr[0];
    int id = 0;
    for (int i = 1; i < num; i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
            id = i;
        }
    }
    confidence = max_val;
    return id;
}

void mRotate90(Mat &img) {
    cv::Point2f center = cv::Point2f(img.rows/2, img.rows/2);
    double angle = 270;
    double scale = 1;
    cv::warpAffine(img, img, cv::getRotationMatrix2D(center, angle, scale), cv::Size(img.rows, img.cols));
}

static unsigned long get_current_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec*1000000 + tv.tv_usec);
}

bool isSameFace(const Bbox &last, const Bbox &next) {
    // if A∩B/A∪B>0.3 then return true
    // 求两个矩形相交的面积
    mOverlap.area = 0;
    mOverlap.minx = max(last.x1, next.x1);
    mOverlap.miny = max(last.y1, next.y1);
    mOverlap.maxx = min(last.x2, next.x2);
    mOverlap.maxy = min(last.y2, next.y2);
    if (mOverlap.minx < mOverlap.maxx && mOverlap.miny < mOverlap.maxy)
        mOverlap.area = (mOverlap.maxx-mOverlap.minx) * (mOverlap.maxy-mOverlap.miny);
    // A∩B/(A+B-A∩B)
    float score = mOverlap.area / (last.area + next.area - mOverlap.area);
    LOGD("%f",score);
    return score > 0.3;
}

int getFinalGender(const mProperties &properties) {
    return properties.gender[0] > properties.gender[1] ? 0 : 1;
}

int getFinalAge(const mProperties &properties) {
    int max = properties.age[0];
    int result = 0;
    for (int i = 1; i < 5; ++i) {
        if (properties.age[i] > max) {
            max = properties.age[i];
            result = i;
        }
    }
    return result;
}
//////////////////////////////// JNI ///////////////////////////////////////////////////////////////
extern "C"
JNIEXPORT jint
Java_com_faceattributes_NativeFunction_makeFace(JNIEnv *env, jobject thiz, jlong jrgbaddr) { // jobject是指针
    LOGD("makeFace start");
    cv::Mat &frame = *(cv::Mat *) jrgbaddr;
    LOGD("frame width:%d, height:%d, step:%d", frame.cols, frame.rows, frame.step[0]);
    mRotate90(frame);

    //cv::resize(frame, frame, Size(frame.cols/2,frame.rows/2));

    ncnn::Mat ncnn_img = ncnn::Mat::from_pixels(frame.data, ncnn::Mat::PIXEL_RGBA2RGB, frame.cols, frame.rows);
    std::vector<Bbox> finalBox;

    //unsigned long detect_time1 = get_current_time();
    mtcnn_->detect(ncnn_img, finalBox);
    //unsigned long detect_time2 = get_current_time();
    //const int num_box = finalBox.size();
    /*
    fout.open(tFaceModelDir+"time.csv", ios::app);
    if (num_box!=0) {
        LOGD("num_box:%d, average detect time: %.3fms", num_box, (detect_time2-detect_time1)/1000.0/num_box);
        fout << num_box << "," << (detect_time2-detect_time1)/1000.0/num_box << ",";
    }
    */
    std::vector<float> gender_scores;
    std::vector<float> age_scores;
    float confidence = 0;
    int total = 0;
    unsigned long classify_time = 0;
    // 属性统计
    std::vector<mProperties> finalProperty;

    for (vector<Bbox>::iterator it = finalBox.begin(); it != finalBox.end(); it++) {
        if (it->exist) {
            total++;
            cv::Mat attCropImg = cropFaceForAttr(frame, it->x1, it->y1, it->x2, it->y2);
            if (!attCropImg.empty() && attCropImg.rows==224 && attCropImg.cols==224){
                unsigned long classify_time1 = get_current_time();
                attribute.classify(attCropImg, gender_scores, age_scores);
                unsigned long classify_time2 = get_current_time();
                classify_time += (classify_time2-classify_time1);
                // 性别
                int genderID = getMaxId(gender_scores, confidence);
                if (genderID == 0)
                    cv::rectangle(frame, Point((*it).x1, (*it).y1), Point((*it).x2, (*it).y2),
                                  Scalar(255, 0, 255), 2, 8, 0);
                else
                    cv::rectangle(frame, Point((*it).x1, (*it).y1), Point((*it).x2, (*it).y2),
                                  Scalar(255, 255, 0), 2, 8, 0);
                // 年龄
                int ageID = getMaxId(age_scores, confidence);
                switch (ageID) {
                    case 0: // 儿童
                        cv::putText(frame, "child", Point((*it).x1, (*it).y1), cv::FONT_HERSHEY_COMPLEX, 1, Scalar(0, 255, 255));
                        break;
                    case 1: // 少年
                        cv::putText(frame, "teenager", Point((*it).x1, (*it).y1), cv::FONT_HERSHEY_COMPLEX, 1, Scalar(0, 255, 255));
                        break;
                    case 2: // 青年
                        cv::putText(frame, "young", Point((*it).x1, (*it).y1), cv::FONT_HERSHEY_COMPLEX, 1, Scalar(0, 255, 255));
                        break;
                    case 3: // 中年
                        cv::putText(frame, "middle", Point((*it).x1, (*it).y1), cv::FONT_HERSHEY_COMPLEX, 1, Scalar(0, 255, 255));
                        break;
                    case 4: // 老年
                        cv::putText(frame, "older", Point((*it).x1, (*it).y1), cv::FONT_HERSHEY_COMPLEX, 1, Scalar(0, 255, 255));
                        break;
                    default:
                        cv::putText(frame, "unknown", Point((*it).x1, (*it).y1), cv::FONT_HERSHEY_COMPLEX, 1, Scalar(0, 255, 255));
                        break;
                }
                // 统计属性
                mProperties temp;
                memset(&temp, 0, sizeof(mProperties));
                temp.gender[genderID] = temp.age[ageID] = 1;
                finalProperty.push_back(temp); // 要clear
            }

        }
    }

    //cv::resize(frame, frame, Size(frame.cols*2,frame.rows*2));
    /*
    if (total!=0) {
        LOGD("total:%d, average classify time: %.3fms", total, classify_time/1000.0/total);
        fout << total << "," << classify_time/1000.0/total << endl;
    }
    fout.close();
    */
    LOGD("makeFace end");

    /**
     * lastBox 上一帧的人脸; mmp保存上一帧的人脸属性
     * 默认lastBox.size == mmp.size
     * finalBox 这一帧的人脸; finalProperty 这一帧的人脸属性
     * 默认finalBox.size == finalProperty.size
     */
    std::vector<mProperties>::iterator it3 = mmp.begin(); // 与it步伐一直
    for (std::vector<Bbox>::iterator it = lastBox.begin(); it != lastBox.end(); it++) {
        bool isLeave = true;
        std::vector<mProperties>::iterator it4 = finalProperty.begin();
        for (std::vector<Bbox>::iterator it2 = finalBox.begin(); it2 != finalBox.end(); it2++) {
            // 跳过旧人
            LOGD("开始判断是否为同一张人脸%d",it4->flag);
            if (it4->flag) {
                it4++;
                continue;
            }

            if (isSameFace(*it, *it2)) { // 情况一：伊人依在
                isLeave = false;
                // 把mmp的内容加到finalProperty中
                it4->gender[0] += it3->gender[0];
                it4->gender[1] += it3->gender[1];
                it4->age[0] += it3->age[0];
                it4->age[1] += it3->age[1];
                it4->age[2] += it3->age[2];
                it4->age[3] += it3->age[3];
                it4->age[4] += it3->age[4];
                it4->flag = 1;
                break;
            }
            it4++;
        }
        if (isLeave) { // 情况二：故人已逝
            a[0] += 1;
            a[getFinalGender(*it3)+1] += 1;
            a[getFinalAge(*it3)+3] += 1;
        }
        it3++;
    }
    // 新人变旧人
    lastBox.assign(finalBox.begin(), finalBox.end());
    mmp.assign(finalProperty.begin(), finalProperty.end());

    // 回调java方法
    if (mCallbackObj) { // 检查回调函数的实例是否存在
        jintArray ja = env->NewIntArray(8);
        env->SetIntArrayRegion(ja, 0, 8, a);
        env->CallVoidMethod(mCallbackObj, displayCount, ja);
        env->ExceptionClear();
        env->DeleteLocalRef(ja);
    }
    return 0;
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_faceattributes_NativeFunction_ModelInit(JNIEnv *env, jobject instance, jstring path_) {
    LOGD("ModelInit start");

    if (NULL == path_) {
        LOGD("path cannot be null");
        return 0;
    }

    const char *path = env->GetStringUTFChars(path_, 0);

    if (NULL == path) return 0;

    tFaceModelDir = path;

    string tLastChar = tFaceModelDir.substr(tFaceModelDir.length() - 1, 1);
    LOGD("init, tFaceModelDir last is %s", tLastChar.c_str());

    //目录补齐'/'
    if ("\\" == tLastChar) {
        tFaceModelDir = tFaceModelDir.substr(0, tFaceModelDir.length()-1) + "/";
    } else if (tLastChar != "/") {
        tFaceModelDir += "/";
    }
    LOGD("init, tFaceModelDir=%s", tFaceModelDir.c_str());

    if (detection_init_ok) LOGD("face detection model has been loaded");
    else {
        LOGD("call mtcnn constructor");
        mtcnn_ = new mtcnn(tFaceModelDir);  // 导入mtcnn
        detection_init_ok = true;
    }
    if (gender_init_ok) LOGD("gender classification model has been loaded");
    else {
        LOGD("call Attribute.setup() function");
        attribute.setup(tFaceModelDir); // 导入mobileNet
        gender_init_ok = true;
    }

    // 打开文件 time.csv
    /*
    fout.open(tFaceModelDir+"time.csv", ios::app);
    if (fout.bad()) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "打开文件失败");
        return 0;
    }
    fout.close();
     */

    return 1;
}

// 设定回调函数的实例
extern "C"
JNIEXPORT void JNICALL
Java_com_faceattributes_NativeFunction_setCallback(JNIEnv *env, jobject thiz, jobject callback_obj) {
    LOGD("native-lib#setCallback start");

    jobject count_callback_obj = env->NewGlobalRef(callback_obj);

    if (!env->IsSameObject(mCallbackObj, count_callback_obj)) {
        displayCount = NULL;
        if (mCallbackObj) {
            env->DeleteGlobalRef(mCallbackObj);
        }
        mCallbackObj = count_callback_obj;
        if (count_callback_obj) {
            jclass clazz = env->GetObjectClass(count_callback_obj);
            if (clazz) {
                displayCount = env->GetMethodID(clazz, "displayCount", "([I)V");
            } else {
                LOGD("获取回调函数的对象失败");
            }
            env->ExceptionClear();
            if (!displayCount) {
                LOGD("找不到回调方法");
                env->DeleteGlobalRef(count_callback_obj);
                mCallbackObj = count_callback_obj = NULL;
            }
        }
    }
}