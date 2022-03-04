#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cstring>
using namespace std;

//comenzi de primire si de citire prin socket pentru comenzi de tip int si char
void sendCMD(int d, char request[100])
{
    if (write(d, request, 100) <= 0)
        perror("[ERROR] Eroare la trimiterea comenzii spre server \n \n");
}


void sendCMD(int d, int request)
{
    if (write(d, &request, sizeof(request)) <= 0)
        perror("[ERROR] Eroare la trimiterea comenzii spre server \n \n");
}

void recieveRSP(int d, char response[100])
{
    if (read(d, response, 100) < 0)
    {
        perror("[ERROR] Eroare la citirea raspunsului de la server \n \n");
    }
}

void recieveRSP(int d,int &response)
{
    if (read(d, &response, sizeof(response)) < 0)
    {
        perror("[ERROR] Eroare la citirea raspunsului de la server \n \n");
    }
}