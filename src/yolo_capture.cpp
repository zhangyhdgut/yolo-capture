#include "yolo_capture.h"
#include "spd_log.h"
#include <opencv2/opencv.hpp>
#include <fstream>
#include "nlohmann/json.hpp"

const float INPUT_WIDTH = 640.0;     //定义推理参数，宽，高，类别置信度阈值，非极大值抑制值，检测框置信度阈值, 图像相似度阈值
const float INPUT_HEIGHT = 640.0;
const float SCORE_THRESHOLD = 0.2;       
const float NMS_THRESHOLD = 0.4;
const float CONFIDENCE_THRESHOLD = 0.4;
const float IMAGE_SIMILAR = 30;

const std::vector<cv::Scalar> colors = {     //存储颜色数据类型容器
    cv::Scalar(255, 255, 0),
    cv::Scalar(0, 255, 0),
    cv::Scalar(0, 255, 255),
    cv::Scalar(255, 0, 0)};

YoloCapture::YoloCapture() : is_capturing_(false)   
{
}

void YoloCapture::Init() 
{   
    nlohmann::json config_;
    try                 //尝试操作，并将结果传给catch
    {
        std::ifstream f("../config/yolocap.conf");
        config_ = nlohmann::json::parse(f);       //jason格式解析配置文件并保存
    }
    catch (...)        //捕获try传来的异常，并输出打印
    {
        std::cerr << "error parsing config file" << std::endl;
    }
    auto net_config = config_["yolov5_capture_config"];      //根据键和值获取spdlog配置文件内容并保存在结构体中
    detect_config_.input_width = net_config["net_config"]["input_width"];
    detect_config_.input_height = net_config["net_config"]["input_height"];
    detect_config_.score_threshold = net_config["net_config"]["score_threshold"];
    detect_config_.nms_threshold = net_config["net_config"]["nms_threshold"];
    detect_config_.confidence_threshold = net_config["net_config"]["confidence_threshold"];
    detect_config_.similarity_threshold = net_config["similarity_threshold"];
    detect_config_.iscuda = net_config["is_cuda"];
    return;
}

YoloCapture::~YoloCapture()
{
    SPD_TRACE("destructor yolo capture, device code: {}", yolo_config_.device_code);
    if (is_capturing_)
    {
        Stop();
    }
}

void YoloCapture::Start(const YoloCaptureConfig &config)
{   
    Init();
    yolo_config_ = config;
    if (!is_capturing_)
    {
        is_capturing_ = true;
        std::cout << yolo_config_.device_code << std::endl;
        SPD_TRACE("start yolo capture, device code: {}", yolo_config_.device_code);
        cap_thread_ = std::thread(&YoloCapture::Run, this);    //将本对象的成员函数Run绑定给一个新的线程cap_tlhread_
    }
}

void YoloCapture::LoadClassList()      
{
    std::ifstream ifs("../config/classes.txt");    //读取文件
    std::string line;
    while (getline(ifs, line))          //读取txt文件的每一行并赋值给line
    {
        class_name_.push_back(line);
    }
}

void YoloCapture::LoadNet(cv::dnn::Net &net, bool is_cuda)
{
    if (detect_config_.iscuda)
    {
        // https://github.com/opencv/opencv/issues/17852
        int device_count = cv::cuda::getCudaEnabledDeviceCount();    //读取设备可用cuda数量
        SPD_DEBUG("cuda device count: {}", device_count);           
        cv::cuda::setDevice(device_count - 1);                   //设置当前cuda设备
    }

    net = cv::dnn::readNet("../config/yolov5s.onnx");       //opencv读取onnx格式的模型
    if (detect_config_.iscuda)
    {
        SPD_DEBUG("running on CUDA");
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);              //设置推理设备为cuda
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);            //设置推理精度
    }
    else
    {
        SPD_DEBUG("running on CPU");
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);           //设置推理引擎opencv
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);                 //设置推理设备cpu
    }
}

void YoloCapture::SaveFrameAsJpg(const cv::Mat &frame)
{
    auto time_point = std::chrono::system_clock::now();   //获取当前时间点
    auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(time_point);    //将时间点转化为毫秒
    auto time_since_epoch = ms.time_since_epoch();         //获取当前时间点的时间戳
    long long milliseconds = time_since_epoch.count();       //将时间戳转化为毫秒数
    std::ostringstream oss;         
    oss << std::setw(15) << std::setfill('0') << std::to_string(milliseconds) << ".jpg";  //将时间戳转化为字符串并填充和扩为以及拼接
    std::string filename = oss.str();     //将其转化为字符串
    cv::imwrite("../image/" + filename, frame);       //写入图像
}

