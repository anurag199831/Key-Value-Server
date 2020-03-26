#include "manager.h"

#include <algorithm>
#include <thread>
#include <unordered_set>

Manager::Manager() {
	conn = virConnectOpen("qemu:///system");
	if (conn == NULL) {
		throw runtime_error("Failed to open connection to qemu:///system");
	}
}

Manager::~Manager() {
	for (auto &i : domains) {
		delete i;
	}
	virConnectClose(conn);
}

void Manager::startNewVm() {
	try {
		VM *vm = new VM(conn);
		domains.push_back(vm);
	} catch (exception &e) {
		cout << e.what() << endl;
	}
}

void Manager::startNewVm(string name) {
	auto vec = VM::getInactiveDomainNames(conn);
	if (find(vec.begin(), vec.end(), name) != vec.end()) {
		cerr << "Manager::startNewVm: no domain with name " << name << endl;
	} else {
		try {
			VM *vm = new VM(conn, name);
			domains.push_back(vm);
		} catch (exception &e) {
			cout << e.what() << endl;
		}
	}
}

void Manager::launch() {
	VM *vm = domains.at(0);
	thread th([vm, this]() {
		while (true) {
			vm->printUtil(conn);
		}
	});
	th.join();
}
