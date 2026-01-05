#include "CreateFileDir.h"

void createDirectory(const std::string& dirPath)
{
#ifdef WIN32
    // Windows 系统创建目录
    if (!CreateDirectory(dirPath.c_str(), NULL)) {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_ALREADY_EXISTS) {
            fprintf(stderr, "Error creating directory %s\n", dirPath.c_str());
        }
    }
#else
    if (mkdir(dirPath.c_str(), 0755) != 0) {
        if (errno != EEXIST) {
            fprintf(stderr, "Error creating directory %s\n", dirPath.c_str());
        }
    }
#endif
}