#include <libvirt/libvirt.h>

#include <iostream>
#include <thread>
#include <vector>

#include "manager.cpp"

using namespace std;

int main(int argc, char* argv[]) {
	Manager mgr;
	string name1 = mgr.startNewVm();
	// thread* th1 = mgr.launch(name1);
	string name2 = mgr.startNewVm();
	// thread* th2 = mgr.launch(name2);

	mgr.debugInfo();
}