#include <libvirt/libvirt.h>

#include <iostream>
#include <unordered_map>

#include "vm.cpp"

using namespace std;

class Manager {
   private:
	virConnectPtr conn;
	unordered_map<string, VM*> domains;
	const string ipFile = "server.dat";

	void watch(string nameOfVm);
	bool writeIpToFile(const string& ip);
	bool deleteIpFromFile(const string& ip);

   public:
	Manager();
	~Manager();
	string startNewVm();
	void shutdown(const string& nameOfVm);
	void startNewVm(const string& nameOfVm);
	thread* launch(const string& nameOfVm);
	void debugInfo();
};