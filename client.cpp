#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define IP_Add "127.0.0.1"
#define PORT "24233" // AWS TCP port
#define BACKLOG 10


void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}


int main(int argc, char *argv[]) {
    // code from beej

    int sockfd, numbytes;
    char buf[1000];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(IP_Add, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s", gai_strerror(rv));
        return 1;
    }
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr), s, sizeof s);
    freeaddrinfo(servinfo);


    while (1) {
        char sendBuffer[1000];

        if (argc != 4) {
            fprintf(stderr, "usage: ./client <Map ID> <Source Vertex Index> <File Size>\n");
            exit(1);
        }
        printf("The client is up and running.\n");
        printf("The client has sent query to AWS using TCP over port 24233: start vertex %s; map %s; file size %s.\n",
               argv[2], argv[1], argv[3]);

        for (int i = 1; i < argc; ++i) {
            strcat(sendBuffer, argv[i]);
            strcat(sendBuffer, " ");
        }


        if ((send(sockfd, sendBuffer, strlen(sendBuffer), 0)) == -1) {
            fprintf(stderr, "Fail to send");
            close(sockfd);
            exit(1);
        } else {


            if ((numbytes = recv(sockfd, buf, 999, 0)) == -1) {
                perror("recv");
                exit(1);
            }
            buf[numbytes] = '\0';
            printf("The client has received results from AWS: \n");
            printf("----------------------------------------------------\n");
            printf("Destination  Min Length    Tt       Tp      Delay\n");
            printf("----------------------------------------------------\n");

            char *line;
            line = strtok(buf, "\n");
            while (line != NULL) {
                printf("%s\n", line);
                line = strtok(NULL, "\n");
            }
            printf("-----------------------------------------------------\n");
            exit(1);
        }
    }

    close(sockfd);
    return 0;
}

