#include "face_detector.h"
#include <iostream>

FaceDetector::FaceDetector(const std::string& cascade_path) {
    if (!face_cascade.load(cascade_path)) {
        std::cerr << "Error: Failed to load cascade from " << cascade_path << std::endl;
    }
}

std::vector<cv::Rect> FaceDetector::detectFaces(const cv::Mat& frame) {
    std::vector<cv::Rect> faces;
    
    if (!isLoaded()) {
        return faces;
    }
    
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(gray, gray);
    
    face_cascade.detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(30, 30));
    
    return faces;
}

bool FaceDetector::isLoaded() const {
    return !face_cascade.empty();
}

