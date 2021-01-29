#include <iostream>
#include <fstream>
#include <string.h>

using namespace std;

void *client_runner(void *params);

int main(int argc, char const *argv[])
{
    // <initialization_part>
    string input_file_path = argv[1];
    ifstream file(input_file_path);

    string line;
    string theater_name;
    string client_count_str;
    int client_count;

    getline(file, theater_name);
    getline(file, client_count_str);
    client_count = stoi(client_count_str);
    cout << client_count << endl;
    string client[4];
    // <!initialization_part>


    // <teller_thread_start>
    //TODO TELLER
    // <!teller_thread_start>

    // <client_thread_start>
    for(int i=0; i<client_count; i++) {
        getline(file, client[0], ',');
        getline(file, client[1], ',');
        getline(file, client[2], ',');
        getline(file, client[3], '\n');
        client_runner(client); // this line should be thread start.
    }
    // <!client_thread_start>


    return 0;
}

void *client_runner(void *params) 
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
