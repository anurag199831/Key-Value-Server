#include <libvirt/libvirt.h>

#include <iostream>
#include <list>
#include <mutex>
#include <unordered_map>

#include "vm.cpp"

using namespace std;

class Manager {
   private:
	virConnectPtr conn;
	unordered_map<string, VM*> domains;
	unordered_map<string, mutex*> locks;
	unordered_map<string, list<int>*> utilList;
	unordered_map<string, bool> threadTerminationFlags;
	unordered_map<string, mutex*> threadTerminationLocks;
	const string ipFile = "server.dat";

	void _watch(string nameOfVm);
	bool _writeIpToFile(const string& ip);
	bool _deleteIpFromFile(const string& ip);

   public:
	Manager();
	~Manager();
	string startNewVm();
	void powerOn(const string& nameOfVm);
	void shutdown(const string& nameOfVm);
	void startNewVm(const string& nameOfVm);
	thread* startWatching(const string& nameOfVm);
	vector<int> getUtilVector(const string& nameOfVm);

	void notifyAboutServer();
	size_t numActiveDomains();
};
