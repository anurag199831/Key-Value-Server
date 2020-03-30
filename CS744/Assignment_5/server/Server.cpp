#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <set>

#include "KVClientHandler.cpp"
#include "sha1.cpp"

using namespace std;

class Server {
   private:
    struct entry {
        int port;
        size_t nodeId;
    };
    const int BACK_LOG = 5;
    const int MAX_BUFF = 1024;
    const int MAX_EVENTS = 20;
    const string pathToDatabase = "database/";
    const size_t port;
    const size_t threadPoolSize;
    const size_t numSetsInCache;
    const size_t sizeOfSet;
    int serverFd;
    int epoll_fd;
    KVClientHandler handler;
    KVServerResponseFormatter formatter;
    map<size_t, entry> fingerTable;
    stringstream ss;

    // Constants specific to CHORD
    const string FIND_SUCC = "find_succ";
    const string FIND_PRED = "find_pred";
    const string FIND_FINGER = "find_finger";
    const string SET_PRED = "set_pred";
    const string SET_SUCC = "set_succ";
    const string TELL_SUCC = "tell_succ";
    const string TELL_PRED = "tell_pred";

    const size_t HASH_SIZE = 16;
    int predecessor_port;
    size_t predecessor_nodeId;
    size_t nodeId;

    string find_succ(int id, int source_port) {
        int nodesucc = fingerTable.at(0).nodeId;
        int nodesucc_port = fingerTable.at(0).port;
        if (id > nodeId && id <= nodesucc)
            return formatter.makeContactResponse(
                source_port, FIND_SUCC, to_string(id), to_string(nodesucc_port),
                to_string(nodesucc));
        else if (id > nodeId && nodesucc < nodeId)
            return formatter.makeContactResponse(
                source_port, FIND_SUCC, to_string(id), to_string(nodesucc_port),
                to_string(nodesucc));

        size_t index = closest_prec_node(nodeId, id);
        cout << "find_succ: closest_predceding_node: " << index << endl;
        size_t nnnode = fingerTable.at(index).nodeId;
        int nnport = fingerTable.at(index).port;
        cout << "find_succ: nnport: " << nnport
             << " source_port: " << source_port << endl;
        if (nnport == source_port) {
            return formatter.makeContactResponse(source_port, FIND_SUCC,
                                                 to_string(id), to_string(port),
                                                 to_string(nodeId));
        } else if (nnport == port) {
            return formatter.makeContactResponse(source_port, FIND_SUCC,
                                                 to_string(id), to_string(port),
                                                 to_string(nodeId));
        }

        int sock = 0, n;
        struct sockaddr_in serv_addr;
        char buffer[MAX_BUFF] = {0};
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
            return "";
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(nnport);
        serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
            0) {
            cout << "find_succ: Connection Failed " << port << "->" << nnport
                 << endl;
            return "";
        }
        string sendxml =
            formatter.makeContactMessage(source_port, FIND_SUCC, to_string(id));
        n = write(sock, sendxml.c_str(), sendxml.length());

