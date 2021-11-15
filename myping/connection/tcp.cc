#include "tcp.hh"

#include <dispatcher/epoller.hh>
#include <dispatcher/ibusiness_event.hh>

#include <thread>

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

constexpr int timeout_sec = 3;

void PingConn::OnReadable(SOCKET Socket) {
    logger("%s", __FUNCTION__);
}

void PingConn::OnWritable(SOCKET Socket) {
    CloseSocket(Socket);
}

void PingConn::OnCloseable(SOCKET Socket) {
    logger("%s", __FUNCTION__);
    CloseSocket(Socket);
}

TcpConn::TcpConn() {
    start_ = system_clock::now();
}

long long TcpConn::Duration() {
    end_ = system_clock::now();
    auto duration = duration_cast<milliseconds>(end_ - start_);
    return duration.count();
}

void PingConn::Start(const char* ip, int port, int count) {
    try {
        long flag = EPOLLOUT;

        IPoller* conn_poller = new EPoller(new PingConn(), 2);
        EPoller::reserved_list_.push_back(conn_poller);

        // Reactor
        std::thread([&] {
            while (true) {
                conn_poller->Poll();
            }
        }).detach();

        struct timeval timeout {
            timeout_sec, 0
        };

        for (int i = 0; i < count; ++i) {
            int ping_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            setsockopt(ping_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            setsockopt(ping_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

            struct sockaddr_in cmd;
            cmd.sin_family = AF_INET;
            cmd.sin_addr.s_addr = inet_addr(ip);
            cmd.sin_port = htons(port);
            auto start = system_clock::now();
            if (connect(ping_socket, (sockaddr*)&cmd, sizeof(cmd)) != SOCKET_ERROR) {
                auto end = system_clock::now();
                auto duration = duration_cast<milliseconds>(end - start);
                auto time = duration.count();
                const char* color = " \e[0m";
                if (time < 50) {
                    color = "\e[1;30;42m";
                }

                else if (time < 100) {
                    color = "\e[1;30;43m";
                }

                else {
                    color = "\e[1;41m";
                }
                logger("\t[%d/%d] From %s%s\e[0m time=%s%lld ms\e[0m", i + 1, count, color, ip, color, time);
                conn_poller->AddSocket(ping_socket, flag);
            }

            else if (errno == EINPROGRESS) {
                logger("\t[%d/%d] From \e[1;41m%s\e[0m time=\e[47;30mtimeout\e[0m", i + 1, count, ip);
                conn_poller->RemoveCloseSocket(ping_socket);
            }

            else {
                NetEx();
            }
        }
    } catch (BaseException& ex) {
        logger("Exception: %s\n[%s] [%s] Line: #%d", ex.result_, ex.file_, ex.function_, ex.line_);
    }
}