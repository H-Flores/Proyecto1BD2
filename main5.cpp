#include "ISAM.h"
#include <chrono>
using namespace std::chrono;

void test1(const string &csvFile) {
    cout << "Testing insertion on " << csvFile << "..." << endl;

    ISAMSparseIndex<long> ISAM(4);  // Asumiendo un tamaño de bloque de 4 para este ejemplo

    // Medir el tiempo de carga
    auto start = high_resolution_clock::now();
    ISAM.load(csvFile);
    auto stop = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(stop - start);
    cout << "Tiempo de carga: " << time.count() << " ms" << endl;

    vector<string> f1 = {"104", "202AAAA3-09-16_fBrazsdil_vs_Argent", "2023-09-16", "Brazil_vs_Argentina", "2", "1", "Brazil", "25000"};
    vector<string> f2 = {"106", "2023-09-17_Germany_vs_Fran", "2023-09-17", "Germany_vs_France", "1", "0", "Germany", "30000"};

    Record record1, record2;
    record1.load(f1);
    record2.load(f2);

    start = high_resolution_clock::now();
    ISAM.add(record1);
    ISAM.add(record2);
    stop = high_resolution_clock::now();
    time = duration_cast<milliseconds>(stop - start);
    cout << "Tiempo de inserción: " << time.count() << " ms" << endl;
}

void test2(const string &csvFile) {
    cout << "Testing search on " << csvFile << "..." << endl;

    ISAMSparseIndex<long> ISAM(4);  // Asumiendo un tamaño de bloque de 4 para este ejemplo
    ISAM.load(csvFile);
    
    long searchKey = 111;
    cout << "------" << endl;

    auto start = high_resolution_clock::now();
    vector<Record> searchResults = ISAM.search(searchKey);
    auto stop = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(stop - start);
    cout << "Tiempo de búsqueda: " << time.count() << " ms" << endl;

    if (searchResults.empty()) {
        cout << "No se encontraron registros con documentID: " << searchKey << endl;
    } else {
        cout << "Registros encontrados con documentID: " << searchKey << endl;
        for (Record &record : searchResults) {
            record.print();
            cout << "------" << endl;
        }
    }
}

void test3(const string &csvFile) {
    cout << "Testing range search on " << csvFile << "..." << endl;

    ISAMSparseIndex<long> ISAM(4);  // Asumiendo un tamaño de bloque de 4 para este ejemplo
    ISAM.load(csvFile);
    
    long beginKey = 100;
    long endKey = 120;
    cout << "------" << endl;

    auto start = high_resolution_clock::now();
    vector<Record> searchResults = ISAM.rangeSearch(beginKey, endKey);
    auto stop = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(stop - start);
    cout << "Tiempo de búsqueda por rango: " << time.count() << " ms" << endl;

    if (searchResults.empty()) {
        cout << "No se encontraron registros entre documentID " << beginKey << " y " << endKey << endl;
    } else {
        cout << "Registros encontrados entre documentID " << beginKey << " y " << endKey << ":" << endl;
        for (Record &record : searchResults) {
            record.print();
            cout << "------" << endl;
        }
    }
}

int main() {
    vector<string> files = {
        //"./results_100.csv"//,
        //"./results_300.csv",
        //"./results_500.csv",
        "./results2_800.csv"
    };

    for (const string &file : files) {
        test1(file);
        test2(file);
        test3(file);
    }
    
    return 0;
}
