#include <iostream>
#include <fstream>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <regex>

using namespace std;

pthread_mutex_t mutex;
bool busy[3] = {false};
sem_t available_tellers;
struct job
{
    string client_name;
    int seat_number;
    int service_time;
};
struct job *jobs;

void *client_runner(void* params);
void *teller_runner(void *params);

int main(int argc, char const *argv[])
{

    pthread_t tellers[3];
    sem_init(&available_tellers, 0, 3);
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
    jobs[client_count];

    pthread_t teller_thread_ids[3];

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

    char a = 'A';
    char b = 'B';
    char c = 'C';
    pthread_create(&teller_thread_ids[0], NULL, &teller_runner, &a);
    pthread_create(&teller_thread_ids[1], NULL, &teller_runner, &b);
    pthread_create(&teller_thread_ids[2], NULL, &teller_runner, &c);

    for (int i = 0; i < client_count; i++)
    {
        getline(file, clients[i]);
        pthread_create(&client_thread_ids[i], NULL, &client_runner, &clients[i]);
    }

    for(int i=0; i < client_count; i++) {
        pthread_join(client_thread_ids[i], NULL);
    }

    for(int i=0; i<3; i++) {
        pthread_join(teller_thread_ids[i], NULL);
    }

    free(seats);
    pthread_mutex_destroy(&mutex);

    return 0;
}



void *client_runner(void* params)
{
    // TODO later can be changed.
    string *client = (string*) params;
    regex reg(",");
    sregex_token_iterator iter((*client).begin(), (*client).end(), reg, -1);
    sregex_token_iterator end;
    vector<string> vec(iter, end);
    
    string client_name = vec[0];
    int arrival_time = stoi(vec[1]);
    int service_time = stoi(vec[2]);
    int seat_number = stoi(vec[3]);

    sleep(arrival_time);

    sem_wait(&available_tellers);
    pthread_mutex_lock(&mutex);
    
    struct job reservation = {
        client_name,
        seat_number,
        service_time
    };
    
    pthread_mutex_unlock(&mutex);
    sem_post(&available_tellers);
    
    pthread_exit(NULL);
    //return 0;
}


void *teller_runner(void *params)
{
    char *teller = (char*) params;
    cout << *teller << endl;
    pthread_exit(0);
}