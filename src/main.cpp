#include <iostream>
#include <fstream>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

void *client(void *params);

pthread_mutex_t mutex; 

int main(int argc, char const *argv[])
{
    // <initialization_part>
    pthread_mutex_init(&mutex, NULL);

    string input_file_path = argv[1];
    ifstream file(input_file_path);

    string line;
    string theater_name;
    string client_count_str;
    int client_count;

    getline(file, theater_name);
    getline(file, client_count_str);
    client_count = stoi(client_count_str);
    string client[4];
    // <!initialization_part>

    // <teller_thread_start>
    //TODO TELLER
    // <!teller_thread_start>
    
    cout << theater_name << endl;
    //cout << "OdaTiyatrosu" << endl;

    //cout << theater_name.compare("OdaTiyatrosu") << endl;   

    bool *seats;

    if(theater_name.compare("OdaTiyatrosu\r") == 0) {
        cout << "here" << endl;
        seats = (bool*) calloc(60 ,sizeof(bool));
    } else if(theater_name.compare("UskudarStudyoSahne\r") == 0) {
        seats = (bool*) calloc(80 ,sizeof(bool));
    } else if(theater_name.compare("KucukSahne\r") == 0) {
        seats = (bool*) calloc(200 ,sizeof(bool));
    }

    for(int i=0; i<60; i++) {
        cout << seats[i] << endl;
    }

    // <client_thread_start>
    for(int i=0; i<client_count; i++) {
        getline(file, client[0], ',');
        getline(file, client[1], ',');
        getline(file, client[2], ',');
        getline(file, client[3], '\n');
    }
    // <!client_thread_start>


    return 0;
}

void *client(void *params) 
{
    string *client = (string*) params;
    string client_name = client[0];
    int arrival_time = stoi(client[1]);
    int service_time =  stoi(client[2]);
    int seat_number =  stoi(client[3]);

    cout << seat_number << endl;
    return 0;
}

void *teller(void *params) 
{
    return 0;
}
