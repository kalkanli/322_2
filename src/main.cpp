#include <iostream>
#include <fstream>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <regex>

using namespace std;

pthread_mutex_t mutex;
pthread_mutex_t decrease_mutex;
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
int seats_left;
int total_seats;

int clients_left;

FILE *output_file;

int main(int argc, char const *argv[])
{
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, 3);
    pthread_mutex_init(&mutex, NULL);

    string input_file_path = argv[1];
    ifstream file(input_file_path);

    output_file = fopen(argv[2], "w");
    fprintf(output_file, "Welcome to the Sync-Ticket!\n");

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

    if (theater_name.compare("OdaTiyatrosu") == 0)
    {
        seats = new bool[60]();
        seats_left = 60;
        total_seats = 60;
    }
    else if (theater_name.compare("UskudarStudyoSahne") == 0)
    {
        seats = new bool[80]();
        ;
        seats_left = 80;
        total_seats = 80;
    }
    else if (theater_name.compare("KucukSahne") == 0)
    {
        seats = new bool[200]();
        seats_left = 200;
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

    fprintf(output_file, "All clients received service.");

    delete[] seats;
    delete[] buffer;
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

    fprintf(output_file, "Teller %c has arrived.\n", *teller);

    while (true)
    {
        int reserved = -1;

        busy[*teller - 65] = false;
        if (checkIfMyTurn(*teller))
        {
            sem_wait(&full);

            if (!clients_left)
            {
                sem_post(&full);
                break;
            }

            if (!checkIfMyTurn(*teller))
            {
                sem_post(&full);
                continue;
            }

            pthread_mutex_lock(&mutex);

            busy[*teller - 65] = true;
            reservation = buffer[buffer_out];
            buffer_out++;
            if (seats_left)
            {
                if (!seats[reservation.seat_number - 1] && reservation.seat_number <= total_seats)
                {
                    seats[reservation.seat_number - 1] = true;
                    reserved = reservation.seat_number;
                }
                else
                {
                    for (int i = 0; i < seats_left; i++)
                    {
                        if (!seats[i])
                        {
                            seats[i] = true;
                            reserved = i + 1;
                            break;
                        }
                    }
                }
                seats_left--;
            }
            clients_left--;
            pthread_mutex_unlock(&mutex);
            sem_post(&empty);

            usleep(reservation.service_time);

            if (reserved == -1)
            {
                fprintf(output_file, "%s requests seat %d, reserves None. Signed by Teller %c.\n", reservation.client_name.c_str(), reservation.seat_number, *teller);
            }
            else
            {
                fprintf(output_file, "%s requests seat %d, reserves seat %d. Signed by Teller %c.\n", reservation.client_name.c_str(), reservation.seat_number, reserved, *teller);
            }
        }
        if (!clients_left)
        {
            sem_post(&full);
            break;
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
