#include <libvirt/libvirt.h>

#include <iostream>
#include <vector>

#include "VM.cpp"

using namespace std;

int main(int argc, char* argv[]) {
	virConnectPtr conn;
	conn = virConnectOpen("qemu:///system");
	if (conn == NULL) {
		cerr << "Failed to open connection to qemu:///system\n";
		return 1;
	}
	try {
		VM* dom = new VM(conn);
	} catch (exception& e) {
		cout << e.what() << endl;
	}

	virConnectClose(conn);
}