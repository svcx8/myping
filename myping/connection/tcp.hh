#ifndef TCP_HEADER
#define TCP_HEADER

#include <dispatcher/ipoller.hh>

#include <chrono>

class TcpConn {
public:
    std::chrono::system_clock::time_point start_;
    std::chrono::system_clock::time_point end_;

    TcpConn();
    long long Duration();
};

class PingConn : public IBusinessEvent {
public:
    void OnAcceptable(SOCKET) override{};
    void OnCloseable(SOCKET) override;
    void OnReadable(SOCKET) override;
    void OnWritable(SOCKET) override;
    static void Start(const char* ip, int port, int count);
};

#endif // tcp.hh