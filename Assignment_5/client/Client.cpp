#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>

#include "KVClientLibrary.cpp"

#define MAX_BUFFER_SIZE 1024
using namespace std;

void printUsage(string name) {
    cout << "Usage (For batch mode): " << name
         << " -server_port=<port> -input=<input_file>  -output=<output_file> "
            "-dest_port=<dest_port>"
         << endl;
    cout << "Usage (For interactive mode): " << name
         << " -server_port=<port> -dest_port=<dest_port>" << endl;
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[]) {
    size_t client_port, server_port, dest_port;
    string inputfile;
    string outputfile;
    string arg;
    size_t pos;
    bool interactive = false;

    if (argc == 5) {
        for (size_t i = 1; i < argc; i++) {
            arg = string(argv[i]);
            if (i == 1 and (pos = arg.find("=")) != string::npos) {
                if (arg.substr(1, pos - 1) == "server_port") {
                    server_port = stoi(arg.substr(pos + 1, arg.length()));
                } else {
                    printUsage(string(argv[0]));
                }
            } else if (i == 2 and (pos = arg.find("=")) != string::npos) {
                if (arg.substr(1, pos - 1) == "input") {
                    inputfile = arg.substr(pos + 1, arg.length());
                } else {
                    printUsage(string(argv[0]));
                }
            } else if (i == 3 and (pos = arg.find("=")) != string::npos) {
                if (arg.substr(1, pos - 1) == "output") {
                    outputfile = arg.substr(pos + 1, arg.length());
                } else {
                    printUsage(string(argv[0]));
                }
            } else if (i == 4 and (pos = arg.find("=")) != string::npos) {
                if (arg.substr(1, pos - 1) == "dest_port") {
                    dest_port = stoi(arg.substr(pos + 1, arg.length()));
                } else {
                    printUsage(string(argv[0]));
                }
            } else {
                printUsage(string(argv[0]));
            }
        }
        // cout << "Batch mode\n";
    } else if (argc == 3) {
        interactive = true;
        for (size_t i = 1; i < argc; i++) {
            arg = string(argv[i]);
            if (i == 1 and (pos = arg.find("=")) != string::npos) {
                cout << arg.substr(1, pos - 1) << endl;
                if (arg.substr(1, pos - 1) == "server_port") {
                    server_port = stoi(arg.substr(pos + 1, arg.length()));
                } else {
                    printUsage(string(argv[0]));
                }
            } else if (i == 2 and (pos = arg.find("=")) != string::npos) {
                cout << arg.substr(1, pos - 1) << endl;
                if (arg.substr(1, pos - 1) == "dest_port") {
                    dest_port = stoi(arg.substr(pos + 1, arg.length()));
                } else {
                    printUsage(string(argv[0]));
                }
            }
            // cout << "Interactive mode\n";
        }
    } else {
        printUsage(string(argv[0]));
    }

    struct sockaddr_in addr = {0};
    int n, sockfd, num = 1;

    /* Create socket and connect to server */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        cerr << "Network Error: Could not create socket\n";
        exit(EXIT_FAILURE);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server_port);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
        cout << "Connected to server on writing port\n";
    } else {
        cerr << "Network Error: Could not connect\n";
        exit(EXIT_FAILURE);
    }

    struct sockaddr server;
    int addrlen = 0;
    struct sockaddr_in s_addr = {0};
    int recvFd = socket(AF_INET, SOCK_STREAM, 0);
    if (recvFd == -1) {
        cerr << "Error getting a socket" << endl;
        exit(EXIT_FAILURE);
    }
    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(dest_port);
    s_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (bind(recvFd, (struct sockaddr *)&s_addr, sizeof(s_addr)) == 0) {
        cout << "Binding successful" << endl;
    } else {
        cerr << "Binding failed" << endl;
        exit(EXIT_FAILURE);
    }
    if (listen(recvFd, 5) == 0) {
        cout << "Listening on port: " << dest_port << endl;
    } else {
        cerr << "Listening failed" << endl;
    }

    string line;
    char buffer[MAX_BUFFER_SIZE];
    KVClientLibrary formatter;

    if (interactive) {
        while (true) {
            cout << "Input: ";
            cin >> line;
            vector<string> vec = formatter.tokenize(line);
            if (vec.size() == 2 and
                (vec.at(0) == "get" or vec.at(0) == "del" or
                 vec.at(0) == "GET" or vec.at(0) == "DEL")) {
                line = formatter.convertToXML(dest_port, vec.at(0), vec.at(1));
            } else if (vec.size() == 3 and
                       (vec.at(0) == "put" or vec.at(0) == "PUT")) {
                line = formatter.convertToXML(dest_port, vec.at(0), vec.at(1),
                                              vec.at(2));
            } else {
                line = "garbage";
            }
            n = write(sockfd, line.c_str(), line.length());
            accept(recvFd, (struct sockaddr *)&server, (socklen_t *)&addrlen);
            memset(buffer, 0, MAX_BUFFER_SIZE);
            n = read(recvFd, buffer, MAX_BUFFER_SIZE);
            line = string(buffer);
            cout << line << endl;
            // vec = formatter.parseXML(line);
            // if (vec.size() == 1) {
            //     cout << vec.at(0) << '\n';
            // } else if (vec.size() == 2) {
            //     cout << vec.at(0) << "," << vec.at(1) << '\n';
            // } else {
            // }
            line = "";
        }
    }

    ifstream infile;
    infile.open(inputfile, ifstream::in);
    if (!infile.is_open()) {
        cerr << "Error opening: " << inputfile << endl;
        exit(EXIT_FAILURE);
    }

    ofstream outfile;
    outfile.open(outputfile, ofstream::out);
    if (!outfile.is_open()) {
        cerr << "Error creating: " << outputfile << endl;
        exit(EXIT_FAILURE);
    }

    while (!infile.eof()) {
        infile >> line;
        if (line.length() == 0) {
            continue;
        }
        vector<string> vec = formatter.tokenize(line);
        if (vec.size() == 2 and (vec.at(0) == "get" or vec.at(0) == "del" or
                                 vec.at(0) == "GET" or vec.at(0) == "DEL")) {
            line = formatter.convertToXML(dest_port, vec.at(0), vec.at(1));
        } else if (vec.size() == 3 and
                   (vec.at(0) == "put" or vec.at(0) == "PUT")) {
            line = formatter.convertToXML(dest_port, vec.at(0), vec.at(1),
                                          vec.at(2));
        } else {
            line = "garbage";
        }
        n = write(sockfd, line.c_str(), line.length());
        memset(buffer, 0, MAX_BUFFER_SIZE);
        n = read(sockfd, buffer, MAX_BUFFER_SIZE);
        line = string(buffer);
        if (vec.at(0) == "get" or vec.at(0) == "GET") {
            vec = formatter.parseXML(line);
            if (vec.size() == 2) {
                outfile << vec.at(0) << "," << vec.at(1) << '\n';
            } else if (vec.size() == 1) {
                outfile << vec.at(0) << '\n';
            } else {
                // cerr << "Error parsing get XML\n";
            }
        } else if (vec.at(0) == "put" or vec.at(0) == "PUT") {
            // cout << line << '\n';
            vec = formatter.parseXML(line);
            if (vec.size() == 1) {
                outfile << vec.at(0) << '\n';
            } else {
                // cerr << "Error parsing put XML\n";
            }
        } else if (vec.at(0) == "del" or vec.at(0) == "DEL") {
            // cout << line << '\n';
            vec = formatter.parseXML(line);
            if (vec.size() == 1) {
                if (vec.at(0) == "Success") {
                    outfile << vec.at(0) << '\n';
                } else {
                    outfile << vec.at(0) << '\n';
                }
            } else {
                // cerr << "Error parsing put XML\n";
            }
        }
        line = "";
    }
    // cout << "File terminated" << endl;
    return 0;
}
