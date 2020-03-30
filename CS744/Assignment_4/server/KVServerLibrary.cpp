#include <iostream>
#include <regex>
using namespace std;

class KVServerResponseFormatter {
private:
	const regex pattern;
	string putResponse() {
		string s =
				"<?xml version=\"1.0\" encoding=\"UTF-8\"?><KVMessage "
				"type=\"resp\"><Message>Success</Message></KVMessage>";
		return s;
	}
	string getResponse(const string& key, const string& value) {
		string s = R"(<?xml version="1.0" encoding="UTF-8"?>)";
		s = s + "<KVMessage type=\"resp\"><Key>" + key + "</Key>";
		s = s + "<Value>" + value + "</Value></KVMessage>";
		return s;
	}
	string deleteResponse() {
		string s =
				"<?xml version=\"1.0\" encoding=\"UTF-8\"?><KVMessage "
				"type=\"resp\"><Message>Success</Message></KVMessage>";
		return s;
	}

public:
	KVServerResponseFormatter()
			: pattern("type=\"(.*?)\"><Key>(.*?)</Key>(<Value>(.*?)</Value>)?") {}
	~KVServerResponseFormatter() = default;

	string getMessage(const string& msg) {
		string s =
				"<?xml version=\"1.0\" encoding=\"UTF-8\"?><KVMessage "
				"type=\"resp\"><Message>" +
				msg + "</Message></KVMessage>";
		return s;
	}

	string convertToXML(const string& func, const string& key,
						const string& value = "") {
		if (func == "put") {
			return putResponse();
		} else if (func == "get") {
			if (value.length() > 0) {
				return getResponse(key, value);
			} else {
				return getMessage("Does not exist");
			}
		} else if (func == "del") {
			if (value.length() > 0) {
				return deleteResponse();
			} else {
				return getMessage("Does not exist");
			}
		} else {
			cerr << "KVServerResponseFormatter: convertToXML: Invalid "
					"Argument\n";
			return "";
		}
	}

	vector<string> parseXML(const string& xml) {
		vector<string> vec;
		smatch match;
		string type, key, value;
		if (regex_search(xml, match, pattern)) {
			type = match.str(1);
			if (type.length() > 3) {
				vec.push_back(type.substr(0, 3));
			}
			if (match.str(2).length() > 0) {
				vec.push_back(match.str(2));
			}
			if (type == "putreq") {
				if (match.str(4).length() > 0) {
					vec.push_back(match.str(4));
				}
			}
		} else {
			// cerr << "Error parsing XML\n";
		}
		return vec;
	}
};

class KVStoreFormatter {
private:
	const regex pattern;

public:
	KVStoreFormatter() : pattern("<Key>(.*?)</Key><Value>(.*?)</Value>") {}
	~KVStoreFormatter() = default;

	string getKVPair(const string& key, const string& value) {
		return "<KVPair><Key>" + key + "</Key><Value>" + value +
			   "</Value></KVPair>";
	}
	vector<string> parseStoreEntry(const string& line) {
		vector<string> vec;
		smatch match;
		if (regex_search(line, match, pattern)) {
			vec.push_back(match.str(1));
			vec.push_back(match.str(2));
		}
		return vec;
	}
};