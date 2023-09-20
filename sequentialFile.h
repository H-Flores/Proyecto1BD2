#ifndef PROYECTO_1_BD2_SEQUENTIALFILE_H
#define PROYECTO_1_BD2_SEQUENTIALFILE_H

#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include "rapidcsv.h"
#include "Record.h"

using namespace std;

int countWrite = 0;
int countRead = 0;

template <typename typeRecord, typename typeKey>
struct fixedRecord
{
    typeRecord record;
    int nextFile = 0;
    long nextPosition = -1;
    int deleted = 0;
    void load(vector<string> data)
    {
        record.load(data);
    }
    typeKey getKey() { return record.getKey(); }
    void print()
    {
        record.print();
        cout << "nextFile: " << nextFile << endl;
        cout << "nextPos: " << nextPosition << endl;
        cout << "isDeleted: " << deleted << endl;
    }
};

template <typename typeRecord, typename typeKey>
ostream &operator<<(ostream &stream, fixedRecord<typeRecord, typeKey> &p)
{
    stream.write((char *)&p, sizeof(p));
    stream << flush;
    countWrite++;
    return stream;
}

template <typename typeRecord, typename typeKey>
istream &operator>>(istream &stream, fixedRecord<typeRecord, typeKey> &p)
{
    stream.read((char *)&p, sizeof(p));
    countRead++;
    return stream;
}

template <typename typeRecord, typename typeKey>
class sequentialFile
{
    const char *dataFile = "./dataFile.dat";
    const char *auxFile = "./auxFile.dat";
    int maxAuxSize;

public:
    explicit sequentialFile(int maxAuxSize) : maxAuxSize(maxAuxSize){};

    void load_data(const string &csvFile)
    {
        // Definición inicial de registros temporales
        fixedRecord<typeRecord, typeKey> temp, header;
        fstream data(dataFile, ios::out | ios::binary);
        fstream aux(auxFile, ios::out | ios::binary);

        // Error
        if (!data.is_open() || !aux.is_open())
            return;

        // Cargar el documento CSV usando rapidcsv
        rapidcsv::Document document(csvFile);
        int len = document.GetRowCount();

        // Configurar el encabezado del archivo binario
        header.nextFile = 0;
        header.nextPosition = 1;
        data.seekp(0);
        data << header;

        // Iterar sobre cada fila del CSV para cargarla en el archivo binario
        for (int i = 0; i < len; i++)
        {
            vector<string> row = document.GetRow<string>(i);
            temp.load(row); // Convertir la fila a un registro!

            // Establecer las propiedades de posición para cada registro
            temp.nextPosition = (i == len - 1) ? -1 : i + 2;
            temp.nextFile = (i == len - 1) ? -1 : 0;

            // Escribir el registro en el archivo binario
            data.seekp((i + 1) * sizeRecord());
            data << temp;
        }

        // Cerramos los archivos
        data.close();
        aux.close();
    }

    // Función auxiliar para imprimir los contenidos de un archivo
    void print_file_contents(const string &filename, const string &fileTitle)
    {
        fstream file(filename, ios::in | ios::binary);
        int pos = 0;
        fixedRecord<typeRecord, typeKey> temp;

        cout << "---- " << fileTitle << " ----" << endl;

        while (file >> temp)
        {
            pos++;
            cout << "-- registro: " << pos << "--" << endl;
            temp.print();
            file.seekg(pos * sizeRecord());
        }
        file.close();
    }

    void print_all()
    {
        print_file_contents(dataFile, "dataFile");
        print_file_contents(auxFile, "auxFile");
    }

    vector<fixedRecord<typeRecord, typeKey>> search(int key)
    {
        // Crear un vector para almacenar los resultados
        vector<fixedRecord<typeRecord, typeKey>> results;
        fixedRecord<typeRecord, typeKey> temp;
        fstream data(dataFile, ios::in | ios::binary);
        fstream aux(auxFile, ios::in | ios::binary);

        if (!data || !aux)
            return results; // Si no se pueden abrir los archivos, retorna vacio

        pair<int, int> loc = findLocation(key); // Buscar la ubicación del registro con la clave proporcionada

        // Si el registro se encuentra en el archivo de datos...
        if (loc.first == 0)
        {
            data.seekg(loc.second * sizeRecord());
            data >> temp;
            // Si la clave coincide y el registro no está marcado como eliminado, agregar al vector
            if (temp.getKey() == key && !temp.deleted)
            {
                results.push_back(temp);
            }
            // Si el registro se encuentra en el archivo auxiliar
        }
        else if (loc.first == 1)
        {
            aux.seekg(loc.second * sizeRecord());
            aux >> temp;
            // Si la clave coincide y el registro no está marcado como eliminado, agregar al vector
            if (temp.getKey() == key && !temp.deleted)
            {
                results.push_back(temp);
            }
        }
        data.close();
        aux.close();
        return results;
    }

