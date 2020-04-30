#include "Manager.h"

#include <fcntl.h>

#include <cstdio>
#include <fstream>
#include <functional>
#include <stack>
#include <thread>
#include <utility>
#include <vector>

Manager::Manager() : loadHandlerTerminationFlag(false) {
	conn = virConnectOpen("qemu:///system");
	if (conn == NULL) {
		throw runtime_error("Failed to open connection to qemu:///system");
	}

	loadHandlerThread = new thread([this] {
		size_t count = 0;
		while (true) {
			// Checking if termination of this thread is called for
			{
				std::lock_guard lck(loadHandlerMutex);
				if (loadHandlerTerminationFlag) {
					std::cout << "Manager::loadHandlerThread: destroying "
								 "load handler thread"
							  << std::endl;
					break;
				}
			}

			// Invoke the load checking every 45 seconds
			if (count > 45) {
				cout << "Manager::loadHandlerThread: Checking for load" << endl;
				bool overloadedServer = true;
				string toShutdown;
				for (auto &&lck : loadLocks) {
					auto it = load.find(lck.first);
					if (it == load.end()) {
						std::cout << "Manager::loadHandlerThread: no load stat "
									 "found for "
								  << lck.first << std::endl;
					} else {
						lock_guard l(*lck.second);
						overloadedServer =
							overloadedServer and get<3>(it->second);
						if (get<4>(it->second)) { toShutdown = it->first; }
					}
				}

				if (overloadedServer) {
					auto name = startNewVm();
					if (not name.empty()) {
						// Resetting all the stat data before starting a new VM
						for (auto &&lck : loadLocks) {
							auto it = load.find(lck.first);
							if (it == load.end()) {
								std::cout << "Manager::loadHandlerThread: no "
											 "load stat "
											 "found for "
										  << lck.first << std::endl;
							} else {
								lock_guard l(*lck.second);
								get<0>(it->second) = 0;
								get<1>(it->second) = 0;
								get<2>(it->second) = 0;
								get<3>(it->second) = false;
								get<4>(it->second) = false;
							}
						}

						cout << "Manager::loadHandlerThread: Powering on "
							 << name << " due to overload." << endl;
						powerOn(name);
						startWatching(name);
					}
				} else if (not toShutdown.empty()) {
					// Resetting all the stat data before shutting down a VM
					for (auto &&lck : loadLocks) {
						auto it = load.find(lck.first);
						if (it == load.end()) {
							std::cout
								<< "Manager::loadHandlerThread: no load stat "
								   "found for "
								<< lck.first << std::endl;
						} else {
							lock_guard l(*lck.second);
							get<0>(it->second) = 0;
							get<1>(it->second) = 0;
							get<2>(it->second) = 0;
							get<3>(it->second) = false;
							get<4>(it->second) = false;
						}
					}

					if (domains.size() > 1) {
						cout << "Manager::loadHandlerThread: Shutting down "
							 << toShutdown << "for inactivity" << endl;
						shutdown(toShutdown);
					}
				}

				count = 0;
			}
			this_thread::sleep_for(chrono::seconds(1));
			count++;
		}
	});
}

