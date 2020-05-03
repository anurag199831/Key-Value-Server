#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <set>

#include "KVClientHandler.cpp"

#define DEFAULT_SERVER_PORT 9999

using namespace std;

class Server {
   private:
	const int BACK_LOG = 5;
	const int MAX_BUFF = 1024;
	const int MAX_EVENTS = 20;
	const string pathToDatabase = "database/";
	const size_t port;
	const size_t threadPoolSize;
	const size_t numSetsInCache;
	const size_t sizeOfSet;
	int serverFd{};
	int epoll_fd{};
	KVClientHandler handler;

   public:
	Server(const size_t port, const size_t threadPoolSize,
		   const size_t numSetsInCache, const size_t sizeOfSet)
		: port(port),
		  threadPoolSize(threadPoolSize),
		  numSetsInCache(numSetsInCache),
		  sizeOfSet(sizeOfSet),
		  handler(threadPoolSize, numSetsInCache, sizeOfSet) {}
	~Server() { close(epoll_fd); }

	void init() {
		struct stat info {};
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

		struct sockaddr_in addr {};
		serverFd = socket(AF_INET, SOCK_STREAM, 0);
		if (serverFd == -1) {
			cerr << "Error getting a socket" << endl;
			exit(EXIT_FAILURE);
		}
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(serverFd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
			cout << "Binding successful" << endl;
		} else {
			cerr << "Binding failed" << endl;
			exit(EXIT_FAILURE);
		}
		if (listen(serverFd, BACK_LOG) == 0) {
			cout << "Listening on port: " << port << endl;
		} else {
			cerr << "Listening failed" << endl;
		}
		/* set O_NONBLOCK on fd */
		long flags = fcntl(serverFd, F_GETFL, 0);
		fcntl(serverFd, F_SETFL, flags | O_NONBLOCK);
	}

	void start() {
		struct sockaddr client {};
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

		while (true) {
			memset(&client, 0, sizeof(client));
			addrlen = sizeof(client);
			sockfd = accept(serverFd, (struct sockaddr *)&client,
							(socklen_t *)&addrlen);
			if (sockfd != -1) {
				cout << "sockfd: " << sockfd << endl;
				cout << "Client connected\n";
				struct epoll_event ev {};
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
			// cout << "nfds: " << nfds << endl;
			int i = 0;
			while (i < nfds) {
				orphanFlag = true;
				memset(buffer, 0, MAX_BUFF);
				n = read(events[i].data.fd, buffer, MAX_BUFF);
				string s = string(buffer);
				if (s.length() > 0) {
					output = handler.handle(events[i].data.fd, s);
					if (events[i].data.fd == output.first) {
						n = write(output.first, output.second.c_str(),
								  output.second.length());
						// cout << "Direct Hit" << endl;
						orphanFlag = false;
						i++;
						// doneNfds.insert(events[i].data.fd);
					} else {
						for (int j = 0; j < nfds; j++) {
							if (events[j].data.fd == output.first) {
								n = write(output.first, output.second.c_str(),
										  output.second.length());
								i++;
								// cout << "Indirect Hit" << endl;
								orphanFlag = false;
								// doneNfds.insert(events[i].data.fd);
								break;
							}
						}
						if (orphanFlag) {
							cout << "Orphan Results: fd: " << output.first
								 << endl;
							// exit(EXIT_FAILURE);
						}
					}
					// cout << "==================================" << endl;
				} else {
					nfds--;
				}
			}
		}
	}
};

void printUsage(string s) {
	cout << "Usage: " << s << " -port=XXXX\n";
	exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[]) {
	size_t threadPoolSize = 16;
	size_t numSetsInCache = 64;
	size_t sizeOfSet = 128;

	Server server(DEFAULT_SERVER_PORT, threadPoolSize, numSetsInCache,
				  sizeOfSet);

	server.init();
	server.start();

	return 0;
}
