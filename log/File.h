//
// Created by airy on 2022/1/1.
//
#ifndef WEB_SERVER_FILE_H
#define WEB_SERVER_FILE_H
#include "../base/common.h"
#include <cstdio>
#include <string>

class File : public NoCopyable {
public:
    File(const std::string& file_name, size_t buff_size = 64 * 1024);
    ~File();
    void append(const char* info, size_t len);
    void flush();
private:
    FILE* m_fp;
    size_t m_buff_size;
    char* buff;
    size_t write(const char* info, size_t len);
};
#endif  // WEB_SERVER_FILE_H