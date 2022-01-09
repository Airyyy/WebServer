//
// Created by airy on 2022/1/3.
//

#ifndef WEB_SERVER_SOCKETUTILS_H
#define WEB_SERVER_SOCKETUTILS_H
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>

const int MAX_LISTEN = 2048;

void setNoBlocking(int fd);
void setNoDelay(int fd);

int createListenFd(int port);

ssize_t readAll(int fd, char* buff, size_t read_len);
ssize_t readAll(int fd, std::string& in_buff);
ssize_t writeAll(int fd, const char* buff, size_t len);
ssize_t writeAll(int fd, std::string& str);
#endif //WEB_SERVER_SOCKETUTILS_H
