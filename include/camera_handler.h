#ifndef CAMERA_HANDLER_H
#define CAMERA_HANDLER_H

#include <opencv2/opencv.hpp>

class CameraHandler {
private:
    cv::VideoCapture camera;
    int camera_id;
    
public:
    CameraHandler(int id = 0);
    ~CameraHandler();
    
    bool open();
    void close();
    bool isOpened() const;
    bool captureFrame(cv::Mat& frame);
};

#endif
