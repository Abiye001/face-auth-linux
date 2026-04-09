#include "face_recognizer.h"
#include <iostream>
#include <cmath>
#include <dlib/image_transforms.h>

FaceRecognizer::FaceRecognizer() : models_loaded(false), net_ptr(nullptr) {}

bool FaceRecognizer::loadModels(const std::string& predictor_path, 
                                const std::string& recognition_model_path) {
    try {
        dlib::deserialize(predictor_path) >> shape_predictor;
        models_loaded = true;
       // std::cout << "Face recognition models loaded successfully!" << std::endl;
        // std::cout << "Note: Using simplified face recognition (histogram-based)" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading models: " << e.what() << std::endl;
        models_loaded = false;
        return false;
    }
}

FaceEncoding FaceRecognizer::generateEncoding(const cv::Mat& face_image) {
    if (!models_loaded) {
        throw std::runtime_error("Models not loaded");
    }
    
    FaceEncoding encoding;
    
    // Convert to grayscale and resize
    cv::Mat gray, resized;
    cv::cvtColor(face_image, gray, cv::COLOR_BGR2GRAY);
    cv::resize(gray, resized, cv::Size(128, 128));
    
    // Get facial landmarks for alignment
    dlib::cv_image<unsigned char> dlib_img(resized);
    dlib::rectangle face_rect(0, 0, resized.cols, resized.rows);
    auto shape = shape_predictor(dlib_img, face_rect);
    
    // Create encoding from facial landmarks and image features
    // 1. Landmark positions (68 landmarks * 2 coordinates = 136 values)
    for (unsigned long i = 0; i < shape.num_parts(); ++i) {
        encoding.push_back(static_cast<float>(shape.part(i).x()) / resized.cols);
        encoding.push_back(static_cast<float>(shape.part(i).y()) / resized.rows);
    }
    
    // 2. Image histogram features
    int histSize = 64;
    float range[] = {0, 256};
    const float* histRange = {range};
    cv::Mat hist;
    cv::calcHist(&resized, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);
    cv::normalize(hist, hist, 0, 1, cv::NORM_MINMAX);
    
    for (int i = 0; i < histSize; i++) {
        encoding.push_back(hist.at<float>(i));
    }
    
    return encoding;
}

double FaceRecognizer::compareEncodings(const FaceEncoding& enc1, const FaceEncoding& enc2) {
    if (enc1.size() != enc2.size()) {
        return 1.0; // Maximum distance
    }
    
    // Calculate Euclidean distance
    double distance = 0.0;
    for (size_t i = 0; i < enc1.size(); i++) {
        double diff = enc1[i] - enc2[i];
        distance += diff * diff;
    }
    
    return std::sqrt(distance);
}

bool FaceRecognizer::matches(const FaceEncoding& enc1, const FaceEncoding& enc2, double threshold) {
    double distance = compareEncodings(enc1, enc2);
    return distance < threshold;
}
