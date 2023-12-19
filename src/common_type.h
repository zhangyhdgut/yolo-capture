#ifndef COMMON_TYPE_H
#define COMMON_TYPE_H

#include <string>
#include <opencv2/opencv.hpp>

struct SpdLogConfig
{
    std::string logger_file_path;
    std::string logger_name;
    std::string level;
    size_t max_file_size;
    size_t max_files;
    bool mt_security;
};

struct YoloDetection
{
    int class_id;
    float confidence;
    cv::Rect box;
};

struct YoloCaptureConfig
{
    bool is_cuda;
    std::string device_code;
    std::string stream_url;
    std::vector<int> ffmepg_props;
};

struct CameraInfos
{
    std::string yolo_model_name;
    std::string device_code;
    std::string stream_url;
};

struct CameraRPConfig
{
    float timelx;    //设置每轮检测时间：单位分钟
    int numlx;         //设置每轮检测的摄像头个数
    int timetotal;     //设置总共检测轮次
};

struct DetectConfig
{
float input_width;     //定义推理参数，宽，高，类别置信度阈值，非极大值抑制值，检测框置信度阈值, 图像相似度阈值
float input_height;
float score_threshold;       
float nms_threshold;
float confidence_threshold;
float similarity_threshold;
int iscuda;
};

#endif // COMMON_TYPE_H
