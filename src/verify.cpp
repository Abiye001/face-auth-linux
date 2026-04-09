#include "face_detector.h"
#include "camera_handler.h"
#include "face_recognizer.h"
#include "data_manager.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono>

bool debug = false;  // Set to true for debugging

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: face-auth-verify <username>" << std::endl;
        return 1;
    }
    
    std::string username = argv[1];
    if (debug) std::cerr << "[DEBUG] Verifying user: " << username << std::endl;
    
    // Initialize components
    CameraHandler camera(0);
    if (!camera.open()) {
        if (debug) std::cerr << "[DEBUG] Failed to open camera!" << std::endl;
        return 1;
    }
    if (debug) std::cerr << "[DEBUG] Camera opened successfully" << std::endl;
    
    FaceDetector detector("/usr/local/share/face-auth/models/haarcascade_frontalface_default.xml");
    if (!detector.isLoaded()) {
        if (debug) std::cerr << "[DEBUG] Failed to load face detector!" << std::endl;
        return 1;
    }
    if (debug) std::cerr << "[DEBUG] Face detector loaded" << std::endl;
    
    FaceRecognizer recognizer;
    if (!recognizer.loadModels(
        "/usr/local/share/face-auth/models/shape_predictor_68_face_landmarks.dat",
        "/usr/local/share/face-auth/models/dlib_face_recognition_resnet_model_v1.dat")) {
        if (debug) std::cerr << "[DEBUG] Failed to load recognizer models!" << std::endl;
        return 1;
    }
    if (debug) std::cerr << "[DEBUG] Face recognizer loaded" << std::endl;
    
    DataManager dataManager("/usr/local/share/face-auth/data/users");
    
    // Load user encodings
    std::vector<FaceEncoding> user_encodings;
    if (!dataManager.loadUserEncodings(username, user_encodings)) {
        if (debug) std::cerr << "[DEBUG] User not enrolled or failed to load: " << username << std::endl;
        return 1;
    }
    if (debug) std::cerr << "[DEBUG] Loaded " << user_encodings.size() << " encodings for user" << std::endl;
    
    // Try to recognize face
    auto start_time = std::chrono::steady_clock::now();
    int max_attempts = 50;
    int faces_detected = 0;
    
    for (int attempt = 0; attempt < max_attempts; attempt++) {
        cv::Mat frame;
        if (!camera.captureFrame(frame)) {
            if (debug) std::cerr << "[DEBUG] Failed to capture frame " << attempt << std::endl;
            continue;
        }
        
        std::vector<cv::Rect> faces = detector.detectFaces(frame);
        
        if (faces.size() == 0) {
            if (debug && attempt % 10 == 0) {
                std::cerr << "[DEBUG] No face detected (attempt " << attempt << ")" << std::endl;
            }
            continue;
        }
        
        if (faces.size() > 1) {
            if (debug) std::cerr << "[DEBUG] Multiple faces detected: " << faces.size() << std::endl;
            continue;
        }
        
        faces_detected++;
        if (debug) std::cerr << "[DEBUG] Face detected! Generating encoding..." << std::endl;
        
        try {
            cv::Mat face_img = frame(faces[0]);
            FaceEncoding encoding = recognizer.generateEncoding(face_img);
            
            if (debug) std::cerr << "[DEBUG] Comparing with " << user_encodings.size() << " stored encodings..." << std::endl;
            
            // Compare with stored encodings
            for (size_t i = 0; i < user_encodings.size(); i++) {
                double distance = recognizer.compareEncodings(encoding, user_encodings[i]);
                if (debug) std::cerr << "[DEBUG] Encoding " << i << " distance: " << distance << std::endl;
                
                if (recognizer.matches(encoding, user_encodings[i], 1.0)) {
                    if (debug) std::cerr << "[DEBUG] MATCH! Authentication SUCCESS" << std::endl;
                    return 0;
                }
            }
            if (debug) std::cerr << "[DEBUG] No match found" << std::endl;
        } catch (const std::exception& e) {
            if (debug) std::cerr << "[DEBUG] Exception: " << e.what() << std::endl;
            continue;
        }
        
        // Check timeout
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
        if (elapsed > 15) {
            if (debug) std::cerr << "[DEBUG] Timeout after " << elapsed << " seconds" << std::endl;
            break;
        }
    }
    
    if (debug) std::cerr << "[DEBUG] Authentication FAILED. Faces detected: " << faces_detected << std::endl;
    return 1;
}
