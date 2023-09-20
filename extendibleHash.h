#ifndef PROYECTO_1_BD2__EXTENDIBLEHASH_H
#define PROYECTO_1_BD2__EXTENDIBLEHASH_H

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include "rapidcsv.h"
#include <set>
#include "Record.h"

using namespace std;

#define last(k, n) ((k) & ((1 << (n)) - 1))

int countWrite = 0;
int countRead = 0;

// Configuracion Global
const int globalDepth = 6;
const int globalSize = (1 << globalDepth);
const int maxSizeBucket = 3;

// Para obtener el tamano del bucket
template <typename typeRecord>
struct Bucket
{
    int size = 0; // size actual
    typeRecord records[globalDepth] = {};
    int nextPosition = -1;
};

template <typename typeRecord>
int sizeBucket()
{
    return sizeof(Bucket<typeRecord>);
}

// Necesario para el indice
struct indexDepth
{
    char bin[globalDepth]; // Valor binario del index
    int lenLast;           // Longitud del ultimo index
    void print()
    {
        cout << "bin: ";
        for (int i = 0; i < globalDepth; i++)
        {
            cout << bin[i];
        }
        cout << endl;
        cout << "lenLast: " << lenLast << endl;
    }
};

int sizeIndex()
{
    return sizeof(indexDepth);
}

// Aqui tengo sobrecargas de operadores para escribir y leer en los buckets y index
template <typename typeRecord>
ostream &operator<<(ostream &stream, Bucket<typeRecord> &p)
{
    stream.write((char *)&p, sizeof(p));
    stream << flush;
    countWrite++;
    return stream;
}

template <typename typeRecord>
istream &operator>>(istream &stream, Bucket<typeRecord> &p)
{
    stream.read((char *)&p, sizeof(p));
    countRead++;
    return stream;
}

ostream &operator<<(ostream &stream, indexDepth &p)
{
    stream.write((char *)&p, sizeof(p));
    stream << flush;
    countWrite++;
    return stream;
}

istream &operator>>(istream &stream, indexDepth &p)
{
    stream.read((char *)&p, sizeof(p));
    countRead++;
    return stream;
}

// Key a Hash
string to_hash(int key, int depth)
{
    int last = (key & ((1 << depth) - 1));
    int pos = 0;
    string hash = "";
    while (pos < depth and last > 0)
    {
        hash += to_string(last % 2);
        last /= 2;
        pos++;
    }
    string ans = "";
    while (pos < depth)
    {
        ans += '0';
        pos++;
    }
    reverse(hash.begin(), hash.end());
    ans += hash;
    return ans;
}

// Cadena binaria a entero
int btoi(string s)
{
    int ans = 0, temp = 1, n = s.size();
    for (int i = 0; i < n; i++)
    {
        ans += (temp * (s[n - i - 1] - '0'));
        temp *= 2;
    }
    return ans;
}

void updateIndex(fstream &f, string h)
{
    indexDepth temp;
    int j = globalDepth - h.size(), pos = 0;
    while (pos < globalSize)
    {
        int sz = 0;
        bool ok = true;
        f.seekg(pos * sizeIndex());
        f >> temp;
        for (int i = j; i < globalDepth; i++)
        {
            if (temp.bin[i] != h[sz])
            {
                ok = false;
                break;
            }
            sz++;
        }
        if (ok)
        {
            temp.lenLast++;
            f.seekp(pos * sizeIndex());
            f << temp;
        }
        pos++;
    }
}

// Generacion de hashes
set<string> generar()
{
    set<string> ans;
    for (int i = 0; i < (1 << globalDepth); i++)
    {
        for (int j = 1; j <= globalDepth; j++)
        {
            ans.insert(to_hash(i, j));
        }
    }
    return ans;
}

