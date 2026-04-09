#include "camera_handler.h"
#include "face_detector.h"
#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    FaceDetector detector("/usr/local/share/face-auth/models/haarcascade_frontalface_default.xml");
    
    for (int cam_id = 0; cam_id <= 3; cam_id++) {
        std::cout << "\n========================================" << std::endl;
        std::cout << "Testing /dev/video" << cam_id << std::endl;
        std::cout << "Watch for camera lights!" << std::endl;
        std::cout << "Press 'q' to test next camera" << std::endl;
        std::cout << "========================================\n" << std::endl;
        
        CameraHandler camera(cam_id);
        if (!camera.open()) {
            std::cout << "Failed to open camera " << cam_id << std::endl;
            continue;
        }
        
        std::cout << "Camera " << cam_id << " opened!" << std::endl;
        
        while (true) {
            cv::Mat frame;
            if (!camera.captureFrame(frame)) {
                std::cout << "Failed to capture" << std::endl;
                break;
            }
            
            auto faces = detector.detectFaces(frame);
            
            for (const auto& face : faces) {
                cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
            }
            
            std::string text = "Camera " + std::to_string(cam_id) + " - Faces: " + std::to_string(faces.size());
            cv::putText(frame, text, cv::Point(10, 30), 
                       cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
            
            cv::imshow("Camera Test", frame);
            
            if (cv::waitKey(1) == 'q') {
                break;
            }
        }
        
        camera.close();
        cv::destroyAllWindows();
    }
    
    return 0;
}
