#include <libvirt/libvirt.h>

#include <unordered_map>
#include <vector>

using namespace std;

class VM {
   private:
	virDomainPtr domPtr;

	inline unordered_map<string, string> getDomainStatRecordMap(
		const virDomainStatsRecordPtr& record);
	inline string GetTypedParamValue(const virTypedParameterPtr& item);

   public:
	VM(const virConnectPtr& connPtr, const string& name);
	VM(const virConnectPtr& connPtr);
	~VM();

	static vector<string> getInactiveDomainNames(const virConnectPtr& conn);

	string getName();
	void shutdown();
	string getIp();
	unordered_map<string, vector<string>> getInterfaceInfo();
	long getVmState(const unordered_map<string, string>& map);
	unordered_map<string, string> getStatsforDomain(const virConnectPtr& conn);
	double convertStatMapToUtil(const unordered_map<string, string>& map);
	tuple<long, double> getVmCpuUtil(const virConnectPtr& conn);
};