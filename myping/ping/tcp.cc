#include "tcp.hh"

#include "misc/net.hh"

using std::thread;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

void SetNonBlocking(int socket) {
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(socket, FIONBIO, &mode);
#else
    int old_opt = fcntl(socket, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(socket, F_SETFL, new_opt);
#endif
}

constexpr int timeout_sec = 3;
constexpr int interval = 1;

void PingConn::Start(const char* ip, int port, int count) {
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(0x0201, &wsa_data);
#endif
    logger("[myping] %s:%d, count: %d", ip, port, count);
    struct timeval timeout {
        timeout_sec, 0
    };

    int recv = 0;
    int fastest = INT_MAX;
    int slowest = 0;

    for (int i = 0; i < count; ++i) {
        SleepSec(interval);
        try {
            int ping_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (ping_socket == SOCKET_ERROR) {
                throw NetEx();
            }
            SetNonBlocking(ping_socket);

            struct sockaddr_in cmd;
            cmd.sin_family = AF_INET;
            cmd.sin_addr.s_addr = inet_addr(ip);
            cmd.sin_port = htons(port);

            auto start = system_clock::now();
            int result = connect(ping_socket, (sockaddr*)&cmd, sizeof(cmd)) != SOCKET_ERROR;
            if (result == SOCKET_ERROR) {
                throw NetEx();
            }

            {
                fd_set set;
                FD_ZERO(&set);
                FD_SET(ping_socket, &set);

                result = select(ping_socket + 1, nullptr, &set, nullptr, &timeout);
                if (result > 0) {
                    if (FD_ISSET(ping_socket, &set)) {
                        auto end = system_clock::now();
                        auto duration = duration_cast<milliseconds>(end - start);
                        int time = duration.count();

                        fastest = fastest > time ? time : fastest;
                        slowest = slowest < time ? time : slowest;

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
                        logger("\t[%d/%d]\tFrom %s%s\e[0m\ttime=%s%d ms\e[0m", i + 1, count, color, ip, color, time);

                        CloseSocket(ping_socket);
                        recv++;
                    }

                } else if (result == 0) {
                    logger("\t[%d/%d]\tFrom \e[1;41m%s\e[0m\ttime=\e[47;30mtimeout\e[0m", i + 1, count, ip);
                    CloseSocket(ping_socket);
                } else {
                    throw NetEx();
                }
            }

        } catch (BaseException& ex) {
            logger("Exception: %s\n[%s] [%s] Line: #%d", ex.result_, ex.file_, ex.function_, ex.line_);
        }
    }

    logger("\n\tSent: %d\tReceived: %d\tLost: %d\t(%d%% loss)", count, recv, count - recv, (count - recv) * 100 / count);
    if (recv)
        logger("\tFastest: %dms\tSlowest: %dms\tAverage: %dms", fastest, slowest, (fastest + slowest) / 2);
}