// Funciones principales que tenia que implementar
template <typename typeRecord>
class extendibleHash
{
    string hashFile = "./hashFile.dat";
    string indexFile = "./indexFile.dat";

public:
    extendibleHash(){};
    void load(const string &csvFile)
    {
        fstream index(indexFile, ios::out | ios::binary);
        fstream data(hashFile, ios::out | ios::binary);

        // Inicializar archivos de índice y datos
        for (int i = 0; i < (1 << globalDepth); i++)
        {
            indexDepth indexD{};
            Bucket<typeRecord> BucketT;
            string temp = to_hash(i, globalDepth);
            strcpy(indexD.bin, temp.c_str());
            indexD.lenLast = 1;

            index.seekp(i * sizeIndex());
            index << indexD;

            data.seekp(i * sizeBucket<typeRecord>());
            data << BucketT;
        }

        // Cargar registros desde el archivo CSV
        rapidcsv::Document document(csvFile);
        auto len = document.GetRowCount();
        for (int i = 0; i < len; i++)
        {
            vector<string> row = document.GetRow<string>(i);
            Record temp{};
            temp.load(row);
            this->insert(temp);
        }

        data.close();
        index.close();
    }

    bool insert(typeRecord record)
    {
        fstream index(indexFile, ios::out | ios::in | ios::binary);
        fstream data(hashFile, ios::out | ios::in | ios::binary);

        if (!data || !index)
            return false;

        if (!search(record.getKey()).empty())
        {
            cerr << "Ya existe la key insertada!\n";
            return false;
        }

        int key = record.getKey() % (1 << globalDepth);
        index.seekg(key * sizeIndex());
        indexDepth temp;
        index >> temp;

        string hashKey = to_hash(key, temp.lenLast);
        data.seekg(btoi(hashKey) * sizeBucket<typeRecord>());
        Bucket<typeRecord> currentBucket;
        data >> currentBucket;

        if (currentBucket.size < maxSizeBucket)
        {
            currentBucket.records[currentBucket.size++] = record;
            data.seekp(btoi(hashKey) * sizeBucket<typeRecord>());
            data << currentBucket;
        }
        else
        {
            while (true) // Loop hasta que el registro sea insertado o se use el encadenamiento
            {
                // Verificar si el local depth es menor que el global depth
                if (temp.lenLast < globalDepth)
                {
                    // Split del bucket
                    string newHash1 = '0' + hashKey;
                    string newHash2 = '1' + hashKey;

                    updateIndex(index, newHash1);
                    updateIndex(index, newHash2);

                    Bucket<typeRecord> newBucket1, newBucket2;
                    newBucket1.size = 0;
                    newBucket2.size = 0;

                    for (int i = 0; i < currentBucket.size; i++)
                    {
                        string itemHash = to_hash(currentBucket.records[i].getKey(), temp.lenLast + 1);
                        if (itemHash == newHash1)
                        {
                            newBucket1.records[newBucket1.size++] = currentBucket.records[i];
                        }
                        else
                        {
                            newBucket2.records[newBucket2.size++] = currentBucket.records[i];
                        }
                    }

                    data.seekp(btoi(newHash1) * sizeBucket<typeRecord>());
                    data << newBucket1;
                    data.seekp(btoi(newHash2) * sizeBucket<typeRecord>());
                    data << newBucket2;

                    // Incrementar el local depth
                    temp.lenLast++;
                    index.seekp(key * sizeIndex());
                    index << temp;

                    // Intentar insertar el registro nuevamente (ahora que el bucket se ha dividido)
                    hashKey = to_hash(record.getKey(), temp.lenLast);
                }
                else
                {
                    // Encadenamiento
                    Bucket<typeRecord> newBucket;
                    newBucket.records[0] = record;
                    newBucket.size = 1;

                    data.seekp(0, ios::end);
                    currentBucket.nextPosition = data.tellp() / sizeBucket<typeRecord>();
                    data << newBucket;
                    data.seekp(btoi(hashKey) * sizeBucket<typeRecord>());
                    data << currentBucket;
                    break;
                }
            }
        }

        data.close();
        index.close();
        return true;
    }

    void printAllBuckets()
    {
        set<string> gen = generar();
        for (auto x : gen)
        {
            printBucket(x);
        }
    }

    void printBucket(string s)
    {
        fstream index(indexFile, ios::out | ios::in | ios::binary);
        fstream data(hashFile, ios::out | ios::in | ios::binary);
        if (!data || !index)
            return;
        indexDepth tempIndex;
        index.seekg(btoi(s) * sizeIndex());
        index >> tempIndex;
        if (s.size() == tempIndex.lenLast)
        {
            Bucket<typeRecord> tempBucket;
            data.seekg(btoi(s) * sizeBucket<typeRecord>());
            data >> tempBucket;
            if (tempBucket.size != 0)
            {
                cout << "--- " << s << " ---" << endl;
                for (int i = 0; i < tempBucket.size; i++)
                {
                    tempBucket.records[i].print();
                }
                int nxtPos = tempBucket.nextPosition;
                while (nxtPos != -1)
                {
                    cout << "-- encadenamiento --\n";
                    data.seekg(nxtPos * sizeBucket<typeRecord>());
                    data >> tempBucket;
                    for (int i = 0; i < tempBucket.size; i++)
                    {
                        tempBucket.records[i].print();
                    }
                    nxtPos = tempBucket.nextPosition;
                }
            }
        }
        data.close();
        index.close();
    }

