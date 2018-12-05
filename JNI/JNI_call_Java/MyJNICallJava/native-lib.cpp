#include <jni.h>
#include <android/log.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <fstream>
#include <algorithm>
#include "mtcnnAttribute.h"

#define TAG "native-lib.cpp"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

#define MAXNULLFRAME 1

static mtcnn *mtcnn_;
Attribute attribute;

// 模型是否已经初始化
bool detection_init_ok = false;
bool gender_init_ok = false;

string tFaceModelDir;

// 人脸计数
jobject mCallbackObj;
jmethodID displayCount; // typedef struct _jmethodID* jmethodID

int a[8] = {0};
struct MaxFace {
    bool isEmpty;
    int nullFrames;
    int gender;
    int age;
    int genders[2];
    int ages[5];
    Bbox box;
};
MaxFace mLastFace = {true}; // 初始化
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

void drawFaceRect(cv::Mat &img, cv::Point p1, cv::Point p2, int ID_gender) {
    Scalar color;
    if (ID_gender == 0) {
        color = Scalar(255,0,255);  // female
    }
    else {
        color = Scalar(255,255,0);  // male
    }

    int x_shift = (p2.x - p1.x)/10;
    int y_shift = (p2.y - p1.y)/10;
    cv::Point p10(p1.x, p1.y);
    cv::Point p11(p1.x + x_shift, p1.y);
    cv::Point p12(p1.x, p1.y + y_shift);
    cv::Point p20(p2.x, p1.y);
    cv::Point p21(p2.x - x_shift, p1.y);
    cv::Point p22(p2.x, p1.y + y_shift);
    cv::Point p30(p1.x, p2.y);
    cv::Point p31(p1.x + x_shift, p2.y);
    cv::Point p32(p1.x, p2.y - y_shift);
    cv::Point p40(p2.x, p2.y);
    cv::Point p41(p2.x - x_shift, p2.y);
    cv::Point p42(p2.x, p2.y - y_shift);
    //cv::rectangle(img, p1, p2, Scalar(255,255,255,0.1),CV_FILLED);
    //cv::Mat roi = img(cv::Rect(100, 100, 300, 300));

    cv::Mat roi;
    if (0 <= p1.x && 0 <= p2.x-p1.x && p1.x <= img.cols && 0 <= p2.y && 0 <= p2.y-p1.y && p1.y <= img.rows)
        roi = img(cv::Rect(p1.x, p1.y, p2.x-p1.x, p2.y-p1.y));
    if (0 <= p1.x && 0 <= p2.x-p1.x && p1.x + roi.cols <= img.cols && 0 <= p2.y && 0 <= p2.y-p1.y && p1.y + roi.rows <= img.rows) {
        cv::Mat color1(roi.size(), CV_8UC4, cv::Scalar(255, 255, 255));
        double alpha = 0.3;
        cv::addWeighted(color1, alpha, roi, 1.0 - alpha, 0.0, roi);
    }
    cv::line(img, p10, p11, color, 2);
    cv::line(img, p10, p12, color, 2);
    cv::line(img, p20, p21, color, 2);
    cv::line(img, p20, p22, color, 2);
    cv::line(img, p30, p31, color, 2);
    cv::line(img, p30, p32, color, 2);
    cv::line(img, p40, p41, color, 2);
    cv::line(img, p40, p42, color, 2);
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


int getFinalGender(const MaxFace &properties) {
    return properties.genders[0] > properties.genders[1] ? 0 : 1;
}

int getFinalAge(const MaxFace &properties) {
    int max = properties.ages[0];
    int result = 0;
    for (int i = 1; i < 5; ++i) {
        if (properties.ages[i] > max) {
            max = properties.ages[i];
            result = i;
        }
    }
    return result;
}


/**
 * 调用之前必须判断finalBoxes是否为空
 * @param finalBoxes 检测到的人脸数组
 * @return result 最大的人脸框
 */
const Bbox &getMaxBox(const std::vector<Bbox> &finalBoxes) { // finalBoxes不能为空
    const Bbox *result = &finalBoxes[0]; // 不修改本体，频繁改变指向，故使用指针
    for (std::vector<Bbox>::const_iterator it = finalBoxes.begin()+1; it != finalBoxes.end(); it++) {
        if (result->area < it->area) {
            result = &*it;
        }
    }
    return *result;
}

//////////////////////////////// JNI ///////////////////////////////////////////////////////////////
extern "C"
JNIEXPORT jint
Java_com_faceattributes_NativeFunction_makeFace(JNIEnv *env, jobject thiz, jlong jrgbaddr) { // jobject是指针
    LOGD("makeFace start");
    cv::Mat &frame = *(cv::Mat *) jrgbaddr;
    LOGD("frame width:%d, height:%d, step:%d", frame.cols, frame.rows, frame.step[0]);
    mRotate90(frame);


    ncnn::Mat ncnn_img = ncnn::Mat::from_pixels(frame.data, ncnn::Mat::PIXEL_RGBA2RGB, frame.cols, frame.rows);
    std::vector<Bbox> finalBox;
    mtcnn_->detect(ncnn_img, finalBox);
    std::vector<float> gender_scores;
    std::vector<float> age_scores;
    float confidence = 0;
    int total = 0;
    unsigned long classify_time = 0;
    // 属性统计
    MaxFace mNextFace = {true};
    if (!finalBox.empty())
        mNextFace.box = finalBox[0];


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
                drawFaceRect(frame, Point((*it).x1, (*it).y1), Point((*it).x2, (*it).y2), genderID);
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

                if (it->area >= mNextFace.box.area) {  // 比较当前box与maxBox的大小
                    mNextFace.isEmpty = false;
                    mNextFace.gender = genderID;
                    mNextFace.age = ageID;
                    mNextFace.box = *it;
                }

            }

        }
    }

    LOGD("makeFace end");

    if (mLastFace.isEmpty) {LOGD("空空");
        if (!mNextFace.isEmpty) {
            LOGD("空b空");
            mLastFace.isEmpty = false;
            mLastFace.nullFrames = 0;
            mLastFace.age = mNextFace.age;
            mLastFace.gender = mNextFace.gender;
            mLastFace.ages[mLastFace.age]++;
            mLastFace.genders[mLastFace.gender]++;
            mLastFace.box = mNextFace.box;
        }
    } else {
        if (mNextFace.isEmpty) {
            if (mLastFace.nullFrames < MAXNULLFRAME) {
                LOGD("nullFrames++");
                mLastFace.nullFrames++;
            } else {
                LOGD("nullframes > MAX");
                a[0] += 1;
                a[getFinalGender(mLastFace)+1] += 1;
                a[getFinalAge(mLastFace)+3] += 1;

                mLastFace = {true};
            }
        } else {
            if (isSameFace(mLastFace.box, mNextFace.box)) {
                LOGD("same face");
                mLastFace.isEmpty = false;
                mLastFace.nullFrames = 0;
                mLastFace.age = mNextFace.age;
                mLastFace.gender = mNextFace.gender;
                mLastFace.ages[mLastFace.age]++;
                mLastFace.genders[mLastFace.gender]++;
                mLastFace.box = mNextFace.box;
            } else {
                LOGD("not same face");
                a[0] += 1;
                a[getFinalGender(mLastFace)+1] += 1;
                a[getFinalAge(mLastFace)+3] += 1;

                mLastFace = {false};
                mLastFace.age = mNextFace.age;
                mLastFace.gender = mNextFace.gender;
                mLastFace.box = mNextFace.box;
            }
        }
    }

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