#include "sequentialFile.h"
#include <chrono>
#include <functional>
using namespace std::chrono;

// Función auxiliar para medir tiempo
long long measure_time(std::function<void()> func)
{
    auto start = high_resolution_clock::now();
    func();
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    return duration.count();
}

// Función auxiliar para imprimir las métricas
void print_metrics(long long time, int reads, int writes)
{
    cout << "Execution Time (ms): " << time << endl;
    cout << "Disk Reads: " << reads << endl;
    cout << "Disk Writes: " << writes << endl;
    cout << "Total Disk Accesses: " << reads + writes << endl;
    cout << "--------------------------" << endl;
}

void test1()
{
    vector<string> f1 = {"102", "24/04/2023_homeTeam1_vs_awayTeam1", "24/04/2023", "homeTeam1_vs_awayTeam1", "2", "1", "Country1", "15000"};
    vector<string> f2 = {"110", "25/04/2023_homeTeam2_vs_awayTeam2", "25/04/2023", "homeTeam2_vs_awayTeam2", "3", "1", "Country2", "20000"};
    vector<string> f3 = {"154", "26/04/2023_homeTeam3_vs_awayTeam3", "26/04/2023", "homeTeam3_vs_awayTeam3", "2", "0", "Country3", "25000"};
    vector<string> f4 = {"181", "27/04/2023_homeTeam4_vs_awayTeam4", "27/04/2023", "homeTeam4_vs_awayTeam4", "1", "1", "Country4", "18000"};
    vector<string> f5 = {"233", "28/04/2023_homeTeam5_vs_awayTeam5", "28/04/2023", "homeTeam5_vs_awayTeam5", "0", "3", "Country5", "22000"};

    fixedRecord<Record, int> fixedRecord1, fixedRecord2, fixedRecord3, fixedRecord4, fixedRecord5;
    fixedRecord1.load(f1);
    fixedRecord2.load(f2);
    fixedRecord3.load(f3);
    fixedRecord4.load(f4);
    fixedRecord5.load(f5);

    sequentialFile<Record, int> SequentialFile(6);
    SequentialFile.load_data("./results_100.csv");

    // Reset disk access counters
    countRead = 0;
    countWrite = 0;

    // Mide el tiempo de inserción
    long long insert_time = measure_time([&]()
                                         {
        // Insertar y validar inserción
        assert(SequentialFile.insert(fixedRecord1) == true);
        assert(SequentialFile.insert(fixedRecord2) == true);
        assert(SequentialFile.insert(fixedRecord3) == true);
        assert(SequentialFile.insert(fixedRecord4) == true);
        assert(SequentialFile.insert(fixedRecord5) == true); });

    // Imprime las métricas
    print_metrics(insert_time, countRead, countWrite);
}

void test2()
{
    vector<string> f1 = {"112", "24/04/2023_homeTeam1_vs_awayTeam1", "24/04/2023", "homeTeam1_vs_awayTeam1", "2", "1", "Country1", "15000"};
    vector<string> f2 = {"150", "25/04/2023_homeTeam2_vs_awayTeam2", "25/04/2023", "homeTeam2_vs_awayTeam2", "3", "1", "Country2", "20000"};
    vector<string> f3 = {"124", "26/04/2023_homeTeam3_vs_awayTeam3", "26/04/2023", "homeTeam3_vs_awayTeam3", "2", "0", "Country3", "25000"};
    vector<string> f4 = {"151", "27/04/2023_homeTeam4_vs_awayTeam4", "27/04/2023", "homeTeam4_vs_awayTeam4", "1", "1", "Country4", "18000"};
    vector<string> f5 = {"213", "28/04/2023_homeTeam5_vs_awayTeam5", "28/04/2023", "homeTeam5_vs_awayTeam5", "0", "3", "Country5", "22000"};
    fixedRecord<Record, int> fixedRecord1, fixedRecord2, fixedRecord3, fixedRecord4, fixedRecord5;
    fixedRecord1.load(f1);
    fixedRecord2.load(f2);
    fixedRecord3.load(f3);
    fixedRecord4.load(f4);
    fixedRecord5.load(f5);
    sequentialFile<Record, int> SequentialFile(3);
    SequentialFile.load_data("./results_100.csv");

    assert(SequentialFile.insert(fixedRecord1) == true);
    assert(SequentialFile.insert(fixedRecord2) == true);
    assert(SequentialFile.insert(fixedRecord3) == true);
    assert(SequentialFile.insert(fixedRecord4) == true);
    assert(SequentialFile.insert(fixedRecord5) == true);

    // Eliminar y validar eliminación
    assert(SequentialFile.removeRecord(7) == true);
    assert(SequentialFile.removeRecord(151) == false);

    // Intentar eliminar un registro ya eliminado y validar
    assert(SequentialFile.removeRecord(7) == false);

    // SequentialFile.print_all();
}

void test3()
{
    // ... (tu código anterior)

    // Reset disk access counters
    countRead = 0;
    countWrite = 0;

    // Mide el tiempo de búsqueda
    long long search_time = measure_time([&](){
    sequentialFile<Record, int> SequentialFile(3);
    SequentialFile.load_data("./results_100.csv");
    
    vector<fixedRecord<Record,int>> records = SequentialFile.search(102);
    for(auto x: records){
        x.print();
    } });

    // Imprime las métricas
    print_metrics(search_time, countRead, countWrite);
}

void test4()
{
    sequentialFile<Record, int> SequentialFile(3);
    SequentialFile.load_data("./results_100.csv");
    
    vector<fixedRecord<Record,int>> records = SequentialFile.range_search(7, 12);
    assert(!records.empty());
    for(auto x: records){
        x.print();
    }

    // Reset disk access counters
    countRead = 0;
    countWrite = 0;
}

void test_1k(){
    // Probando con 1k datos..

    sequentialFile<Record, int> SequentialFile(6);
    countRead = 0;
    countWrite = 0;
    long long insert_time = measure_time([&](){
        SequentialFile.load_data("./test_1k.csv");
    });

    print_metrics(insert_time, countRead, countWrite);
}

void test_4k(){
    // Probando con 1k datos..

    sequentialFile<Record, int> SequentialFile(6);
    countRead = 0;
    countWrite = 0;
    long long insert_time = measure_time([&](){
        SequentialFile.load_data("./test_4k.csv");
    });

    print_metrics(insert_time, countRead, countWrite);
}

void test_7k(){
    // Probando con 1k datos..

    sequentialFile<Record, int> SequentialFile(6);
    countRead = 0;
    countWrite = 0;
    long long insert_time = measure_time([&](){
        SequentialFile.load_data("./test_7k.csv");
    });

    print_metrics(insert_time, countRead, countWrite);
}

void test_10k(){
    // Probando con 1k datos..

    sequentialFile<Record, int> SequentialFile(6);
    countRead = 0;
    countWrite = 0;
    long long insert_time = measure_time([&](){
        SequentialFile.load_data("./test_10k.csv");
    });

    print_metrics(insert_time, countRead, countWrite);
}

int main()
{
    //test1();
    // test2();
    // test3();
    // test4();

    test_1k();
    std::cout<< "-------------" <<endl;
    test_4k();
    std::cout<< "-------------" <<endl;
    test_7k();
    std::cout<< "-------------" <<endl;
    test_10k();

    return 0;
}
