#include <iostream>
#include <fstream>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <regex>

using namespace std;

pthread_mutex_t mutex;
pthread_mutex_t print_mutex;
sem_t full;
sem_t empty;

struct job
{
    string client_name;
    int seat_number;
    double service_time;
};
struct job *buffer;

bool busy[3] = {false};

void *client_runner(void *params);
void *teller_runner(void *params);
bool checkIfMyTurn(char teller);

int buffer_in = 0;
int buffer_out = 0;

bool *seats;
int taken_seats = 0;
int total_seats;

int clients_left;

int main(int argc, char const *argv[])
{
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, 3);
    pthread_mutex_init(&mutex, NULL);

    string input_file_path = argv[1];
    ifstream file(input_file_path);

    string line;
    string theater_name;
    string client_count_str;

    getline(file, theater_name);
    getline(file, client_count_str);

    int client_count = stoi(client_count_str);
    clients_left = client_count;

    pthread_t client_thread_ids[client_count];
    string clients[client_count];
    buffer = new job[client_count];

    pthread_t teller_thread_ids[3];

    if (theater_name.compare("OdaTiyatrosu\r") == 0)
    {
        seats = (bool *)calloc(60, sizeof(bool));
        //seats = new bool[60]();
        total_seats = 60;
    }
    else if (theater_name.compare("UskudarStudyoSahne\r") == 0)
    {
        seats = (bool *)calloc(80, sizeof(bool));
        total_seats = 80;
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

    for (int i = 0; i < 3; i++)
    {
        pthread_join(teller_thread_ids[i], NULL);
    }

    // delete buffer;
    free(seats);
    pthread_mutex_destroy(&mutex);
    sem_destroy(&full);
    sem_destroy(&empty);

    return 0;
}

void *client_runner(void *params)
{
    string *client = (string *)params;
    regex reg(",");
    sregex_token_iterator iter((*client).begin(), (*client).end(), reg, -1);
    sregex_token_iterator end;
    vector<string> vec(iter, end);

    string client_name = vec[0];
    double arrival_time = stoi(vec[1]) * 1000;
    double service_time = stoi(vec[2]) * 1000;
    int seat_number = stoi(vec[3]);
    usleep(arrival_time);

    sem_wait(&empty);
    pthread_mutex_lock(&mutex);

    struct job reservation = {client_name, seat_number, service_time};
    buffer[buffer_in] = reservation;
    buffer_in++;

    pthread_mutex_unlock(&mutex);
    sem_post(&full);

    pthread_exit(NULL);
}

void *teller_runner(void *params)
{
    char *teller = (char *)params;
    bool print = false;
    struct job reservation;
    int reserved;

    while (true)
    {
        if (clients_left == 0)
        {
            break;
        }

        bool turn = checkIfMyTurn(*teller);
        if (turn)
        {
            sem_wait(&full);
            pthread_mutex_lock(&mutex);
            busy[*teller - 65] = true;
            reservation = buffer[buffer_out];
            buffer_out++;
            if (taken_seats != total_seats)
            {
                if (!seats[reservation.seat_number - 1])
                {
                    seats[reservation.seat_number - 1] = true;
                    reserved = reservation.seat_number;
                }
                else
                {
                    for (int i = 0; i < total_seats; i++)
                    {
                        if (!seats[i])
                        {
                            seats[i] = true;
                            reserved = i + 1;
                            break;
                        }
                    }
                }
                clients_left--;
                taken_seats++;
            }
            pthread_mutex_unlock(&mutex);
            sem_post(&empty);
        }

        if (busy[*teller - 65])
        {
            usleep(reservation.service_time);
            if (taken_seats == total_seats)
            {
                printf("%s requests seat %d, reserves None. Signed by Teller %c.\n", reservation.client_name.c_str(), reservation.seat_number, *teller);
            }
            else
            {
                printf("%s requests seat %d, reserves seat %d. Signed by Teller %c.\n", reservation.client_name.c_str(), reservation.seat_number, reserved, *teller);
            }
            busy[*teller - 65] = false;
        }
    }
    pthread_exit(NULL);
}

bool checkIfMyTurn(char teller)
{
    if (teller == 'A')
    {
        return true;
    }
    else if (teller == 'B')
    {
        if (busy[0])
        {
            return true;
        }
    }
    else if (teller == 'C')
    {
        if (busy[0] && busy[1])
        {
            return true;
        }
    }
    return false;
}
