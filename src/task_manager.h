#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include "yolo_capture.h"

#include <memory>

class TaskManager
{
    using YoloCapturePtr = std::shared_ptr<YoloCapture>;   //使用简化命名

public:
    TaskManager();

    void Start(const int &numlx);     //开始检测

    void Stop();     //结束所有检测

    ~TaskManager();

private:
    std::vector<YoloCapturePtr> yolo_captures_;     //定义指向yolo_captures_类智能指针容器
};

#endif // TASK_MANAGER_H
