#include <libvirt/libvirt.h>

#include <iostream>
#include <vector>

#include "vm.cpp"

using namespace std;

class Manager {
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
	virConnectPtr conn;
	vector<VM*> domains;
	vector<thread> threadsOfDomains;

   public:
	Manager();
	~Manager();
	void startNewVm();
	void startNewVm(string name);
	void watch(VM*& vm);
	void launch();
};