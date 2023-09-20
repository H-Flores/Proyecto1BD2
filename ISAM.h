#ifndef PROYECTO_1_BD2__ISAMSPARSEINDEX_H
#define PROYECTO_1_BD2__ISAMSPARSEINDEX_H

#include <fstream>
#include <vector>
#include <stdexcept>
#include <cstring>
#include "Record.h"
#include "rapidcsv.h"
using namespace std;

int countRead = 0;
int countWrite = 0;

template <typename T>
class ISAMSparseIndex
{
private:
    string dataFile = "./dataISAM.dat";
    string indexFile = "./indexISAM.dat";
    string overflowFile = "./overflowISAM.dat";
    int blockSize;

    struct IndexEntry
    {
        T key;
        long offset;
        IndexEntry() : key(0), offset(0) {}
        IndexEntry(T k, long o) : key(k), offset(o) {}
    };

public:
    explicit ISAMSparseIndex(int bSize) : blockSize(bSize) {}

    // Inserción
    bool add(const Record &record)
    {
        if (record.deleted)
            return false;

        // Verificar si ya existe un registro con esa clave.
        vector<Record> existingRecords = search(record.getKey());
        if (!existingRecords.empty())
        {
            // Ya existe un registro con esa clave.
            return false;
        }

        fstream data(dataFile, ios::out | ios::in | ios::binary | ios::app);
        fstream overflow(overflowFile, ios::out | ios::in | ios::binary | ios::app);

        if (!data)
            return false;
        long position = data.tellp();
        if (position / sizeof(Record) % blockSize == 0)
        {
            fstream index(indexFile, ios::out | ios::in | ios::binary | ios::app);
            IndexEntry entry(record.getKey(), position);
            index.write((char *)&entry, sizeof(IndexEntry));
            countWrite++; 
            index.close();
        }
        else
        {
            overflow.write((char *)&record, sizeof(Record));
            countWrite++; 
            overflow.close();
            return true;
        }

        data.write((char *)&record, sizeof(Record));
        countWrite++; 
        data.close();
        return true;
    }

    void load(const string &csvFile)
    {
        // Cargar registros desde el archivo CSV
        rapidcsv::Document document(csvFile);
        auto len = document.GetRowCount();
        for (int i = 0; i < len; i++)
        {
            vector<string> row = document.GetRow<string>(i);
            Record temp{};
            temp.load(row);
            this->add(temp);
        }
    }

    // Búsqueda específica
    vector<Record> search(T key)
    {
        fstream index(indexFile, ios::in | ios::binary);
        fstream overflow(overflowFile, ios::in | ios::binary);
        vector<Record> results;

        if (!index)
            throw runtime_error("Error al abrir el archivo de índice.");

        IndexEntry entry;
        long prevOffset = 0;

        while (index.read((char *)&entry, sizeof(IndexEntry)))
        {
            countRead++;
            if (entry.key >= key)
                break;
            prevOffset = entry.offset;
        }
        index.close();

        fstream data(dataFile, ios::in | ios::binary);
        data.seekg(prevOffset);
        Record record;

        while (data.read((char *)&record, sizeof(Record)))
        {
            countRead++;
            if (record.getKey() == key)
            {
                results.push_back(record);
                // Si encontramos el registro, salimos
                break;
            }
        }
        data.close();

        // Si no encontramos el registro en el archivo de datos, revisamos en overflow
        if (results.empty())
        {
            while (overflow.read((char *)&record, sizeof(Record)))
            {
                countRead++;
                if (record.getKey() == key)
                    results.push_back(record);
            }
        }
        overflow.close();

        vector<Record> validRecords;
        for (const auto &record : results)
        {
            if (!record.deleted)
                validRecords.push_back(record);
        }
        return validRecords;
    }

    // Búsqueda por rango
    vector<Record> rangeSearch(T beginKey, T endKey)
{
    fstream index(indexFile, ios::in | ios::binary);
    fstream overflow(overflowFile, ios::in | ios::binary);
    vector<Record> results;

    if (!index)
        throw runtime_error("Error al abrir el archivo de índice.");

    IndexEntry entry;
    long prevOffset = 0;

    while (index.read((char *)&entry, sizeof(IndexEntry)))
    {
        countRead++;
        if (entry.key >= beginKey)
            break;
        prevOffset = entry.offset;
    }
    index.close();

    fstream data(dataFile, ios::in | ios::binary);
    data.seekg(prevOffset);
    Record record;

    while (data.read((char *)&record, sizeof(Record)))
    {
        countRead++;
        if (record.getKey() > endKey) // Si la clave del registro es mayor que endKey, termina la lectura.
            break;
        if (record.getKey() >= beginKey) 
            results.push_back(record);
    }
    data.close();

    // Revisar overflow
    while (overflow.read((char *)&record, sizeof(Record)))
    {
        countRead++;
        if (record.getKey() > endKey) 
            break;
        if (record.getKey() >= beginKey && record.getKey() <= endKey)
            results.push_back(record);
    }
    overflow.close();

    vector<Record> validRecords;
    for (const auto &record : results)
    {
        if (!record.deleted)
            validRecords.push_back(record);
    }
    return validRecords;
}


    // Eliminación
    bool remove(T key)
    {
        vector<Record> records = search(key);
        for (auto &record : records)
        {
            record.deleted = true;
        }
        return true;
    }

    void print()
    {

        // 2. Imprimir los registros del archivo de datos
        cout << "\n=== Datos ===" << endl;
        fstream data(dataFile, ios::in | ios::binary);
        Record record;
        while (data.read((char *)&record, sizeof(Record)))
        {
            if (!record.deleted)
            { // Solo imprimimos registros no eliminados
                record.print();
                cout << "-------------" << endl; 
            }
        }
        data.close();
    }
};

#endif // PROYECTO_1_BD2__ISAMSPARSEINDEX_H
