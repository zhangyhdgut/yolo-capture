#include <random>
#include <set>
#include "task_manager.h"
#include "spd_log.h"
#include "camera_config.h"
#include "common_type.h"


TaskManager::TaskManager()
{
}

void TaskManager::Start(const int &numlx)
{
    // https://forum.opencv.org/t/videocapture-open-lag-with-rtsp/10855
    std::vector<int> props;
    props.push_back(cv::CAP_PROP_OPEN_TIMEOUT_MSEC);    //设置视频捕获器打开超时时间30000ms
    props.push_back(30000);
    props.push_back(cv::CAP_PROP_READ_TIMEOUT_MSEC);         //设置视频捕获器读取超时时间10000ms
    props.push_back(10000);

    SPD_TRACE("start yolo capture task manager");
    std::vector<CameraInfos> camera_list_ = CameraConfig::Instance().GetCameraList();     //获取摄像头信息列表
    YoloCaptureConfig config;                          //更新yolo推理摄像头信息
    CameraInfos camera;
    std::vector<std::shared_ptr<YoloCapture>> yolovector(numlx);   //创建指定数目个摄像头检测容器
    int minRange = 0;
    int maxRange = camera_list_.size() - 1;
    std::random_device rd;  // 用于获取随机数种子
    std::mt19937 gen(rd()); // 使用 Mersenne Twister 算法生成随机数
    std::uniform_int_distribution<> dis(minRange, maxRange); // 定义随机数的范围
    std::vector<int> camera_index;
    while (camera_index.size() < numlx) {
        int num = dis(gen);
        if (std::find(camera_index.begin(), camera_index.end(), num) == camera_index.end()) {
            camera_index.push_back(num);
        }
    }
    for(int i = 0; i < numlx; ++i){
        int randomInRange = camera_index[i];
        camera = camera_list_[randomInRange];            //读取第一个摄像头
        config.is_cuda = true;             //更新配置信息
        config.ffmepg_props = props;
        config.device_code = camera.device_code;
        config.stream_url = camera.stream_url;
        yolovector[i] = std::make_shared<YoloCapture>();   //构建容器实体
        yolo_captures_.push_back(yolovector[i]);             //加入容器中 
        yolovector[i]->Start(config);                     //执行线程
    }
}

void TaskManager::Stop()           //结束所有检测
{
    SPD_TRACE("stop all yolo captures");
    for (auto &cap : yolo_captures_)
    {
        cap->Stop();
    }
    yolo_captures_.clear();
}

TaskManager::~TaskManager()
{
}