#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "KVClientLibrary.cpp"

#define MAX_BUFFER_SIZE 1024
#define BATCH_SIZE 1024 * 16
#define DEFAULT_SERVER_PORT 9999
#define MAX_IDLE_TIME 30

using namespace std;

class Client
{
private:
	unordered_map<string, int> servers;
	KVClientFormatter formatter;
	const string serverFile =
		"../debug-manager/server.dat";

	int _connectToServer(const string& ip, int port);
	string __readFile(const string& file);
	unordered_set<string> _getAddressesFromFile(const string& file);
	void _updateServerConnections(const unordered_set<string>& ips);

public:
	Client();
	~Client();

	void start(const string& inputFile, const string& outFile);
};

Client::Client() = default;

Client::~Client() = default;

int Client::_connectToServer(const string& ip, int port)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr = {0};
	if (sockfd == -1)
	{
		cerr << "Client::_connectToServer: Network Error: Could not create "
				"socket for ip "
			 << ip << endl;
		exit(EXIT_FAILURE);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip.c_str());

	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0)
	{
		cout << "Connected to server\n";
	}
	else
	{
		cerr << "Network Error: Could not connect to " << ip << " on port "
			 << port << endl;
		return -1;
	}
	return sockfd;
}

void Client::start(const string& inputFile, const string& outFile)
{
	int idleCount = 0;
	string line;
	int sockfd;
	int status = 0;
	int request = 0;
	char buffer[MAX_BUFFER_SIZE];

	ifstream infile;

	infile.open(inputFile, ifstream::in);
	if (!infile.is_open())
	{
		cerr << "Client::start: Error opening: " << inputFile << endl;
		exit(EXIT_FAILURE);
	}

	ofstream outfile;
	outfile.open(outFile, ofstream::out);
	if (!outfile.is_open())
	{
		cerr << "Client::start: Error creating: " << outFile << endl;
		infile.close();
		exit(EXIT_FAILURE);
	}

	while (idleCount < MAX_IDLE_TIME)
	{
		request = 0;
		auto ips = _getAddressesFromFile(serverFile);
		_updateServerConnections(ips);
		if (servers.empty())
		{
			cout << "Client::start: No active servers found terminating after "
				 << MAX_IDLE_TIME - idleCount << endl;
			this_thread::sleep_for(chrono::seconds(2));
			idleCount++;
			continue;
		}
		vector<int> fds;
		transform(servers.begin(), servers.end(), back_inserter(fds),
				  [](const unordered_map<string, int>::value_type& val) {
					  return val.second;
				  });
		this_thread::sleep_for(chrono::seconds(1));
		while ((infile >> line) and request < BATCH_SIZE)
		{
			request++;
			sockfd = fds.at(request % fds.size());
			idleCount = 0;

			if (line.length() == 0)
			{
				continue;
			}
			vector<string> vec = formatter.tokenize(line);
			if (vec.size() == 2 and
				(vec.at(0) == "get" or vec.at(0) == "del" or
				 vec.at(0) == "GET" or vec.at(0) == "DEL"))
			{
				line = formatter.convertToXML(vec.at(0), vec.at(1));
			}
			else if (vec.size() == 3 and
					 (vec.at(0) == "put" or vec.at(0) == "PUT"))
			{
				line = formatter.convertToXML(vec.at(0), vec.at(1), vec.at(2));
			}
			else
			{
				line = "garbage";
			}
			cout << "Sending data to server. Request: " << request << endl;
			status = write(sockfd, line.c_str(), line.length());
			if (status == -1)
			{
				cout << "Client::start: Error sending data to server" << endl;
				break;
			}
			memset(buffer, 0, MAX_BUFFER_SIZE);
			status = read(sockfd, buffer, MAX_BUFFER_SIZE);
			if (status == -1)
			{
				cout << "Client::start: Error receiving data from server"
					 << endl;
				break;
			}
			cout << "Received data from server. Request: " << request
				 << " fd: " << sockfd << endl;
			line = string(buffer);
			if (vec.at(0) == "get" or vec.at(0) == "GET")
			{
				vec = formatter.parseXML(line);
				if (vec.size() == 2)
				{
					outfile << vec.at(0) << "," << vec.at(1) << '\n';
				}
				else if (vec.size() == 1)
				{
					outfile << vec.at(0) << '\n';
				}
				else
				{
					cerr << "Error parsing get XML\n";
				}
			}
			else if (vec.size() != 0 and
					 (vec.at(0) == "put" or vec.at(0) == "PUT"))
			{
				vec = formatter.parseXML(line);
				if (vec.size() == 1)
				{
					outfile << vec.at(0) << '\n';
				}
				else
				{
					cerr << "Error parsing put XML\n";
					break;
				}
			}
			else if (vec.size() != 0 and
					 (vec.at(0) == "del" or vec.at(0) == "DEL"))
			{
				vec = formatter.parseXML(line);
				if (vec.size() == 1)
				{
					outfile << vec.at(0) << '\n';
				}
				else
				{
					cerr << "Error parsing put XML\n";
					break;
				}
			}
			else
			{
				cerr << "Error parsing put XML\n";
				break;
			}
			line = "";
		}
	}
}

// Reads a files and returns a single string of the entire text.
// Returns empty string in case of error.
string Client::__readFile(const string& file)
{
	ifstream serverFile;
	string retStr = "", str; /* code */
	serverFile.open(file, ifstream::in);
	if (!serverFile.is_open())
	{
		cerr << "Client::__readFile: Error opening file " << file << endl;
		return retStr;
	}
	while ((serverFile >> str))
	{
		retStr += (str + '\n');
	}
	serverFile.close();
	return retStr;
}

// Returns the vector of all IPs that the client can connect.
// Returns empty vector in case of error.
unordered_set<string> Client::_getAddressesFromFile(const string& file)
{
	unordered_set<string> s;

	string text = __readFile(file);
	if (text.empty()) return s;

	stringstream ss(text);
	string ip;

	while ((ss >> ip))
	{
		if (formatter.validateIp(ip))
		{
			s.emplace(ip);
		}
	}
	return s;
}

void Client::_updateServerConnections(const unordered_set<string>& ips)
{
	int sockfd;
	unordered_set<string> inactiveServers;
	transform(servers.begin(), servers.end(),
			  inserter(inactiveServers, inactiveServers.begin()),
			  [](const unordered_map<string, int>::value_type& val) {
				  return val.first;
			  });

	for (auto&& i : ips)
	{
		if (inactiveServers.find(i) == inactiveServers.end())
		{
			sockfd = _connectToServer(i, DEFAULT_SERVER_PORT);
			if (sockfd != -1)
			{
				cout << "Client::_updateServerConnections: Connected to " << i
					 << endl;
				servers[i] = sockfd;
			}
			else
			{
				cerr << "Client::_updateServerConnections: Error opening a "
						"socket on "
					 << i << endl;
			}
		}
		else
		{
			inactiveServers.erase(i);
		}
	}
	for (auto&& i : inactiveServers)
	{
		cout << "Client::_updateServerConnections: Closing connection with "
			 << i << endl;
		sockfd = servers.at(i);
		close(sockfd);
		servers.erase(i);
	}
}

int main(int argc, char const* argv[])
{
	Client client;
	int random = rand() % 10;
	client.start("input0.csv", "out" + to_string(random) + ".csv");
}
