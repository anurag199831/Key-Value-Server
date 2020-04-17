#include <libvirt/libvirt.h>

#include <unordered_map>
#include <vector>

using namespace std;

class VM {
   private:
	virDomainPtr domPtr;

	inline unordered_map<string, string> _getDomainStatRecordMap(
		const virDomainStatsRecordPtr& record);
	inline string _getTypedParamValue(const virTypedParameterPtr& item);

	double _convertStatMapToUtil(const unordered_map<string, string>& map);
	long _getVmStateFromMap(const unordered_map<string, string>& map);
	unordered_map<string, string> _getStatsforDomain(const virConnectPtr& conn);

   public:
	VM(const virConnectPtr& connPtr, const string& name);
	explicit VM(const virConnectPtr& connPtr);
	~VM();

	static vector<string> getInactiveDomainNames(const virConnectPtr& conn);

	string getName();
	void powerOn();
	void shutdown();
	string getIp();
	unordered_map<string, vector<string>> getInterfaceInfo();
	tuple<long, double> getVmCpuUtil(const virConnectPtr& conn);
	bool isPoweredOn();
};