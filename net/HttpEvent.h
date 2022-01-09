//
// Created by airy on 2022/1/4.
//

#ifndef WEB_SERVER_HTTPEVENT_H
#define WEB_SERVER_HTTPEVENT_H
#include <bits/stdc++.h>
#include <cstdio>
#include "./../base/MutexLock.h"

const int HTTP_IN_BUFF_SIZE = 2048;
const int HTTP_OUT_BUFF_SIZE = 4096;

enum METHOD_TYPE { METHOD_GET = 0, METHOD_POST, METHOD_HEAD };
enum HTTP_VERSION { HTTP10 = 0, HTTP11 };
/**
 * CHECK_STATE_REQUEST_LINE: state machine is processing line in body.
 * CHECK_STATE_HEADER: state machine is processing head of request.
 */
enum CHECK_STATUS { CHECK_STATE_REQUEST_LINE = 0, CHECK_STATE_HEADER };
/**
 * LINE_OK: read one line with no error.
 * LINE_BAD: wrong format.
 * LINE_OPEN: read line not finished.
 */
enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };
/**
 * NO_REQUEST: request is incompletely.
 * GET_REQUEST: got complete request.
 * BAD_REQUEST: request format is wrong.
 * FORBIDDEN_REQUEST: no access permission.
 * INTERNAL_ERROR: error in web server.
 * CLOSED_CONNECTION: client closed the connection.
 */
enum HTTP_CODE { GET_REQUEST = 0, NO_REQUEST, BAD_REQUEST, UNSUPPORTED_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };

const std::string http_name[] = {"GET_REQUEST", "NO_REQUEST", "BAD_REQUEST"};

enum HTTP_HANDLE_RESULT { HTTP_NORMAL = 0, HTTP_CLOSE, HTTP_ERROR, HTTP_WRITE_AGAIN, HTTP_READ_AGAIN };

const std::string http_handle[] = {"HTTP_NORMAL", "HTTP_CLOSE", "HTTP_ERROR", "HTTP_WRITE_AGAIN", "HTTP_READ_AGAIN"};

class HttpEvent {
public:
    static int http_cnt;
    static int http_destroy;
    static Mutex mutex;
    explicit HttpEvent(int fd);
    ~HttpEvent();

    void resetIn();
    void resetOut();
    void resetState();

    HTTP_HANDLE_RESULT handleRead();
    HTTP_HANDLE_RESULT handleWrite();
    HTTP_HANDLE_RESULT handleError(int err_num, std::string msg);

    bool isLongLink();

private:
    LINE_STATUS parseLine(char* buff, ssize_t& checked_index, ssize_t& tail_index);
    HTTP_CODE parseRequestLine(char* buff, CHECK_STATUS& check_state);
    HTTP_CODE parseHeader(char* buff);
    HTTP_CODE parseContent(char* buff, ssize_t& checked_index, CHECK_STATUS& check_state, ssize_t& read_index, ssize_t& start_line);

    HTTP_HANDLE_RESULT getOutData();

private:
    int m_fd;
    bool m_long_link;
    ssize_t m_checked_index;
    ssize_t m_tail_index;
    ssize_t m_start_line;
    CHECK_STATUS m_check_state;

    ssize_t m_write_tail;

    char m_in_buff[HTTP_IN_BUFF_SIZE];
    // std::string m_in_buff;
    std::string m_out_buff;
//    char m_key[256];
//    char m_val[256];

    std::string m_file_name;
    std::string m_content_type;
    METHOD_TYPE m_method_type;
    HTTP_VERSION m_http_version;

    std::map<std::string, std::string> m_map;

};

#endif //WEB_SERVER_HTTPEVENT_H
