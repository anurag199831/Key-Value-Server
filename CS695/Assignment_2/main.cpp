#include <libvirt/libvirt.h>

#include <iostream>
#include <vector>

#include "manager.cpp"

using namespace std;

int main(int argc, char* argv[]) {
	Manager mgr;
	mgr.startNewVm();
	mgr.launch();
}