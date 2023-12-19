#include "spd_log.h"
#include "spdlog/sinks/rotating_file_sink.h"

void SpdLog::SetLevel(const std::string &level)
{
    char L = toupper(level[0]);     //将小写字母变为大写字母
    if (L == 'T')
    {
        logger_ptr_->set_level(spdlog::level::trace);       //设置日志记录器最低记录级别
        logger_ptr_->flush_on(spdlog::level::trace);       //设置日志记录器立即写入日志的最低级别
    }
    else if (L == 'D')
    {
        logger_ptr_->set_level(spdlog::level::debug);
        logger_ptr_->flush_on(spdlog::level::debug);
    }
    else if (L == 'I')
    {
        logger_ptr_->set_level(spdlog::level::info);
        logger_ptr_->flush_on(spdlog::level::info);
    }
    else if (L == 'W')
    {
        logger_ptr_->set_level(spdlog::level::warn);
        logger_ptr_->flush_on(spdlog::level::warn);
    }
    else if (L == 'E')
    {
        logger_ptr_->set_level(spdlog::level::err);
        logger_ptr_->flush_on(spdlog::level::err);
    }
    else if (L == 'C')
    {
        logger_ptr_->set_level(spdlog::level::critical);
        logger_ptr_->flush_on(spdlog::level::critical);
    }
    else
    {
        logger_ptr_->set_level(spdlog::level::info);
        logger_ptr_->flush_on(spdlog::level::info);
    }
}

void SpdLog::Init(const SpdLogConfig &config)
{
    if (config.mt_security)    //根据log配置文件创建多线程或单线程的日志记录器对象
    {
        logger_ptr_ = spdlog::rotating_logger_mt(config.logger_name,
                                                 config.logger_file_path,
                                                 config.max_file_size,
                                                 config.max_files);
    }
    else
    {
        logger_ptr_ = spdlog::rotating_logger_st(config.logger_name,
                                                 config.logger_file_path,
                                                 config.max_file_size,
                                                 config.max_files);
    }
    SetLevel(config.level);      
    // https://cloud.tencent.com/developer/article/2102105
    // |12-11 13:52:14.817|D|44285| hello world
    logger_ptr_->set_pattern("|%m-%d %H:%M:%S.%e|%L|%t| %v");     //设置日志记录方式
}

std::shared_ptr<spdlog::logger> SpdLog::Logger()
{
    return logger_ptr_;
}
