#include <iostream>
#include <fstream>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <regex>

using namespace std;

pthread_mutex_t print_mutex;
pthread_mutex_t mutex;
sem_t available_tellers;
sem_t ready_clients[3];
struct job
{
    string client_name;
    int seat_number;
    double service_time;
};
struct job buffer[3];
int buffer_in = 0;
int buffer_out = 0;

bool *seats;
int taken_seats = 0;
int total_seats;

bool busy[3] = {false};

void *client_runner(void *params);
void *teller_runner(void *params);

int main(int argc, char const *argv[])
{

    sem_init(&ready_clients[0], 0, 0);
    sem_init(&ready_clients[1], 0, 0);
    sem_init(&ready_clients[2], 0, 0);

    sem_init(&available_tellers, 0, 3);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&print_mutex, NULL);

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

    buffer[3];
    // buffer[0] = {"", 0, 0, true};
    // buffer[1] = {"", 0, 0, true};
    // buffer[2] = {"", 0, 0, true};

    pthread_t teller_thread_ids[3];

    if (theater_name.compare("OdaTiyatrosu\r") == 0)
    {
        seats = (bool *)calloc(60, sizeof(bool));
        total_seats = 60;
    }
    else if (theater_name.compare("UskudarStudyoSahne\r") == 0)
    {
        seats = (bool *)calloc(80, sizeof(bool));
        total_seats = 60;
    }
    else if (theater_name.compare("KucukSahne\r") == 0)
    {
        seats = (bool *)calloc(200, sizeof(bool));
        total_seats = 200;
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

    for (int i = 0; i < client_count; i++)
    {
        pthread_join(client_thread_ids[i], NULL);
    }

    for (int i = 0; i < 1; i++)
    {
        pthread_join(teller_thread_ids[i], NULL);
    }

    free(seats);
    pthread_mutex_destroy(&mutex);

    return 0;
}

void *client_runner(void *params)
{
    // later can be changed.
    string *client = (string *)params;
    regex reg(",");
    sregex_token_iterator iter((*client).begin(), (*client).end(), reg, -1);
    sregex_token_iterator end;
    vector<string> vec(iter, end);

    string client_name = vec[0];
    double arrival_time = stoi(vec[1]) * 0.001;
    double service_time = stoi(vec[2]) * 0.001;
    int seat_number = stoi(vec[3]);
    sleep(arrival_time);




    /* CRITICAL SECTION ENTER */
    sem_wait(&available_tellers);
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < 3; i++)
    {
        if (!busy[i])
        {
            buffer_in = i;
            break;
        }
    }
    // cout << client_name << "->" << char(buffer_in+65) << endl; 
    busy[buffer_in] = true;
    //cout << client_name << " asks for service from " << char(buffer_in + 65) << endl;
    struct job reservation = {client_name, seat_number, service_time};
    buffer[buffer_in] = reservation;
    pthread_mutex_unlock(&mutex);
    sem_post(&ready_clients[buffer_in]);
    /* CRITICAL SECTION EXIT */

    pthread_exit(NULL);
    //return 0;
}

void *teller_runner(void *params)
{
    char *teller = (char *)params;
    int reserved_seat_number = 0;
    int buffer_index = *teller - 65;
    while (true)
    {
        sem_wait(&ready_clients[buffer_index]);

        pthread_mutex_lock(&mutex);
        struct job reservation = buffer[buffer_index];
        pthread_mutex_unlock(&mutex);
        
        sleep(reservation.service_time);
        
        pthread_mutex_lock(&mutex);
        if (taken_seats == total_seats)
        {
            cout << "no reservation" << endl;
            continue;
        }

        if (!seats[reservation.seat_number - 1])
        { // seat not taken
            seats[reservation.seat_number - 1] = true;
            reserved_seat_number = reservation.seat_number;
        }
        else
        { // seat already taken.
            for (int i = 0; i < total_seats; i++)
            {
                if (!seats[i])
                {
                    seats[i] = true;
                    reserved_seat_number = i + 1;
                    break;
                }
            }
        }
        cout << reservation.client_name << " reserved seat number " << reserved_seat_number
            << ". signed by teller " << *teller << endl;
        busy[buffer_index] = false;
        pthread_mutex_unlock(&mutex);
        sem_post(&available_tellers);
    }
    pthread_exit(NULL);
}
