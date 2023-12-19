#include "capture_config.h"

#include <iostream>
#include <fstream>

void CaptureConfig::Init() 
{
    try                 //尝试操作，并将结果传给catch
    {
        std::ifstream f("../config/yolocap.conf");
        config_ = json::parse(f);       //jason格式解析配置文件并保存
    }
    catch (...)        //捕获try传来的异常，并输出打印
    {
        std::cerr << "error parsing config file" << std::endl;
    }
}

SpdLogConfig CaptureConfig::GetSpdLogConfig()
{
    SpdLogConfig log_config;
    auto spd_log_config = config_["spd_log_config"];      //根据键和值获取spdlog配置文件内容并保存在结构体中
    log_config.logger_file_path = spd_log_config["logger_file_path"];
    log_config.logger_name = spd_log_config["logger_name"];
    log_config.level = spd_log_config["level"];
    log_config.max_file_size = spd_log_config["max_file_size"];
    log_config.max_files = spd_log_config["max_files"];
    log_config.mt_security = true;
    return log_config;
}

CameraRPConfig CaptureConfig::GetCameraRPConfig()
{
    CameraRPConfig camera_rp_config;
    auto spd_log_config = config_["camera_rollpoll_config"];      //根据键和值获取spdlog配置文件内容并保存在结构体中
    camera_rp_config.timelx = spd_log_config["timelx"];
    camera_rp_config.numlx = spd_log_config["numlx"];
    camera_rp_config.timetotal = spd_log_config["timetotal"];
    return camera_rp_config;
}


int CaptureConfig::IsCUDA()
{
    return config_["is_cuda"];
}
