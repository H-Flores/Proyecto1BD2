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

void test1(const string &csvFile)
{
    cout << "Testing insertion on " << csvFile << "..." << endl;

    sequentialFile<Record, int> SequentialFile(6); // Assuming maxAuxSize = 6
    SequentialFile.load_data(csvFile);

    vector<string> f1 = {"104", "202AAAA3-09-16_fBrazsdil_vs_Argent", "2023-09-16", "Brazil_vs_Argentina", "2", "1", "Brazil", "25000"};
    vector<string> f2 = {"106", "2023-09-17_Germany_vs_Fran", "2023-09-17", "Germany_vs_France", "1", "0", "Germany", "30000"};

    fixedRecord<Record, int> fixedRecord1, fixedRecord2;
    fixedRecord1.load(f1);
    fixedRecord2.load(f2);

    countRead = 0;
    countWrite = 0;

    long long insert_time = measure_time([&]()
                                         {
        assert(SequentialFile.insert(fixedRecord1) == true);
        assert(SequentialFile.insert(fixedRecord2) == true);
    });

    print_metrics(insert_time, countRead, countWrite);
}

void test2(const string &csvFile)
{
    cout << "Testing search on " << csvFile << "..." << endl;

    sequentialFile<Record, int> SequentialFile(6); // Assuming maxAuxSize = 6
    SequentialFile.load_data(csvFile);
    
    int searchKey = 111;
    cout << "------" << endl;

    countRead = 0;
    countWrite = 0;

    long long search_time = measure_time([&]()
                                         {
    vector<fixedRecord<Record,int>> searchResults = SequentialFile.search(searchKey);

    if (searchResults.empty()) {
        cout << "No se encontraron registros con MatchID: " << searchKey << endl;
    } else {
        cout << "Registros encontrados con MatchID: " << searchKey << endl;
        for (auto &record : searchResults) {
            record.print();
            cout << "------" << endl;
        }
    }
    });

    print_metrics(search_time, countRead, countWrite);
}

int main()
{
    vector<string> files = {
        "./results2_100.csv",
        //"./results2_300.csv",
        //"./results2_500.csv",
        //"./results2_800.csv"
    };

    for (const string &file : files) {
        test1(file);
        test2(file);
    }
    
    return 0;
}
