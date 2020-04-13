#include <cstdio>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

#include "KVServerLibrary.cpp"

using namespace std;

class KVStore {
   private:
	const string pathToDatabase = "database/";
	const size_t maxNumberOfFiles;
	KVStoreFormatter formatter;

   public:
	KVStore(string  path, const size_t maxNumberOfFiles)
		: pathToDatabase(std::move(path)), maxNumberOfFiles(maxNumberOfFiles) {}

	explicit KVStore(const size_t maxNumberOfFiles)
		: maxNumberOfFiles(maxNumberOfFiles) {}

	KVStore(const KVStore& obj)
		: pathToDatabase(obj.pathToDatabase),
		  maxNumberOfFiles(obj.maxNumberOfFiles) {}

	~KVStore() = default;

	void writeToFile(size_t fileID, const string& key, const string& value) {
		string fileName = to_string(fileID);
		fstream outFile;
		outFile.open(pathToDatabase + fileName, fstream::out);
		if (!outFile.is_open()) {
			cerr << "IO Error\n";
		}
		outFile << "<KVStore>\n";
		outFile << formatter.getKVPair(key, value) << '\n';
		outFile << "</KVStore>\n";
		outFile.close();
	}

	string findInFile(size_t fileID, const string& key) {
		string fileName = pathToDatabase + to_string(fileID);
		ifstream inFile;
		inFile.open(fileName);
		if (!inFile.is_open()) {
			// cerr << "findInFile: File does'nt exist\n";
			return "";
		}
		string line;
		while (!inFile.eof()) {
			inFile >> line;
			if (line.find("<KVStore>") != string::npos) {
			} else if (line.find("</KVStore>") != string::npos) {
			} else {
				vector<string> vec = formatter.parseStoreEntry(line);
				if (vec.size() != 2) {
					cout << "KVStore: Error parsing XML entry" << endl;
				} else if (key == vec.at(0)) {
					return vec.at(1);
				}
			}
		}
		inFile.close();
		return "";
	}

	bool updateInFile(size_t fileID, const string& key, const string& value,
					  bool put) {
		string fileName = pathToDatabase + to_string(fileID);
		ifstream inFile;
		ofstream outFile;
		bool kvflag = false;
		bool delFlag = false;
		inFile.open(fileName, ifstream::in);
		if (!inFile.is_open()) {
			writeToFile(fileID, key, value);
			inFile.close();
			return false;
		}
		outFile.open(fileName + "_temp", ofstream::out);
		if (!outFile.is_open()) {
			cout << "updateInFile: outFile not opened\n";
			exit(EXIT_FAILURE);
		}
		string line;
		size_t pos = 0;
		bool flag = true;
		while ((inFile >> line)) {
			if (line.find("<KVStore>") != string::npos) {
				outFile << line << '\n';
			} else if (line.find("</KVStore>") != string::npos) {
				kvflag = true;
			} else {
				vector<string> vec = formatter.parseStoreEntry(line);
				if (vec.size() != 2) {
					cout << "KVStore: Error parsing store entry" << endl;
				} else if (key != vec.at(0)) {
					outFile << line << '\n';
				} else {
					if (put) {
						outFile << formatter.getKVPair(key, value) << '\n';
						flag = false;
					} else {
						delFlag = true;
					}
				}
			}
		}
		if (put and flag) {
			outFile << formatter.getKVPair(key, value) << '\n';
		}
		if (kvflag) {
			outFile << "</KVStore>\n";
		}
		inFile.close();
		outFile.close();
		if (remove(fileName.c_str()) != 0) {
			cerr << "KVStore: Error deleting file\n";
		} else {
			if (rename((fileName + "_temp").c_str(), fileName.c_str())) {
				cerr << "KVStore: Error renaming file file\n";
			}
		}
		return delFlag;
	}


	vector<pair<string, string>> restoreFromFile(const size_t fileId,
												 const size_t numberOfEntries) {
		vector<pair<string, string>> vec;
		ifstream inFile;
		inFile.open(pathToDatabase + to_string(fileId), ifstream::in);
		if (!inFile.is_open()) {
			return vec;
		}
		string line;
		vector<string> items;

		while ((inFile >> line)) {
			if (line.find("<KVStore>") != string::npos) {
			} else if (line.find("</KVStore>") != string::npos) {
			} else {
				items = formatter.parseStoreEntry(line);
				vec.emplace_back(items.at(0), items.at(1));
				if (vec.size() == numberOfEntries) {
					break;
				}
			}
		}
		inFile.close();
		return vec;
	}
};

