#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>

#include "KVClientLibrary.cpp"

#define MAX_BUFFER_SIZE 1024
using namespace std;

void printUsage(const string &name) {
	cout << "Usage: " << name
		 << "-ip=<ip> -port=<port> -input=<input_file>  -output=<output_file>"
		 << endl;
	exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[]) {
	string ip;
	size_t port;
	string inputfile;
	string outputfile;
	string arg;
	size_t pos;
	bool interactive = false;

	if (argc == 5) {
		for (int i = 1; i < argc; i++) {
			arg = string(argv[i]);
			if (i == 1 and (pos = arg.find('=')) != string::npos) {
				if (arg.substr(1, pos - 1) == "ip") {
					ip = arg.substr(pos + 1, arg.length());
				} else {
					printUsage(string(argv[0]));
				}
			} else if (i == 2 and (pos = arg.find('=')) != string::npos) {
				if (arg.substr(1, pos - 1) == "port") {
					port = stoi(arg.substr(pos + 1, arg.length()));
				} else {
					printUsage(string(argv[0]));
				}
			} else if (i == 3 and (pos = arg.find('=')) != string::npos) {
				if (arg.substr(1, pos - 1) == "input") {
					inputfile = arg.substr(pos + 1, arg.length());
				} else {
					printUsage(string(argv[0]));
				}
			} else if (i == 4 and (pos = arg.find('=')) != string::npos) {
				if (arg.substr(1, pos - 1) == "output") {
					outputfile = arg.substr(pos + 1, arg.length());
				} else {
					printUsage(string(argv[0]));
				}
			} else {
				printUsage(string(argv[0]));
			}
		}
	} else {
		printUsage(string(argv[0]));
	}

	struct sockaddr_in addr = {0};
	int n, sockfd;

	/* Create socket and connect to server */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		cerr << "Network Error: Could not create socket\n";
		exit(EXIT_FAILURE);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip.c_str());

	if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
		// cout << "Connected to server\n";
	} else {
		cerr << "Network Error: Could not connect\n";
		exit(EXIT_FAILURE);
	}

	string line;
	char buffer[MAX_BUFFER_SIZE];
	KVClientLibrary formatter;

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
			line = formatter.convertToXML(vec.at(0), vec.at(1));
		} else if (vec.size() == 3 and
				   (vec.at(0) == "put" or vec.at(0) == "PUT")) {
			line = formatter.convertToXML(vec.at(0), vec.at(1), vec.at(2));
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
				outfile << vec.at(0) << '\n';
			} else {
				// cerr << "Error parsing put XML\n";
			}
		}
		line = "";
	}
	// cout << "File terminated" << endl;
	return 0;
}
