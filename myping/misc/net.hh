#ifndef NET_HEADER
#define NET_HEADER

#include "myexception.hh"

// https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code
// https://docs.microsoft.com/en-us/windows/win32/winsock/complete-client-code
// _WIN32(WSAStartup())-->socket()——>bind()——>listen()——>accpet()——>recv() or send()——>closesocket()->WSACleanup()

#include "logger.hh"
#ifdef __unix__
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>

#define SleepSec(sec) sleep(sec)
#define SOCKET_ERROR (-1)
#define CloseSocket close

#else
#include <winsock.h>

#define SleepSec(sec) Sleep(sec * 1000)
#define CloseSocket closesocket
#endif

class NetException : public BaseException {
public:
#if defined WIN32
    char buffer_[128];
#endif
    NetException(const char* func, const char* file, int line) : BaseException(func, file, line) {
#if defined WIN32
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, WSAGetLastError(),
                       MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                       (LPSTR)&buffer_, 128, NULL);
        result_ = buffer_;
#endif

#if defined __unix__
        result_ = strerror(errno);
#endif
    }
};

#define NetEx() NetException(__FUNCTION__, __FILE__, __LINE__)

#if defined WIN32
inline void MyWinSocketSetup() {
    WSADATA wsaData;
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw NetEx();
    }
}
#endif

#endif // net.hpp