#ifndef TCP_HEADER
#define TCP_HEADER

#include <dispatcher/ipoller.hh>

#include <chrono>
#include <map>

class TcpConn {
public:
    std::chrono::system_clock::time_point start_;
    std::chrono::system_clock::time_point end_;
    long long Duration();
    TcpConn();
};

class EchoConn : public IBusinessEvent {
public:
    void OnAcceptable(SOCKET) override{};
    void OnCloseable(SOCKET) override;
    void OnReadable(SOCKET) override;
    void OnWritable(SOCKET) override;
    static std::map<int, TcpConn> conn_list_;
    static void Start(const char* ip, int port, int count);
};

#endif // tcp.hh