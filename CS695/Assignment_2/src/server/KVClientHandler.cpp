#include "ThreadPool.cpp"
using namespace std;

class KVClientHandler {
   private:
	ThreadPool pool;
	KVServerResponseFormatter formatter;

   public:
	KVClientHandler(size_t numberOfThreads, size_t numberOfSets,
					size_t entriesPerSet)
		: pool(numberOfThreads, numberOfSets, entriesPerSet) {}
	~KVClientHandler() = default;

	pair<int, string> handle(int fd, const string& xml) {
		pair<int, string> output;
		// cout << xml << endl;
		output.first = fd;
		vector<string> vec = formatter.parseXML(xml);
		// for (auto &i : vec) {
		//     cout << i << " ";
		// }
		// cout << endl;

		if (vec.size() == 2 and (vec.at(0) == "get" or vec.at(0) == "del")) {
			if (vec.at(1).length() > 256) {
				return make_pair(fd, formatter.getMessage("Oversized Key"));
			}
			Task task(fd, vec.at(0), vec.at(1));
			pool.addTaskToQueue(task);
			output = pool.getResults();
			output.second =
				formatter.convertToXML(vec.at(0), vec.at(1), output.second);
		} else if (vec.size() == 3 and vec.at(0) == "put") {
			if (vec.at(1).length() > 256) {
				return make_pair(fd, formatter.getMessage("Oversized Key"));
			} else if (vec.at(2).length() > 256 * 1024) {
				return make_pair(fd, formatter.getMessage("Oversized Value"));
			}
			Task task(fd, vec.at(0), vec.at(1), vec.at(2));
			pool.addTaskToQueue(task);
			output.second =
				formatter.convertToXML(vec.at(0), vec.at(1), output.second);
		} else {
			return make_pair(fd,
							 formatter.getMessage(
								 "XML Error: Received unparseable message"));
		}
		return output;
	}
};
