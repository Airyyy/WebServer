//
// Created by airy on 2022/1/4.
//

#include "HttpEvent.h"
#include "socketUtils.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>

const size_t SMALL_SIZE = 100;

int HttpEvent::http_cnt = 0;
int HttpEvent::http_destroy = 0;
Mutex HttpEvent::mutex;

const char* content_type(char* file) {
    // char extension[SMALL_SIZE];
//    char file_name[SMALL_SIZE];
//    strcpy(file_name, file);
//    strtok(file_name, ".");
    char* temp = strtok(file, ".");
    temp = strtok(nullptr, ".");
    if (!temp) return "text/plain";

    if (!strcmp(temp, "html") || !strcmp(temp, "htm")) {
        return "text/html";
    } else {
        return "text/plain";
    }
}

HttpEvent::HttpEvent(int fd)
    : m_fd(fd),
      m_long_link(false),
      m_checked_index(0),
      m_tail_index(0),
      m_start_line(0),
      m_check_state(CHECK_STATE_REQUEST_LINE),
      m_in_buff(),
      m_out_buff(),
      m_file_name(),
      m_http_version(HTTP11),
      m_method_type(METHOD_GET),
      m_map(){
    memset(m_in_buff, 0, sizeof(m_in_buff));
    // memset(m_out_buff, 0, sizeof(m_out_buff));
    LockGuard locker(mutex);
    ++HttpEvent::http_cnt;
}

HttpEvent::~HttpEvent() {
    LockGuard locker(mutex);
    ++HttpEvent::http_destroy;
}

void HttpEvent::resetState() {
    // m_checked_index = 0;
    // m_tail_index = 0;
    // m_start_line = 0;
    m_check_state = CHECK_STATE_REQUEST_LINE;
}

void HttpEvent::resetIn() {
    ssize_t len = m_tail_index - m_checked_index;
    if (len == 0) {
        memset(m_in_buff, 0, sizeof(m_in_buff));
    } else {
        strncpy(m_in_buff, m_in_buff+m_checked_index, len);
    }

    m_checked_index = 0;
    m_start_line = 0;
    m_tail_index = len;
}

void HttpEvent::resetOut() {
    // memset(m_out_buff, 0, sizeof(m_out_buff));;
    m_out_buff.clear();
}

HTTP_HANDLE_RESULT HttpEvent::handleRead() {
    do {
        ssize_t read_len = readAll(m_fd, m_in_buff+m_tail_index, sizeof(m_in_buff)-m_tail_index);
        if (read_len <= 0) {
            // printf("returned\n");
            return HTTP_CLOSE;
        }
        m_tail_index += read_len;

        HTTP_CODE parse_res = parseContent(m_in_buff, m_checked_index, m_check_state, m_tail_index, m_start_line);
        // printf("Http code is %s\n", http_name[parse_res].c_str());
        HTTP_HANDLE_RESULT handle_res;
        switch (parse_res) {
            case GET_REQUEST: {
                handle_res = getOutData();
                resetState();
                resetIn();  // one http request's important info has been saved.

                if (handle_res != HTTP_NORMAL) return handle_res;
                handle_res = handleWrite();
                // return HTTP_NORMAL/HTTP_WRITE_AGAIN/HTTP_CLOSE
                if (handle_res != HTTP_NORMAL) return handle_res;
                break;
            }
            case NO_REQUEST: {
                return HTTP_READ_AGAIN;
            }
            default: {
                return HTTP_CLOSE;
            }
        }
    } while (m_checked_index < m_tail_index);  // deal with multiple request.

    resetIn();
    return HTTP_NORMAL;
}

// return HTTP_NORMAL/HTTP_WRITE_AGAIN/HTTP_CLOSE
HTTP_HANDLE_RESULT HttpEvent::handleWrite() {
    ssize_t write_len = writeAll(m_fd, m_out_buff);
    if (write_len <= 0) return HTTP_CLOSE;

    if (!m_out_buff.empty()) return HTTP_WRITE_AGAIN;
    m_out_buff.clear();
    return HTTP_NORMAL;
}

