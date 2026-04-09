#include "camera_handler.h"
#include <iostream>

CameraHandler::CameraHandler(int id) : camera_id(id) {}

CameraHandler::~CameraHandler() {
    close();
}

bool CameraHandler::open() {
    camera.open(camera_id, cv::CAP_V4L2);  // Force V4L2 backend
    
    if (!camera.isOpened()) {
        std::cerr << "Error: Cannot open camera " << camera_id << std::endl;
        return false;
    }
    
    // Set camera properties for better quality
    camera.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    
    return true;
}

void CameraHandler::close() {
    if (camera.isOpened()) {
        camera.release();
    }
}

bool CameraHandler::isOpened() const {
    return camera.isOpened();
}

bool CameraHandler::captureFrame(cv::Mat& frame) {
    if (!isOpened()) {
        return false;
    }
    return camera.read(frame);
}

