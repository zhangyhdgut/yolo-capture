#include "camera_config.h"
#include "csv.h"
#include "spd_log.h"

#include <random>
#include <algorithm>

void CameraConfig::Init()
{
    // read camera config for csv file
    io::CSVReader<3> in("../config/camera.csv");      //读取camera.csv格式，并且csv格式的列数为3
    in.read_header(io::ignore_missing_column, "model", "code", "url");     //读取CSV文件的头部（即列名），忽略缺失的列名
    std::string model_name;
    std::string device_code;
    std::string stream_url;
    while (in.read_row(model_name, device_code, stream_url))    //循环读取csv文件的每一行，并将列数据填充到对应的变量中
    {
        if (!(model_name.empty() || device_code.empty() || stream_url.empty()))    //判断三个是否都读取到数据
        {
            CameraInfos camera_info;          //存储摄像头信息
            camera_info.yolo_model_name = model_name;
            camera_info.device_code = device_code;
            camera_info.stream_url = stream_url;
            camera_list_.push_back(camera_info);
        }
        else
        {
            SPD_ERROR("camera config error, model_name: {}, device_code: {}, stream_url: {}",
                      model_name, device_code, stream_url);             //记录错误日志
        }
    }

    std::random_device rd;         //设置随机数种子
    std::mt19937 g(rd());          //设置随机数生成器
    std::shuffle(camera_list_.begin(), camera_list_.end(), g);          //利用随机数生成器对摄像头列表进行随机打乱

    SPD_INFO("camera_num: {}", camera_list_.size());               //输出连接的摄像头个数
    for(const auto &camera_info : camera_list_)                //遍历摄像头列表，日志记录所有信息
    {
        SPD_INFO("camera config, model_name: {}, device_code: {}, stream_url: {}",
                 camera_info.yolo_model_name, camera_info.device_code, camera_info.stream_url);
    }
}

std::vector<CameraInfos> CameraConfig::GetCameraList()
{
    return camera_list_;
}
