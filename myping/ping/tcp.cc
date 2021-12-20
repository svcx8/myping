#include "tcp.hh"

#include "misc/net.hh"

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
    int multiple_of_ten = count / 10;
    char count_prefix[6] = "%d";

    if (multiple_of_ten) {
        int count_of_zero = 1;
        while (multiple_of_ten) {
            count_of_zero++;
            multiple_of_ten /= 10;
        }

        count_prefix[1] = 0x30;
        if (count_of_zero == 10) {
            count_prefix[2] = 0x31;
            count_prefix[3] = 0x30;
            count_prefix[4] = 0x64;

        }

        else {
            count_prefix[2] = 0x30 + count_of_zero;
            count_prefix[3] = 0x64;
        }
    }

    char full_prefix[15] = "\t[";
    int strlen_of_count_prefix = strlen(count_prefix);
    memcpy(&full_prefix[2], count_prefix, strlen_of_count_prefix);
    memcpy(&full_prefix[strlen_of_count_prefix + 2], "/", 1);
    memcpy(&full_prefix[strlen_of_count_prefix + 3], count_prefix, strlen_of_count_prefix);
    memcpy(&full_prefix[strlen_of_count_prefix + strlen_of_count_prefix + 3], "]", 1);

    char normal_log_format[32 + 15]{};
    int strlen_of_full_prefix = strlen(full_prefix);
    memcpy(normal_log_format, full_prefix, strlen_of_full_prefix);
    memcpy(&normal_log_format[strlen_of_full_prefix], "\tFrom %s%s\e[0m\ttime=%s%d ms\e[0m\n", strlen("\tFrom %s%s\e[0m\ttime=%s%d ms\e[0m\n"));

    char timeout_log_format[45 + 15]{};
    memcpy(timeout_log_format, full_prefix, strlen_of_full_prefix);
    memcpy(&timeout_log_format[strlen_of_full_prefix], "\tFrom \e[1;41m%s\e[0m\ttime=\e[47;30mtimeout\e[0m\n", strlen("\tFrom \e[1;41m%s\e[0m\ttime=\e[47;30mtimeout\e[0m\n"));

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
                        std::printf(normal_log_format, i + 1, count, color, ip, color, time);

                        CloseSocket(ping_socket);
                        recv++;
                    }

                } else if (result == 0) {
                    std::printf(timeout_log_format, i + 1, count, ip);
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