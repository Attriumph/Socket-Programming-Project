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
#include <errno.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <ctype.h>
#include <vector>
#include <set>
#include <map>

#define MYPORT "21233"   // UDP port of serverA
#define MAXBUFLEN 1000
#define PORTAWS "23233"
#define IP_ADD "127.0.0.1"
#define BACKLOG 10

// edge class
class EData {
public:
    int start, end, weight;

public:
    EData() {}

    EData(int s, int e, int w){
        start = s;
        end = e;
        weight = w;
    }

};


// map class
class CityMap {
#define MAX 999
#define INF         (~(0x1<<31))
private:
    class ENode {
    public:
        int vexIndex;
        ENode *nextEdge;
        int weight;
        friend class CityMap;
    };

    class VNode {
    public:
        ENode *firstEdge;
        int data;
        int vex;
        friend class CityMap;
    };

public:
    int mVexNum;  
    double propagate_speed;
    char map_id;
    double transmit_speed;
    int mEdgNum;
    VNode mVexs[20];

public:
    CityMap(int vlen, int elen, std::vector<EData> *edges, std::set<int> vexs, double trasmit_speed,
            double propagate_speed);

    ~CityMap();

    // Dijkstra
    int *path_finding(int vs, int vexs[]);

public:
    void print();

    void linkLast(ENode *list, ENode *node);

    int getIndex(int vex);

    int getPosition(int ch);

    int getWeight(int start, int end);

};

std::map<char, CityMap> maps;

CityMap::CityMap(int vlen, int elen, std::vector<EData> *edges_vector, std::set<int> vexs_set, double ts, double ps) {

    int i;
    int p1;
    int p2;
    int v1;
    int v2;
    int weight;
    ENode *node1;
    ENode *node2;

    std::vector<int> vexs_vector(vexs_set.begin(), vexs_set.end());
    int vexs[vexs_vector.size()];
    std::copy(vexs_vector.begin(), vexs_vector.end(), vexs);
//
//    std::cout << "chu ru vex" << " ";
//    for (auto i = 0; i < vlen; i++) {
//        std::cout << vexs[i] << " ";
//    }

    EData edges[edges_vector->size()];
    std::copy(edges_vector->begin(), edges_vector->end(), edges);

//    std::cout << "chu ru edge" << " ";
//    for (auto i = 0; i < elen; i++) {
//        std::cout << edges[i].start << " ";
//        std::cout << edges[i].end << " ";
//        std::cout << edges[i].weight << " \n";
//    }


    transmit_speed = ts;
    propagate_speed = ps;
    mVexNum = vlen;
    mEdgNum = elen;

    for (i = 0; i < mVexNum; i++) {
        mVexs[i].data = vexs[i];
        mVexs[i].firstEdge = NULL;
    }


    for (i = 0; i < mEdgNum; i++) {
        
        weight = edges[i].weight;
        v1 = edges[i].start;
        v2 = edges[i].end;

        p1 = getPosition(v1);
        p2 = getPosition(v2);

        node1 = new ENode();
        node2 = new ENode();
        node1->vexIndex = p2;
        node2->vexIndex = p1;
        node1->weight = weight;
        node2->weight = weight;

        if (mVexs[p1].firstEdge) {
            linkLast(mVexs[p1].firstEdge, node1);          
        } else {
            mVexs[p1].firstEdge = node1;
        }    

        if (mVexs[p2].firstEdge) {
            linkLast(mVexs[p2].firstEdge, node2);
        } else {
            mVexs[p2].firstEdge = node2;
        }

    }
}

int CityMap::getIndex(int vex) {

    for(int itr = 0; itr < sizeof(mVexs); itr++ ) {
        VNode cur = mVexs[itr];
        if (cur.data == vex) {
            return itr;
        }
    }
    return -1;
}

