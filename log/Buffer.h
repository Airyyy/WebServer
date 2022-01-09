//
// Created by airy on 2022/1/2.
//

#ifndef WEB_SERVER_BUFFER_H
#define WEB_SERVER_BUFFER_H
#include "./../base/common.h"
#include <cstring>

const size_t BUFF_SIZE_LOWER = 1024;
const size_t BUFF_SIZE_UPPER = 4 * 1024 * 1024;
const size_t WRITE_BUFF_UPPER = 25;

class Buffer : public NoCopyable {
public:
    Buffer(size_t buff_size = BUFF_SIZE_UPPER);
    ~Buffer();
    /* return successfully append size */
    size_t append(const char* info, size_t len);
    bool available(size_t required_size);
    size_t size();
    char* getBuff();
    void reset();
    void zeroFill();

private:
    size_t m_buff_size;
    size_t m_curr_idx;
    char* m_buff;
};
#endif //WEB_SERVER_BUFFER_H
