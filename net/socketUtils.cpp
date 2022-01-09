//
// Created by airy on 2022/1/4.
//
#include "socketUtils.h"
#include <cstdio>
#include <unistd.h>
#include <cerrno>
#include <string>
#include <netinet/tcp.h>

void setNoBlocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

void setNoDelay(int fd) {
    int enable = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable, sizeof(enable));
}

int createListenFd(int port) {
    if (port < 0 || port > 65535) return -1;
    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    int opt_val = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
    sockaddr_in addr{};
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
        exit(1);
        // printf("socketUtils::createListenFd::bind()\n");
    }

    if (listen(listen_fd, MAX_LISTEN) == -1) {
        exit(1);
        // printf("socketUtils::createListenFd::listen()\n");
    }
    // printf("Listening.\n");
    return listen_fd;
}

ssize_t readAll(int fd, char* buff, size_t read_len) {
    ssize_t remain = read_len;
    ssize_t curr_size = 0;
    ssize_t already_read_size = 0;
    while (remain > 0) {
        if ((curr_size = read(fd, buff, remain)) < 0) {
            if (errno == EINTR) {
                curr_size = 0;
            } else if (errno == EAGAIN) {
                return already_read_size;
            } else {
                return -1;
            }
        } else if (curr_size == 0) {
            return already_read_size;
        }

        already_read_size += curr_size;
        remain -= curr_size;
        buff += curr_size;
    }
    return already_read_size;
}

ssize_t readAll(int fd, std::string& in_buffer) {
    ssize_t read_size = 0;
    ssize_t curr_size;
    while (true) {
        // printf("readAll looping\n");
        char buff[4096];
        if ((curr_size = readAll(fd, buff, 4096)) < 0) {
            // printf("Curr size: %ld\n", curr_size);
            if (errno == EINTR) {
                continue;
            }
            else if (errno == EAGAIN) {
                return read_size;
            } else {
                return -1;
            }
        } else if (curr_size == 0) {
            // printf("redsum = %d\n", readSum);
            return read_size;
        }
        // printf("Read: %s\n", buff);
        read_size += curr_size;
        in_buffer += std::string(buff, buff + curr_size);
        // printf("string: %s\n", in_buffer.c_str());
    }
}

ssize_t writeAll(int fd, const char* buff, size_t len) {
    ssize_t write_size = 0;
    ssize_t curr_size;
    while (write_size < len) {
        curr_size = write(fd, buff, len);
        if (curr_size < 0) {
            if (errno == EINTR) {
                continue;
            } else if (errno == EAGAIN) {
                return write_size;
            } else {
                return -1;
            }
        } else {
            write_size += curr_size;
        }
    }
    return write_size;
}

ssize_t writeAll(int fd, std::string& sbuff) {
    size_t nleft = sbuff.size();
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    const char *ptr = sbuff.c_str();
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0) {
                if (errno == EINTR) {
                    nwritten = 0;
                    continue;
                } else if (errno == EAGAIN)
                    break;
                else
                    return -1;
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    if (writeSum == static_cast<int>(sbuff.size()))
        sbuff.clear();
    else
        sbuff = sbuff.substr(writeSum);
    return writeSum;
}
