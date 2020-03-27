#include <libvirt/libvirt.h>

#include <unordered_map>
#include <vector>

using namespace std;

class VM {
   private:
	class failed_call_exception : public exception {
	   private:
		string msg;

	   public:
		failed_call_exception(const char* calling_function,
							  const char* failing_function) {
			this->msg = string(calling_function) + string(": failed call to ") +
						string(failing_function);
		}
		~failed_call_exception() final = default;
		const char* what() { return msg.c_str(); }
	};

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
	long getVmState(const unordered_map<string, string>& map);
	unordered_map<string, string> getStatsforDomain(const virConnectPtr& conn);
	double convertStatMapToUtil(const unordered_map<string, string>& map);
	tuple<long, double> getVmCpuUtil(const virConnectPtr& conn);
};