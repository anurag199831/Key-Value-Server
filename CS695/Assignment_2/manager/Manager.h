//
// Created by pranav on 18/04/20.
//

#ifndef ASSIGNMENT_2_MANAGER_H
#define ASSIGNMENT_2_MANAGER_H

#include <libvirt/libvirt.h>

#include <iostream>
#include <list>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <thread>
#include "VM.h"


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
	bool isVmPowered(const string &nameOfVm);
	string getIP(const string &nameOfVm);
	thread* startWatching(const string& nameOfVm);
	vector<int> getUtilVector(const string& nameOfVm);
	vector<string> getAllDefinedDomainNames();
	void notifyAboutServer();
};


#endif //ASSIGNMENT_2_MANAGER_H
