#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <unordered_map>

#include "KVClientLibrary.cpp"

#define MAX_BUFFER_SIZE 1024
#define BATCH_SIZE 128
#define DEFAULT_SERVER_PORT 9999

using namespace std;

class Client {
   private:
	unordered_map<string, int> servers;
	KVClientFormatter formatter;

	int _connectToServer(const string& ip, int port);
	string __readFile(const string& file);
	bool _validateIp(const string& str);
	vector<string> _getAddressesFromFile(const string& file);
	void _updateServerConnections(const vector<string>& ips);

   public:
	Client();
	Client(const string& ip, int port);
	~Client();

	void start(const string& inputFile, const string& outFile);
};

Client::Client() {}

Client::~Client() {}

int Client::_connectToServer(const string& ip, int port) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr = {0};
	if (sockfd == -1) {
		cerr << "Client::_connectToServer: Network Error: Could not create "
				"socket for ip "
			 << ip << endl;
		exit(EXIT_FAILURE);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip.c_str());

	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
		cout << "Connected to server\n";
	} else {
		cerr << "Network Error: Could not connect\n";
		return -1;
	}
	return sockfd;
}

void Client::start(const string& inputFile, const string& outFile) {
	int idleCount = 0;
	string line;
	int sockfd;
	int status = 0;
	int request = 0;
	char buffer[MAX_BUFFER_SIZE];
	const string serverFile = "../vm_manager/server.dat";

	ifstream infile;
	infile.open(inputFile, ifstream::in);
	if (!infile.is_open()) {
		cerr << "Client::start: Error opening: " << inputFile << endl;
		exit(EXIT_FAILURE);
	}

	ofstream outfile;
	outfile.open(outFile, ofstream::out);
	if (!outfile.is_open()) {
		cerr << "Client::start: Error creating: " << outFile << endl;
		exit(EXIT_FAILURE);
	}

	while (idleCount < 30) {
		request = 0;
		auto ips = _getAddressesFromFile(serverFile);
		_updateServerConnections(ips);
		if (servers.empty()) {
			idleCount++;
			continue;
		}
		vector<int> fds;
		transform(servers.begin(), servers.end(), back_inserter(fds),
				  [](const unordered_map<string, int>::value_type& val) {
					  return val.second;
				  });

		while (!infile.eof() and request < BATCH_SIZE) {
			request++;
			sockfd = fds.at(request / fds.size());
			idleCount = 0;

			infile >> line;
			if (line.length() == 0) {
				continue;
			}
			vector<string> vec = formatter.tokenize(line);
			if (vec.size() == 2 and
				(vec.at(0) == "get" or vec.at(0) == "del" or
				 vec.at(0) == "GET" or vec.at(0) == "DEL")) {
				line = formatter.convertToXML(vec.at(0), vec.at(1));
			} else if (vec.size() == 3 and
					   (vec.at(0) == "put" or vec.at(0) == "PUT")) {
				line = formatter.convertToXML(vec.at(0), vec.at(1), vec.at(2));
			} else {
				line = "garbage";
			}
			status = write(sockfd, line.c_str(), line.length());
			if (status == -1) {
				cout << "Client::start: Error sending data to server" << endl;
				continue;
			}
			memset(buffer, 0, MAX_BUFFER_SIZE);
			status = read(sockfd, buffer, MAX_BUFFER_SIZE);
			line = string(buffer);
			if (vec.at(0) == "get" or vec.at(0) == "GET") {
				vec = formatter.parseXML(line);
				if (vec.size() == 2) {
					outfile << vec.at(0) << "," << vec.at(1) << '\n';
				} else if (vec.size() == 1) {
					outfile << vec.at(0) << '\n';
				} else {
					cerr << "Error parsing get XML\n";
				}
			} else if (vec.size() != 0 and
					   (vec.at(0) == "put" or vec.at(0) == "PUT")) {
				vec = formatter.parseXML(line);
				if (vec.size() == 1) {
					outfile << vec.at(0) << '\n';
				} else {
					cerr << "Error parsing put XML\n";
				}
			} else if (vec.size() != 0 and
					   (vec.at(0) == "del" or vec.at(0) == "DEL")) {
				vec = formatter.parseXML(line);
				if (vec.size() == 1) {
					outfile << vec.at(0) << '\n';
				} else {
					cerr << "Error parsing put XML\n";
				}
			} else {
				cerr << "Error parsing put XML\n";
			}
			line = "";
		}
	}
}

// Reads a files and returns a single string of the entire text.
// Returns empty string in case of error.
string Client::__readFile(const string& file) {
	string str;
	struct flock fl = {F_WRLCK, SEEK_SET, 0, 0, 0};
	int fd;

	fl.l_pid = getpid();
	fl.l_type = F_RDLCK;

	if ((fd = open(file.c_str(), O_RDONLY)) == -1) {
		cerr << "Client::_readFile: Error opening file " << file << endl;
		return str;
	}

	if (fcntl(fd, F_SETLK, &fl) == -1) {
		cerr << "Client::_readFile: Couldn't obtain a lock on file " << file
			 << endl;
		return str;
	}

	char buff[MAX_BUFFER_SIZE];
	int n;

	while ((n = read(fd, buff, MAX_BUFFER_SIZE)) != 0) {
		if (n == -1) {
			return str;
		}
		str += string(buff);
	}

	fl.l_type = F_UNLCK;
	if (fcntl(fd, F_SETLK, &fl) == -1) {
		cerr << "Client::_readFile: Error releasing lock on file " << file
			 << endl;
	}
	return str;
}

// Returns the vector of all IPs that the client can connect.
// Returns empty vector in case of error.
vector<string> Client::_getAddressesFromFile(const string& file) {
	vector<string> vec;

	string text = __readFile(file);
	if (text.empty()) return vec;

	stringstream ss(text);
	string ip;

	while (!ss.eof()) {
		ss >> ip;
		if (formatter.validateIp(ip)) {
			vec.emplace_back(ip);
		}
	}
	return vec;
}

void Client::_updateServerConnections(const vector<string>& ips) {
	int sockfd;
	for (auto&& i : ips) {
		if (servers.find(i) == servers.end()) {
			sockfd = _connectToServer(i, DEFAULT_SERVER_PORT);
			if (sockfd != -1) {
				servers[i] = sockfd;
			}
		}
	}
}

int main(int argc, char const* argv[]) {
	Client client;
	KVClientFormatter formatter;
}
