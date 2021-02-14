#include <iostream>
#include <fstream>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <regex>

using namespace std;

pthread_mutex_t mutex; // main mutex for teller and client threads.
pthread_mutex_t busy_mutex; // to read and modify the "busy" array out of the "mutex" we use "busy_mutex".
pthread_mutex_t clients_left_mutex; // to read and modify the "clinets_left" integer out of the "mutex" we use "clients_left_mutex".
sem_t full; // initialized with 0. holds the number of clients in the "buffer".
sem_t empty; // initialized with 3. holds the maximum capacity of the "buffer".

struct job // job is the general definition of client in the "buffer" array
{
    string client_name;
    int seat_number;
    double service_time;
};
struct job *buffer; // buffer array. later initialized with length "client_count".

bool busy[3] = {false}; // array that holds boolean value representing teller threads' availability.

void *client_runner(void *params); // client thread function
void *teller_runner(void *params); // teller thread function
bool checkIfMyTurn(char teller); // method that checks if it is given teller's turn.

int buffer_in = 0; // index of "buffer" where client threads will place a job.
int buffer_out = 0; // index of "buffer" where teller threads will take a job.

bool *seats; // holds the boolean value of the seats availability. true means taken, and false not taken.
int seats_left; // holds the number of seats left in the theater.
int total_seats; // holds the total number of seats according to the theater name.

int clients_left; // clients that have not been serviced yet. initialized to "total_client_count".

FILE *output_file; // pointer to the file that output is written.

int main(int argc, char const *argv[])
{
    // semaphores and mutex initialization.
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, 3);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&busy_mutex, NULL);
    pthread_mutex_init(&clients_left_mutex, NULL);


    // input file path is obtained.
    string input_file_path = argv[1];
    ifstream file(input_file_path);

    // output file path is obtained. and file created.
    output_file = fopen(argv[2], "w");
    fprintf(output_file, "Welcome to the Sync-Ticket!\n");

    string line; // line that is read from the input file.  
    string theater_name; // theater_name
    string client_count_str; // client count (string)

    // theater name and client count values obtained.
    getline(file, theater_name); 
    getline(file, client_count_str);

    int client_count = stoi(client_count_str); // string to int conversion
    clients_left = client_count; 

    pthread_t client_thread_ids[client_count]; // array that will holld the client thread ids
    string clients[client_count]; // array that holds each line that was written from the input file. e.g. "Client1,10,50,5".
    buffer = new job[client_count]; 

    pthread_t teller_thread_ids[3]; // array that will holld the teller thread ids

    // part where we decide on the theater name and initialize the variables like total_seats accordingly.
    if (theater_name.compare("OdaTiyatrosu") == 0)
    {
        seats = new bool[60]();
        seats_left = 60;
        total_seats = 60;
    }
    else if (theater_name.compare("UskudarStudyoSahne") == 0)
    {
        seats = new bool[80]();
        seats_left = 80;
        total_seats = 80;
    }
    else if (theater_name.compare("KucukSahne") == 0)
    {
        seats = new bool[200]();
        seats_left = 200;
        total_seats = 200;
    }

    // teller threads created with parameters of their character.
    char a = 'A';
    char b = 'B';
    char c = 'C';
    pthread_create(&teller_thread_ids[0], NULL, &teller_runner, &a);
    pthread_create(&teller_thread_ids[1], NULL, &teller_runner, &b);
    pthread_create(&teller_thread_ids[2], NULL, &teller_runner, &c);

    // with lines we read from the input file we create the client threads.
    for (int i = 0; i < client_count; i++)
    {
        getline(file, clients[i]);
        pthread_create(&client_thread_ids[i], NULL, &client_runner, &clients[i]);
    }

    // joining the client threads and main thread.
    for (int i = 0; i < client_count; i++)
    {
        pthread_join(client_thread_ids[i], NULL);
    }
    
    // joining the teller threads and main thread.
    for (int i = 0; i < 3; i++)
    {
        pthread_join(teller_thread_ids[i], NULL);
    }

    // final output.
    fprintf(output_file, "All clients received service.");

    // destroying the variables allocated to heap memoery.
    delete[] seats;
    delete[] buffer;
    pthread_mutex_destroy(&mutex);
    sem_destroy(&full);
    sem_destroy(&empty);

    return 0;
}

