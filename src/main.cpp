#include "face_detector.h"
#include "camera_handler.h"
#include "face_recognizer.h"
#include "data_manager.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <map>

void enrollUser(CameraHandler& camera, FaceDetector& detector, 
                FaceRecognizer& recognizer, DataManager& dataManager) {
    std::string username;
    std::cout << "\nEnter username to enroll: ";
    std::cin >> username;
    
    std::cout << "Position your face in front of the camera..." << std::endl;
    std::cout << "Press SPACE to capture (capture 5 images)" << std::endl;
    std::cout << "Press 'q' to cancel" << std::endl;
    
    int captures = 0;
    int target_captures = 5;
    
    while (captures < target_captures) {
        cv::Mat frame;
        if (!camera.captureFrame(frame)) {
            continue;
        }
        
        std::vector<cv::Rect> faces = detector.detectFaces(frame);
        
        // Draw rectangles
        for (const auto& face : faces) {
            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
        }
        
        std::string text = "Captures: " + std::to_string(captures) + "/" + std::to_string(target_captures);
        cv::putText(frame, text, cv::Point(10, 30), 
                    cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
        
        if (faces.size() == 1) {
            cv::putText(frame, "Press SPACE to capture", cv::Point(10, 70),
                       cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
        } else if (faces.size() == 0) {
            cv::putText(frame, "No face detected!", cv::Point(10, 70),
                       cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
        } else {
            cv::putText(frame, "Multiple faces detected!", cv::Point(10, 70),
                       cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
        }
        
        cv::imshow("Enrollment", frame);
        
        int key = cv::waitKey(1);
        if (key == ' ' && faces.size() == 1) {  // Space bar
            // Extract face region
            cv::Mat face_img = frame(faces[0]);
            
            try {
                // Generate encoding
                FaceEncoding encoding = recognizer.generateEncoding(face_img);
                
                // Save encoding
                if (dataManager.saveUserEncoding(username, encoding)) {
                    captures++;
                    std::cout << "Captured " << captures << "/" << target_captures << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error generating encoding: " << e.what() << std::endl;
            }
        } else if (key == 'q') {
            break;
        }
    }
    
    cv::destroyWindow("Enrollment");
    
    if (captures == target_captures) {
        std::cout << "\n✓ Enrollment complete for " << username << "!" << std::endl;
    } else {
        std::cout << "\n✗ Enrollment cancelled" << std::endl;
    }
}

void recognizeUser(CameraHandler& camera, FaceDetector& detector,
                   FaceRecognizer& recognizer, DataManager& dataManager) {
    std::cout << "\nStarting face recognition..." << std::endl;
    std::cout << "Press 'q' to quit" << std::endl;
    
    // Load all user encodings
    std::vector<std::string> users = dataManager.listUsers();
    std::map<std::string, std::vector<FaceEncoding>> all_encodings;
    
    for (const auto& user : users) {
        std::vector<FaceEncoding> encodings;
        if (dataManager.loadUserEncodings(user, encodings)) {
            all_encodings[user] = encodings;
        }
    }
    
    std::cout << "Loaded " << users.size() << " user(s)" << std::endl;
    
    if (users.empty()) {
        std::cout << "No enrolled users! Please enroll someone first." << std::endl;
        return;
    }
    
    while (true) {
        cv::Mat frame;
        if (!camera.captureFrame(frame)) {
            continue;
        }
        
        std::vector<cv::Rect> faces = detector.detectFaces(frame);
        
        for (const auto& face_rect : faces) {
            cv::Mat face_img = frame(face_rect);
            
            try {
                FaceEncoding encoding = recognizer.generateEncoding(face_img);
                
                // Find best match
                std::string best_match = "Unknown";
                double best_distance = 1.0;
                
                for (const auto& [user, user_encodings] : all_encodings) {
                    for (const auto& user_enc : user_encodings) {
                        double distance = recognizer.compareEncodings(encoding, user_enc);
                        if (distance < best_distance) {
                            best_distance = distance;
                            best_match = user;
                        }
                    }
                }
                
                // Display result
                cv::Scalar color = (best_distance < 0.6) ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
                cv::rectangle(frame, face_rect, color, 2);
                
                std::string label = best_match + " (" + std::to_string(best_distance).substr(0, 4) + ")";
                cv::putText(frame, label, cv::Point(face_rect.x, face_rect.y - 10),
                           cv::FONT_HERSHEY_SIMPLEX, 0.6, color, 2);
                
            } catch (const std::exception& e) {
                cv::rectangle(frame, face_rect, cv::Scalar(0, 0, 255), 2);
                cv::putText(frame, "Error", cv::Point(face_rect.x, face_rect.y - 10),
                           cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
            }
        }
        
        cv::imshow("Face Recognition", frame);
        
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }
    
    cv::destroyWindow("Face Recognition");
}

int main() {
    std::cout << "==================================" << std::endl;
    std::cout << "  Face Authentication System" << std::endl;
    std::cout << "==================================" << std::endl;
    
    // Initialize components
    CameraHandler camera(0);
    if (!camera.open()) {
        std::cerr << "Failed to open camera!" << std::endl;
        return 1;
    }
    
    FaceDetector detector("models/haarcascade_frontalface_default.xml");
    if (!detector.isLoaded()) {
        std::cerr << "Failed to load face detector!" << std::endl;
        return 1;
    }
    
    FaceRecognizer recognizer;
    if (!recognizer.loadModels(
        "models/shape_predictor_68_face_landmarks.dat",
        "models/dlib_face_recognition_resnet_model_v1.dat")) {
        std::cerr << "Failed to load face recognition models!" << std::endl;
        return 1;
    }
    
    DataManager dataManager("data/users");
    
    std::cout << "\nAll systems initialized successfully!" << std::endl;
    
    // Main menu
    while (true) {
        std::cout << "\n==================================" << std::endl;
        std::cout << "1. Enroll new user" << std::endl;
        std::cout << "2. Recognize faces" << std::endl;
        std::cout << "3. List enrolled users" << std::endl;
        std::cout << "4. Exit" << std::endl;
        std::cout << "==================================" << std::endl;
        std::cout << "Choice: ";
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                enrollUser(camera, detector, recognizer, dataManager);
                break;
            case 2:
                recognizeUser(camera, detector, recognizer, dataManager);
                break;
            case 3: {
                auto users = dataManager.listUsers();
                std::cout << "\nEnrolled users (" << users.size() << "):" << std::endl;
                if (users.empty()) {
                    std::cout << "  (none)" << std::endl;
                } else {
                    for (const auto& user : users) {
                        std::cout << "  ✓ " << user << std::endl;
                    }
                }
                break;
            }
            case 4:
                std::cout << "Goodbye!" << std::endl;
                return 0;
            default:
                std::cout << "Invalid choice! Please try again." << std::endl;
        }
    }
    
    return 0;
}
