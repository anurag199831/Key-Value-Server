#include <libvirt/libvirt.h>

#include <unordered_map>
#include <vector>

using namespace std;

class VM {
   private:
	virDomainPtr domPtr;

	// Inline private member functions
	inline unordered_map<string, string> _getDomainStatRecordMap(
		const virDomainStatsRecordPtr& record);
	inline string _getTypedParamValue(const virTypedParameterPtr& item);

	// private member functions
	double _convertStatMapToUtil(const unordered_map<string, string>& map);
	long _getVmStateFromMap(const unordered_map<string, string>& map);
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