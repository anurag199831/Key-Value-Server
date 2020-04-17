#include "vm.h"

#include <algorithm>
#include <iostream>
#include <thread>

using namespace std;

// Constructor to create a VM object and start a VM with the given name.
VM::VM(const virConnectPtr &connPtr, const string &name) {
    if (connPtr == NULL) throw invalid_argument("Invalid connection object\n");
    vector<string> names = getInactiveDomainNames(connPtr);
    if (find(names.begin(), names.end(), name) == names.end()) {
        throw invalid_argument(
            "VM::VM(): no inactive VM found with name=" + name + "\n");
    }
    domPtr = virDomainLookupByName(connPtr, name.c_str());
    if (domPtr == NULL) {
        throw runtime_error("VM::VM(): call failed to virDomainLookupByName\n");
    }
    if (virDomainCreate(domPtr) < 0) {
        throw runtime_error(
            ("VM::startAnyInactiveDomain: Unable to boot guest "
             "configuration for" +
             name));
    }
}

// Constructor to create a VM object and start any random vm.
VM::VM(const virConnectPtr &conn) {
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
        if (domPtr == NULL) {
            throw runtime_error(
                "VM::VM(): call failed to virDomainLookupByName\n");
        }
        if (virDomainCreate(dom) < 0) {
            throw runtime_error(
                ("VM::startAnyInactiveDomain: Unable to boot guest "
                 "configuration for" +
                 inactiveDomains.at(0)));
        }
    }
    domPtr = dom;
}

// Destructor for VM objects
VM::~VM() {
    virDomainFree(domPtr);
    domPtr = NULL;
}

// Returns back the string representation of the item value
// or returns empty string if invalid item passed.
string VM::_getTypedParamValue(const virTypedParameterPtr &item) {
    string str;
    if (item == NULL) {
        cerr << "VM::_getTypedParamValue : NULL item passed" << endl;
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
            cerr << "VM::_getTypedParamValue: unimplemented parameter type "
                 << item->type << endl;
    }
    return str;
}

// Returns the statics for domain in a unordered map given the record pointer.
// Returns an empty map if invalid records are passed.
unordered_map<string, string> VM::_getDomainStatRecordMap(
    const virDomainStatsRecordPtr &record) {
    unordered_map<string, string> map;
    string param;
    if (record == NULL) {
        cerr << "VM::getDomainStatRecord: NULL record passed" << endl;
        return map;
    }
    map["domain.name"] = string(virDomainGetName(record->dom));
    for (int i = 0; i < record->nparams; i++) {
        param = _getTypedParamValue(record->params + i);
        if (!param.empty()) map[string(record->params[i].field)] = param;
    }
    return map;
}

