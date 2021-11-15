#include "connection/tcp.hh"

#include <cstring>

#include <misc/getopt.h>

void help() {
    printf("Usage: myping [ip] [-c count] [-p port]\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    int opt = 0;
    int count = 10;
    int port = 80;
    while ((opt = getopt(argc, argv, "c:p:")) != EOF) {
        switch (opt) {
        case 'c':
            count = atoi(optarg);
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case '?':
            printf("Unknow option: %c\n", optopt);
            break;
        default:
            help();
        }
    }

    argc -= optind;
    argv += optind;

    if (argc > 0)
        PingConn::Start(*argv, port, count);
    else
        help();
}