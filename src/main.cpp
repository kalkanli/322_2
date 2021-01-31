#include <iostream>
#include <fstream>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

pthread_mutex_t mutex;
bool busy[3] = {false};

void *client_runner(void* params);


int main(int argc, char const *argv[])
{

    pthread_t tellers[3];
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
    pthread_t client_thread_ids[client_count];
    string clients[client_count];


    bool *seats;
    if (theater_name.compare("OdaTiyatrosu\r") == 0)
    {
        seats = (bool *)calloc(60, sizeof(bool));
    }
    else if (theater_name.compare("UskudarStudyoSahne\r") == 0)
    {
        seats = (bool *)calloc(80, sizeof(bool));
    }
    else if (theater_name.compare("KucukSahne\r") == 0)
    {
        seats = (bool *)calloc(200, sizeof(bool));
    }

    //client_count = 3;

    for (int i = 0; i < client_count; i++)
    {
        getline(file, clients[i]);
        pthread_create(&client_thread_ids[i], NULL, &client_runner, &clients[i]);
    }

    for(int i=0; i < client_count; i++) {
        pthread_join(client_thread_ids[i], NULL);
    }


    free(seats);
    pthread_mutex_destroy(&mutex);

    return 0;
}



void *client_runner(void* params)
{
    string *client = (string *)params;
    cout << client[0] << endl;
    // string client_name = client[0];
    // int arrival_time = stoi(client[1]);
    // int service_time = stoi(client[2]);
    // int seat_number = stoi(client[3]);
    // cout << client_name << " " << arrival_time << service_time << seat_number << endl;

    pthread_mutex_lock(&mutex);
    printf("Hello I am client\n" );
    // %s, I would like the seat number %d", client_name.c_str(), seat_number
    sleep(2);
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
    //return 0;
}




void *teller(void *params)
{
    return 0;
}
