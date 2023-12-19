#ifndef CAPTURE_CONFIG_H
#define CAPTURE_CONFIG_H

#include "common_type.h"
#include "nlohmann/json.hpp"

class CaptureConfig
{
    using json = nlohmann::json;            //宏定义命名

public:
    static CaptureConfig &Instance()        //实现单例模式
    {
        static CaptureConfig instance;
        return instance;
    }

    void Init();                             //读取总配置文件，保存在成员变量config_中
 
    SpdLogConfig GetSpdLogConfig();          //根据总jason配置文件获取spdlog配置结构体

    CameraRPConfig GetCameraRPConfig();          //根据总jason配置文件获取spdlog配置结构体

    int IsCUDA();                   //记录是否可用cuda

    CaptureConfig(const CaptureConfig &) = delete;         //禁用拷贝构造函数，防止通过拷贝构造创建对象的副本。
    CaptureConfig(CaptureConfig &&) = delete;            // 禁用了移动构造函数，防止通过移动构造创建对象的副本。
    CaptureConfig &operator=(const CaptureConfig &) = delete;    //禁用了拷贝赋值操作符，防止通过拷贝赋值创建对象的副本。
    CaptureConfig &operator=(CaptureConfig &&) = delete;           //禁用了移动赋值操作符，防止通过移动赋值创建对象的副本。

private:
    CaptureConfig() = default;         //重默认无参构造函数

    json config_;                      //jason格式解析的配置文件
};

#endif // CAPTURE_CONFIG_H