    vector<typeRecord> search(int key)
    {
        fstream index(indexFile, ios::in | ios::binary);
        fstream data(hashFile, ios::in | ios::binary);
        if (!data || !index)
            return {};

        // Calcular el index key
        int indexKey = key % (1 << globalDepth);

        // Buscar la entrada correspondiente en el index
        index.seekg(indexKey * sizeIndex());
        indexDepth temp;
        index >> temp;

        // Obtener la clave hash de los últimos 'lenLast' bits
        string hashKey = to_hash(key, temp.lenLast);

        // Dirigirse al bucket correspondiente en el archivo de datos
        data.seekg(btoi(hashKey) * sizeBucket<typeRecord>());
        Bucket<typeRecord> bucket;
        data >> bucket;

        // Crear un vector para almacenar los registros que coincidan con la key
        vector<typeRecord> result;
        // Se busca dentro del bucket correspondiente
        for (int i = 0; i < bucket.size; i++)
        {
            // Si el registro coincide con la key, agregarlo al vector result
            if (bucket.records[i].getKey() == key)
                result.push_back(bucket.records[i]);
        }

        // Si no se encuentra en el bucket, buscar en los siguientes (solo si hay encadenamiento)
        int nextPosition = bucket.nextPosition;
        while (nextPosition != -1)
        {
            // Nos dirigimos al siguiente bucket en la cadena
            data.seekg(nextPosition * sizeBucket<typeRecord>());
            data >> bucket;
            // Buscar registros coincidentes en el bucket
            for (int i = 0; i < bucket.size; i++)
            {
                if (bucket.records[i].getKey() == key)
                    result.push_back(bucket.records[i]);
            }
            // Actualizar la posición
            nextPosition = bucket.nextPosition;
        }
        data.close();
        index.close();
        return result;
    }

    bool removeRecordFromBucket(fstream &data, int position, int key)
    {
        data.seekg(position * sizeBucket<typeRecord>());
        Bucket<typeRecord> bucket;
        data >> bucket;

        for (int i = 0; i < bucket.size; i++)
        {
            if (bucket.records[i].getKey() == key)
            {
                for (int j = i; j < bucket.size - 1; j++)
                {
                    bucket.records[j] = bucket.records[j + 1];
                }
                typeRecord emptyRec;
                bucket.records[bucket.size - 1] = emptyRec;
                bucket.size--;

                data.seekp(position * sizeBucket<typeRecord>());
                data << bucket;
                return true;
            }
        }
        return false;
    }

    bool remove(int key)
    {
        fstream index(indexFile, ios::in | ios::binary);
        fstream data(hashFile, ios::in | ios::binary);

        int indexKey = key % (1 << globalDepth);
        index.seekg(indexKey * sizeIndex());
        indexDepth temp;
        index >> temp;

        string hashKey = to_hash(key, temp.lenLast);
        int bucketPosition = btoi(hashKey);

        if (removeRecordFromBucket(data, bucketPosition, key))
        {
            index.close();
            data.close();
            return true;
        }

        // Chequear en encadenamientos 
        data.seekg(bucketPosition * sizeBucket<typeRecord>());
        Bucket<typeRecord> bucket;
        data >> bucket;
        int nextPosition = bucket.nextPosition;

        while (nextPosition != -1)
        {
            if (removeRecordFromBucket(data, nextPosition, key))
            {
                index.close();
                data.close();
                return true;
            }
            data.seekg(nextPosition * sizeBucket<typeRecord>());
            data >> bucket;
            nextPosition = bucket.nextPosition;
        }

        cerr << "Registro no existe....";
        index.close();
        data.close();
        return false;
    }
};

#endif // PROYECTO_1_BD2__EXTENDIBLEHASH_H