// Dijkstra
int *CityMap::path_finding(int vs, int prev[]) {
    static int dist[MAX];
    int flag[MAX];      // flag[i]=1 i is index

    int j, k;
    int min, tmp;

    int vs_index = getIndex(vs);
    
    int i = 0;
    while (i < mVexNum ) {
        flag[i] = 0;
        prev[i] = 0;
        dist[i] = getWeight(vs_index, i);
        i++;
    }


    // std::cout << "vs:" << vs << std::endl;

    // std::cout << "initize dist" << " ";
    // for (auto i = 0; i < mVexNum; i++) {
    //     std::cout << dist[i] << " ";
    // }

    flag[vs_index] = 1;
    dist[vs_index] = 0;

    for (i = 1; i < mVexNum; i++) {
        min = INF;
        for (j = 0; j < mVexNum; j++) {
             if (flag[j] == 0) {
                 if (dist[j] < min){
                     min = dist[j];
                     k = j;
                 }
             }
        }
        flag[k] = 1;

        j = 0;
        while( j < mVexNum) {
            tmp = getWeight(k, j);
            if (tmp == INF) {
                tmp = INF;
            } else {
                tmp = min + tmp;
            }
            if (flag[j] == 0 && (tmp < dist[j])) {
                dist[j] = tmp;
                prev[j] = k;
            }
            j++;
        }
    }

    // std::cout << "dist" << " ";
    // for (auto i = 0; i < mVexNum; i++) {
    //     std::cout << dist[i] << " ";
    // }
    return dist;

}


CityMap::~CityMap() {
}

void CityMap::linkLast(ENode *list, ENode *node) {
    ENode *p = list;

    while (p->nextEdge)
        p = p->nextEdge;
    p->nextEdge = node;
}


int CityMap::getPosition(int ch) {
    int i;
    for (i = 0; i < mVexNum; i++)
        if (ch == mVexs[i].data) {
            return i;
        }
    return -1;
}


void CityMap::print() {
    int i, j;
    
    ENode *node;

    for (i = 0; i < mVexNum; i++) {
        std::cout << i << "(" << mVexs[i].data << "): ";
        node = mVexs[i].firstEdge;
        while (node) {
            std::cout << mVexs[node->vexIndex].data << "(" << node->weight << ") ";
            node = node->nextEdge;
        }
        std::cout << std::endl;
    }
}


int CityMap::getWeight(int start, int end) {
    ENode *node;
    
    node = mVexs[start].firstEdge;

    if (start == end) {
        return 0;
    }

    while (node) {
        if (end == node->vexIndex)
            return node->weight;
        node = node->nextEdge;
    }

    return INF;
}


/* Construct the map */
void map_construction() {
    std::ifstream read_map;
    read_map.open("./map.txt");
//    std::cout << read_map;
    std::string line;
    std::string last_line;
    int map_num = 0;
    getline(read_map, line);
//    std::cout << "Debug: read map, line is: " << line << "\n";
    if (line.compare("") == 0) {
        return;
    }

    printf("------------------------------------------\n");
    printf("Map ID    Num Vertices       Num Edges\n");
    printf("------------------------------------------\n");

    do {
        map_num++;

        char map_id = line[0];
        std::vector<EData> *edges = new std::vector<EData>;
        std::set<int> vexs;

        // get the propgate speed
        getline(read_map, line);
//        std::cout <<" propagate speed" << line << std::endl;
        std::stringstream temp1_double(line);
        double propagate_speed;
        temp1_double >> propagate_speed;

        // get the transmit speed
        getline(read_map, line);
//        std::cout <<" transmit speed" << line << std::endl;

        std::stringstream temp2_double(line);
        double trasmit_speed;
        temp2_double >> trasmit_speed;

        getline(read_map, line);

        // read edges
        do {
            std::istringstream edge_info(line);

            int edge_temp[3];
            int i = 0;

            do {
                // Read a start, end and weight
                std::string temp;
                edge_info >> temp;
                std::stringstream temp_int(temp);
                temp_int >> edge_temp[i];
                if (i == 0 || i == 1) {
                    vexs.insert(edge_temp[i]);
                }
                i++;
                // While there is more to read
            } while (edge_info);

            EData edge(edge_temp[0], edge_temp[1], edge_temp[2]);
            edges->push_back(edge);
            last_line = line;
            getline(read_map, line);
//            std::cout << "line is " << line <<std::endl;
        } while (isalpha(line[0]) == 0 && (last_line != line)); // if it is not a letter
//        std::cout << "The elements in set are: ";
//        for (auto it = vexs.begin(); it != vexs.end(); it++)
//            std::cout << *it << " ";

        int vlen = vexs.size();
        int elen = edges->size();
//        std::cout << "The edges  are: ";
//        for (auto it = edges->begin(); it != edges->end(); it++) {
//            std::cout << it->start << " ";
//            std::cout << it->end << " ";
//            std::cout << it->weight << " \n";
//        }

        CityMap cmap(vlen, elen, edges, vexs, trasmit_speed, propagate_speed);

//        cmap.print();
        maps.insert(std::pair<char, CityMap>(map_id, cmap));
        std::cout << map_id << "           " << vlen << "                    " << elen << std::endl;

    } while (isalpha(line[0]) != 0); // if it is  a letter

    printf("------------------------------------------\n");
    std::cout << "The Server A has constructed a list of " << map_num << " maps." << std::endl;

    read_map.close();
}


