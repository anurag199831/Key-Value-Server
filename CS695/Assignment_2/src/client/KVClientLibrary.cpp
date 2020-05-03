#include <iostream>
#include <regex>
#include <vector>

using namespace std;

class KVClientFormatter {
   private:
	const regex pattern;
	const regex errorPattern;

	string getXML(const string& key) {
		string s =
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?><KVMessage "
			"type=\"getreq\"><Key>" +
			key + "</Key></KVMessage>";
		return s;
	}

	string putXML(const string& key, const string& value) {
		string s =
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?><KVMessage "
			"type=\"putreq\"><Key>" +
			key + "</Key><Value>" + value + "</Value></KVMessage>";
		return s;
	}

	string deleteXML(const string& key) {
		string s =
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?><KVMessage "
			"type=\"delreq\"><Key>" +
			key + "</Key></KVMessage>";
		return s;
	}

   public:
	KVClientFormatter()
		: pattern("<Key>(.*?)</Key><Value>(.*?)</Value>"),
		  errorPattern("<Message>(.*?)</Message>") {}

	string convertToXML(const string& func, const string& key,
						const string& value = "") {
		if (func == "get" or func == "GET") {
			return getXML(key);
		} else if (func == "put" or func == "PUT") {
			return putXML(key, value);
		} else if (func == "del" or func == "DEL") {
			return deleteXML(key);
		}
		return "";
	}

	vector<string> tokenize(const string& str) {
		string input = str;
		vector<string> vec;
		const string delimiter = ",";
		size_t pos = 0;
		while ((pos = input.find(delimiter)) != string::npos) {
			vec.push_back(input.substr(0, pos));
			input.erase(0, pos + delimiter.length());
		}
		if (input.length() != 0) {
			vec.push_back(input);
		}
		return vec;
	}

	vector<string> parseXML(const string& xml) {
		vector<string> vec;
		smatch match;
		string type, key, value, message;
		if (regex_search(xml, match, pattern)) {
			key = match.str(1);
			value = match.str(2);
			vec.push_back(key);
			vec.push_back(value);
		} else if (regex_search(xml, match, errorPattern)) {
			message = match.str(1);
			vec.push_back(message);
		} else {
			// cerr << "Error parsing XML\n";
		}
		return vec;
	}

	bool validateIp(const string& str) {
		regex pattern(
			R"(\b([[:digit:]]+)\.([[:digit:]]+).([[:digit:]]+).([[:digit:]]+)\b)");
		smatch match;
		int octet = -1;
		if (regex_search(str, match, pattern)) {
			if (match.size() == 5) {
				for (size_t i = 1; i < match.size(); i++) {
					octet = atol(match.str().c_str());
					if (octet > 255 and octet < 0) {
						return false;
					}
				}
			}
		}
		return true;
	}
};