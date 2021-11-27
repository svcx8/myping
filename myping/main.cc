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
            if (count < 1) {
                logger("It needs 1 count at least.");
                return 0;
            }
            break;
        case 'p':
            port = atoi(optarg);
            if (port < 1 || port > 65535) {
                logger("The valid tcp port is 1~65535");
                return 0;
            }
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