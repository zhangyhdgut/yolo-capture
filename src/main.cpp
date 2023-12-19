#include <chrono>
#include <thread>
#include "spd_log.h"
#include "camera_config.h"
#include "capture_config.h"
#include "task_manager.h"


int main(int argc, char **argv)
{
    //读取总配置文件，并将其中的日志记录配置文件保存在CaptureConfig类中
    std::cout << "OpenCV版本:" << CV_VERSION << std::endl;
    CaptureConfig::Instance().Init();       

    SpdLogConfig log_config = CaptureConfig::Instance().GetSpdLogConfig();     //获取spdlog配置
    CameraRPConfig camera_rp_config = CaptureConfig::Instance().GetCameraRPConfig();     //摄像头轮询配置

    std::cout << "log level: " << log_config.level << std::endl;

    //根据日志配置，配置SpdLog对象并简化日志记录方法
    SpdLog::Instance().Init(log_config);

    std::cout << "start main yolo capture" << std::endl;

    SPD_TRACE("start main yolo capture");

    //读取摄像头配置文件，并将其中的摄像头信息保存在CameraConfig类中
    CameraConfig::Instance().Init();

    TaskManager manager;
    for(int n = 0; n < camera_rp_config.timetotal; ++n){
        std::string strn = std::to_string(n+1);
        SPD_TRACE("yolo capture : {} ", strn + "th");
        auto start = std::chrono::steady_clock::now();    //获取开始时间
        manager.Start(camera_rp_config.numlx);      //开始检测
        while(1)
        {
            auto current = std::chrono::steady_clock::now();     //获取当前时间
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(current - start);    //获取时间间隔
            if (duration.count() >= camera_rp_config.timelx * 60) {               //判断是否达到时间间隔
                break;
            }
        }
        manager.Stop();               //结束检测
    }

    for(int i = 0; i < 5; ++i)            //目的是等待，让其他线程彻底运行完毕
    {
        SPD_TRACE("main continue running");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    SPD_TRACE("ended main yolo capture");

    return 0;
}