LINE_STATUS HttpEvent::parseLine(char* buff, ssize_t& checked_index, ssize_t& tail_index) {
    // line is ended with \r\n
    char temp;
    for (; checked_index < tail_index; ++checked_index) {
        temp = buff[checked_index];
        if (temp == '\r') {
            if (checked_index + 1 == tail_index) {
                return LINE_OPEN;
            } else if (buff[checked_index+1] == '\n') {
                // the former chars will be treated in the same line.
                buff[checked_index++] = '\0';
                buff[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        } else if (temp == '\n') {
            if (checked_index > 1 && buff[checked_index-1] == '\r') {
                buff[checked_index-1] = '\0';
                buff[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

// GET /index.html HTTP/1.0
HTTP_CODE HttpEvent::parseRequestLine(char* temp, CHECK_STATUS& check_state) {
    // find the first index that char in " \t" occurs
    // return nullptr for no match.
    char* url = strpbrk(temp, " \t");
    if (!url) {
        return BAD_REQUEST;
    }
    *url++ = '\0';
    char* method = temp;

    if (strcasecmp(method, "GET") == 0) {
        m_method_type = METHOD_GET;
    } else if (strcasecmp(method, "HEAD") == 0) {
        m_method_type = METHOD_HEAD;
    } else {
        return UNSUPPORTED_REQUEST;
    }

    // return first index that char not in " \t", aiming to skip blank characters;=
    url += strspn(url, " \t");
    char* version = strpbrk(url, " \t");
    if (!version) {
        return BAD_REQUEST;
    }
    *version++ = '\0';
    version += strspn(version, " \t");
    if (strcasecmp(version, "HTTP/1.1") != 0) {
        return BAD_REQUEST;
    }
    m_http_version = HTTP11;

    if (strncasecmp(url, "http://", 7) == 0) {
        url += 7;
        // url = strchr(url, '/');
    }

    if (!url) {
        return BAD_REQUEST;
    }
    if (url[0] == '/') url++;
    m_file_name = url;
    m_content_type = content_type(url);
    check_state = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

HTTP_CODE HttpEvent::parseHeader(char* buff) {
    // there is one empty line after header.
    if (buff[0] == '\0') {
        return GET_REQUEST;
    }

    char key[256], val[256];
    memset(key, 0, sizeof(key));
    memset(val, 0, sizeof(val));
    sscanf(buff, "%[^:]: %s", key, val);
    if (strlen(key) == 0 || strlen(val) == 0) return BAD_REQUEST;
    m_map[key] = val;
    return NO_REQUEST;
}

HTTP_CODE HttpEvent::parseContent(char* buff, ssize_t& checked_index, CHECK_STATUS& check_state, ssize_t& tail_index, ssize_t& start_line) {
    LINE_STATUS line_state = LINE_OK;
    HTTP_CODE http_code = NO_REQUEST;

    while ((line_state = parseLine(buff, checked_index, tail_index)) == LINE_OK) {
        char* temp = buff + start_line;
        start_line = checked_index;
        switch (check_state) {
            case CHECK_STATE_REQUEST_LINE: {
                http_code = parseRequestLine(temp, check_state);
                if (http_code == BAD_REQUEST) {
                    return BAD_REQUEST;
                }
                break;
            }
            case CHECK_STATE_HEADER: {
                http_code = parseHeader(temp);
                if (http_code == BAD_REQUEST) {
                    return BAD_REQUEST;
                } else if (http_code == GET_REQUEST) {
                    if (strcasecmp(m_map["Connection"].c_str(), "keep-alive") == 0) {
                        m_long_link = true;
                    }
                    return GET_REQUEST;
                }
                break;
            }
            default: {
                return INTERNAL_ERROR;
            }
        }
    }

    if (line_state == LINE_OPEN) {
        return NO_REQUEST;
    } else {
        return BAD_REQUEST;
    }
}

// return HTTP_NORMAL or HTTP_ERROR
HTTP_HANDLE_RESULT HttpEvent::getOutData() {
    std::string header;
    header += "HTTP/1.1 200 OK\r\n";
    // echo test
    if (m_file_name == "hello") {
        m_out_buff = "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\nHello World";
        return HTTP_NORMAL;
    }


    m_file_name = "/home/airy/study/code/cpp/Web-Server/" + m_file_name;

    struct stat sbuf{};
    if (stat(m_file_name.c_str(), &sbuf) < 0) {
        header.clear();
        handleError(404, "Not Found!");
        return HTTP_ERROR;
    }
    header += "Content-Type: " + m_content_type + "\r\n";
    header += "Content-Length: " + std::to_string(sbuf.st_size) + "\r\n";
    header += "Server: Airy's Web Server\r\n";
    // 头部结束
    header += "\r\n";
    m_out_buff += header;

    if (m_method_type == METHOD_HEAD) return HTTP_NORMAL;

    int src_fd = open(m_file_name.c_str(), O_RDONLY, 0);
    if (src_fd < 0) {
        m_out_buff.clear();
        handleError(404, "Not Found!");
        return HTTP_ERROR;
    }
    void *mmapRet = mmap(nullptr, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
    close(src_fd);
    if (mmapRet == (void *)-1) {
        munmap(mmapRet, sbuf.st_size);
        m_out_buff.clear();
        handleError(404, "Not Found!");
        return HTTP_ERROR;
    }
    char *src_addr = static_cast<char *>(mmapRet);
    m_out_buff += std::string(src_addr, src_addr + sbuf.st_size);
    munmap(mmapRet, sbuf.st_size);
    return HTTP_NORMAL;

//    if (!m_file_name) {
//        FILE* fp = fopen(m_file_name, "r");
//        if (fp == nullptr) {
//            printf("%s not exist\n", m_file_name);
//            return handleError(404, "File not found.");
//        }
//        while (fgets(m_out_buff, HTTP_OUT_BUFF_SIZE, fp) != nullptr);
//        fclose(fp);
//        return HTTP_NORMAL;
//    }
//    return handleError(404, "File not found.");
}

HTTP_HANDLE_RESULT HttpEvent::handleError(int err_num, std::string short_msg) {
    short_msg = " " + short_msg;
    char send_buff[4096];
    std::string body_buff, header_buff;
    body_buff += "<html><title>Something Wrong.</title>";
    body_buff += "<body bgcolor=\"ffffff\">";
    body_buff += std::to_string(err_num) + short_msg;
    body_buff += "<hr><em> Airy's Web Server</em>\n</body></html>";

    header_buff += "HTTP/1.1 " + std::to_string(err_num) + short_msg + "\r\n";
    header_buff += "Content-Type: text/html\r\n";
    header_buff += "Connection: Close\r\n";
    header_buff += "Content-Length: " + std::to_string(body_buff.size()) + "\r\n";
    header_buff += "Server: Airy's Web Server\r\n";
    ;
    header_buff += "\r\n";
    // 错误处理不考虑writen不完的情况
    sprintf(send_buff, "%s", header_buff.c_str());
    writeAll(m_fd, send_buff, strlen(send_buff));
    sprintf(send_buff, "%s", body_buff.c_str());
    writeAll(m_fd, send_buff, strlen(send_buff));
    return HTTP_CLOSE;
}

bool HttpEvent::isLongLink() { return m_long_link; }
