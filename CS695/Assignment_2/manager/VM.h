//
// Created by pranav on 18/04/20.
//

#ifndef ASSIGNMENT_2_VM_H
#define ASSIGNMENT_2_VM_H

#include <libvirt/libvirt.h>

#include <unordered_map>
#include <vector>

using namespace std;

class VM {
private:
	virDomainPtr domPtr;

	// Inline private member functions
	static inline unordered_map<string, string> _getDomainStatRecordMap(
			const virDomainStatsRecordPtr& record);
	static inline string _getTypedParamValue(const virTypedParameterPtr& item);

	// private member functions
	static double _convertStatMapToUtil(const unordered_map<string, string>& map);
	static long _getVmStateFromMap(const unordered_map<string, string>& map);
	unordered_map<string, string> _getStatsforDomain(const virConnectPtr& conn);

public:
	VM(const virConnectPtr& connPtr, const string& name);
	explicit VM(const virConnectPtr& connPtr);
	~VM();

	// member functions
	string getName();
	bool powerOn();
	bool shutdown();
	string getIp();
	bool isPoweredOn();
	unordered_map<string, vector<string>> getInterfaceInfo();
	tuple<long, double> getVmCpuUtil(const virConnectPtr& conn);

	// static functions
	static vector<string> getAllDefinedDomainNames(const virConnectPtr& conn);
	static vector<string> getInactiveDomainNames(const virConnectPtr& conn);
};


#endif //ASSIGNMENT_2_VM_H
