#include <sys/types.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include </home/alexu101/Desktop/Offline Messenger/commandAndResponse.h>
#include <fstream>
using namespace std;

//ERORI
// -1 eroare la creearea socketului
// -2 eroare la conectarea la server

int startClient(int port, int socketDescriptor, sockaddr_in server) //subprogram ce porneste clientul
{
    if (socketDescriptor == -1) //verificare creare socket
    {
        perror("[ERROR] Eroare la crearea socketului \n \n");
        return -1;
    }

    //definirea familiei de socketuri
    //familia socket-ului
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    //conectare la server
    if (connect(socketDescriptor, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[ERROR] Eroare la conectare a clientului \n \n"); //in caz de eroare la conectarea cu clientul trimitem
        return -2;                                                //mesajul corespunzator si iesim din program
    }

    cout << "******FELICITARI! V-ATI CONECTAT CU SUCCES LA SERVERUL APLICATIEI <OFFLINE MESSENGER> "
         << "*****" << endl
         << endl;

    return 0;
}

bool logged = false; //variabila care retine daca userul curent este logat

//aplicatie care cere userului sa introduca o comanda din lista de comenzi

bool convStarted = false; //variabila care ne spune daca userul se afla intr-o convorbire

int takeInput() //subprogram ce interactioneaza la nivel de input si comenzi cu userul
{
    if (logged == false) //daca nu e logat ii cerem sa se logheze sau sa se inregistreze
    {
        int alegere = -1;
        while (alegere != 1 && alegere != 2 && alegere != 0) //1 pentru login 2 pentru register 0 pentru exit
        {
            cout << "[CLIENT] INTRODUCETI NUMARUL CORESPUNZATOR AL UNEIA DIN COMEZNILE :" << endl;
            cout << "1. LOGIN" << endl
                 << "2. REGISTER" << endl
                 << "0. EXIT" << endl;
            cout << "NUMARUL DVS. :";
            cin >> alegere;

            if (alegere != 1 && alegere != 2 && alegere != 0) //tratam si cazul in care un user a introdus o comanda ce nu era specificata
            {
                cout << "[ERROR] ALEGETI UNUL DINTRE NUMERELE SPECIFICATE! ";
                cout << endl
                     << endl;
            }
        }
        return alegere; //returnam alegerea userului(numarul comenzii)
    }
    else if (convStarted == false) //daca suntem logati putem alege dintr-un alt set de comenzi
    {
        int alegere = 0;
        while (alegere != 1 && alegere != 2 && alegere != 3) //1 pentru mesaj 2 pentru history 3 pentru logout
        {
            cout << "[CLIENT] INTRODUCETI NUMARUL CORESPUNZATOR AL UNEIA DIN COMEZNILE:" << endl;
            cout << "1. MESSAGE" << endl
                 << "2. SHOW CONVERSATION HISTORY WITH SOMEONE" << endl
                 << "3. LOGOUT " << endl;
            cout << "NUMARUL DVS. :";
            cin >> alegere;

            if (alegere != 1 && alegere != 2 && alegere != 3)
            {
                cout << "[ERROR] ALEGETI UNUL DINTRE NUMERELE SPECIFICATE! ";
                cout << endl
                     << endl;
            }
        }
        return alegere + 2; //ca sa fie comenzile de la 1 la 5 (1 si 2 logarea si registerul)
    }
    return -1;
}

char currUser[100]; //variabila in care retinem userul curent logat

void showUsers() //functii care arata toti userii existenti in sistem. functia are scopul de a vedea cui putem sa trimitem mesaje
{
    cout << endl
         << endl;
    cout << "Puteti conversa cu urmatorii useri: " << endl;
    char line[100];
    ifstream f;
    f.open("credentials.txt"); //deschidem fisierul cu useri
    while (f.getline(line, 100))
    {
        if (isalnum(line[0])) //testam posibile erori din fisier(cuvinte interzise, buguri din fisierul .txt)
        {
            string credentials = line;
            string username = credentials.substr(0, credentials.find(" ")); //extragem usernameul
            string curr = currUser;
            if (curr != username) //afisam toti userii mai putin cel activ(sa nu poata vorbi cu el insusi)
                cout << username << endl;
        }
    }
    f.close();
    cout << endl;
}

//subprogramul principal care va trata fiecare comanda
void processCmd(int alegere, int socketDescriptor, char response[100]) //trimitem cererea de logare/inregistrare spre server
{

    strcpy(response, ""); //raspunsul folosit pt a comunica cu serverul
    if (alegere == 0)     //comanda de exit
    {
        sendCMD(socketDescriptor, alegere); //ii trimitem comanda de exit
        recieveRSP(socketDescriptor, response);
    }
    if (alegere == 1) //comnda de logare
    {

        sendCMD(socketDescriptor, alegere); //trimitem comanda la server

        char id[100], parola[100];
        cout << "[CLIENT] Introduceti datele de logare : \n";
        cout << "id: ";
        cin >> id;                     //introducem id-ul
        sendCMD(socketDescriptor, id); //il trimitem la server
        cout << "parola: ";
        cin >> parola;                     //introducem parola
        sendCMD(socketDescriptor, parola); //o trimitem la server
        cout << endl
             << endl;
        recieveRSP(socketDescriptor, response); //primim raspuns de la server
        cout << endl
             << response << endl;
        if (strcmp(response, "Successfully logged in !") == 0) //daca logarea s-a putut realiza
        {
            logged = true;        //marcam faptul ca suntem logati
            strcpy(currUser, id); //retinem userul care tocmai s a conectat
            showUsers();          //afisam userii cu care poate sa comunice userul curent
        }
    }

    else if (alegere == 2) //daca s a introdus comanda pentru inregistrare
    {
        sendCMD(socketDescriptor, alegere); //trimitem comanda la server
        char id[100], parola[100];
        cout << "[CLIENT] Introduceti un id si o parola pentru a va inregistra(utilizati doar cifre si litere): \n";

        while (strcmp(response, "ID already exists, try again! ") == 0 || strcmp(response, "") == 0) //cat timp nu ne am inregistrat
        {
            if (strcmp(response, "ID already exists, try again!\n\n ") == 0) //testam daca inregistrarea s-a realizat cu succes
                cout << response;

            //inregistrarea efeciva
            bool correctPassword = false;
            while (correctPassword == false)
            {
                char check[100];
                cout << "id: ";
                cin >> id; //id-ul
                cout << "parola: ";
                cin >> parola; //parola
                cout << "confirmare parola: ";
                cin >> check; //confirmare parola

                //testam daca parolele se potrivesc
                if (strcmp(check, parola) == 0)
                    correctPassword = true;
                else //afisam mesajul corespunzator
                    cout << "Parolele nu se potrivesc, incercati din nou\n\n";
            }

            sendCMD(socketDescriptor, id);     //trimitem la server id-ul
            sendCMD(socketDescriptor, parola); //trimitem la server parola

            cout << endl
                 << endl;

            recieveRSP(socketDescriptor, response);                                       //primim de la server raspunsul cu privire la inregistrare
            if (strcmp(response, "Succcessfully registered! You are now logged in") == 0) //daca ne-am inregistrat cu succes
            {
                logged = true;        //marcam si faptul ca suntem logati acum
                strcpy(currUser, id); //retinem userul care tocmai s-a inregistrat drept user activ curent
                showUsers();          //ii aratam userii cu care poate conversa
            }
        }
        cout << endl
             << response << endl;
    }

    else if (alegere == 3) //daca se doreste inceperea unei conversatii
    {
        if (convStarted == false) //daca conversatia nu a inceput inca
        {
            convStarted = true; //intram pe aceasta ramura si marcam faptul ca conversatia a inceput
            char user[100];     //userul cu care dorim sa comunicam

            sendCMD(socketDescriptor, alegere); //trimitem comanda la server

            int correctUser = 0;
            while (correctUser == 0)
            {
                recieveRSP(socketDescriptor, response); //serverul ne va cere sa alegem cu cine vrem sa vorbim
                cout << response;
                cin >> user;                     //selectam cu cine vrem sa vorbim
                sendCMD(socketDescriptor, user); //trimitem inapoi la server persoana cu care am vrea sa vorbim
                recieveRSP(socketDescriptor, correctUser);
                if (correctUser == 0) //verificam daca userul a fost corect
                    cout << "This user doesn't exist!\n";
            }

            recieveRSP(socketDescriptor, response); //primim de la server raspunsul si il vom afisa
            cout << response << endl
                 << endl;

            cout << endl; //ii spunem userului ce comenzi trebuie sa introduca pentr
                          //a parasi conversatia si pentru a raspunde la un anume mesaj
            cout << "Pentru a parasi conversatia introduceti mesajul <EXIT> " << endl
                 << "Pentru a raspunde la un mesaj anume se va folosi comanda <reply>" << endl;
        }

        bool chatting = true; //marcam faptul ca ne aflam intr-o conversatie

        //cu ajorul unui fork vom trata primirea si trimiterea de mesaje
        int child = fork();

        //procesul copil verifica primirea de mesaje
        if (child == 0)
        {

            while (1)
            {

                char message[100];
                strcpy(message, "");

                recieveRSP(socketDescriptor, message); //receptionam mesajul de la server
                //if (strstr(message, "reply") == NULL)  //tratam cazul in care mesajul este "reply"(kind of a bug)
                cout << message << endl; //afisam mesajul
            }
        }

        //procesul parinte trimite mesajele
        else
        {

            while (chatting == true) //cat timp ne aflam in conversatie
            {
                char msg[100];

                cin.getline(msg, 100); //citim mesajul

                //tratam cazul de reply
                bool dontSend = false; //variabila asta asigura faptul ca comanda reply nu se va trimite de 2 ori
                                       //odata acum, si odata dupa tratarea cazului

                //daca userul a dat reply
                if (strcmp(msg, "reply") == 0)
                {
                    dontSend = true;                //activam variabila dontSend
                    sendCMD(socketDescriptor, msg); //trimitem comanda la server
                    char r[100], msgToRespond[100];

                    recieveRSP(socketDescriptor, r); //serverul ne va cere sa introducem mesajul la care vrem sa raspundem
                    cout << r;
                    cin >> msgToRespond;                     //il citim
                    sendCMD(socketDescriptor, msgToRespond); //il trimitem la server

                    int messageExists;                           //variabila utilizata pentru cazul in care mesajul la care
                                                                 //vrea sa se raspunda nu exista
                    recieveRSP(socketDescriptor, messageExists); //actualizam valoarea acesteia

                    if (messageExists == 1) //daca mesajul exista in conversatie
                    {
                        char raspunsMesaj[100];                  //continuam cu raspunsul la acesta
                        cout << "Introduceti raspunsul : ";      //cerem userului sa introduca un raspuns
                        cin >> raspunsMesaj;                     //citim raspunsul
                        sendCMD(socketDescriptor, raspunsMesaj); //il trimitem la server
                    }
                    else
                    {
                        //daca mesajul nu exista vom afisa un raspuns corespunzator
                        cout << "\nNu exista acest mesaj in conversatia cu userul dorit!\n\n";
                    }
                }

                //tratam cazul in care utiliztorul a dat EXIT(a iesit din conversatie)
                if (strcmp(msg, "EXIT") == 0) //testam daca userul 1 vrea sa paraseasca conversatia
                {
                    //daca a iesit din conversatie
                    chatting = false;//iesim din while
                    convStarted = false;//trecem inapoi la fereastra de logare(input)
                }

                //daca nu s a dat vreun reply deja
                if (dontSend == false)//trimitem mesajul la server
                    sendCMD(socketDescriptor, msg); //trimitem mesajul la server
            }
        }
        recieveRSP(socketDescriptor, response);//receeptionam raspunsul general de la server
    }

    //daca userul a cerut sa vada un istoric al conversatiei
    else if (alegere == 4)
    {
        char persoana[100];//variabila in care se retine persoana cu care comunicam
        sendCMD(socketDescriptor, alegere);//trimtem alegerea

        char response[100];
        recieveRSP(socketDescriptor, response);//primim raspunsul de la server
        cout << response;cin >> persoana;     //introducem persoana cu care am vrea sa vedem istoricul
        sendCMD(socketDescriptor, persoana); //ii trimitem serverului persoana

        int found;//variabila ce retine daca persoana introdusa exista in server sau nu
        recieveRSP(socketDescriptor, found);//primim aceasta confirmare de la server

        //daca persoana exista 
        if (found == true)
        {
            cout << endl
                 << "\n Your conversation history with " << persoana << " is: \n\n";

            //tratam cazul in care converastia este goala
            int convExists;

            recieveRSP(socketDescriptor, convExists);//serverul ne zice daca exista un istoric al conversatiei cu aceasta persoana

            if (convExists == 0)//daca  nu exista exista
            {
                char msg[100];
                recieveRSP(socketDescriptor, msg);//afisam raspunsul dat de server
                cout << msg;
            }
            else//daca exista
            {   
                //afisam toate mesajele din istoricul conversatiei
                //acestea sunt stocate in fisierul cu numele user1_user2.txt(numele celor doi useri)
                char msg[100];
                recieveRSP(socketDescriptor, msg);
                cout << msg;
                while (strcmp(msg, "EXIT"))//cat timp nu s a cerut sa se iasa din conversatie
                {
                    recieveRSP(socketDescriptor, msg);//receptionam mesajele 
                    if (strcmp(msg, "EXIT"))//si daca nu sunt EXIT
                        cout << msg;//le vom afisa
                }
            }
        }
    }

    //daca userul a cerut sa se delogheze
    else if (alegere == 5)
    {
        logged = false;//il delogam
        cout << "\n\nYou successfully logged out!\n\n";//afisam mesajul corespunzator
    }
}

int main()
{
    int port = 1026;//setam o valoare prestabilita a portului folosit
    struct sockaddr_in server;
    int socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    //raspunsul pe care il utilizam pentru a comunica cu serverul
    char response[100];

    //pornim clientul cu variabilele prestabilite pentru port
    //descriptorul socketului si 
    //adresa socketului
    startClient(port, socketDescriptor, server);

    //marcam faptul ca nicio conversatie nu este inceputa 
    //atunci cand pornim aplicatia si ca ne aflam in faza
    //de login
    bool convOn = false;

    while (1)
    {   
        //cazul in care ne aflam intr-o conversatie
        if (convStarted)
        {   
            //vom executa comanda 3(cea pentru mesaje)
            processCmd(3, socketDescriptor, response);
        }
        else//in caz contrar
        {   
            //vom cere input utilizatorului si vom executa comenzile cerute
            processCmd(takeInput(), socketDescriptor, response);

            //caz particular pentru inchiderea conversatiei
            if (strcmp(response, "\n Ati parasit conversatia! \n") == 0)
                cout << response;

            //caz particular pentru inchiderea aplicatiei
            if (strcmp(response, "\nApp closed!\n") == 0) //daca s a dat comanda de inchis aplicatia
            {
                cout << response;
                return 0; //inchidem aplicatia
            }
        }
    }

    //inchidem socketul 
    close(socketDescriptor);
}