    vector<fixedRecord<typeRecord, typeKey>> range_search(int keyBegin, int keyEnd)
    {
        vector<fixedRecord<typeRecord, typeKey>> results;
        fixedRecord<typeRecord, typeKey> temp;

        // Si el rango no es válido, retornar vacio
        if (keyBegin > keyEnd)
        {
            return results;
        }

        fstream data(dataFile, ios::in | ios::binary);
        fstream aux(auxFile, ios::in | ios::binary);

        if (!data || !aux)
            return results; // Si no se pueden abrir los archivos, retorna nada

        pair<int, int> locBegin = findLocation(keyBegin); // Buscar la ubicación del primer registro en el rango
        pair<int, int> locEnd = findLocation(keyEnd);     // Buscar la ubicación del último registro en el rango

        // Si los registros están en el archivo de datos
        if (locBegin.first == 0 && locBegin.second != -1)
        {
            for (int pos = locBegin.second; pos <= locEnd.second; ++pos)
            {
                data.seekg(pos * sizeRecord());
                data >> temp;
                // Si la clave está en el rango y el registro no está marcado como eliminado, agregar al vector
                if (temp.getKey() >= keyBegin && temp.getKey() <= keyEnd && !temp.deleted)
                {
                    results.push_back(temp);
                }
            }
        }

        // Si alguno de los registros está en el archivo auxiliar
        if (locBegin.first == 1 || locEnd.first == 1)
        {
            aux.seekg(0, ios::end);
            long sizeAux = aux.tellg();
            long numRecords = sizeAux / sizeRecord();
            for (int pos = 0; pos < numRecords; ++pos)
            {
                aux.seekg(pos * sizeRecord());
                aux >> temp;
                // Si la clave está en el rango y el registro no está marcado como eliminado, agregar al vector
                if (temp.getKey() >= keyBegin && temp.getKey() <= keyEnd && !temp.deleted)
                {
                    results.push_back(temp);
                }
            }
        }
        data.close();
        aux.close();
        return results;
    }

    bool insert(fixedRecord<typeRecord, typeKey> record)
    {
        fstream data(dataFile, ios::out | ios::in | ios::binary);
        fstream aux(auxFile, ios::out | ios::in | ios::binary);

        // Control de error...
        if (!data.is_open() || !aux.is_open())
            return false;

        // Calculo
        long sizeAux = aux.tellg() / sizeRecord();

        // Si el aux esta lleno.. merge!
        if (sizeAux >= maxAuxSize)
        {
            merge_data();
            data.close();
            remove(dataFile);
            rename("./dataFile2.dat", dataFile);
            aux.clear();
            aux.seekp(0, ios::beg);
        }

        pair<int, int> prev = findLocation(record.getKey());
        fixedRecord<typeRecord, typeKey> temp;
        fstream &sourceFile = (prev.first == 0) ? data : aux;
        sourceFile.seekg(prev.second * sizeRecord());
        sourceFile >> temp;

        if (temp.getKey() == record.getKey())
            return false;

        // Preparamos el registro para ser insertado
        record.nextPosition = temp.nextPosition;
        record.nextFile = temp.nextFile;
        temp.nextPosition = sizeAux;
        temp.nextFile = 1;
        sourceFile.seekp(prev.second * sizeRecord()) << temp;

        // Lo metemos al aux
        aux.seekp(sizeAux * sizeRecord()) << record;

        return true;
    }

    bool removeRecord(int key)
    {
        fstream data(dataFile, ios::in | ios::out | ios::binary);
        fstream aux(auxFile, ios::in | ios::out | ios::binary);

        if (!data || !aux)
            return false;

        fixedRecord<typeRecord, typeKey> temp, tempPrev;

        // Obtener el registro actual
        pair<int, int> loc = findLocation(key);
        fstream &currFile = (loc.first == 0) ? data : aux;
        currFile.seekg(loc.second * sizeRecord());
        currFile >> temp;

        if (temp.getKey() != key || temp.deleted)
        {
            cerr << "No existe registro con ese key\n";
            data.close();
            aux.close();
            return false;
        }

        // Obtener el registro anterior
        pair<int, int> locPrev = findLocation(key - 1);
        fstream &prevFile = (locPrev.first == 0) ? data : aux;
        prevFile.seekg(locPrev.second * sizeRecord());
        prevFile >> tempPrev;

        // Actualizar punteros y marcar como eliminado
        tempPrev.nextFile = temp.nextFile;
        tempPrev.nextPosition = temp.nextPosition;
        temp.deleted = 1;

        // Escribir de nuevo el registro anterior y el actual
        prevFile.seekp(locPrev.second * sizeRecord());
        prevFile << tempPrev;
        currFile.seekp(loc.second * sizeRecord());
        currFile << temp;

        data.close();
        aux.close();
        return true;
    }

