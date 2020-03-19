#include <iostream>
#include <vector>

#include "helper.cpp"

using namespace std;

int main(int argc, char* argv[]) {
	virConnectPtr conn;

	conn = virConnectOpen("qemu:///system");
	if (conn == NULL) {
		cerr << "Failed to open connection to qemu:///system\n";
		return 1;
	}

	virConnectClose(conn);
}