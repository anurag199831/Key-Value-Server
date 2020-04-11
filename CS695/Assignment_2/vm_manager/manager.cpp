#include "manager.h"

#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
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
		delete i.second;
	}
	virConnectClose(conn);
	conn = NULL;
}

string Manager::startNewVm() {
	string name;
	try {
		VM *vm = new VM(conn);
		name = vm->getName();
		domains.insert(make_pair(name, move(vm)));
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
			cout
				<< "Manager::startNewVm: Waiting for 30 secs for the VM to boot"
				<< endl;
			this_thread::sleep_for(chrono::seconds(30));
			domains.insert(make_pair(nameOfVm, move(vm)));
		} catch (exception &e) {
			cout << e.what() << endl;
		}
	}
}

void Manager::watch(string nameOfVm) {
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
	threadWorker.add([this](const string &nameOfVm) {
		VM *vm = domains.at(nameOfVm);
		domains.erase(nameOfVm);
		delete vm;
	});
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

thread *Manager::launch(const string &nameOfVm) {
	auto *th = new thread([this, nameOfVm]() { watch(nameOfVm); });
	return th;
}

void Manager::shutdown(const string &nameOfVm) {
	VM *vm;
	try {
		vm = domains.at(nameOfVm);
		cout << "Deleteing IP " << vm->getIp() << endl;
		deleteIpFromFile(vm->getIp());
		vm->shutdown();
	} catch (exception &e) {
		cout << e.what() << endl;
	}
}

void Manager::debugInfo() {
	cout << "Is domains vectorEmpty? :" << domains.empty() << endl;
	for (auto &&i : domains) {
		cout << "Domain: " << i.second->getName() << endl;
		auto m = i.second->getInterfaceInfo();
		for (auto &&i : m) {
			cout << "hwaddr: " << i.first << endl;
			for (auto &j : i.second) {
				cout << "nwaddr: " << j << endl;
				writeIpToFile(j);
			}
		}
	}
}

bool Manager::writeIpToFile(const string &ip) {
	ofstream serverfile;
	serverfile.open(ipFile, fstream ::out | fstream::app);
	if (!serverfile.is_open()) {
		cerr << "Manager::writeToFile: Error opening file " << ipFile << endl;
		return false;
	}

	serverfile << ip << endl;
	serverfile.close();
	return true;
}

bool Manager::deleteIpFromFile(const string &ip) {
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
	remove(ipFile.c_str());
	rename((ipFile + "_temp").c_str(), ipFile.c_str());
	return true;
}