    void merge_data()
    {
        fstream data(dataFile, ios::in | ios::binary);
        fstream aux(auxFile, ios::in | ios::binary);
        fstream data2("./dataFile2.dat", ios::out | ios::binary);

        if (!data.is_open() || !aux.is_open())
            return;

        fixedRecord<typeRecord, typeKey> header, temp;
        int newSize = maxAuxSize + (data.tellg() / sizeRecord()) - countD(dataFile, auxFile);
        data.seekg(0);
        data >> header;

        temp.nextPosition = header.nextPosition;
        temp.nextFile = header.nextFile;

        data2.seekp(0) << header;
        int pos = 1;

        while (temp.nextPosition != -1 && temp.nextFile != -1)
        {
            fixedRecord<typeRecord, typeKey> curr;
            fstream &currentFile = (temp.nextFile == 0) ? data : aux;
            currentFile.seekg(temp.nextPosition * sizeRecord());
            currentFile >> curr;

            curr.nextPosition = (pos + 1 < newSize) ? pos + 1 : -1;
            curr.nextFile = (pos + 1 < newSize) ? 0 : -1;

            data2.seekp(pos * sizeRecord()) << curr;

            temp = curr;
            pos++;
        }

        data.close();
        aux.close();
    }

    pair<int, int> findLocation(int key) {
    fstream data(dataFile, ios::in | ios::binary);
    fstream aux(auxFile, ios::in | ios::binary);

    if (!data.is_open() || !aux.is_open()) {
        return {-1, -1};
    }

    fixedRecord<typeRecord, typeKey> temp;

    // Búsqueda binaria en dataFile
    long l = 0;
    data.seekg(0, ios::end);
    long r = (data.tellg() / sizeRecord()) - 1;
    long m;

    while (l <= r) {
        m = l + (r - l) / 2;
        data.seekg(m * sizeRecord());
        data >> temp;

        if (temp.getKey() == key && !temp.deleted) {
            return {0, m};  // Key found in dataFile
        }

        if (temp.getKey() < key) {
            l = m + 1;
        } else {
            r = m - 1;
        }
    }

    // Asumimos inicialmente que la posición es en el dataFile
    int file = 0;
    int index = max(0l, l - 1);

    // Si la última posición revisada en dataFile apunta a auxFile, seguimos esa cadena.
    while (temp.nextFile == 1) {
        aux.seekg(temp.nextPosition * sizeRecord());
        aux >> temp;

        if (temp.getKey() == key) {
            file = 1;
            index = temp.nextPosition;
            break;
        }

        if (temp.getKey() > key) {
            break;  // Salimos porque encontramos una clave mayor
        }

        // Continuamos siguiendo el enlace
        file = 1;
        index = temp.nextPosition;
    }

    data.close();
    aux.close();
    return {file, index};
}


    int countD(const char *d, const char *a)
    {
        fstream data(d, ios::in | ios::binary);
        fstream aux(a, ios::in | ios::binary);
        int countDeleted = 0;
        fixedRecord<typeRecord, typeKey> recIt;
        data.seekg(0, ios::end);
        int pos = 0, cant = data.tellg() / sizeRecord();
        while (pos <= cant)
        {
            data.seekg(pos * sizeRecord());
            data >> recIt;
            if (recIt.deleted)
                countDeleted++;
            pos++;
        }
        aux.seekg(0, ios::end);
        pos = 0, cant = aux.tellg() / sizeRecord();
        while (pos <= cant)
        {
            aux.seekg(pos * sizeRecord());
            aux >> recIt;
            if (recIt.deleted)
                countDeleted++;
            pos++;
        }
        data.close();
        aux.close();
        return countDeleted;
    }

    int sizeRecord()
    {
        return sizeof(fixedRecord<typeRecord, typeKey>);
    }
};

#endif // PROYECTO_1_BD2_SEQUENTIALFILE_H