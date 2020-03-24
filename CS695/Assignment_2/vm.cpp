#include "vm.h"

#include <algorithm>
#include <iostream>
#include <thread>

using namespace std;

VM::VM(virConnectPtr &connPtr, string &name) {
	if (connPtr == NULL) throw invalid_argument("Invalid connection object\n"s);
	vector<string> names = getInactiveDomainNames(connPtr);
	if (find(names.begin(), names.end(), name) == names.end()) {
		throw invalid_argument(
			"VM::VM(): no inactive VM found with name=" + name + "\n");
	}
	domPtr = virDomainLookupByName(connPtr, name.c_str());
	if (domPtr == NULL) {
		throw failed_call_exception("VM::VM()", "virDomainLookupByName");
	}
}

VM::VM(virConnectPtr &conn) {
	vector<string> inactiveDomains = getInactiveDomainNames(conn);
	virDomainPtr dom;
	if (inactiveDomains.empty()) {
		throw runtime_error(
			"VM::startAnyInactiveDomain: no inactive domain configuration "
			"found");
	} else {
		cout << "VM::startAnyInactiveDomain: starting domain with name "
			 << inactiveDomains.at(0) << endl;
		dom = virDomainLookupByName(conn, inactiveDomains.at(0).c_str());
		if (virDomainCreate(dom) < 0) {
			throw runtime_error(
				("VM::startAnyInactiveDomain: Unable to boot guest "
				 "configuration for" +
				 inactiveDomains.at(0)));
		}
	}
	domPtr = dom;
}

VM::~VM() { virDomainFree(domPtr); }

string VM::GetTypedParamValue(virTypedParameterPtr item) {
	string str;
	if (item == NULL) {
		cerr << "VM::GetTypedParamValue : NULL item passed" << endl;
		return str;
	}

	switch (item->type) {
		case VIR_TYPED_PARAM_INT:
			str = to_string(item->value.i);
			break;

		case VIR_TYPED_PARAM_UINT:
			str = to_string(item->value.ui);
			break;

		case VIR_TYPED_PARAM_LLONG:
			str = to_string(item->value.l);
			break;

		case VIR_TYPED_PARAM_ULLONG:
			str = to_string(item->value.ul);
			break;

		case VIR_TYPED_PARAM_DOUBLE:
			str = to_string(item->value.d);
			break;

		case VIR_TYPED_PARAM_BOOLEAN:
			if (item->value.b)
				str = "yes";
			else
				str = "no";
			break;

		case VIR_TYPED_PARAM_STRING:
			str = string(item->value.s);
			break;

		default:
			cerr << "unimplemented parameter type " << item->type << endl;
	}
	return str;
}

unordered_map<string, string> VM::getDomainStatRecord(
	virDomainStatsRecordPtr record) {
	unordered_map<string, string> map;
	string param;
	if (record == nullptr or record == NULL) {
		cerr << "VM::getDomainStatRecord: NULL record passed" << endl;
		return map;
	}
	map["domain.name"] = string(virDomainGetName(record->dom));
	for (int i = 0; i < record->nparams; i++) {
		param = GetTypedParamValue(record->params + i);
		if (!param.empty()) map[string(record->params[i].field)] = param;
	}
	return map;
}

vector<string> VM::getInactiveDomainNames(virConnectPtr &conn) {
	vector<string> names;
	int numDomains = virConnectNumOfDefinedDomains(conn);
	if (numDomains == -1) {
		return names;
	}
	names.reserve(numDomains);
	char *inactiveDomainsNames[numDomains];
	numDomains =
		virConnectListDefinedDomains(conn, inactiveDomainsNames, numDomains);
	for (int i = 0; i < numDomains; i++) {
		names.emplace_back(inactiveDomainsNames[i]);
	}
	return names;
}

void VM::getStatsforDomain(virConnectPtr &conn) {
	int statFlag = VIR_DOMAIN_STATS_VCPU | VIR_DOMAIN_STATS_CPU_TOTAL |
				   VIR_DOMAIN_STATS_STATE;
	unordered_map<string, string> currMap, prevMap, testMap;
	int status = 0;
	int count = 0;
	virDomainStatsRecordPtr *records = NULL;
	virDomainStatsRecordPtr *next = NULL;
	while (count < 100) {
		status = virConnectGetAllDomainStats(conn, statFlag, &records, 0);
		if (status == -1) {
			cerr << "VM::getStatsforDomain: call to "
					"virConnectGetAllDomainStats failed"
				 << endl;
		} else if (records == NULL) {
			cerr << "VM::getStatsforDomain: no stat data returned from "
					"virConnectGetAllDomainStats"
				 << endl;
		} else {
			next = records;
			while (*next) {
				testMap = getDomainStatRecord(*next);
				if (testMap.at("domain.name") ==
					string(virDomainGetName(domPtr))) {
					currMap = testMap;
				}
				if (*(++next))
					;
			}
			printMap(currMap);
			if (!prevMap.empty() and !currMap.empty()) {
				long diff_time = atol(currMap.at("vcpu.0.time").c_str()) -
								 atol(prevMap.at("vcpu.0.time").c_str());
				double util_vcpu = 100.0 * static_cast<double>(diff_time) /
								   (1000.0 * 1000.0 * 1000.0);
				cout << "VCPU Util: " << max(0.0, min(100.0, util_vcpu))
					 << endl;
			}
			prevMap = currMap;
			virDomainStatsRecordListFree(records);
			records = next = NULL;
		}
		count += 1;
		this_thread::sleep_for(chrono::milliseconds(1000));
	}
}

void VM::printMap(unordered_map<string, string> &map) {
	cout << "==================================" << endl;
	for (auto &&i : map) {
		cout << i.first << "=" << i.second << endl;
	}
}