Manager::~Manager() {
	{
		std::lock_guard lck(loadHandlerMutex);
		loadHandlerTerminationFlag = true;
	}

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

	try {
		remove(ipFile.c_str());
	} catch (exception &e) { cout << "Manager::~Manager" << e.what() << endl; }

	for (auto &i : utilThreads) {
		if (i != nullptr) {
			i->join();
			delete i;
			cout << "Manager:~Manager: utilThread destroyed" << endl;
		}
	}

	if (loadHandlerThread != nullptr) {
		loadHandlerThread->join();
		delete loadHandlerThread;
		cout << "Manager:~Manager: loadHandlerThread destroyed" << endl;
	}

	for (auto &&i : domains) {
		cout << "Manager::~Manager: " << i.first << " VM deleted" << endl;
		delete i.second;
	}

	for (auto &&i : locks) {
		cout << "Manager::~Manager: " << i.first << " util mutex deleted"
			 << endl;
		delete i.second;
	}

	for (auto &&i : loadLocks) {
		cout << "Manager::~Manager: " << i.first << " load mutex deleted"
			 << endl;
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
}

string Manager::startNewVm() {
	string name;
	try {
		VM *vm = new VM(conn);
		if (vm == nullptr or vm == NULL) { return name; }
		auto *m = new mutex;
		auto *n = new mutex;
		auto *o = new mutex;
		auto *lst = new list<int>();
		bool terminationFlag = true;
		auto t = tuple<int, int, int, bool, bool>(0, 0, 0, false, false);

		name = vm->getName();

		domains.insert(make_pair(name, vm));
		utilList.insert(make_pair(name, lst));
		threadTerminationFlags.insert(make_pair(name, terminationFlag));
		locks.insert(make_pair(name, m));
		threadTerminationLocks.insert(make_pair(name, n));
		loadLocks.insert(make_pair(name, o));
		load.insert(make_pair(name, t));
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
		auto *o = new mutex;
		auto *lst = new list<int>();
		bool terminationFlag = true;
		auto t = tuple<int, int, int, bool, bool>(0, 0, 0, false, false);

		domains.insert(make_pair(nameOfVm, vm));
		utilList.insert(make_pair(nameOfVm, lst));
		locks.insert(make_pair(nameOfVm, m));
		threadTerminationFlags.insert(make_pair(nameOfVm, terminationFlag));
		threadTerminationLocks.insert(make_pair(nameOfVm, n));
		loadLocks.insert(make_pair(nameOfVm, o));
		load.insert(make_pair(nameOfVm, t));
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
			// Update Load Data
			{
				auto lck = loadLocks.find(nameOfVm);
				if (lck == loadLocks.end()) {
					cerr << "Manager::_watch: No load lock found for "
						 << nameOfVm << endl;
				}
				auto loadStat = load.find(nameOfVm);
				if (loadStat == load.end()) {
					cerr << "Manager::_watch: No load tuple found for "
						 << nameOfVm << endl;
				} else {
					lock_guard l(*lck->second);
					if (util >= LOAD_OVERLOAD_THRESHOLD_PERCENT) {
						get<0>(loadStat->second) += 1;
						get<2>(loadStat->second) = 0;
						get<1>(loadStat->second) = 0;
						get<4>(loadStat->second) = false;
					} else if (util <= LOAD_IDLE_THRESHOLD_PERCENT) {
						get<1>(loadStat->second) += 1;
						if (get<1>(loadStat->second) > 2) {
							get<3>(loadStat->second) = false;
						}
						get<2>(loadStat->second) = 0;
						get<0>(loadStat->second) =
							max(0, get<0>(loadStat->second) - 2);
					} else if (util <= LOAD_RESET_THRESHOLD_PERCENT) {
						get<2>(loadStat->second) += 1;
						if (get<2>(loadStat->second) > 3) {
							get<3>(loadStat->second) = false;
						}
						get<1>(loadStat->second) = 0;
						get<0>(loadStat->second) =
							max(0, get<0>(loadStat->second) - 2);
					}
					if (get<2>(loadStat->second) >
						LOAD_RESET_THRESHOLD_TIME_IN_SECS) {
						// Totally Reset Stats
						get<0>(loadStat->second) = 0;
						get<1>(loadStat->second) = 0;
						get<2>(loadStat->second) = 0;
						get<3>(loadStat->second) = false;
						get<4>(loadStat->second) = false;
					} else if (get<0>(loadStat->second) >
							   LOAD_OVERLOAD_THRESHOLD_TIME_IN_SECS) {
						// Set as overloaded
						get<0>(loadStat->second) = 0;
						get<1>(loadStat->second) = 0;
						get<2>(loadStat->second) = 0;
						get<3>(loadStat->second) = true;
						get<4>(loadStat->second) = false;
					} else if (get<1>(loadStat->second) >
							   LOAD_IDLE_THRESHOLD_TIME_IN_SECS) {
						// Set as idle
						get<0>(loadStat->second) = 0;
						get<1>(loadStat->second) = 0;
						get<2>(loadStat->second) = 0;
						get<3>(loadStat->second) = false;
						get<4>(loadStat->second) = true;
					}
					cout << get<0>(loadStat->second) << " "
						 << get<1>(loadStat->second) << " "
						 << get<2>(loadStat->second) << " "
						 << get<3>(loadStat->second) << " "
						 << get<4>(loadStat->second) << " " << endl;
				}
			}
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
	utilThreads.push_back(th);
}

// Shutdown the VM with the name.
// Invalid name if passed has no effect.
void Manager::shutdown(const string &nameOfVm) {
	auto it = threadTerminationLocks.find(nameOfVm);
	if (it != threadTerminationLocks.end()) {
		lock_guard l(*it->second);
		auto iter = threadTerminationFlags.find(nameOfVm);
		if (iter != threadTerminationFlags.end()) {
			iter->second = true;
		} else {
			cerr << "Manager::shutdown: no thread termination flag for VM with "
					"name '"
				 << nameOfVm << "' found." << endl;
		}
	} else {
		cerr << "Manager::shutdown: no thread termination lock for VM with "
				"name '"
			 << nameOfVm << "' found." << endl;
		return;
	}

	try {
		auto vm = domains.at(nameOfVm);
		auto m = locks.at(nameOfVm);
		auto lst = utilList.at(nameOfVm);
		auto threadLock = threadTerminationLocks.at(nameOfVm);
		auto loadLock = loadLocks.at(nameOfVm);

		_deleteIpFromFile(vm->getIp());

		vm->shutdown();
		delete vm;
		delete m;
		delete lst;
		delete threadLock;
		delete loadLock;
		domains.erase(nameOfVm);
		utilList.erase(nameOfVm);
		locks.erase(nameOfVm);
		threadTerminationFlags.erase(nameOfVm);
		threadTerminationLocks.erase(nameOfVm);
		loadLocks.erase(nameOfVm);
		load.erase(nameOfVm);

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
// The name passed to the method should be a defined VMs name.
// On passing invalid name, no effect.
void Manager::powerOn(const string &nameOfVm) {
	auto it = domains.find(nameOfVm);
	if (it == domains.end()) {
		cerr << "Manager::powerOn: No active vm with name " << nameOfVm
			 << " found" << endl;
		return;
	}
	auto vm = it->second;
	if (not vm->isPoweredOn()) { vm->powerOn(); }
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

// Acquires the virDomainPtr over all the running VMs and starts tracking the
// CPU util as well.
void Manager::attachToAlreadyRunningVms() {
	auto runningVms = VM::getAllActiveDomainNames(conn);
	for (auto &&vm : runningVms) {
		startNewVm(vm);
		startWatching(vm);
	}
	notifyAboutServer();
}