void callAWS(char *Message = NULL, int numA = 0) {
    struct addrinfo hintsAws, *servinfoAws, *pAws;
    int sockfdAWS, numbytes, rvAWS;

    memset(&hintsAws, 0, sizeof hintsAws);
    hintsAws.ai_family = AF_UNSPEC;
    hintsAws.ai_socktype = SOCK_DGRAM;
    if ((rvAWS = getaddrinfo(IP_ADD, PORTAWS, &hintsAws, &servinfoAws)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rvAWS));
        return;
    }
    // loop through all the results and make a socket
    for (pAws = servinfoAws; pAws != NULL; pAws = pAws->ai_next) {
        if ((sockfdAWS = socket(pAws->ai_family, pAws->ai_socktype,
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
    Message[numA] = '\0';
    if ((numbytes = sendto(sockfdAWS, Message, strlen(Message), 0,
                           pAws->ai_addr, pAws->ai_addrlen)) == -1) {
        perror("talker: send to");
        exit(1);
    }
    printf("The Server A has sent shortest paths to AWS.\n");
//    std::cout << "sent pathRes:" << Message << std::endl;

    freeaddrinfo(servinfoAws);
    close(sockfdAWS);
    return;

}


int string_to_int(std::string str) {
    std::stringstream temp_int(str);
    int x = 0;
    temp_int >> x;
    return x;
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}


int main() {

    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
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
    printf("The server A is up and running using UDP on port 21233.\n");

    // Construct maps
    map_construction();

    // receive query data from aws and do dijstra
    addr_len = sizeof their_addr;
    while (1) {
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                                 (struct sockaddr *) &their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        buf[numbytes] = '\0';

       std::cout << "reveice from aws " << buf << std::endl;
    //    for( int i = 0 ; i < strlen(buf) ; i ++ ){
    //        std::cout << buf[i] << std::endl;
    //    }

        char *token = strtok(buf, " ");

        int position = 0;
        char map_id;
        int source;

        while (token != NULL) {
         
            if (position == 0) {
                map_id = *(token + 1);
                // std::cout << "token is: " << token  <<std::endl;
                // std::cout << "token + 1:" << *(token + 1) <<std::endl;
                // std::cout << "token + 0:" << *(token + 0) <<std::endl;
            } else if (position == 1) {
                source = atoi(token);
            }
            position++;
            token = strtok(NULL, " ");
        }

        
        std::cout << "The Server A has received input for finding shortest paths: starting vertex " << source << " of map " << map_id << " ." << std::endl;


        std::map<char, CityMap>::iterator p;

        // find value given key and run dijstra
        p = maps.find(map_id);
        int *dist;
        char pathRes[1000] = "";
        if (p != maps.end()) {
            CityMap curMap = p->second;
            int prev[curMap.mVexNum];
            dist = curMap.path_finding(source, prev);

        //    std::cout << "dist" << " ";
        //    for (auto i = 0; i < curMap.mVexNum; i++) {
        //        std::cout << dist[i] << " ";
        //    }

            char ts_char[100], ps_char[100];
            sprintf(ps_char, "%.2f", curMap.propagate_speed);
            sprintf(ts_char, "%.2f", curMap.transmit_speed);
            strcpy(pathRes, ps_char);
            strcat(pathRes, "#");
            strcat(pathRes, ts_char);
            strcat(pathRes, "#");
//            std::cout << "1st pathRes:" << pathRes << std::endl;

            printf("The Server A has identified the following shortest paths:\n");
            printf("------------------------------------------\n");
            printf("Destination         Min Length\n");
            printf("------------------------------------------\n");

            for (int i = 0; i < curMap.mVexNum; i++) {
                if (dist[i] == 0) {
                    continue;
                }
                printf("%d                   ", curMap.mVexs[i].data);
                printf(" %d\n", dist[i]);

                char vex_char[100], distance_char[100];

                sprintf(vex_char, "%d", curMap.mVexs[i].data);
                sprintf(distance_char, "%d", *(dist + i));


                strcat(pathRes, vex_char);
                strcat(pathRes, " ");
                strcat(pathRes, distance_char);
                strcat(pathRes, "#");

//                std::cout << "pathRes:" << pathRes << std::endl;

            }
            printf("------------------------------------------\n");

            callAWS(pathRes, strlen(pathRes));
        }


    }

    close(sockfd);
    return 0;
}

