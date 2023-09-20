#include "sequentialFile.h"
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
    cout << "Testing with " << filename << "..." << endl;

    sequentialFile<Record, int> SequentialFile(6);
    countRead = 0;
    countWrite = 0;

    long long insert_time = measure_time([&](){
        SequentialFile.load_data(filename);
    });

    print_metrics(insert_time, countRead, countWrite);
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
    }
    
    return 0;
}
