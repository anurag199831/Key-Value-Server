#include <fstream>
#include <iostream>
#include <regex>
#include <vector>

using namespace std;

class KVClientLibrary {
   private:
    const regex pattern;
    const regex errorPattern;

    string getXML(const string key, const int dest_port) {
        string s =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?><Destination port=" +
            to_string(dest_port) +
            "><KVMessage "
            "type=\"getreq\"><Key>" +
            key + "</Key></KVMessage>";
        return s;
    }

    string putXML(const string key, const string value, const int dest_port) {
        string s =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?><Destination port=" +
            to_string(dest_port) +
            "><KVMessage "
            "type=\"putreq\"><Key>" +
            key + "</Key><Value>" + value + "</Value></KVMessage>";
        return s;
    }

    string deleteXML(const int dest_port, const string key) {
        string s =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?><Destination port=" +
            to_string(dest_port) +
            "><KVMessage "
            "type=\"delreq\"><Key>" +
            key + "</Key></KVMessage>";
        return s;
    }

   public:
    // const static string delimiter = ",";

    KVClientLibrary()
        : pattern("<Key>(.*?)</Key><Value>(.*?)</Value>"),
          errorPattern("<Message>(.*?)</Message>") {}

    string convertToXML(const int port, const string func, const string key,
                        const string value = "") {
        if (func == "get" or func == "GET") {
            return getXML(key, port);
        } else if (func == "put" or func == "PUT") {
            return putXML(key, value, port);
        } else if (func == "del" or func == "DEL") {
            return deleteXML(port, key);
        }
        return "";
    }

    vector<string> tokenize(const string str) {
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

    vector<string> parseXML(string xml) {
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
};