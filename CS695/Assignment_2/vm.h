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

	unordered_map<string, string> getDomainStatRecord(
		virDomainStatsRecordPtr record);
	void printMap(unordered_map<string, string>& map);

	inline string GetTypedParamValue(virTypedParameterPtr item);

   public:
	VM(virConnectPtr& connPtr, string& name);
	VM(virConnectPtr& connPtr);
	~VM();

	static vector<string> getInactiveDomainNames(virConnectPtr& conn);
	void getStatsforDomain(virConnectPtr& conn);
};