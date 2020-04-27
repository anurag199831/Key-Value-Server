//
// Created by pranav on 18/04/20.
//

#include "Manager.h"

#include <fcntl.h>

#include <cstdio>
#include <fstream>
#include <functional>
#include <stack>
#include <thread>
#include <utility>
#include <vector>

Manager::Manager() {
	conn = virConnectOpen("qemu:///system");
	if (conn == NULL) {
		throw runtime_error("Failed to open connection to qemu:///system");
	}
}

Manager::~Manager() {
	try {
		remove(ipFile.c_str());
	} catch (exception &e) { cout << "Manager::~Manager" << e.what() << endl; }
	for (auto &&i : threadTerminationLocks) {
		lock_guard lck(*i.second);
		auto it = threadTerminationFlags.find(i.first);
		if (it != threadTerminationFlags.end()) {
			it->second = true;
		} else {
			cerr << "Manager::~Manager: No termination flag for " << i.first
				 << " found" << endl;
		}
	}

	for (auto &i : threads) {
		if (i != nullptr) {
			i->join();
			cout << "Manager:~Manager: utilThread destroyed" << endl;
		}
	}

	for (auto &&i : domains) {
		cout << "Manager::~Manager: " << i.first << " VM deleted" << endl;
		delete i.second;
	}

	for (auto &&i : locks) {
		cout << "Manager::~Manager: " << i.first << " mutex deleted" << endl;
		delete i.second;
	}

	for (auto &&i : utilList) {
		cout << "Manager::~Manager: " << i.first << " util list deleted"
			 << endl;
		delete i.second;
	}

	for (auto &&i : threadTerminationLocks) {
		cout << "Manager::~Manager: " << i.first
			 << " threadTerminationLocks deleted" << endl;
		delete i.second;
	}

	virConnectClose(conn);
	conn = NULL;

	//	for (auto &&i : threadTerminationLocks) {
	//		cout << "Manager::~Manager: " << i.first << " threadTermination
	// mutex deleted"
	//			 << endl;
	//		delete i.second;
	//	}
}

string Manager::startNewVm() {
	string name;
	try {
		VM *vm = new VM(conn);
		auto *m = new mutex;
		auto *n = new mutex;
		auto *lst = new list<int>();
		bool terminationFlag = true;
		name = vm->getName();

		domains.insert(make_pair(name, vm));
		utilList.insert(make_pair(name, lst));
		locks.insert(make_pair(name, m));
		threadTerminationFlags.insert(make_pair(name, terminationFlag));
		threadTerminationLocks.insert(make_pair(name, n));
	} catch (exception &e) { cout << e.what() << endl; }
	return name;
}

void Manager::startNewVm(const string &nameOfVm) {
	auto vec = VM::getAllDefinedDomainNames(conn);
	if (find(vec.begin(), vec.end(), nameOfVm) == vec.end()) {
		cerr << "Manager::startNewVm: no inactive domain with name " << nameOfVm
			 << " found" << endl;
	}

	try {
		VM *vm = new VM(conn, nameOfVm);
		auto *m = new mutex;
		auto *n = new mutex;
		auto *lst = new list<int>();
		bool terminationFlag = true;

		domains.insert(make_pair(nameOfVm, vm));
		utilList.insert(make_pair(nameOfVm, lst));
		locks.insert(make_pair(nameOfVm, m));
		threadTerminationFlags.insert(make_pair(nameOfVm, terminationFlag));
		threadTerminationLocks.insert(make_pair(nameOfVm, n));
	} catch (exception &e) { cout << e.what() << endl; }
}

void Manager::_watch(string nameOfVm) {
	class Worker {
		stack<function<void(string)>> exit_funcs;
		string nameOfVm;

	   public:
		explicit Worker(string name) : nameOfVm(std::move(name)) {}
		Worker(Worker const &) = delete;
		void operator=(Worker const &) = delete;
		~Worker() {
			while (!exit_funcs.empty()) {
				exit_funcs.top()(nameOfVm);
				exit_funcs.pop();
			}
		}
		void add(function<void(string)> func) { exit_funcs.push(move(func)); }
	};

	thread_local Worker threadWorker(std::move(nameOfVm));

	// threadWorker.add([this](const string &nameOfVm) {
	// 	VM *vm = domains.at(nameOfVm);
	// 	domains.erase(nameOfVm);
	// 	delete vm;
	// });

	threadWorker.add([this](const string &nameOfVm) {
		bool terminationFlag;
		long status;
		double util = 0;

		{
			auto lck = threadTerminationLocks.at(nameOfVm);
			lock_guard l(*lck);
			terminationFlag = threadTerminationFlags.at(nameOfVm);
		}

		auto vm = domains.at(nameOfVm);
		auto m = locks.at(nameOfVm);
		auto lst = utilList.at(nameOfVm);

		while (not terminationFlag) {
			{
				auto lck = threadTerminationLocks.at(nameOfVm);
				lock_guard l(*lck);
				terminationFlag = threadTerminationFlags.at(nameOfVm);
				if (terminationFlag) { break; }
				auto t = vm->getVmCpuUtil(conn);
				status = get<0>(t);
				if (status >= 4 and status <= 6) {
					cout << "Manager::_watch: " << vm->getName()
						 << " is powered off" << endl;
					terminationFlag = true;
				}
				if (not terminationFlag) {
					util = get<1>(t);
					{
						lock_guard lock(*m);
						lst->push_back(util);
						if (lst->size() > 40) { lst->erase(lst->begin()); }
					}
				}
			}
			this_thread::sleep_for(chrono::milliseconds(10));
		}
	});
}

