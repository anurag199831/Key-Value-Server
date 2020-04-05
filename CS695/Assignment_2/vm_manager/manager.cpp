#include "manager.h"

#include <algorithm>
#include <functional>
#include <stack>
#include <thread>
#include <utility>

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
		cout << "Cleanup code called for " << nameOfVm << endl;
		VM *vm = domains.at(nameOfVm);
		domains.erase(nameOfVm);
		delete vm;
	});
	threadWorker.add([this](const string &nameOfVm) {
		cout << "Looping code called for " << nameOfVm << endl;
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
			}
		}
	}
}