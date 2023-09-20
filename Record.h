#ifndef PROYECTO_1_BD2__RECORD_H
#define PROYECTO_1_BD2__RECORD_H

#include <iostream>
#include <vector>
using namespace std;

struct Record {
    int MatchID;
    char GameSignature[100];
    char MatchDate[11];
    char EncounterID[100];
    int HomeGoals;
    int AwayGoals;
    char HostCountry[100];
    int TicketsSold;
    bool deleted = false;

    void load(vector<string> data){
        MatchID = stoi(data[0]);
        strcpy(GameSignature, data[1].c_str());
        strcpy(MatchDate, data[2].c_str());
        strcpy(EncounterID, data[3].c_str());
        HomeGoals = stoi(data[4]);
        AwayGoals = stoi(data[5]);
        strcpy(HostCountry, data[6].c_str());
        TicketsSold = stoi(data[7]);
    }

    long getKey() const { return MatchID; }

    void print(){
        cout << "MatchID: " << MatchID << endl;
        cout << "GameSignature: " << GameSignature << endl;
        cout << "MatchDate: " << MatchDate << endl;
        cout << "EncounterID: " << EncounterID << endl;
        cout << "HomeGoals: " << HomeGoals << endl;
        cout << "AwayGoals: " << AwayGoals << endl;
        cout << "HostCountry: " << HostCountry << endl;
        cout << "TicketsSold: " << TicketsSold << endl;
    }
};

#endif //PROYECTO_1_BD2__RECORD_H
