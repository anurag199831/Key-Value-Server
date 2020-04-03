#include <libvirt/libvirt.h>

#include <iostream>
#include <unordered_map>

#include "vm.cpp"

using namespace std;

class Manager {
   private:
	virConnectPtr conn;
	unordered_map<string, VM*> domains;
	void watch(string nameOfVm);

   public:
	Manager();
	~Manager();
	string startNewVm();
	void shutdown(const string& nameOfVm);
	void startNewVm(const string& nameOfVm);
	thread* launch(const string& nameOfVm);
	void debugInfo();
};