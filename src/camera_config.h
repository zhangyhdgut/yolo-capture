#ifndef CAMERA_CONFIG_H
#define CAMERA_CONFIG_H

#include "common_type.h"

class CameraConfig
{
public:
    static CameraConfig &Instance()
    {
        static CameraConfig instance;
        return instance;
    }

    void Init();

    std::vector<CameraInfos> GetCameraList();

    // Disable copy/move constructors and assignment operators
    CameraConfig(const CameraConfig &) = delete;
    CameraConfig(CameraConfig &&) = delete;
    CameraConfig &operator=(const CameraConfig &) = delete;
    CameraConfig &operator=(CameraConfig &&) = delete;

private:
    CameraConfig() {} // Private constructor

private:
    std::vector<CameraInfos> camera_list_;
};

#endif // CAMERA_CONFIG_H
