#include <iostream>
#include <unistd.h>
#include <memory>
#include <vector>
#include <functional>

#include "threadUtils/ThreadPool.h"
#include "log/AsyncLogging.h"
#include "log/Logging.h"
#include "net/Server.h"
#include <getopt.h>

int main(int argc, char* argv[]) {
    int thread_num = 4;
    int port = 8080;
    std::string log_path = "./AiryWebServer.log";

    int opt;
    const char *str = "t:l:p:";
    while ((opt = getopt(argc, argv, str)) != -1) {
        switch (opt) {
            case 't': {
                thread_num = atoi(optarg);
                break;
            }
            case 'l': {
                log_path = optarg;
                if (log_path.size() < 2 || optarg[0] != '/') {
                    printf("log path should start with \"/\"\n");
                    abort();
                }
                break;
            }
            case 'p': {
                port = atoi(optarg);
                break;
            }
            default:
                break;
        }
    }

    AsyncLogging async_logging(log_path);
    Logger::setOutputFunc([&async_logging](const char* info, size_t len) { async_logging.append(info, len); });
    Thread<AsyncLogging> t(&async_logging);
    t.start();

    Server server(port, thread_num);
    server.start();
}