void *client_runner(void *params)
{
    // tokenizes the line by ",", and gets attributes of each client. 
    string *client = (string *)params;
    regex reg(",");
    sregex_token_iterator iter((*client).begin(), (*client).end(), reg, -1);
    sregex_token_iterator end;
    vector<string> vec(iter, end);

    string client_name = vec[0];
    double arrival_time = stoi(vec[1]) * 1000;
    double service_time = stoi(vec[2]) * 1000;
    int seat_number = stoi(vec[3]);
     
    usleep(arrival_time); // sleeps as long as arrival time. so it won't get into queue.

    // critical section starts.
    sem_wait(&empty);
    pthread_mutex_lock(&mutex);

    struct job reservation = {client_name, seat_number, service_time}; // creates the job structure.
    buffer[buffer_in] = reservation; // places the job into the buffer so tellers can process it.
    buffer_in++; // increasing the buffer_in so next client will not place to the same index.

    pthread_mutex_unlock(&mutex);
    sem_post(&full);
    // critical section ends.

    pthread_exit(NULL);
}

void *teller_runner(void *params)
{
    char *teller = (char *)params; // teller is a pointer to the character of each teller. e.g. "A".
    struct job reservation; 

    // first teller output.
    fprintf(output_file, "Teller %c has arrived.\n", *teller);

    while (true)
    {
        int reserved = -1; // holds the seat reserved by the client.

        if (checkIfMyTurn(*teller)) // checks if it is the turn of the teller.
        {
            pthread_mutex_lock(&busy_mutex); 
            busy[*teller - 65] = false; // before going into critical section it sets its value to false.
            pthread_mutex_unlock(&busy_mutex);
            
            // critical section start.
            sem_wait(&full);
            
            if (!clients_left) // if clients_left is zero, this means that tellers need to exit.
            {
                sem_post(&full);
                break;
            }

            // tellers checks this twice each iteration because while a teller is waiting
            // another teller which is ranked higher than the other might finish its job
            // and become available again.
            if (!checkIfMyTurn(*teller))
            {
                sem_post(&full); // since this teller is leaving the critical section it increases the full semaphore.
                continue;
            }

            pthread_mutex_lock(&mutex);

            pthread_mutex_lock(&busy_mutex);
            busy[*teller - 65] = true; // teller sets itself to busy.
            pthread_mutex_unlock(&busy_mutex);

            reservation = buffer[buffer_out]; // takes the job from buffer_out index.
            buffer_out++; // increases the buffer_out by one so next teller won't get the same job.
            if (seats_left) // if there is any seat left, teller finds the right seat for a client.
            {
                if (!seats[reservation.seat_number - 1] && reservation.seat_number <= total_seats) // if desired seat is not taken, and desired seat number is smaller than the total seat number.
                {
                    seats[reservation.seat_number - 1] = true; // reserves the seat by setting the appropriate index to true.
                    reserved = reservation.seat_number; // holds the reserved seat number for printing it later.
                }
                else // if desired seat taken, teller finds the available seat with lowest seat number.
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
                seats_left--; // seat number decreased.
            }
            clients_left--; // client number decreased.
            pthread_mutex_unlock(&mutex);
            sem_post(&empty);
            // critical section ends.

            usleep(reservation.service_time); // sleeps as long as service time before printing the output message and setting itself to not busy by continuing the iteration.

            if (reserved == -1)
            {
                fprintf(output_file, "%s requests seat %d, reserves None. Signed by Teller %c.\n", reservation.client_name.c_str(), reservation.seat_number, *teller);
            }
            else
            {
                fprintf(output_file, "%s requests seat %d, reserves seat %d. Signed by Teller %c.\n", reservation.client_name.c_str(), reservation.seat_number, reserved, *teller);
            }
        }
        pthread_mutex_lock(&clients_left_mutex);
        if (!clients_left) // checks if there are client left, if not exits iteration.
        {
            pthread_mutex_unlock(&clients_left_mutex);
            sem_post(&full);
            break;
        }
        pthread_mutex_unlock(&clients_left_mutex);
        

    }
    pthread_exit(NULL);
}

bool checkIfMyTurn(char teller) // this method is used for finding out if its specific teller's turn.
{
    pthread_mutex_lock(&busy_mutex);
    if (teller == 'A')
    {
        pthread_mutex_unlock(&busy_mutex);
        return true;
    }
    else if (teller == 'B')
    {
        if (busy[0])
        {
            pthread_mutex_unlock(&busy_mutex);
            return true;
        }
    }
    else if (teller == 'C')
    {
        if (busy[0] && busy[1])
        {
            pthread_mutex_unlock(&busy_mutex);
            return true;
        }
    }
    pthread_mutex_unlock(&busy_mutex);
    return false;
}
