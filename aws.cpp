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
#include <iostream>
#include <sstream>
#include <vector>

#define PORTA "21233"
#define PORTB "22233"
#define UDP_PORT "23233"
#define IP_ADD "127.0.0.1"
#define TCP_PORT "24233"
#define PORT 24233
#define BACKLOG 10
#define MAXBUFLEN 1000

void callServerB(char *Message = NULL, int numB = 0) {

    int sockfdB, new_fdB;
    struct addrinfo hintsB, *servinfoB, *pB;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN], recvbuff[999];
    char buf[999];
    char Tempbuff[999], buffer_test[999] = "";
    int rv, num, rvB, numbytes1;

    memset(&hintsB, 0, sizeof hintsB);
    hintsB.ai_family = AF_UNSPEC;
    hintsB.ai_socktype = SOCK_DGRAM;
    if ((rvB = getaddrinfo(IP_ADD, PORTB, &hintsB, &servinfoB)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rvB));
        return;
    }
    // loop through all the results and make a socket
    for (pB = servinfoB; pB != NULL; pB = pB->ai_next) {
        if ((sockfdB = socket(pB->ai_family, pB->ai_socktype,
                              pB->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }
        break;
    }
    if (pB == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return;
    }


    Message[numB] = '\0';

//    std::cout << "sent to B " << std::endl;
//    for (int i = 0; i < strlen(Message); i++) {
//        std::cout << Message[i];
//    }
    // printf("talker: Buffer_test  is %s\n",  buffer_test);
    if ((numbytes1 = sendto(sockfdB, Message, strlen(Message), 0,
                            pB->ai_addr, pB->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }

    freeaddrinfo(servinfoB);
    // printf("talker: sent %d bytes to %s\n", numbytes1, IP_ADD);
    close(sockfdB);
    return;

}

void callServerA(char *Message = NULL, int numA = 0) {
    int sockfdA, new_fdA;
    struct addrinfo hintsA, *servinfoA, *pA;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN], recvbuff[999];
    char buf[999];
    char Tempbuff[999];
    int rv, num, rvA, numbytes;

    memset(&hintsA, 0, sizeof hintsA);
    hintsA.ai_family = AF_UNSPEC;
    hintsA.ai_socktype = SOCK_DGRAM;
    if ((rvA = getaddrinfo(IP_ADD, PORTA, &hintsA, &servinfoA)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rvA));
        return;
    }
    // loop through all the results and make a socket
    for (pA = servinfoA; pA != NULL; pA = pA->ai_next) {
        if ((sockfdA = socket(pA->ai_family, pA->ai_socktype,
                              pA->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }
        break;
    }
    if (pA == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return;
    }
    Message[numA] = '\0';
    if ((numbytes = sendto(sockfdA, Message, strlen(Message), 0,
                           pA->ai_addr, pA->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }
    freeaddrinfo(servinfoA);
//    std::cout << "send to A" << Message << std::endl;

    close(sockfdA);
    return;

}

void sigchld_handler(int s) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}


void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

int string_to_int(std::string str) {
    std::stringstream temp_int(str);

    int x = 0;
    temp_int >> x;
    return x;
}


int main() {
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN], buffClient[999];
    int rv, num, rvA, numbytes;


    int sockfdAB;
    struct addrinfo hintsAB, *servinfoAB, *pAB;
    int rvAB;
    int numbytesAB;
    struct sockaddr_storage their_addrAB;
    socklen_t addr_lenAB;
    char sAB[INET6_ADDRSTRLEN];


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, TCP_PORT, &hints, &servinfo)) != 0) {

        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Server: bind");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    // TCP needs listen
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    printf("AWS is up and running\n");

    // start AWS UDP port
    memset(&hintsAB, 0, sizeof hintsAB);
    hintsAB.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hintsAB.ai_socktype = SOCK_DGRAM;
    hintsAB.ai_flags = AI_PASSIVE; // use my IP
    if ((rvAB = getaddrinfo(NULL, UDP_PORT, &hintsAB, &servinfoAB)) != 0) {

        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rvAB));
        return 0;
    }

    for (pAB = servinfoAB; pAB != NULL; pAB = pAB->ai_next) {
        if ((sockfdAB = socket(pAB->ai_family, pAB->ai_socktype,
                               pAB->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }
        if (bind(sockfdAB, pAB->ai_addr, pAB->ai_addrlen) == -1) {
            close(sockfdAB);
            perror("listener: bind");
            continue;
        }
        break;
    }
    if (pAB == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 0;
    }
    freeaddrinfo(servinfoAB);


    // listen or receive
    while (1) {
        sin_size = sizeof their_addr;
        char bufPath[MAXBUFLEN] = "";

        // TCP create child socket
        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            exit(1);
        }
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);


        if ((num = recv(new_fd, buffClient, 999, 0)) == -1) {
            perror("fail recv\n");
            exit(1);
        } else {
            buffClient[num] = '\0';
            printf("Debug:The AWS received buff from client %s \n", buffClient);
            char buffCopy[sizeof(buffClient)];
            strcpy(buffCopy, buffClient);

            char *query;
            query = strtok(buffClient, " ");
            int k = 0;
            char map_id;
            int sv;
            double file_size;

            while (query != NULL) {
                if (k == 0) {
                    map_id = *(query + 1);
                } else if (k == 1) {
                    sv = atoi(query);
                } else if (k == 2) {
                    file_size = atof(query);
                }
                query = strtok(NULL, " ");
                k++;
            }

            printf("The AWS has received map ID %c, start vertex %d and file size %.2f from the client using TCP over port 24233\n",
                   map_id, sv, file_size);

//            printf("Debug:The AWS received buff from client %s \n", buffCopy);

            callServerA(buffCopy, num); // send recvbuff info to server A
            printf("The AWS has sent map ID and starting vertex to server A using UDP over port 23233\n");
            addr_lenAB = sizeof their_addrAB;

            // aws receive shortest path found in server A
            if ((numbytesAB = recvfrom(sockfdAB, bufPath, MAXBUFLEN - 1, 0,
                                       (struct sockaddr *) &their_addrAB, &addr_lenAB)) == -1) {
                perror("recvfrom");
            }

            bufPath[numbytesAB] = '\0';

//            std::cout << "reveice from ServerA " << std::endl;
//            for( int i = 0 ; i < strlen(bufAB) ; i ++ ){
//                std::cout << bufAB[i];
//            }
//            std::cout << std::endl;
            printf("The AWS has received shortest path from server A:\n");

            char bufFrB[MAXBUFLEN] = "";
            char bufToB[sizeof(bufPath)];
            strcpy(bufToB, bufPath);

            // extra shortest paths and file size needed by server B
            int i = 0;
            double ts, ps;
            const char delim = '\0';
            char pathRes[1000] = "";
            char *line;

            line = strtok(bufPath, "#");

            while (line) {

                if (i == 0) {
                    ps = atof(line);
                } else if (i == 1) {
                    ts = atof(line);

                    printf("The AWS has received shortest path from server A:\n");
                    printf("-----------------------------\n");
                    printf("Destination    Min Length\n");
                    printf("-----------------------------\n");
                } else {
                    int vex, length;

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


                    printf(" %d \t \t %d\n", vex, length);

                }
                line = strtok(NULL, "#");
                i++;

            }
            printf("-----------------------------\n");

            char filesize_char[100];
            sprintf(filesize_char, "%.2f", file_size);

            strcat(bufToB, "*");
            strcat(bufToB, filesize_char);
            strcat(bufToB, "\n");

            callServerB(bufToB, sizeof(bufToB));
            printf("The AWS has sent path length, propagation speed and transmission speed to server B using UDP over port 23233\n");

            addr_lenAB = sizeof their_addrAB;

            if ((numbytesAB = recvfrom(sockfdAB, bufFrB, MAXBUFLEN - 1, 0,
                                       (struct sockaddr *) &their_addrAB, &addr_lenAB)) == -1) {
                perror("recvfrom");
            }
            bufFrB[numbytesAB] = '\0';

            // aws receive delays of shortest path
            printf("The AWS has received delays from server B:\n");
            printf("--------------------------------------------\n");
            printf("Destination      Tt         Tp         Delay\n");
            printf("--------------------------------------------\n");

            char newbufAB2[sizeof(bufFrB)];
            strcpy(newbufAB2, bufFrB);

            std::vector<char *> v;
            char *chars_array = strtok(newbufAB2, "\n");
            while (chars_array) {
                v.push_back(chars_array);
                chars_array = strtok(NULL, "\n");
            }

            for (size_t n = 0; n < v.size(); ++n) {
                char *delay_line = strtok(v[n], " ");
                int m = 0;
                int vex, length;
                double tt, tp, delay;
                while (delay_line) {
                    if (m == 0) {
                        vex = atoi(delay_line);
                    } else if (m == 1) {
                        length = atoi(delay_line);
                    } else if (m == 2) {
                        tt = atof(delay_line);
                    } else if (m == 3) {
                        tp = atof(delay_line);
                    } else if (m == 4) {
                        delay = atof(delay_line);
                    }
                    m++;

                    delay_line = strtok(NULL, " ");
                }
                printf("%d             %.2f        %.2f        %.2f \n", vex, tt, tp, delay);

            }

            printf("--------------------------------------------\n");


            send(new_fd, bufFrB, numbytesAB, 0);
            printf("The AWS has sent calculated delay to client using TCP over port 24233.\n");

        }


    }

    close(sockfdAB);
    close(new_fd);
    close(sockfd);
    return 0;
}

