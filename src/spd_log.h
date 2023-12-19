#ifndef SPD_LOG_H
#define SPD_LOG_H

#include "common_type.h"
#include "spdlog/spdlog.h"

#include <memory>

class SpdLog
{
public:
    static SpdLog &Instance()       //实现单例模式
    {
        static SpdLog instance;
        return instance;
    }

    void Init(const SpdLogConfig &config);     //根据log配置文件结构体构造日志记录器对象

    std::shared_ptr<spdlog::logger> Logger();    //获取私有成员变量logger_ptr_

    //禁用复制/移动构造函数和相关的运算符
    SpdLog(const SpdLog &) = delete;
    SpdLog(SpdLog &&) = delete;
    SpdLog &operator=(const SpdLog &) = delete;
    SpdLog &operator=(SpdLog &&) = delete;

private:
    SpdLog() = default;       //重载默认无参构造函数为空
 
    std::shared_ptr<spdlog::logger> logger_ptr_;     //指向日志记录器对象的智能指针成员变量

    void SetLevel(const std::string &level);         //根据配置文件设置日志记录器的记录级别和方式
};

#define SPD_TRACE(...)       SpdLog::Instance().Logger().get()->trace(__VA_ARGS__)  //宏定义多参数日志记录方法
#define SPD_DEBUG(...)       SpdLog::Instance().Logger().get()->debug(__VA_ARGS__)
#define SPD_INFO(...)        SpdLog::Instance().Logger().get()->info(__VA_ARGS__)
#define SPD_WARN(...)        SpdLog::Instance().Logger().get()->warn(__VA_ARGS__)
#define SPD_ERROR(...)       SpdLog::Instance().Logger().get()->error(__VA_ARGS__)
#define SPD_CRITICAL(...)    SpdLog::Instance().Logger().get()->critical(__VA_ARGS__)

#endif // SPD_LOG_H
