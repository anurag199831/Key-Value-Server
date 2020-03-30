#include <functional>
#include <iostream>
#include <mutex>
#include "KVStore.cpp"

struct entry {
    string key;
    string value;
    bool isReferenced = false;
    bool valid = false;
};

class CacheSet {
   private:
    vector<struct entry> entries;
    const size_t noOfEntries;
    size_t evictionPointer = 0;
    const size_t setID;

    // NOTE: Type Code
    //    (!(-1),false) Hit
    //    (!(-1),true) Hole
    //    (-1, false) Miss
    pair<int, bool> getEntryID(string key) {
        int holeIndex = -1;
        bool type = false;
        bool firstFlag = false;
        for (size_t i = 0; i < noOfEntries; i++) {
            if (!firstFlag and entries.at(i).valid == false) {
                holeIndex = i;
                firstFlag = true;
                type = true;
            } else if (entries.at(i).key == key) {
                entries.at(i).valid = true;
                type = false;
                // cout << "KVCache: getEntryID: "
                //      << "Hit encountered" << endl;
                return make_pair<int, int>(i, type);
            }
        }
        if (holeIndex != -1) {
            entries.at(holeIndex).key = key;
            entries.at(holeIndex).valid = true;
            // cout << "KVCache: getEntryID: "
            //      << "Hole encountered" << endl;
        }
        return make_pair(holeIndex, type);
    }

   public:
    CacheSet(const size_t setID, const size_t noOfEntries,
             const vector<pair<string, string>> init)
        : setID(setID), noOfEntries(noOfEntries) {
        entries.resize(noOfEntries);
        for (size_t i = 0; i < init.size(); i++) {
            entries.at(i).isReferenced = false;
            entries.at(i).valid = true;
            entries.at(i).key = init.at(i).first;
            entries.at(i).value = init.at(i).second;
        }
    }

    ~CacheSet() {}

    void putEntry(KVStore &store, const string key, const string value,
                  bool updateOnlyInCache = false) {
        if (updateOnlyInCache) {
            // cout << "UPDATEONLYINCACHE: Called for key: " << key << endl;
        }
        pair<int, int> p = getEntryID(key);
        // cout << "putEntry: "
        //      << "index: " << p.first << " type: " << p.second << endl;
        if (p.first != -1 and p.second) {  // the cache entry was a hole
            entries.at(p.first).isReferenced = true;
            entries.at(p.first).value = value;
            if (!updateOnlyInCache) {
                store.updateInFile(setID, key, value, true);
            }
        } else if (p.first != -1 and !p.second) {
            entries.at(p.first).isReferenced = true;
            entries.at(p.first).value = value;
            if (!updateOnlyInCache) {
                store.updateInFile(setID, key, value, true);
            }
        } else {
            while (true) {
                if (entries.at(evictionPointer).isReferenced == true and
                    entries.at(evictionPointer).valid) {
                    entries.at(evictionPointer).isReferenced = false;
                    // cout << "Here: " << evictionPointer << endl;
                } else {
                    entries.at(evictionPointer).isReferenced = true;
                    entries.at(evictionPointer).valid = true;
                    entries.at(evictionPointer).key = key;
                    entries.at(evictionPointer).value = value;
                    evictionPointer = (++evictionPointer) % noOfEntries;
                    if (!updateOnlyInCache)
                        store.updateInFile(setID, key, value, true);
                    break;
                }
                evictionPointer = (++evictionPointer) % noOfEntries;
            }
        }
    }

    string getEntry(KVStore store, string key) {
        for (size_t i = 0; i < noOfEntries; i++) {
            if (entries.at(i).key == key and entries.at(i).valid) {
                entries.at(i).isReferenced = true;
                // cout << "KVCache: get serviced(from Cache): "
                // << entries.at(i).value << endl;
                return entries.at(i).value;
            }
        }
        string value = store.findInFile(setID, key);
        putEntry(store, key, value, true);
        // cout << "KVCache: get serviced: " << value << endl;
        return value;
    }

    bool deleteEntry(KVStore store, string key) {
        for (size_t i = 0; i < noOfEntries; i++) {
            if (entries.at(i).key == key) {
                entries.at(i).valid = false;
                entries.at(i).isReferenced = false;
                return store.updateInFile(setID, key, "", false);
            }
        }
        return store.updateInFile(setID, key, "", false);
    }

    void printcacheSet() {
        for (size_t i = 0; i < noOfEntries; i++) {
            if (entries.at(i).key == "") {
                continue;
            }
            cout << i << "\t" << entries.at(i).key << "\t"
                 << entries.at(i).value << "\t" << entries.at(i).isReferenced
                 << "\t" << entries.at(i).valid << endl;
        }
    }
};

class KVCache {
   private:
    KVStore store;
    mutex *locks;
    const size_t numberOfSets;
    const size_t entriesPerSet;
    vector<CacheSet> vec;
    hash<string> hasher;

    size_t getSetID(string key) { return hasher(key) % numberOfSets; }

   public:
    KVCache(const size_t numberOfSets, const size_t entriesPerSet)
        : numberOfSets(numberOfSets),
          entriesPerSet(entriesPerSet),
          store(numberOfSets) {
        locks = new mutex[numberOfSets];
        vec.reserve(numberOfSets);
        for (size_t i = 0; i < numberOfSets; i++) {
            vec.emplace_back(i, entriesPerSet,
                             store.restoreFromFile(i, entriesPerSet));
        }
        // printCache();
    }

    ~KVCache() { delete[] locks; }

    string operateOnCache(const string func, const string key,
                          const string value) {
        // cout << "operateOnCache: "
        //      << "func: " << func << " , "
        //      << "key: " << key << " , "
        //      << "value: " << value << endl;
        size_t setID = getSetID(key);
        unique_lock<mutex> lck(locks[setID]);
        // cout << "KVCache: operateIncache: setID: " << setID << "\n";
        if (func == "put") {
            //     cout << "KVCache: operateIncache: "
            //          << "putEntry\n";
            vec.at(setID).putEntry(store, key, value);
        } else if (func == "get") {
            // cout << "KVCache: operateIncache: "
            //      << "getEntry\n";
            return vec.at(setID).getEntry(store, key);
        } else if (func == "del") {
            // cout << "KVCache: operateIncache: "
            //      << "delEntry\n";
            if (vec.at(setID).deleteEntry(store, key)) {
                return "Success";
            } else {
                return "";
            }
        } else {
            cerr << "KVCache: Invalid function string\n";
        }
        return "";
    }

    void printCache() {
        for (size_t i = 0; i < numberOfSets; i++) {
            cout << "================"
                 << "Set " << i << "===================" << endl;
            vec.at(i).printcacheSet();
        }
    }
};

// int main(int argc, char const *argv[]) {
//     KVCache k(20, 32);
//     string func, key, value;
//     while (true) {
//         cout << "command: ";
//         cin >> func;
//         // cout << "func:" << func << endl;
//         if (func == "get") {
//             cin >> key;
//             // cout << "key:" << key << endl;
//             cout << k.operateOnCache(func, key, value) << "\n";
//         } else if (func == "put") {
//             cin >> key >> value;
//             // cout << "key:" << key << " value: " << value;
//             k.operateOnCache(func, key, value);
//         } else if (func == "del") {
//             cin >> key;
//             k.operateOnCache(func, key, value);
//         } else if (func == "print") {
//             k.printCache();
//         } else if (func == "exit") {
//             exit(EXIT_SUCCESS);
//         } else {
//             cout << "Invalid function\n";
//         }
//     }
// }
