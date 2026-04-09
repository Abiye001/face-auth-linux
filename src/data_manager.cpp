#include "data_manager.h"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

DataManager::DataManager(const std::string& dir) : data_dir(dir) {
    fs::create_directories(data_dir);
}

bool DataManager::saveUserEncoding(const std::string& username, const FaceEncoding& encoding) {
    try {
        std::string filepath = data_dir + "/" + username + ".dat";
        
        // Load existing encodings
        std::vector<FaceEncoding> encodings;
        if (fs::exists(filepath)) {
            std::ifstream fin(filepath, std::ios::binary);
            size_t num_encodings;
            fin.read(reinterpret_cast<char*>(&num_encodings), sizeof(num_encodings));
            
            for (size_t i = 0; i < num_encodings; i++) {
                size_t enc_size;
                fin.read(reinterpret_cast<char*>(&enc_size), sizeof(enc_size));
                FaceEncoding enc(enc_size);
                fin.read(reinterpret_cast<char*>(enc.data()), enc_size * sizeof(float));
                encodings.push_back(enc);
            }
            fin.close();
        }
        
        // Add new encoding
        encodings.push_back(encoding);
        
        // Save all encodings
        std::ofstream fout(filepath, std::ios::binary);
        size_t num_encodings = encodings.size();
        fout.write(reinterpret_cast<const char*>(&num_encodings), sizeof(num_encodings));
        
        for (const auto& enc : encodings) {
            size_t enc_size = enc.size();
            fout.write(reinterpret_cast<const char*>(&enc_size), sizeof(enc_size));
            fout.write(reinterpret_cast<const char*>(enc.data()), enc_size * sizeof(float));
        }
        fout.close();
        
        std::cout << "Saved encoding for user: " << username << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving encoding: " << e.what() << std::endl;
        return false;
    }
}

bool DataManager::loadUserEncodings(const std::string& username, std::vector<FaceEncoding>& encodings) {
    try {
        std::string filepath = data_dir + "/" + username + ".dat";
        
        if (!fs::exists(filepath)) {
            return false;
        }
        
        std::ifstream fin(filepath, std::ios::binary);
        size_t num_encodings;
        fin.read(reinterpret_cast<char*>(&num_encodings), sizeof(num_encodings));
        
        encodings.clear();
        for (size_t i = 0; i < num_encodings; i++) {
            size_t enc_size;
            fin.read(reinterpret_cast<char*>(&enc_size), sizeof(enc_size));
            FaceEncoding enc(enc_size);
            fin.read(reinterpret_cast<char*>(enc.data()), enc_size * sizeof(float));
            encodings.push_back(enc);
        }
        fin.close();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading encodings: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::string> DataManager::listUsers() {
    std::vector<std::string> users;
    
    for (const auto& entry : fs::directory_iterator(data_dir)) {
        if (entry.path().extension() == ".dat") {
            users.push_back(entry.path().stem().string());
        }
    }
    
    return users;
}

bool DataManager::userExists(const std::string& username) {
    std::string filepath = data_dir + "/" + username + ".dat";
    return fs::exists(filepath);
}