// Returns a vector of strings representing the names of the inactive vms.
// Returns empty vector if no inactive VMs found.
vector<string> VM::getInactiveDomainNames(const virConnectPtr &conn) {
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

// Returns a stat map for the given domain.
// Returns empty map otherwise.
unordered_map<string, string> VM::_getStatsforDomain(
    const virConnectPtr &conn) {
    unsigned int statFlag = VIR_DOMAIN_STATS_VCPU | VIR_DOMAIN_STATS_CPU_TOTAL |
                            VIR_DOMAIN_STATS_STATE;

    unordered_map<string, string> currMap, testMap;
    int status = 0;
    virDomainStatsRecordPtr *records = NULL;
    virDomainStatsRecordPtr *next = NULL;

    status = virConnectGetAllDomainStats(conn, statFlag, &records, 0);
    if (status == -1) {
        cerr << "VM::_getStatsforDomain: call to "
                "virConnectGetAllDomainStats failed"
             << endl;
    } else if (records == NULL) {
        cerr << "VM::_getStatsforDomain: no stat data returned from "
                "virConnectGetAllDomainStats"
             << endl;
    } else {
        next = records;
        while (*next) {
            testMap = _getDomainStatRecordMap(*next);
            if (!testMap.empty() and
                testMap.at("domain.name") == string(virDomainGetName(domPtr))) {
                currMap = testMap;
                break;
            }
            if (*(++next))
                ;
        }
        virDomainStatsRecordListFree(records);
        records = next = NULL;
    }
    return currMap;
}

// Shuts down the running VM
void VM::shutdown() {
    cout << "VM::shutdown: Shutting down :" << getName() << endl;
    virDomainShutdown(domPtr);
}

// Returns the name of the VM.
string VM::getName() {
    string name = virDomainGetName(domPtr);
    return name;
}

// Returns a enum value of type virDomainState.
long VM::_getVmStateFromMap(const unordered_map<string, string> &map) {
    if (map.empty()) {
        return 0;
    }
    auto state = map.find("state.state");
    if (state != map.end()) {
        return atol(state->second.c_str());
    }
    return 0;
}

// Returns the CPU utilization of the domain, given the stat map.
// Returns 0 otherwise.
double VM::_convertStatMapToUtil(const unordered_map<string, string> &map) {
    if (map.empty()) {
        return 0;
    }
    size_t vcpu_current = 0, vcpu_maximum = 0;
    auto n_curr = map.find("vcpu.current");
    auto n_max = map.find("vcpu.maximum");
    if (n_curr != map.end() and n_max != map.end()) {
        vcpu_current = atol(n_curr->second.c_str());
        vcpu_maximum = atol(n_max->second.c_str());
    } else {
        cerr << "ManageisPoweredOnr::convertStatMapToUtil: invalid stat map "
                "passed"
             << endl;
        return 0;
    }
    string cpu_name;
    unordered_map<string, string>::const_iterator iter;
    double cpu_util = 0;

    for (size_t i = 0; i < vcpu_maximum; i++) {
        cpu_name = "vcpu." + to_string(i);
        iter = map.find(cpu_name + ".state");
        if (iter != map.end() and iter->second == "1") {
            iter = map.find(cpu_name + ".time");
            if (iter != map.end()) cpu_util += atol(iter->second.c_str());
        }
    }
    return cpu_util / vcpu_current;
}

// Returns a tuple of (status,util) where status is of type virDomainState and
// util is of type double.
tuple<long, double> VM::getVmCpuUtil(const virConnectPtr &conn) {
    string domain_name = getName();
    auto prev_map = _getStatsforDomain(conn);
    double time_prev = _convertStatMapToUtil(prev_map);
    this_thread::sleep_for(chrono::milliseconds(1000));
    auto curr_map = _getStatsforDomain(conn);
    double time_curr = _convertStatMapToUtil(curr_map);
    double avg_util = 100 * (time_curr - time_prev) / (1000 * 1000 * 1000);
    avg_util = max(0.0, min(100.0, avg_util));
    return make_tuple(_getVmStateFromMap(curr_map), avg_util);
}

// Returns the mapping of hwaddr: nwaddrs as a map of (string,vector<string>).
// Returns empty otherwise.
unordered_map<string, vector<string>> VM::getInterfaceInfo() {
    int max_retry = 30;
    virDomainInterfacePtr *ifaces = NULL;
    unordered_map<string, vector<string>> map;
    int ifacesCount = virDomainInterfaceAddresses(
        domPtr, &ifaces, VIR_DOMAIN_INTERFACE_ADDRESSES_SRC_LEASE, 0);
    while ((ifacesCount == -1 or ifacesCount == 0) and max_retry != 0) {
        cout << "VM::getInterfaceInfo: "
             << "Retrying for " << 30 - max_retry << " time" << endl;
        cout << "VM::getInterfaceInfo: Waiting for VM to start" << endl;
        this_thread::sleep_for(chrono::seconds(2));
        ifacesCount = virDomainInterfaceAddresses(
            domPtr, &ifaces, VIR_DOMAIN_INTERFACE_ADDRESSES_SRC_LEASE, 0);
        max_retry--;
    }
    string hwaddr;
    vector<string> naddrs;
    for (int i = 0; i < ifacesCount; i++) {
        hwaddr = string(ifaces[i]->hwaddr);
        for (uint32_t j = 0; j < ifaces[i]->naddrs; j++) {
            if (ifaces[i]->addrs[j].type == 0) {
                naddrs.emplace_back(ifaces[i]->addrs[j].addr);
            };
        }
        map.emplace(hwaddr, naddrs);
        virDomainInterfaceFree(ifaces[i]);
    }
    return map;
}

// Returns the IP address of the VM
// Returns empty otherwise.
string VM::getIp() {
    auto m = getInterfaceInfo();
    if (m.empty()) {
        cerr << "VM::getIp: VM::getInterfaceInfo returned empty" << endl;
        return "";
    }
    for (auto &&i : m) {
        return i.second.front();
    }
    return "";
}

// Returns false when VM is crashed, being powered off or shut down
// Otherwise, returns true
bool VM::isPoweredOn() {
    int state, reason;
    if (virDomainGetState(domPtr, &state, &reason, 0) != 0) {
        cerr << "VM::isPoweredOn: determining the state for " << getName()
             << " failed" << endl;
        return false;
    }

    if (state == VIR_DOMAIN_NOSTATE)
        cout << "VM::isPoweredOn: " << getName() << " has no state" << endl;
    else if (state == VIR_DOMAIN_RUNNING)
        cout << "VM::isPoweredOn: " << getName() << " is running" << endl;
    else if (state == VIR_DOMAIN_BLOCKED)
        cout << "VM::isPoweredOn: " << getName() << " is blocked on resource"
             << endl;
    else if (state == VIR_DOMAIN_PAUSED)
        cout << "VM::isPoweredOn: " << getName() << " is paused by user"
             << endl;
    else if (state == VIR_DOMAIN_SHUTDOWN)
        cout << "VM::isPoweredOn: " << getName() << " is being shut down"
             << endl;
    else if (state == VIR_DOMAIN_SHUTOFF)
        cout << "VM::isPoweredOn: " << getName() << " is shut off" << endl;
    else if (state == VIR_DOMAIN_CRASHED)
        cout << "VM::isPoweredOn: " << getName() << " is crashed" << endl;
    else if (state == VIR_DOMAIN_PMSUSPENDED)
        cout << "VM::isPoweredOn: " << getName()
             << " is suspended by guest power "
                "management"
             << endl;
    if (state >= 4 and state <= 6) {
        return false;
    }
    return true;
}