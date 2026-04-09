cmake_minimum_required(VERSION 3.10)
project(FaceAuth)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenCV REQUIRED)
find_package(dlib REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

# Enrollment tool
set(ENROLL_SOURCES
    src/main.cpp
    src/face_detector.cpp
    src/camera_handler.cpp
    src/face_recognizer.cpp
    src/data_manager.cpp
)
add_executable(face-auth-enroll ${ENROLL_SOURCES})
target_link_libraries(face-auth-enroll ${OpenCV_LIBS} dlib::dlib)

# Verification tool (for PAM)
set(VERIFY_SOURCES
    src/verify.cpp
    src/face_detector.cpp
    src/camera_handler.cpp
    src/face_recognizer.cpp
    src/data_manager.cpp
)
add_executable(face-auth-verify ${VERIFY_SOURCES})
target_link_libraries(face-auth-verify ${OpenCV_LIBS} dlib::dlib)

# PAM module
add_library(pam_face_auth SHARED src/pam_module.cpp)
target_link_libraries(pam_face_auth pam)

# Camera tester
set(TEST_SOURCES
    test_all_cameras.cpp
    src/face_detector.cpp
    src/camera_handler.cpp
)
add_executable(test-cameras ${TEST_SOURCES})
target_link_libraries(test-cameras ${OpenCV_LIBS})

# Installation
install(TARGETS face-auth-enroll DESTINATION /usr/local/bin)
install(TARGETS face-auth-verify DESTINATION /usr/local/bin)
install(TARGETS pam_face_auth DESTINATION /lib/security)
install(DIRECTORY models/ DESTINATION /usr/local/share/face-auth/models)
