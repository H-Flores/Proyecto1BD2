#include "extendibleHash.h"
#include <chrono>
#include <functional>
using namespace std::chrono;

long long measure_time(std::function<void()> func)
{
    auto start = high_resolution_clock::now();
    func();
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    return duration.count();
}

void print_metrics(long long time, int reads, int writes)
{
    cout << "Execution Time (ms): " << time << endl;
    cout << "Disk Reads: " << reads << endl;
    cout << "Disk Writes: " << writes << endl;
    cout << "Total Disk Accesses: " << reads + writes << endl;
    cout << "--------------------------" << endl;
}

void test_data_load(const string &filename)
{
    cout << "Testing LOAD with " << filename << "..." << endl;

    extendibleHash<Record> ExtendibleHash;
    countRead = 0;
    countWrite = 0;

    long long load_time = measure_time([&](){
        ExtendibleHash.load(filename);
    });

    print_metrics(load_time, countRead, countWrite);
    cout << "-------------" << endl;
}

void test_data_search(const string &filename, int searchKey)
{
    cout << "Testing SEARCH for key " << searchKey << " in " << filename << "..." << endl;

    extendibleHash<Record> ExtendibleHash;
    ExtendibleHash.load(filename);
    countRead = 0;
    countWrite = 0;

    long long search_time = measure_time([&](){
        vector<Record> searchResults = ExtendibleHash.search(searchKey);
    });

    print_metrics(search_time, countRead, countWrite);
    cout << "-------------" << endl;
}

int main()
{
    vector<string> files = {
        "./test_1k.csv",
        "./test_4k.csv",
        "./test_7k.csv",
        "./test_10k.csv"
    };

    for (const string &file : files) {
        test_data_load(file);
        //test_data_search(file, 111); // key
    }
    
    return 0;
}
