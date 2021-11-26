#include "ping/tcp.hh"

#include <cstring>

#include "misc/getopt.h"
#include "misc/logger.hh"

void help() {
    logger("Usage: myping [ip] [-c count] [-p port]\n\tmyping [-c count] [-p port] [ip]");
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    int opt = 0;
    int count = 10;
    int port = 80;
    bool ip_before_options = false;

PARSEOPTION:
    while ((opt = getopt(argc, argv, "c:p:")) != EOF) {
        switch (opt) {
        case 'c':
            count = atoi(optarg);
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case '?':
            logger("Unknow option: %c", optopt);
            break;
        default:
            help();
        }
    }

    if (optind == 1 && argc > 2) {
        optind = 2;
        ip_before_options = true;
        goto PARSEOPTION;
    }

    if (ip_before_options) {
        argv++;
    } else {
        argc -= optind;
        argv += optind;
    }

    if (argc > 0) {
        PingConn::Start(*argv, port, count);
    }

    else
        help();
}