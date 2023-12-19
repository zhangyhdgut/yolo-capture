#ifndef YOLO_CAPTURE_H
#define YOLO_CAPTURE_H

#include "common_type.h"

#include <thread>
#include <string>

class YoloCapture
{
public:
    YoloCapture();             //构造函数初始化为不检测图像中

    ~YoloCapture();        //结束当前类

    void LoadClassList();           //根据类别配置文件读取可检测类别保存在class_name_

    void LoadNet(cv::dnn::Net &net, bool is_cuda);      //根据推理信息加载推理网络并设置推理引擎

    cv::Mat FormatYolov5(const cv::Mat &source);           //根据输入图像返回一个填充方形的图像

    void Detect(cv::dnn::Net &net, const cv::Mat &image, std::vector<YoloDetection> &output);  //根据模型，原图得到输出结果

    void DrawDetection(const cv::Mat &image, const std::vector<YoloDetection> &detections);     //根据原图和检测值绘制图像

    void Start(const YoloCaptureConfig &config);     //创建一个线程并执行检测

    float FrameSimilar(const cv::Mat &image_old, const cv::Mat &image_cur);          //判断当前帧是否与上一帧相似

    void Init();    //初始化模型配置文件

    void Stop();                //停止检测并释放线程

private: 
    void Run();         //利用opencv获取视频，并每隔10帧推理一次，输出日志和保存

    void SaveFrameAsJpg(const cv::Mat &frame);    //将图像以当前时间命名写入文件中

private:
    std::vector<std::string> class_name_;         //记录可检测类别
    YoloCaptureConfig yolo_config_;              //保存推理时的摄像头信息
    std::thread cap_thread_;         //创建一个线程
    bool is_capturing_;              //记录是否正在推理
    cv::Mat frame_old;               //记录上一时刻的图像帧
    DetectConfig detect_config_;     //模型配置文件

};

#endif // YOLO_CAPTURE_H
