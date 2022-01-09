//
// Created by airy on 2022/1/4.
//

#include "Server.h"
#include "socketUtils.h"

Server::Server(int port, int thread_num)
    : m_listen_fd(createListenFd(port)),
      m_start(false),
      m_main_loop(),
      m_thread_pool(thread_num){
    setNoBlocking(m_listen_fd);
}

Server::~Server() {
    if (m_start) close(m_listen_fd);
}

void Server::getInfo() {
    m_main_loop.getInfo();
    m_thread_pool.getInfo();
}

void Server::start() {
    std::shared_ptr<Channel> accept_channel(new Channel(m_listen_fd));
    accept_channel->setEvents(EPOLLIN | EPOLLET);
    accept_channel->setReadHandle([this]()->CHANNEL_HANDLE_RESULT{ return this->handleNewConnection(); });
    m_main_loop.addToEpoll(accept_channel);

    printf("Server started.\n");

    m_thread_pool.start();
    m_start = true;
    m_main_loop.loop();
}

CHANNEL_HANDLE_RESULT Server::handleNewConnection() {
    sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);
    memset(&client_addr, 0, sizeof(client_addr));
    int client_fd = 0;
    while ((client_fd = accept(m_listen_fd, (sockaddr*)&client_addr, &addr_len)) > 0) {
        if (client_fd >= MAX_CONNECTION_NUM) {
            close(client_fd);
            continue;
        }

        setNoBlocking(client_fd);
        setNoDelay(client_fd);
//        std::cout << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":"
//            << ntohs(client_addr.sin_port) << " bind to " << client_fd << " All: " << m_cnt << std::endl;
        // LOG << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":"
        // << ntohs(client_addr.sin_port) << " Link on " << client_fd;

        std::shared_ptr<Channel> client_channel(new Channel(client_fd, std::shared_ptr<HttpEvent>(new HttpEvent(client_fd))));

        auto& event_loop = m_thread_pool.getNextThread();
        (*event_loop)->addToWaitingQueue(client_channel);
    }
    return CHANNEL_NORMAL;
}
