#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <string>
#include <vector>

using FaceEncoding = std::vector<float>;

struct UserData {
    std::string username;
    std::vector<FaceEncoding> encodings;
};

class DataManager {
private:
    std::string data_dir;
    
public:
    DataManager(const std::string& dir = "data/users");
    
    bool saveUserEncoding(const std::string& username, const FaceEncoding& encoding);
    bool loadUserEncodings(const std::string& username, std::vector<FaceEncoding>& encodings);
    std::vector<std::string> listUsers();
    bool userExists(const std::string& username);
};

#endif
