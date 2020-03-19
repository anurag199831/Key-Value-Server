#include <libvirt/libvirt.h>

#include <iostream>
#include <vector>
using namespace std;

vector<string> getInactiveDomainNames(virConnectPtr& conn) {
	vector<string> names;
	int numDomains = virConnectNumOfDefinedDomains(conn);
	if (numDomains == -1) {
		cerr << "No Inactive domains found" << endl;
		return names;
	}
	names.reserve(numDomains);
	char* inactiveDomainsNames[numDomains];
	numDomains =
		virConnectListDefinedDomains(conn, inactiveDomainsNames, numDomains);

	for (int i = 0; i < numDomains; i++) {
		// cout << string(inactiveDomainsNames[i]) << endl;
		names.push_back(string(inactiveDomainsNames[i]));
	}
	return names;
}

bool startAnyInactiveDomain(virConnectPtr& conn) {
	vector<string> inactiveDomains = getInactiveDomainNames(conn);
	if (inactiveDomains.size() == 0) {
		return false;
	} else {
		virDomainPtr dom =
			virDomainLookupByName(conn, inactiveDomains.at(0).c_str());
		virDomainCreate(dom);
	}
	return true;
}