// Start watching the utilization of the VM with the passed name and store it in
// utilVector.
void Manager::startWatching(const string &nameOfVm) {
	auto it = threadTerminationFlags.find(nameOfVm);
	if (it == threadTerminationFlags.end()) {
		cerr << "Manager::startWatching: no Vm with name " << nameOfVm
			 << " found" << endl;
		return;
	} else {
		it->second = false;
	}
	auto th = new thread([this, nameOfVm]() { _watch(nameOfVm); });
	threads.push_back(th);
}

// Shutdown the VM with the name.
// Invalid name if passed has no effect.
void Manager::shutdown(const string &nameOfVm) {
	auto it = threadTerminationLocks.find(nameOfVm);
	if (it != threadTerminationLocks.end()) {
		lock_guard l(*it->second);
		auto iter = threadTerminationFlags.find(nameOfVm);
		iter->second = true;
	} else {
		cerr << "Manager::shutdown: no active VM with name '" << nameOfVm
			 << "' found." << endl;
		return;
	}
	this_thread::sleep_for(chrono::seconds(1));
	VM *vm;
	mutex *m;
	list<int> *lst;
	try {
		vm = domains.at(nameOfVm);
		m = locks.at(nameOfVm);
		lst = utilList.at(nameOfVm);
		_deleteIpFromFile(vm->getIp());
		vm->shutdown();
		delete vm;
		delete m;
		delete lst;
		domains.erase(nameOfVm);
		utilList.erase(nameOfVm);
		locks.erase(nameOfVm);

	} catch (exception &e) { cout << e.what() << endl; }
}

// Notifies the client about the server.
void Manager::notifyAboutServer() {
	remove(ipFile.c_str());
	for (auto &&i : domains) {
		auto m = i.second->getInterfaceInfo();
		for (auto &&j : m) {
			for (auto &k : j.second) { _writeIpToFile(k); }
		}
	}
}

// Appends IP to the file.
bool Manager::_writeIpToFile(const string &ip) {
	ofstream serverFile;
	serverFile.open(ipFile, fstream ::out | fstream::app);
	if (!serverFile.is_open()) {
		cerr << "Manager::writeToFile: Error opening file " << ipFile << endl;
		return false;
	}

	serverFile << ip << endl;
	serverFile.close();
	return true;
}

// Deletes the ip from the file if present.
bool Manager::_deleteIpFromFile(const string &ip) {
	ifstream inputFile;
	ofstream outFile;
	inputFile.open(ipFile, ifstream::in);
	if (!inputFile.is_open()) { return false; }
	outFile.open(ipFile + "_temp", ofstream::out);
	if (!outFile.is_open()) { return false; }
	string line;
	while (inputFile >> line) {
		if (line == ip) {
			cout << "Manager::deleteIpFromFile: " << ip << " deleted from file"
				 << endl;
		} else {
			outFile << line << endl;
		}
	}
	inputFile.close();
	outFile.close();
	remove(ipFile.c_str());
	rename((ipFile + "_temp").c_str(), ipFile.c_str());
	return true;
}

// Returns the vector that contains the utilization of the VM over a defined
// period.
vector<int> Manager::getUtilVector(const string &nameOfVm) {
	vector<int> vec;

	auto m = locks.find(nameOfVm);
	if (m == locks.end()) { return vec; }

	lock_guard l(*m->second);

	auto lst = utilList.at(nameOfVm);
	for (auto &&i : *lst) { vec.push_back(i); }
	return vec;
}

// Returns the vector of string that represents, all the defined domains.
vector<string> Manager::getAllDefinedDomainNames() {
	return VM::getAllDefinedDomainNames(conn);
}

// Powers on the VM with the passed name.
// The name passed to the method should be obtained by startNewVm.
// On passing invalid name, no effect.
void Manager::powerOn(const string &nameOfVm) {
	auto it = domains.find(nameOfVm);
	if (it == domains.end()) {
		cerr << "Manager::powerOn: No active vm with name " << nameOfVm
			 << " found" << endl;
		return;
	}
	auto vm = it->second;
	if(not vm->isPoweredOn()) { vm->powerOn(); }
	//	cout << "Manager::powerOn: Waiting for 30 secs for VM to boot" << endl;
	//	this_thread::sleep_for(chrono::seconds(30));
	notifyAboutServer();
}

// Returns a bool representing the power status of the VM.
bool Manager::isVmPowered(const string &nameOfVm) {
	auto it = domains.find(nameOfVm);
	if (it == domains.end()) { return false; }
	auto vm = it->second;
	return vm->isPoweredOn();
}

// Returns the IP of the VM with the passed name.
// Returns empty otherwise
string Manager::getIP(const string &nameOfVm) {
	auto it = domains.find(nameOfVm);
	if (it == domains.end()) {
		cerr << "Manager::getIP: No active vm with name " << nameOfVm
			 << " found" << endl;
		return "";
	}
	auto vm = it->second;
	return vm->getIp();
}

void Manager::attachToAlreadyRunningVms(){
	auto runningVms= VM::getAllActiveDomainNames(conn);
	for(auto&& vm: runningVms){
		startNewVm(vm);
		startWatching(vm);
	}
}