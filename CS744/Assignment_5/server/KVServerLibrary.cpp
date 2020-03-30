#include <iostream>
#include <map>
#include <regex>
using namespace std;

class KVServerResponseFormatter {
   private:
    const regex requestPattern, contactRequestPattern, contactResponsePattern;
    string putResponse() {
        string s =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?><KVMessage "
            "type=\"resp\"><Message>Success</Message></KVMessage>";
        return s;
    }
    string getResponse(string key, string value) {
        string s = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
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
        : requestPattern(
              "<Destination "
              "port=(.*?)>.*?type=\"(.*?)\"><Key>(.*?)</Key>(<Value>(.*?)</"
              "Value>)?"),
          contactRequestPattern(
              "<source port=(.*?)><operation "
              "type=\"(.*?)\" node=(.*?)>"),
          contactResponsePattern(
              "<source port=(.*?)><operation "
              "type=\"(.*?)\" "
              "node=(.*?)><response port=(.*?) "
              "nodeId=(.*?)>") {}
    ~KVServerResponseFormatter() {}

    string makeContactMessage(const size_t port, const string op_type,
                              const string node) {
        string s = "<ContactRequest><source port=" + to_string(port) +
                   "><operation type=\"" + op_type + "\" node=" + node +
                   "></ContactRequest>";
        return s;
    }

    string makeContactResponse(const size_t source, const string op_type,
                               const string node, const string resp_port,
                               const string resp_nodeId) {
        string s = "<ContactResponse><source port=" + to_string(source) +
                   "><operation type=\"" + op_type + "\" node=" + node + ">" +
                   "<response port=" + resp_port + " nodeId=" + resp_nodeId +
                   "></ContactResponse>";
        return s;
    }

    map<string, string> parseContactRequest(const string xml) {
        map<string, string> m;
        smatch match;
        // cout << xml << endl;
        if (regex_search(xml, match, contactRequestPattern)) {
            if (match.str(1).length() > 0) {
                m["source"] = match.str(1);
            }
            if (match.str(2).length() > 0) {
                m["op_type"] = match.str(2);
            }
            if (match.str(3).length() > 0) {
                m["nodeId"] = match.str(3);
            }
        } else {
            cerr << "Error parsing XML\n";
        }
        return m;
    }

    map<string, string> parseContactResponse(const string xml) {
        map<string, string> m;
        smatch match;
        if (regex_search(xml, match, contactResponsePattern)) {
            if (match.str(1).length() > 0) {
                m["source"] = match.str(1);
            }
            if (match.str(2).length() > 0) {
                m["op_type"] = match.str(2);
            }
            if (match.str(3).length() > 0) {
                m["nodeId"] = match.str(3);
            }
            if (match.str(4).length() > 0) {
                m["resp_port"] = match.str(4);
            }
            if (match.str(5).length() > 0) {
                m["resp_nodeId"] = match.str(5);
            }
        } else {
            cerr << "Error parsing XML\n";
        }
        return m;
    }

    string getMessage(string msg) {
        string s =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?><KVMessage "
            "type=\"resp\"><Message>" +
            msg + "</Message></KVMessage>";
        return s;
    }

    string convertToXML(const string func, const string key,
                        string value = "") {
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

    vector<string> parseXML(string xml) {
        vector<string> vec;
        smatch match;
        string type, key, value;
        if (regex_search(xml, match, requestPattern)) {
            // int len = match.size();
            // for (size_t i = 0; i < len; i++) {
            //     cout << match.str(i) << endl;
            // }

            if (match.str(1).length() > 0) {
                vec.push_back(match.str(1));
            }
            type = match.str(2);
            if (type.length() > 3) {
                vec.push_back(type.substr(0, 3));
            }
            if (match.str(3).length() > 0) {
                vec.push_back(match.str(3));
            }
            if (type == "putreq") {
                if (match.str(5).length() > 0) {
                    vec.push_back(match.str(5));
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
    ~KVStoreFormatter() {}

    string getKVPair(string key, string value) {
        return "<KVPair><Key>" + key + "</Key><Value>" + value +
               "</Value></KVPair>";
    }
    vector<string> parseStoreEntry(string line) {
        vector<string> vec;
        smatch match;
        if (regex_search(line, match, pattern)) {
            vec.push_back(match.str(1));
            vec.push_back(match.str(2));
        }
        return vec;
    }
};