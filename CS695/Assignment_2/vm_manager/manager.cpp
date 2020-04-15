#include "manager.h"

#include <fcntl.h>

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <functional>
#include <stack>
#include <thread>
#include <utility>

#define MAX_BUFFER_SIZE 1024

Manager::Manager() {
	conn = virConnectOpen("qemu:///system");
	if (conn == NULL) {
		throw runtime_error("Failed to open connection to qemu:///system");
	}
}

Manager::~Manager() {
	for (auto &&i : domains) {
		cout << "Manager::~Manager: " << i.first << " VM deleted";
		delete i.second;
	}
	for (auto &&i : locks) {
		cout << "Manager::~Manager: " << i.first << " mutex deleted";
		delete i.second;
	}
	for (auto &&i : utilVec) {
		cout << "Manager::~Manager: " << i.first << " util vector deleted";
		delete i.second;
	}
	virConnectClose(conn);
	conn = NULL;
}

string Manager::startNewVm() {
	string name;
	try {
		VM *vm = new VM(conn);
		mutex *m = new mutex;
		vector<int> *vec = new vector<int>();

		name = vm->getName();
		cout << "Manager::startNewVm: Waiting for 30 secs for the VM to boot"
			 << endl;

		this_thread::sleep_for(chrono::seconds(30));
		domains.insert(make_pair(name, move(vm)));
		utilVec.insert(make_pair(name, vec));
		locks.insert(make_pair(name, m));
		notifyAboutServer();
	} catch (exception &e) {
		cout << e.what() << endl;
	}
	return name;
}

void Manager::startNewVm(const string &nameOfVm) {
	auto vec = VM::getInactiveDomainNames(conn);
	if (find(vec.begin(), vec.end(), nameOfVm) != vec.end()) {
		cerr << "Manager::startNewVm: no domain with name " << nameOfVm << endl;
	} else {
		try {
			VM *vm = new VM(conn, nameOfVm);
			mutex *m = new mutex;
			vector<int> *vec = new vector<int>();

			cout
				<< "Manager::startNewVm: Waiting for 30 secs for the VM to boot"
				<< endl;
			this_thread::sleep_for(chrono::seconds(30));
			domains.insert(make_pair(nameOfVm, move(vm)));
			utilVec.insert(make_pair(nameOfVm, vec));
			locks.insert(make_pair(nameOfVm, m));
			notifyAboutServer();
		} catch (exception &e) {
			cout << e.what() << endl;
		}
	}
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
		bool flag = true;
		long status = 0;
		double util = 0;
		long count = 0;
		VM *vm = domains.at(nameOfVm);
		while (flag) {
			count++;
			auto t = vm->getVmCpuUtil(conn);
			status = get<0>(t);
			if (status >= 4 and status <= 6) {
				cout << "Manager::watch: " << vm->getName() << " is powered off"
					 << endl;
				flag = false;
			}
			if (flag) {
				util = get<1>(t);
				cout << vm->getName() << " util: " << util << "%" << endl;
			}
		}
	});
}

thread *Manager::startWatching(const string &nameOfVm) {
	auto *th = new thread([this, nameOfVm]() { _watch(nameOfVm); });
	return th;
}

void Manager::shutdown(const string &nameOfVm) {
	VM *vm;
	mutex *m;
	vector<int> *vec;
	try {
		vm = domains.at(nameOfVm);
		m = locks.at(nameOfVm);
		vec = utilVec.at(nameOfVm);
		cout << "Deleting IP " << vm->getIp() << endl;
		_deleteIpFromFile(vm->getIp());
		vm->shutdown();
		delete vm;
		delete m;
		delete vec;
		domains.erase(nameOfVm);
		utilVec.erase(nameOfVm);
		locks.erase(nameOfVm);

	} catch (exception &e) {
		cout << e.what() << endl;
	}
}

void Manager::notifyAboutServer() {
	// cout << "Is domains vectorEmpty? :" << domains.empty() << endl;
	remove(ipFile.c_str());
	for (auto &&i : domains) {
		// cout << "Domain: " << i.second->getName() << endl;
		auto m = i.second->getInterfaceInfo();
		for (auto &&i : m) {
			// cout << "hwaddr: " << i.first << endl;
			for (auto &j : i.second) {
				// cout << "nwaddr: " << j << endl;
				_writeIpToFile(j);
			}
		}
	}
}

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

bool Manager::_deleteIpFromFile(const string &ip) {
	ifstream inputFile;
	ofstream outFile;
	inputFile.open(ipFile, ifstream::in);
	if (!inputFile.is_open()) {
		return false;
	}
	outFile.open(ipFile + "_temp", ofstream::out);
	if (!outFile.is_open()) {
		return false;
	}
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

size_t Manager::numActiveDomains() { return domains.size(); }