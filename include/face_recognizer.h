#ifndef FACE_RECOGNIZER_H
#define FACE_RECOGNIZER_H

#include <opencv2/opencv.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing.h>
#include <dlib/dnn.h>
#include <vector>
#include <string>

using FaceEncoding = std::vector<float>;

class FaceRecognizer {
private:
    dlib::shape_predictor shape_predictor;
    bool models_loaded;
    
public:
    FaceRecognizer();
    bool loadModels(const std::string& predictor_path, 
                    const std::string& recognition_model_path);
    
    FaceEncoding generateEncoding(const cv::Mat& face_image);
    double compareEncodings(const FaceEncoding& enc1, const FaceEncoding& enc2);
    bool matches(const FaceEncoding& enc1, const FaceEncoding& enc2, 
                double threshold = 0.6);
    bool isLoaded() const { return models_loaded; }
    
private:
    void* net_ptr;  // Opaque pointer to avoid template issues
};

#endif
