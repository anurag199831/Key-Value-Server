#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "KVCache.cpp"

using namespace std;

struct Task {
    Task() {}
    Task(const int fd, string func, string key, string value)
        : fd(fd), func(func), key(key), value(value) {}
    Task(const int fd, string func, string key)
        : fd(fd), func(func), key(key) {}
    string key;
    string value;
    string func;
    int fd;
};

class ThreadPool {
   private:
    vector<thread> threads;
    const size_t numberOfThreads;
    KVCache cache;
    queue<Task> q;
    queue<pair<int, string>> output;
    mutex queueMutex;
    mutex outputMutex;
    condition_variable cv;
    mutex threadPoolMutex;
    bool terminatePool = false;
    bool stopped = false;

    void infiniteLoopFunction() {
        while (true) {
            Task task;
            {
                unique_lock<mutex> lock(queueMutex);
                cv.wait(lock, [this]() { return !q.empty() || terminatePool; });
                if (stopped and q.empty()) {
                    return;
                }
                task = q.front();
                q.pop();
            }
            string s = cache.operateOnCache(task.func, task.key, task.value);
            if (task.func == "get" or task.func == "del") {
                unique_lock<mutex> outLock(outputMutex);
                output.push(make_pair(task.fd, s));
                cv_results.notify_one();
            }
        }
    }

   public:
    condition_variable cv_results;
    ThreadPool(size_t numberOfThreads, size_t numberOfSets,
               size_t entriesPerSet)
        : numberOfThreads(numberOfThreads), cache(numberOfSets, entriesPerSet) {
        for (size_t i = 0; i < numberOfThreads; i++) {
            threads.push_back(thread([this] { this->infiniteLoopFunction(); }));
        }
    }

    pair<int, string> getResults() {
        pair<int, string> s;
        {
            unique_lock<mutex> outLock(outputMutex);
            cv_results.wait(outLock, [this] { return !output.empty(); });
            s = output.front();
            output.pop();
        }
        return s;
    }

    pair<int, string> peekResults() {
        pair<int, string> top;
        {
            unique_lock<mutex> outLock(outputMutex);
            top = output.front();
        }
        return top;
    }

    void addTaskToQueue(Task task) {
        {
            cout << "Adding task to queue" << endl;
            unique_lock<mutex> lock(queueMutex);
            q.push(task);
        }
        cv.notify_one();
    }

    void shutdown() {
        {
            unique_lock<mutex> lock(threadPoolMutex);
            terminatePool = true;
            stopped = true;
        }
        cv.notify_all();
        for (std::thread &every_thread : threads) {
            every_thread.join();
        }
        threads.empty();
    }

    ~ThreadPool() {}
};
