#include <libvirt/libvirt.h>

#include <iostream>
#include <thread>
#include <vector>

#include "manager.cpp"

using namespace std;

int main(int argc, char* argv[]) {
	Manager mgr;
	// thread *th1 = nullptr, *th2 = nullptr;
	// string name1 = mgr.startNewVm();
	// if (name1.size() != 0 and not name1.empty())
	// 	thread* th1 = mgr.startWatching(name1);
	// string name2 = mgr.startNewVm();
	// if (name2.size() != 0 and not name2.empty())
	// 	thread* th2 = mgr.startWatching(name2);
	// this_thread::sleep_for(chrono::seconds(10));
	// mgr.shutdown(name1);
	// if (th1 != nullptr and th1->joinable()) {
	// 	cout << "main: joining thread" << endl;
	// 	th1->join();
	// }
	// if (th1 != nullptr and th2->joinable()) {
	// 	cout << "main: joining thread" << endl;
	// 	th2->join();
	// }
	for (auto&& i : mgr.getAllDefinedDomainNames()) {
		cout << i << endl;
	}
}