#ifndef FACE_DETECTOR_H
#define FACE_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

class FaceDetector {
private:
    cv::CascadeClassifier face_cascade;
    
public:
    FaceDetector(const std::string& cascade_path);
    std::vector<cv::Rect> detectFaces(const cv::Mat& frame);
    bool isLoaded() const;
};

#endif
