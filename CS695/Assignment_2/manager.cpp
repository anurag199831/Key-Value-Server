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
	for (std::thread &th : threadsOfDomains) {
		if (th.joinable()) th.join();
	}
	for (auto &i : domains) {
		delete i;
	}
	virConnectClose(conn);
	conn = NULL;
}

void Manager::startNewVm() {
	try {
		VM *vm = new VM(conn);
		domains.push_back(vm);
		watch(vm);
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
			watch(vm);
		} catch (exception &e) {
			cout << e.what() << endl;
		}
	}
}
void Manager::watch(VM *&vm) {
	thread th([vm, this]() {
		bool flag = true;
		long status = 0;
		double util = 0;
		long count = 0;
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
	threadsOfDomains.push_back(move(th));
}

void Manager::launch() {
	VM *vm = domains.at(0);
	thread th([vm, this]() {
		bool flag = true;
		long status = 0;
		double util = 0;
		long count = 0;
		while (flag) {
			cout << "count: " << count << " ";
			count++;
			auto t = vm->getVmCpuUtil(conn);
			status = get<0>(t);
			cout << "status: " << status << endl;
			if (status >= 4 and status <= 6) {
				cout << "Manager::launch: VM is powered off" << endl;
				flag = false;
			}
			util = get<1>(t);
			cout << vm->getName() << " util: " << util << "%" << endl;
		}
	});
}
