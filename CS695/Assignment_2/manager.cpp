#include "manager.h"

#include <algorithm>
#include <functional>
#include <stack>
#include <thread>
#include <unordered_set>

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
		domains[name] = vm;
	} catch (exception &e) {
		cout << e.what() << endl;
	}
	return name;
}

void Manager::startNewVm(string name) {
	auto vec = VM::getInactiveDomainNames(conn);
	if (find(vec.begin(), vec.end(), name) != vec.end()) {
		cerr << "Manager::startNewVm: no domain with name " << name << endl;
	} else {
		try {
			VM *vm = new VM(conn, name);
			domains[name] = vm;
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
		Worker(string name) : nameOfVm(name) {}
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
	thread_local Worker threadWorker(nameOfVm);
	threadWorker.add([this](string nameOfVm) {
		cout << "Cleanup code called for " << nameOfVm << endl;
		VM *vm = domains.at(nameOfVm);
		domains.erase(nameOfVm);
		delete vm;
	});
	threadWorker.add([this](string nameOfVm) {
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

thread *Manager::launch(string name) {
	thread *th = new thread([this, name]() { watch(name); });
	cout << "Manager: launch: Testing this point" << endl;
	return th;
}

void Manager::debugInfo() {
	cout << "Is domains vectorEmpty? :" << domains.empty() << endl;
}