cv::Mat YoloCapture::FormatYolov5(const cv::Mat &source)
{
    int col = source.cols;
    int row = source.rows;
    int _max = MAX(col, row);
    cv::Mat result = cv::Mat::zeros(_max, _max, CV_8UC3);    //创建遮罩
    source.copyTo(result(cv::Rect(0, 0, col, row)));        //将source复制到result的指定矩形区域
    return result;
}

void YoloCapture::Detect(cv::dnn::Net &net, const cv::Mat &image, std::vector<YoloDetection> &output)
{
    cv::Mat blob;
    auto input_image = FormatYolov5(image);     //前处理缩放和填充原图
    //图像归一化以及交换颜色通道,不需要裁剪
    cv::dnn::blobFromImage(input_image, blob, 1. / 255., cv::Size(detect_config_.input_width, detect_config_.input_height), cv::Scalar(), true, false);  
    net.setInput(blob);       //设置输入图像
    std::vector<cv::Mat> outputs;      //cv::Mat是opencv在c++中类似于opencv在python中的numpy数据格式
    net.forward(outputs, net.getUnconnectedOutLayersNames());     //网络前向传播并保存输出

    float x_factor = input_image.cols / detect_config_.input_width;      //记录缩放因子
    float y_factor = input_image.rows / detect_config_.input_height;

    const int dimensions = 85;        //yolo输出值的维度，85个类别，25200个检测框
    const int rows = 25200;

    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    float *data = (float *)outputs[0].data;     //读取输出的第一个节点取到cpu上，并赋值给float指针
    for (int i = 0; i < rows; ++i)        //通过指针移动来读取二维数组
    {
        float confidence = data[4];       //读取检测置信度
        if (confidence >= detect_config_.confidence_threshold)        
        {
            float *classes_scores = data + 5;        //如果有物体则读取各个类别得分
            cv::Mat scores(1, class_name_.size(), CV_32FC1, classes_scores);      //赋值给sorces
            cv::Point class_id;
            double max_class_score;
            minMaxLoc(scores, 0, &max_class_score, 0, &class_id);     //查找scores中类别得分最高的并赋值索引值和类别
            if (max_class_score > detect_config_.score_threshold)     //
            {
                confidences.push_back(confidence);
                class_ids.push_back(class_id.x);    //读取检测框位置
                float x = data[0];
                float y = data[1];
                float w = data[2];
                float h = data[3];
                int left = int((x - 0.5 * w) * x_factor);
                int top = int((y - 0.5 * h) * y_factor);
                int width = int(w * x_factor);
                int height = int(h * y_factor);
                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }
        data += 85;           //指针偏移
    }

    std::vector<int> nms_result;   //
    cv::dnn::NMSBoxes(boxes, confidences, detect_config_.score_threshold, detect_config_.nms_threshold, nms_result);   //执行非极大值抑制，并将结果保存
    for (int i = 0; i < nms_result.size(); i++)
    {
        int idx = nms_result[i];   //非极大值抑制候根据输入的框得到剩余框的索引
        YoloDetection result;
        result.class_id = class_ids[idx];
        result.confidence = confidences[idx];
        result.box = boxes[idx];
        output.push_back(result);
    }
}

void YoloCapture::DrawDetection(const cv::Mat &frame, const std::vector<YoloDetection> &detections)
{
    for (int i = 0; i < detections.size(); ++i)    //遍历每个输出结果值
    {
        auto detection = detections[i];
        auto box = detection.box;
        auto classId = detection.class_id;
        const auto color = colors[classId % colors.size()];
        cv::rectangle(frame, box, color, 3);    //绘制矩形框
        cv::rectangle(frame, cv::Point(box.x, box.y - 20), cv::Point(box.x + box.width, box.y), color, cv::FILLED);    //绘制填充矩形框
        cv::putText(frame, class_name_[classId].c_str(), cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
        //绘制填充文字
    }
     SaveFrameAsJpg(frame);     //保存图像
}

void YoloCapture::Stop()
{
    SPD_TRACE("stop yolo capture, device code: {}", yolo_config_.device_code);
    if (is_capturing_)
    {
        is_capturing_ = false;
        SPD_TRACE("join yolo capture ended, device code: {}", yolo_config_.device_code);
        cap_thread_.join();    //调用 join 会阻塞当前线程，直到关联的线程执行完成,并释放线程
    }
}


float YoloCapture::FrameSimilar(const cv::Mat &image_old, const cv::Mat &image_cur)    //使用Phash计算图像相似度
{   
    cv::Ptr<cv::ORB> orb = cv::ORB::create();    //创建orb对象
    std::vector<cv::KeyPoint> keypoints_old, keypoints_cur;    //创建关键点容器
    cv::Mat descriptors_old, descriptors_cur;          //创建描述子容器
    orb->detectAndCompute(image_old, cv::noArray(), keypoints_old, descriptors_old);   //计算关键点和描述子
    orb->detectAndCompute(image_cur, cv::noArray(), keypoints_cur, descriptors_cur);  

    cv::FlannBasedMatcher matcher(new cv::flann::LshIndexParams(20, 10, 2));    //创建匹配器
    std::vector<cv::DMatch> matches;     //创建匹配结果容器
    matcher.match(descriptors_old, descriptors_cur, matches);   //匹配描述子

    // 计算匹配点的平均距离，作为图像的相似度
    double avg_dist = 0.0;
    for (size_t i = 0; i < matches.size(); i++)
    {
        avg_dist += matches[i].distance;
    }
    avg_dist /= matches.size();
    
    return avg_dist;
}

void YoloCapture::Run()
{
    // 如果将 net 参数调整为类成员，程序将崩溃
    LoadClassList();      //读取类别
    cv::dnn::Net net;
    LoadNet(net, yolo_config_.is_cuda);    //加载网络

    SPD_DEBUG("stream url: {}", yolo_config_.stream_url);
    cv::VideoCapture capture(yolo_config_.stream_url, cv::CAP_FFMPEG, yolo_config_.ffmepg_props);
    //opencv使用FFmpeg后端打开一个视频流或文件，并将相关设置传递给该视频捕获对象
    if (!capture.isOpened())
    {
        SPD_ERROR("open stream url failed: {}", yolo_config_.stream_url);
    }

    SPD_DEBUG("open stream url success: {}", yolo_config_.stream_url);

    int skip_frames = 0;
    int total_frames = 0;
    int open_num = 0;
    int detect_num = 0;
    float avg_dist = 0.0;

    cv::Mat frame;
    while (is_capturing_)
    {
        capture.read(frame);    //从视频流中捕获一张图像
        if (frame.empty())
        {   
            open_num++;
            SPD_DEBUG("read stream url failed: {}, try_nums: {}", yolo_config_.stream_url, std::to_string(open_num));
            if(open_num >= 5){
                break;
            }
            continue;
        }
        skip_frames++;
        if (skip_frames == 10)   //读取第10帧图片，并推理
        {
            if (detect_num == 0)
            {
                frame_old = frame.clone();
            }
            skip_frames = 0;
            avg_dist = FrameSimilar(frame_old, frame);    //计算图像相似度
            if (detect_num >= 1 && avg_dist <= detect_config_.similarity_threshold)    //启用图像去重
            // if (detect_num >= 1 && FrameSimilar(frame_old, frame))     //不启动图像去重
            {
                continue;
            }
            std::cout << avg_dist << std::endl;
            frame_old = frame.clone();
            std::vector<YoloDetection> detections;
            Detect(net, frame, detections);        //推理图像
            detect_num++;
            if (detections.empty()){
                SPD_DEBUG("device code: {}, targets: None", yolo_config_.device_code);    
            }
            else {
                total_frames++;
                DrawDetection(frame, detections);      //绘制图像
                std::string detection_name = "";
                int size = detections.size();
                for(int m = 0; m < size; m = m+1){
                    detection_name = detection_name + class_name_[detections[m].class_id] + " ";
                }
                std::cout << detection_name << std::endl;
                SPD_DEBUG("device code: {}, targets: {}", yolo_config_.device_code, detection_name);   
            }
        }
    }

    capture.release();    //停止获取视频流并释放capture
    SPD_DEBUG("yolo capture ended, code: {}, total detect frame: {}", yolo_config_.device_code, total_frames);
}
