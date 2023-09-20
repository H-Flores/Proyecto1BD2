#include "ISAM.h"
#include <chrono>
using namespace std::chrono;

void print_metrics(long long time, int reads, int writes) {
    cout << "Execution Time (ms): " << time << endl;
    cout << "Disk Reads: " << reads << endl;
    cout << "Disk Writes: " << writes << endl;
    cout << "Total Disk Accesses: " << reads + writes << endl;
    cout << "--------------------------" << endl;
}

void test_load_data(const string &filename) {
    cout << "Testing with " << filename << "..." << endl;

    ISAMSparseIndex<long> ISAM(4);  
    countRead = 0;
    countWrite = 0;

    auto start = high_resolution_clock::now();
    ISAM.load(filename);
    auto stop = high_resolution_clock::now();
    auto load_time = duration_cast<milliseconds>(stop - start);

    ISAM.print();

    print_metrics(load_time.count(), countRead, countWrite);
}

void test_insertion(const string &filename) {
    cout << "Testing insertion on " << filename << "..." << endl;

    ISAMSparseIndex<long> ISAM(4);
    countRead = 0;
    countWrite = 0;

    vector<string> f1 = {"104", "202AAAA3-09-16_fBrazsdil_vs_Argent", "2023-09-16", "Brazil_vs_Argentina", "2", "1", "Brazil", "25000"};
    vector<string> f2 = {"106", "2023-09-17_Germany_vs_Fran", "2023-09-17", "Germany_vs_France", "1", "0", "Germany", "30000"};

    Record record1, record2;
    record1.load(f1);
    record2.load(f2);

    auto start = high_resolution_clock::now();
    ISAM.add(record1);
    ISAM.add(record2);
    auto stop = high_resolution_clock::now();
    auto insert_time = duration_cast<milliseconds>(stop - start);

    print_metrics(insert_time.count(), countRead, countWrite);
}

void test_search(const string &filename) {
    cout << "Testing search on " << filename << "..." << endl;

    ISAMSparseIndex<long> ISAM(4);
    ISAM.load(filename);
    
    long searchKey = 50;
    countRead = 0;
    countWrite = 0;

    auto start = high_resolution_clock::now();
    vector<Record> searchResults = ISAM.search(searchKey);
    auto stop = high_resolution_clock::now();
    auto search_time = duration_cast<milliseconds>(stop - start);

    for (const auto &record : searchResults) {
    record.print();
    cout << "-------------" << endl; // Añadimos un separador para claridad
    }
    print_metrics(search_time.count(), countRead, countWrite);
}

void test_range_search(const string &filename) {
    cout << "Testing range search on " << filename << "..." << endl;

    ISAMSparseIndex<long> ISAM(4);
    ISAM.load(filename);
    
    long beginKey = 50;
    long endKey = 65;
    countRead = 0;
    countWrite = 0;

    auto start = high_resolution_clock::now();
    vector<Record> searchResults = ISAM.rangeSearch(beginKey, endKey);
    auto stop = high_resolution_clock::now();
    auto range_search_time = duration_cast<milliseconds>(stop - start);

    for (const auto &record : searchResults) {
    record.print();
    cout << "-------------" << endl; // Añadimos un separador para claridad
    }

    print_metrics(range_search_time.count(), countRead, countWrite);
}

int main() {
    vector<string> files = {
        "./test_100.csv",
        //"./test_4k.csv",
        //"./test_7k.csv",
        //"./test_10k.csv"
    };

    for (const string &file : files) {
        //test_load_data(file);
        //test_insertion(file);
        //test_search(file);
        test_range_search(file);
    }
    
    return 0;
}
