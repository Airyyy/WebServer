//
// Created by airy on 2022/1/1.
//

#include "File.h"


File::File(const std::string &file_name, size_t buff_size)
    : m_fp(fopen(file_name.c_str(), "a")), m_buff_size(buff_size){
    buff = new char[buff_size];
    setbuffer(m_fp, buff, m_buff_size);
}

File::~File() {
    delete [] buff;
    buff = nullptr;
    fclose(m_fp);
}

void File::flush() {
    fflush(m_fp);
}

size_t File::write(const char* info, size_t len) {
    return fwrite_unlocked(info, 1, len, m_fp);
}

void File::append(const char* info, size_t len) {
    size_t remain = len;
    while (remain > 0) {
        size_t temp = this->write(info, remain);
        if (temp == 0) {
            int err = ferror(m_fp);
            if (err) {
                fprintf(stderr, "File::append() failed.\n");
                break;
            }
        }
        remain -= temp;
    }
}
