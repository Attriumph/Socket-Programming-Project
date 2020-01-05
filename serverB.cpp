#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <map>


#define MYPORT "22233"   // the port users will be connecting to
#define PORTAWS "23233"
#define MAXBUFLEN 1000
#define IP_ADD "127.0.0.1"
#define BACKLOG 10

int string_to_int(std::string str) {
    std::stringstream temp_int(str);
    int x = 0;
    temp_int >> x;
    return x;
}


void callAWS(char *message = NULL, int numA = 0) {
    int sockfdAws;
    struct addrinfo hintsAws, *servinfoAws, *pAws;
    int rvAws, numbytes;

    memset(&hintsAws, 0, sizeof hintsAws);
    hintsAws.ai_family = AF_UNSPEC;
    hintsAws.ai_socktype = SOCK_DGRAM;
    if ((rvAws = getaddrinfo(IP_ADD, PORTAWS, &hintsAws, &servinfoAws)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rvAws));
        return;
    }
    // loop through all the results and make a socket
    for (pAws = servinfoAws; pAws != NULL; pAws = pAws->ai_next) {
        if ((sockfdAws = socket(pAws->ai_family, pAws->ai_socktype,
                                pAws->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }
        break;
    }
    if (pAws == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return;
    }
    message[numA] = '\0';
    if ((numbytes = sendto(sockfdAws, message, strlen(message), 0,
                           pAws->ai_addr, pAws->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }

//    std::cout << "server B send back to aws \n" << message << std::endl;
    freeaddrinfo(servinfoAws);
    close(sockfdAws);
    return;

}


void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

int main(void) {
    int sockfd, rv;
    int numbytes = 0;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;

    char buf[MAXBUFLEN];
    socklen_t addr_len;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }
    freeaddrinfo(servinfo);
    printf("The serverB is up and running over UDP port 22233.\n");


    addr_len = sizeof their_addr;
    while (1) {
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                                 (struct sockaddr *) &their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        //printf("listener: got packet from %s\n",
        //  inet_ntop(their_addr.ss_family,
        //      get_in_addr((struct sockaddr *)&their_addr),
        //     s, sizeof s));
        printf("The Server B has received data for calculation:\n");

        buf[numbytes] = '\0';

//        std::cout << "reveice from AWS \n" << std::endl;
//        for( int i = 0 ; i < strlen(buf) ; i ++ ){
//            std::cout << buf[i];
//        }
//        std::cout << std::endl;

        int i = 0;
        double ts, ps, file_size;
        const char delim = '\0';
        char delayRes[1000] = "";
        std::map<int, double> entry;

        std::vector<char *> v;
        char *chars_array = strtok(buf, "*");
        while (chars_array) {
            v.push_back(chars_array);
            chars_array = strtok(NULL, "*");
        }

        // file size
        file_size = atof(v[1]);

        char *line;
        line = strtok(v[0], "#");

        while (line != NULL) {
            if (i == 0) {
                ps = atof(line);
            } else if (i == 1) {
                ts = atof(line);
                printf("* Propagation speed: %.2f km/s;\n", ps);
                printf("* Transmission speed %.2f Bytes/s;\n", ts);
            } else {
                int vex, length;
                double tt, tp, delay;
                char copy_line[sizeof(line)];
                strcpy(copy_line, line);

                int j = 0;
                std::istringstream split_string(copy_line);

                do {
                    std::string in_line;
                    split_string >> in_line;

                    if (j == 0) {
                        vex = string_to_int(in_line);
                    } else if (j == 1) {
                        length = string_to_int(in_line);
                    }
                    j++;

                    // While there is more to read
                } while (split_string);


                printf("* Path length for destination %d: %d\n", vex, length);

                tt = file_size / (ts * 8);
                tp = length / ps;
//                delay = tt + tp;
//                        printf( "length :%d\n", length);
//                        printf("tt: %.4f\n", tt);
//                        printf("tp:%.4f\n", tp);

                char vex_char[100], length_char[100], delay_char[100], tt_char[100], tp_char[100];
                sprintf(vex_char, "%d", vex);
                sprintf(length_char, "%d", length);
                sprintf(tt_char, "%.2f", tt);
                sprintf(tp_char, "%.2f", tp);

                delay = atof(tt_char) + atof(tp_char);
                entry.insert(std::pair<int, double>(vex, delay));;

                sprintf(delay_char, "%.2f", delay);

                strcat(delayRes, vex_char);
                strcat(delayRes, "                 ");
                strcat(delayRes, length_char);
                strcat(delayRes, "      ");
                strcat(delayRes, tt_char);
                strcat(delayRes, "      ");
                strcat(delayRes, tp_char);
                strcat(delayRes, "      ");
                strcat(delayRes, delay_char);
                strcat(delayRes, "\n");
            }
            line = strtok(NULL, "#");
            i++;
        }

        printf("The Server B has finished the calculation of the delays:\n");
        printf("------------------------\n");
        printf("Destination      Delay\n");
        printf("------------------------\n");
        std::map<int, double>::iterator itr;
        for (itr = entry.begin(); itr != entry.end(); ++itr) {
            std::cout << '\t' << itr->first
                      << "\t " << itr->second << '\n';
        }
        std::cout << "";
        printf("------------------------\n");

        callAWS(delayRes, sizeof(delayRes));
        printf("The Server B has finished sending the output to AWS.\n");
    }
    close(sockfd);
    return 0;
}