        n = read(sock, buffer, MAX_BUFF);
        string response = string(buffer);
        return response;
    }

    string find_pred(const size_t id, const int source_port) {
        // cout << "FIND_PRED FUNC: Called" << endl;
        // int nodesuc = predecessor_nodeId;
        // if ((id > nodeId and id <= nodesuc) or
        //     (id > nodeId and nodesuc < nodeId)) {
        // }
        // return formatter.makeContactResponse(source_port, FIND_PRED,
        //                                      to_string(id), to_string(port),
        //                                      to_string(nodeId));

        // return formatter.makeContactResponse(source_port, FIND_PRED,
        //                                      to_string(id), to_string(port),
        //                                      to_string(nodeId));

        // size_t index = closest_prec_node(nodeId, id);
        // cout << index << endl;
        // if (fingerTable.at(index).port == port) {
        //     return formatter.makeContactResponse(source_port, FIND_PRED,
        //                                          to_string(id),
        //                                          to_string(port),
        //                                          to_string(nodeId));
        // }
        // size_t nnnode = fingerTable.at(index).nodeId;
        // int nnport = fingerTable.at(index).port;

        string xml = find_succ(id, source_port);
        auto m = formatter.parseContactResponse(xml);
        int resp_port = stoi(m.at("resp_port"));
        cout << "find_pred: resp_port: " << resp_port << endl;
        if (resp_port == port) {
            return formatter.makeContactResponse(
                source_port, FIND_PRED, to_string(id),
                to_string(predecessor_port), to_string(predecessor_nodeId));
        }

        int sock = 0;
        int n;
        struct sockaddr_in serv_addr;

        char buffer[MAX_BUFF] = {0};
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
            return "";
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(resp_port);
        serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
            0) {
            cout << "find_pred: Connection Failed " << port << "->" << resp_port
                 << endl;
            return "";
        }
        string sendxml =
            formatter.makeContactMessage(source_port, TELL_PRED, to_string(id));
        n = write(sock, sendxml.c_str(), sendxml.length());

        n = read(sock, buffer, MAX_BUFF);
        string response = string(buffer);
        // cout << "FIND_PRED FUNC: Returning" << endl;
        return response;
        // auto m = formatter.parseContactResponse(response);
        // return formatter.makeContactResponse(source_port, FIND_PRED,
        //                                      to_string(id), m.at("port"),
        //                                      m.at("nodeId"));
    }

    long long int __pow(long long int a, long long int b) {
        long long int q = 1;
        for (long long int i = 0; i <= b - 1; i++) {
            q = q * a;
        }

        return q;
    }

    int closest_prec_node(int node, int id) {
        // cout << "closest_prec_node: called" << endl;
        for (int i = HASH_SIZE - 1; i >= 0; i--) {
            // cout << "Looping with " << i << endl;
            if ((fingerTable.at(i).nodeId > node and
                 fingerTable.at(i).nodeId <= id) or
                (id < node and fingerTable.at(i).nodeId <= id) or
                (id < node and fingerTable.at(i).nodeId > node)) {
                // cout << "closest_prec_node: returning" << endl;
                return i;
            }
        }
        // cout << "closest_prec_node: returning with ERROR" << endl;
        return 0;
    }

    void updateFingers() {
        int next = 1;
        while (next < 16) {
            string response = find_succ(
                (nodeId + __pow(2, next) - 1) % __pow(2, HASH_SIZE), port);
            auto m = formatter.parseContactResponse(response);
            updateFingerInTable(next, stoi(m.at("resp_port")),
                                stoi(m.at("resp_nodeId")));
            next++;
        }
    }

    void makeContact(const size_t contact_port) {
        struct sockaddr_in addr = {0};
        int n, sockfd;
        char buffer[MAX_BUFF];

        /* Create socket and connect to contact server */
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            cerr << "Network Error: Could not create socket for contact\n";
            exit(EXIT_FAILURE);
        }
        addr.sin_family = AF_INET;
        addr.sin_port = htons(contact_port);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
            cout << "Connected to contact server\n";
        } else {
            cerr << "Network Error: Could not connect\n";
            exit(EXIT_FAILURE);
        }

        string resp, msg;

        // FIND_PRED
        msg = formatter.makeContactMessage(this->port, FIND_PRED,
                                           to_string(nodeId));
        n = write(sockfd, msg.c_str(), msg.length());
        cout << "FIND_PRED sent to contact server" << endl;
        n = read(sockfd, buffer, MAX_BUFF);
        resp = string(buffer);
        auto m = formatter.parseContactResponse(resp);
        predecessor_port = stoi(m.at("resp_port"));
        predecessor_nodeId = stoi(m.at("resp_nodeId"));
        cout << "FIND_PRED returned with nodeId " << predecessor_nodeId
             << "and port " << predecessor_port << endl;
        close(sockfd);

        /* Create socket and connect to contact server */
        memset(&addr, 0, sizeof addr);
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            cerr << "Network Error: Could not create socket for contact\n";
            exit(EXIT_FAILURE);
        }
        addr.sin_family = AF_INET;
        addr.sin_port = htons(predecessor_port);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
            cout << "Connected to contact server\n";
        } else {
            cerr << "Network Error: Could not connect\n";
            exit(EXIT_FAILURE);
        }
        // TELL SUCC
        msg = formatter.makeContactMessage(this->port, TELL_SUCC,
                                           to_string(nodeId));
        n = write(sockfd, msg.c_str(), msg.length());
        cout << "TELL_SUCC sent to contact server" << endl;
        n = read(sockfd, buffer, MAX_BUFF);
        resp = string(buffer);
        m = formatter.parseContactResponse(resp);
        size_t successor_port = stoi(m.at("resp_port"));
        size_t successor_nodeId = stoi(m.at("resp_nodeId"));
        cout << "TELL_SUCC returned with nodeId " << successor_nodeId
             << "and port " << successor_port << endl;
        // SET SUCC
        msg = formatter.makeContactMessage(this->port, SET_SUCC,
                                           to_string(nodeId));
        n = write(sockfd, msg.c_str(), msg.length());
        cout << "SET_SUCC called " << endl;
        close(sockfd);
        updateFingerInTable(0, successor_port, successor_nodeId);

        // Connecting to successor
        memset(&addr, 0, sizeof addr);
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            cerr << "Network Error: Could not create socket for contact\n";
            exit(EXIT_FAILURE);
        }
        addr.sin_family = AF_INET;
        addr.sin_port = htons(successor_port);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
            cout << "Connected to contact server\n";
        } else {
            cerr << "Network Error: Could not connect\n";
            exit(EXIT_FAILURE);
        }
        // SET PRED
        msg = formatter.makeContactMessage(this->port, SET_PRED,
                                           to_string(nodeId));
        n = write(sockfd, msg.c_str(), msg.length());
        cout << "SET PRED packet sent" << endl;
        close(sockfd);
    }

    void printFingerTable() {
        cout << "Index\tnodeId\tport\n";
        for (size_t i = 0; i < fingerTable.size(); i++) {
            cout << i << "\t" << fingerTable.at(i).nodeId << "\t"
                 << fingerTable.at(i).port << "\n";
        }
        cout << "Predecessor: port=" << predecessor_port
             << " nodeId=" << predecessor_nodeId << endl;
    }

    void updateFingerInTable(size_t index, int port, int nodeId) {
        fingerTable.at(index).port = port;
        fingerTable.at(index).nodeId = nodeId;
        printFingerTable();
    }

   public:
    Server(const size_t port)
        : port(port),
          threadPoolSize(64),
          numSetsInCache(32),
          sizeOfSet(64),
          handler(64, 32, 64) {
        ss << std::hex << sha1(to_string(port)).substr(0, 4);
        ss >> nodeId;
        cout << "Server started with node Id: " << nodeId << endl;

        predecessor_port = port;
        predecessor_nodeId = nodeId;
        for (size_t i = 0; i < HASH_SIZE; i++) {
            entry e;
            e.port = this->port;
            e.nodeId = this->nodeId;
            fingerTable[i] = e;
        }
        printFingerTable();
    }

    ~Server() {
        close(epoll_fd);
        close(serverFd);
    }

    void fixFingers() {
        updateFingers();
        int successor_port = fingerTable.at(0).port;

        struct sockaddr_in addr = {0};
        int n, sockfd, num = 1;

        /* Create socket and connect to server */
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            cerr << "Network Error: Could not create socket\n";
            exit(EXIT_FAILURE);
        }
        addr.sin_family = AF_INET;
        addr.sin_port = htons(successor_port);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
            // cout << "Connected to server\n";
        } else {
            cerr << "Network Error: Could not connect\n";
            exit(EXIT_FAILURE);
        }
        string s =
            formatter.makeContactMessage(port, FIND_FINGER, to_string(nodeId));
        write(sockfd, s.c_str(), s.length());
        close(sockfd);
    }

    void init(const size_t contact_port) {
        struct stat info;
        if (stat(pathToDatabase.c_str(), &info) != 0) {
            cout << "Creating directory for KVStore\n";
            if (mkdir(pathToDatabase.c_str(), 0777) == -1) {
                cerr << "Error Creating directory for KVStore\n";
            } else {
                cout << "Directory created for KVStore\n";
            }
        } else if (info.st_mode & S_IFDIR) {
            cout << "Directory for KVStore already exists\n";
        }

        if (contact_port != 0) {
            makeContact(contact_port);
            fixFingers();
        }

        struct sockaddr_in addr;
        serverFd = socket(AF_INET, SOCK_STREAM, 0);
        if (serverFd == -1) {
            cerr << "Error getting a socket" << endl;
            exit(EXIT_FAILURE);
        }
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(serverFd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
            // cout << "Binding successful" << endl;
        } else {
            cerr << "Binding failed" << endl;
            exit(EXIT_FAILURE);
        }
        if (listen(serverFd, BACK_LOG) == 0) {
            // cout << "Listening on port: " << port << endl;
        } else {
            cerr << "Listening failed" << endl;
        }
        /* set O_NONBLOCK on fd */
        long flags = fcntl(serverFd, F_GETFL, 0);
        fcntl(serverFd, F_SETFL, flags | O_NONBLOCK);
    }

    void start() {
        struct sockaddr client;
        int addrlen = 0;
        epoll_fd = epoll_create1(0);
        if (epoll_fd == -1) {
            cerr << "Error creating epoll instance" << endl;
            exit(EXIT_FAILURE);
        }
        struct epoll_event events[MAX_EVENTS];
        int sockfd;
        int nfds = 0;
        char buffer[MAX_BUFF];
        int n = 0;
        set<int> doneNfds;
        pair<int, string> output;
        bool orphanFlag = true;
        map<int, int> points;
        while (true) {
            memset(&client, 0, sizeof(client));
            addrlen = sizeof(client);
            sockfd = accept(serverFd, (struct sockaddr *)&client,
                            (socklen_t *)&addrlen);
            if (sockfd != -1) {
                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = sockfd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ev.data.fd, &ev) == -1) {
                    cerr << "Failed to add file descriptor to epoll\n";
                    close(epoll_fd);
                    exit(EXIT_FAILURE);
                }
            }
            do {
                nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);
            } while (errno == EINTR);
            cout << "nfds: " << nfds << endl;
            int i = 0;
            while (i < nfds) {
                orphanFlag = true;
                memset(buffer, 0, MAX_BUFF);
                n = read(events[i].data.fd, buffer, MAX_BUFF);
                string s = string(buffer);
                cout << s << endl;
                if (s.length() > 0 and s.substr(0, 16) == "<ContactRequest>") {
                    auto m = formatter.parseContactRequest(s);
                    if (m.at("op_type") == TELL_SUCC) {
                        s = formatter.makeContactResponse(
                            stoi(m.at("source")), m.at("op_type"),
                            m.at("nodeId"), to_string(fingerTable.at(0).port),
                            to_string(fingerTable.at(0).nodeId));
                        write(events[i].data.fd, s.c_str(), s.length());
                    } else if (m.at("op_type") == TELL_PRED) {
                        s = formatter.makeContactResponse(
                            stoi(m.at("source")), m.at("op_type"),
                            m.at("nodeId"), to_string(predecessor_port),
                            to_string(predecessor_nodeId));
                        write(events[i].data.fd, s.c_str(), s.length());
                    } else if (m.at("op_type") == SET_SUCC) {
                        updateFingerInTable(0, stoi(m.at("source")),
                                            stoi(m.at("nodeId")));
                        printFingerTable();
                    } else if (m.at("op_type") == SET_PRED) {
                        // cout << "SET_PRED Called" << endl;
                        predecessor_port = stoi(m.at("source"));
                        predecessor_nodeId = stoi(m.at("nodeId"));
                        printFingerTable();
                    } else if (m.at("op_type") == FIND_SUCC) {
                        string resp = find_succ(stoi(m.at("nodeId")),
                                                stoi(m.at("source")));
                        write(events[i].data.fd, resp.c_str(), resp.length());
                    } else if (m.at("op_type") == FIND_PRED) {
                        // cout << "FIND_PRED: Called" << endl;
                        size_t nodeId = stoi(m.at("nodeId"));
                        int source = stoi(m.at("source"));
                        string resp = find_pred(nodeId, source);
                        // cout << "FIND_PRED : Returned" << endl;
                        write(events[i].data.fd, resp.c_str(), resp.length());
                    } else if (m.at("op_type") == FIND_FINGER) {
                        cout << "updateFinger: called" << endl;
                        if (stoi(m.at("source")) != port) {
                            updateFingers();
                            int successor_port = fingerTable.at(0).port;
                            struct sockaddr_in addr = {0};
                            int n, sockfd, num = 1;

                            /* Create socket and connect to server */
                            sockfd = socket(AF_INET, SOCK_STREAM, 0);
                            if (sockfd == -1) {
                                cerr << "Network Error: Could not create "
                                        "socket\n";
                                exit(EXIT_FAILURE);
                            }
                            addr.sin_family = AF_INET;
                            addr.sin_port = htons(successor_port);
                            addr.sin_addr.s_addr = inet_addr("127.0.0.1");

                            if (connect(sockfd, (struct sockaddr *)&addr,
                                        sizeof(addr)) == 0) {
                                // cout << "Connected to server\n";
                            } else {
                                cerr << "Network Error: Could not connect\n";
                                exit(EXIT_FAILURE);
                            }
                            write(sockfd, s.c_str(), s.length());
                            close(sockfd);
                        }
                        printFingerTable();
                    }
                    i++;
                } else if ((s.length() > 0)) {
                    // Key requets handles Here
                    auto vec = formatter.parseXML(s);
                    points[events[i].data.fd] = stoi(vec[0]);
                    size_t key_hash;
                    // cout << vec[0] << " " << vec[1] << " " << vec[2] << endl;
                    ss << hex << sha1(vec[2]).substr(0, 4);
                    ss >> key_hash;
                    cout << "key_hash: " << key_hash << endl;
                    if (key_hash <= nodeId and key_hash > predecessor_nodeId or
                        key_hash >= nodeId and key_hash > predecessor_nodeId and
                            nodeId <= predecessor_nodeId or
                        key_hash <= nodeId and key_hash < predecessor_nodeId and
                            nodeId <= predecessor_nodeId) {
                        cout << "Key on same node" << endl;
                        output = handler.handle(stoi(vec[0]), s);
                        if (points[events[i].data.fd] == output.first) {
                            points.erase(events[i].data.fd);
                            sendXMLtoPort(output.first, output.second);
                            i++;
                            orphanFlag = false;
                        } else {
                            for (int j = 0; j < nfds; j++) {
                                if (points[events[i].data.fd] == output.first) {
                                    points.erase(events[i].data.fd);
                                    sendXMLtoPort(output.first, output.second);
                                    i++;
                                    orphanFlag = false;
                                    break;
                                }
                            }
                            if (orphanFlag) {
                                cout << "Orphan Results: fd: " << output.first
                                     << endl;
                                // exit(EXIT_FAILURE);
                            }
                        }
                    } else if (key_hash > nodeId and
                                   key_hash <= fingerTable.at(0).nodeId or
                               key_hash > nodeId and
                                   key_hash >= fingerTable.at(0).nodeId and
                                   nodeId >= fingerTable.at(0).nodeId or
                               key_hash <= fingerTable.at(0).nodeId and
                                   key_hash < nodeId and
                                   nodeId >= fingerTable.at(0).nodeId) {
                        cout << "Forwarding to Sucessor" << endl;
                        sendXMLtoPort(fingerTable.at(0).port, s);
                    } else {
                        cout << "Forwarding..." << endl;
                        forward(key_hash);
                    }
                } else {
                    nfds--;
                }
            }
        }
    }

    void sendXMLtoPort(int new_port, string xml) {
        cout << "Sending XML to port: " << new_port << endl;
        struct sockaddr_in addr = {0};
        int n, sockfd, num = 1;

        /* Create socket and connect to server */
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            cerr << "Network Error: Could not create socket\n";
            exit(EXIT_FAILURE);
        }
        addr.sin_family = AF_INET;
        addr.sin_port = htons(new_port);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
            // cout << "Connected to server\n";
        } else {
            cerr << "Network Error: Could not connect\n";
            exit(EXIT_FAILURE);
        }
        write(sockfd, xml.c_str(), xml.length());
        close(sockfd);
    }

    int forward(size_t key_hash) {
        size_t last_nodeId = numeric_limits<size_t>::min();
        int port = -1;
        size_t min = numeric_limits<size_t>::max();
        if (key_hash > nodeId) {
            for (int i = 0; i < HASH_SIZE; i++) {
                if (fingerTable.at(i).nodeId > last_nodeId and
                    fingerTable.at(i).nodeId < key_hash) {
                    last_nodeId = fingerTable.at(i).nodeId;
                }
            }
            return last_nodeId;
        } else if (key_hash < nodeId) {
            for (int i = 0; i < HASH_SIZE; i++) {
                if (fingerTable.at(i).nodeId > min and
                    fingerTable.at(i).nodeId < key_hash) {
                    min = fingerTable.at(i).nodeId;
                }
            }
            if (min != 0) {
                return min;
            } else {
                last_nodeId = numeric_limits<size_t>::min();
                for (int i = 0; i < HASH_SIZE; i++) {
                    if (fingerTable.at(i).nodeId > last_nodeId) {
                        last_nodeId = fingerTable.at(i).nodeId;
                    }
                }
                return last_nodeId;
            }
        }
        return last_nodeId;
    }
};

void printUsage(string s) {
    cout << "Usage: " << s << " -port=XXXX -contact=XXXX\n";
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printUsage(string(argv[0]));
    }

    size_t port;
    size_t contact_port;

    string arg;
    size_t pos = 0;
    for (size_t i = 1; i < argc; i++) {
        arg = string(argv[i]);
        if (i == 1 and (pos = arg.find("=")) != string::npos) {
            if (arg.substr(1, pos - 1) == "port") {
                port = stoi(arg.substr(pos + 1, arg.length()));
                // cout << port << endl;
            } else {
                printUsage(string(argv[0]));
            }
        } else if (i == 2 and (pos = arg.find("=")) != string::npos) {
            if (arg.substr(1, pos - 1) == "contact") {
                contact_port = stoi(arg.substr(pos + 1, arg.length()));
                // cout << contact_port << endl;
            } else {
                printUsage(string(argv[0]));
            }
        } else {
            printUsage(string(argv[0]));
        }
    }

    Server server(port);
    server.init(contact_port);
    server.start();

    return 0;
}
