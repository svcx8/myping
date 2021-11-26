#ifndef TCP_HEADER
#define TCP_HEADER

#include <chrono>

class PingConn {
public:
    static void Start(const char* ip, int port, int count);
};

#endif // tcp.hh