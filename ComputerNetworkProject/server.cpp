#include <sys/types.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fstream>
#include <regex>
#include </home/alexu101/Desktop/Offline Messenger/commandAndResponse.h>

using namespace std;
//portul utilizat
#define PORT 1026

//ERORI
//-1 eroare la startServer
//-2 eroare la bindSocket
//-3 eroare la serverListen
//-4 eroare la trimiterea raspunsului catre client

//subprogram ce va realiza bind-ul pentru a face posibila conectarea cu clientul
int bindSocket(int socketDescriptor, sockaddr_in server)
{

    if (bind(socketDescriptor, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("Eroare la Bind\n \n");//daca bind-ul a esuat afisam acest caz
        /*cout << "[SERVER] Eroare la bind!" << endl
             << endl;*/
        return -1;
    }
    return 0;
}

//subprogram de porneste serverul
int startServer(sockaddr_in server, int socketDescriptor)
{   
    if (socketDescriptor == -1) //testarea erorilor la crearea socketului
    {
        cout << "[SERVER] Eroare la crearea socketului" << endl
             << endl;

        return -1;
    }

    //definirea familiei de socketuri
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    //realizam bindul 
    bindSocket(socketDescriptor, server);
    return 0;
}

//subprogram ce verifica credentialele userului(nume+parola) in momentul logarii/inregistrarii
fstream f;
bool verifyCredentials(char credentials[100])
{
    char cred[101];

    f.open("credentials.txt");//deschidem fisierul cu datele utilizatorilor
    while (f.getline(cred, 100))
    {   
        //citim datele de conectare una cate una
        if (strcmp(cred, credentials) == 0)
        {   
            //daca userul exista returnam true
            f.close();
            return true;
        }
    }
    //returnam false in caz contrar
    f.close();
    return false;
}

//functie cu ajutorul careia adaugam 
//in fisierul cu useri useri noi
bool addCredentials(char credentials[100], char user[100])
{

    //verificam daca userul nu cumva exista deja
    f.open("credentials.txt");

    char s[100];
    while (f.getline(s, 100))
    {

        if (strstr(s, user) - s == 0)

            if (strstr(s, user) - s == 0)
                if ((s[strlen(user)] == ' '))//daca s-a mai gasit userul 
                {
                    f.close();
                    return false;//returnam false
                }
    }
    f.close();

    f.open("credentials.txt", std::ios::app);//deschidem fisierul pentru scriere la finalul acestuia
    f << endl;//trecem pe urmatorul rand
    f << credentials;//introducem datele noului utilizator
    f.close();//inchidem fisierul
    return true;//returnam true(faptul ca userul a fost adaugat in sistem si nu mai exista altul identic cu el)
}

//verifica daca un nume de utilizator exista in server
bool existentUser(char user[100])
{
    char line[100];
    ifstream g("credentials.txt");//deschidem pentru citire fisierul cu datele utilizatorilor
   
    while (g.getline(line, 100))
    {
        string credentials = line;//citim intreaga linie
        string username = credentials.substr(0, credentials.find(" "));//extragem numele de utilizator
        string u = user;
        if (user == username)//daca userul deja exista
        {   
           
            g.close();
            return true;
        }
    }
    g.close();
    return false;
}

/*
-> inainte de a ajunge in fisierul cu istoricul conversatiei
dintre cei doi useri, fiecare mesaj va fi scris intr-un fisier
temporar (temp).
-> acesta va ramane activ(nesters) pana cand se va citi din el un mesaj
de catre userul care va trebui sa primeasca acest mesaj. dupa citire
fisierul temporar va fi sters.
->acest fisier temp ne va indica practic daca un user doreste sa trimita un mesaj
catre altul, deci practic va facilita trimiterea de mesaje
->fisierul temp va fi utilizat strict pentru trimiterea individuala a fiecarui mesaj
in parte si nu pentru altceva
*/

//cu aceasta functie vom citi din fisierul temporar mesajul(mesajele)
//va avea ca parametri:
// ->o variabila in care se va memora mesajul
// ->cate o variabila pentru numele celor doi utilizatori 
//intre care se realizeaza conversatia
void sendMessageToTemp(char message[100], char user1[100], char user2[100])
{
    FILE *temp;

    //construim numele fisierului temp
    char fileName[100];//variabila in il vom construi

    //numele fisierului va fi de forma user1_user2_temp.txt"
    //user1 si user2 fiind numele utilizatorilor
    strcpy(fileName, user1);
    strcat(fileName, "_");
    strcat(fileName, user2);
    strcat(fileName, "_");
    strcat(fileName, "temp");
    strcat(fileName, ".txt");

    //daca nu s a putut realiza deschiderea fisierului
    if ((temp = fopen(fileName, "a+")) == NULL)
    {
        cout << "Error opening file";//afisam mesajul corespunzator
        return;//iesim din aplicatie
    }

    //scriem in temp mesajul 
    //mesajele vor avea un separator specific ce va ajuta ulterior la citirea lor
    fprintf(temp, "%s@#!", message); //@#! este deilimitatorul mesajelor
    fclose(temp);//inchidem fisierul temp

    sleep(2);
}

//functie ce creeaza numele fisierului conversatiei a doi utilizatori
//acest fisier va servi ca istoric de mesaje
//parametrii sunt numele celor doi utilizatori si variabila in carfe va fi creat numele fisierului
void createTempName(char user1[100], char user2[100], char tempName[100])
{
    strcpy(tempName, user1);
    strcat(tempName, "_");
    strcat(tempName, user2);
    strcat(tempName, "_");
    strcat(tempName, "temp");
    strcat(tempName, ".txt");
}


//functie ce citeste mesajul(mesajele) din fisierul temporar
//are drept parametri mesajul in care se va face citirea si 
//numele fisierului de unde se va citi
int readMessageFromTemp(char fileName[100], char message[100])
{
    FILE *temp;

    //daca nu s-a putut deschide fisierul pentru citire
    if ((temp = fopen(fileName, "r")) == NULL)
    {
        return -1;
        //inseamna ca nu s-a trimis niciun mesaj
        //nu exista in temp niciun mesaj ce ar indica o conversatie
        //acitva intre doi utilizatori
        
    }

    sleep(2);

    //citim toate mesajele(posibil aflate in bulk, separate de delimitatorul specificat)
    //vom construi un singur mesaj mare care va pleca la server

    char word[100];
    fgets(word, sizeof(word), temp);
    char *p;
    strcpy(message, "");
    p = strtok(word, "@#!");
    while (p)
    {
        strcat(message, p);
        strcat(message, "\n");
        p = strtok(NULL, "@#!");
    }

    fclose(temp);     //inchidem fisierul
    remove(fileName); //stergem fisierul temporar pentru a marca 
                      //faptul ca nu mai exista niciun mesaj netrimis
    return 1;
}

//functie ce creeaza numele fisierelor in care se stocheaza istoricul conversatiilor
//intre cei oricare 2 utilizatori
void createChatConversationFile(char user1[100], char user2[100], char fileName[100])
{
    strcpy(fileName, user1);
    strcat(fileName, "_");
    strcat(fileName, user2);
    strcat(fileName, ".txt");
}

//in aceasta variabila vom memora primul utilizator
//utilizatorul care va incepe conversatia
char user1[100];

//functie ce testeaza daca un mesaj anume exista intr-o conversatie
//functia aceasta ne ajuta atunci cand un user va dori sa dea reply
//la un anumit mesaj
bool messageExistsInConv(char mesaj[100], char user1[100], char user2[100])
{
    char fileName[100];
    createChatConversationFile(user1, user2, fileName);//cream numele fisierului in care vom cauta(istoric de conversatie)

    ifstream fis(fileName);//deschidem fisierul
    char msg[100];//mesajele citite linie cu linie din fisier
    while (fis.getline(msg, 100))
    {
        if (strstr(msg, mesaj) != NULL)//daca mesajul exista
        {
            return true;
            fis.close();
        }
    }
    fis.close();
    return false;
}

//subprogramul principal ce va executa comenzile actionate
//de client si va oferi raspunsurile corespunzatoare
//ca parametri va avea numarul comenzii si clientul acceptat de socket
void executeCommand(int command, int clientSocketDescriptor)
{
    char rsp[100];//vom folosi rsp pentru raspunsurile trimise catre client

    //daca userul a da comanda EXIT
    if (command == 0)
    {   
        //procesam comanda si o trimtem catre client
        //acesta se va inchide si va fisa un raspuns
        strcpy(rsp, "\nApp closed!\n");
        sendCMD(clientSocketDescriptor, rsp);
    }

    //daca se da comanda de login
    if (command == 1) 
    {
        char id[100];//variabila in care se va memora id-ul
        recieveRSP(clientSocketDescriptor, id);//preluam de la client id-ul
        char parola[100];//variabila in care se va memora parola
        recieveRSP(clientSocketDescriptor, parola);//preluam de la client parola

        //verificam credentialele
        //cream un credential cu formatul respectiv
        char crd[100];
        strcpy(crd, id);
        strcat(crd, " ");
        strcat(crd, parola);

        //facem verificarea ceruta
        if (verifyCredentials(crd))//daca userul exista
        {   
            //logam pe server userul respectiv 
            //ii trimitem si un mesaj de confirmare
            strcpy(user1, id);
            strcpy(rsp, "Successfully logged in !");
            sendCMD(clientSocketDescriptor, rsp);
        }
        else
        {   
            //ii trimitem un mesaj cu faptul ca nu exista pe server
            //datele introduse de el nu au fost bune
            strcpy(rsp, "Login failed! Incorrect id or password! ");
            sendCMD(clientSocketDescriptor, rsp);
        }
    }

    //daca userul vrea sa se inregistreze pe server
    else if (command == 2)
    {

        char id[100];//variabila in care se va memora id-ul
        recieveRSP(clientSocketDescriptor, id);//preluam de la client id-ul
        char parola[100];//variabila in care se va memora parola
        recieveRSP(clientSocketDescriptor, parola);//preluam de la client parola

        //cream un credential cu formatul dorit
        char crd[100];
        strcpy(crd, id);
        strcat(crd, " ");
        strcat(crd, parola);

        //daca userul deja exista in sistem
        if (addCredentials(crd, id) == false)
        {   
            //trimitem catre client mesajul corespunzator
            strcpy(rsp, "ID already exists, try again! ");
            sendCMD(clientSocketDescriptor, rsp);
        }
        else
        {   
            //daca nu exista, atunci inregistrarea s-a realizat
            //cu succes si un mesaj de confirmare va fi trimis catre client
            strcpy(rsp, "Succcessfully registered! You are now logged in");
            sendCMD(clientSocketDescriptor, rsp);
        }
    }

    //daca cineva vrea sa porneasca o conversatie
    else if (command == 3) 
    {

        char user2[100];//variabila ce retine destinatarul
        int correctUser = 0;//variabila ce ne zice daca userul exista in baza de date

        //ii cerem clientului sa ne dea un destinatar valid
        while (correctUser == 0)
        {

            strcpy(rsp, "PICK SOMEONE TO CHAT WITH: ");
            sendCMD(clientSocketDescriptor, rsp);
            recieveRSP(clientSocketDescriptor, user2);
            if (existentUser(user2) == true)//daca s-a dat un destiantar valid oprim cererile
                correctUser = 1;
            sendCMD(clientSocketDescriptor, correctUser);
        }
      
        //ii confirmam clientului ca inceput conversatia cu userul dorit 
        char s[100];
        strcpy(s, "Ai inceput conversatia cu userul ");
        strcat(s, user2);
        sendCMD(clientSocketDescriptor, s);
    

        //cream fisierele de conversation history
        char fileName[100];

        //fisierul pentru primul user
        strcpy(fileName, "");
        createChatConversationFile(user1, user2, fileName);
        ofstream historyFile1(fileName, std::ios_base::app); //il desdchidem in append mode ca sa nu stergem continutul

        //fisierul pentru cel de al doilea user
        strcpy(fileName, "");
        createChatConversationFile(user2, user1, fileName);
        ofstream historyFile2(fileName, std::ios_base::app);


        //utilizam un fork pentru primirea si trimiterea de mesaje
        int child = fork();
        if (child == 0) 
        {
            //procesul copil se va ocupa de citirea mesajelor din fisierul temporar
            //si de expedierea acestora catre client in conversatia lui
            while (1)
            {
                char message[100];//variabila in care se va retine mesajul
                strcpy(message, "");

                char tempName[100];//variabila in care se retine numele fisierului temporar
                strcpy(tempName, ""); //cream numele fisierului temp
                createTempName(user1, user2, tempName);

                int x = readMessageFromTemp(tempName, message); //se citeste din temp daca se poate

                if (x != -1)//daca citirea s-a realizat
                {
                    if (strcmp(message, "EXIT") == 0)//verificam daca s-a dat exit
                        exit(0);

                    //trimitem mesajul la client
                    sendCMD(clientSocketDescriptor, message);
                }
            }
        }

        //procesul parinte va primi mesajele de la destinatar
        //va trata posibilul caz de reply
        //si va scriele mesajele in fisierul temp
        else
        {   
            while (1)
            {

                char message[100];//variabila in care primim mesajul de la client
                recieveRSP(clientSocketDescriptor, message);

                //tratam cazul de reply
                if (strcmp(message, "reply") == 0)
                {
                    char r[100], msgToRespond[100];//daca s-a dat reply
                    strcpy(r, "Introduceti mesajul la care vreti sa raspundeti: ");//ii cerem clientului sa ne spuna
                                                                                   //la ce mesaj vrea sa dea reply
                    sendCMD(clientSocketDescriptor, r);

                    //preluam de la client mesajul la care vrea sa raspunda
                    recieveRSP(clientSocketDescriptor, msgToRespond);

                    int messageExists;//variabila ce ne spune daca un mesaj exista intr-un conversation history
                    if (messageExistsInConv(msgToRespond, user1, user2))//verificam daca mesajul exista
                    {
                        messageExists = 1;
                    }
                    else//marcam verificarea
                    {
                        messageExists = 0;
                    }

                    //trimtem rezultatul cautarii la client
                    sendCMD(clientSocketDescriptor, messageExists);

                    //daca mesajul exista
                    if (messageExists == 1)
                    {
                        char raspunsMesaj[100];//ii cerem un raspuns la mesajul dat
                        recieveRSP(clientSocketDescriptor, raspunsMesaj);

                        //construim un raspuns final care va pleca
                        //ulterior spre destinatar(raspunsul va contine)
                        //si sender-ul sau
                        char finalAnswer[100];
                        strcpy(finalAnswer, raspunsMesaj);
                        strcat(finalAnswer, " ( raspuns la : ");
                        strcat(finalAnswer, msgToRespond);
                        strcat(finalAnswer, " )");
                        strcpy(message, finalAnswer);
                    }
                }

                //testam posibile mesaje invalide pe care nu am vrea sa le trimitem
                if (message[0] != '\u0003' && int(message[0]) != 0)
                {
                    
                    //verificam daca nu s-a dat exit din conversatie
                    if (strcmp(message, "EXIT") == 0)
                    {
                        //daca da, nu mai trimitem niciun mesaj
                        break;
                    }

                    //in fullMsg vom construi mesajul final
                    //care va fi scris in fisierul temp
                    //urmand sa fie citit de procesul copil si trimis catre
                    //destintar(client)
                    char fullMsg[101];
                    strcpy(fullMsg, user1);
                    strcat(fullMsg, ": ");
                    strcat(fullMsg, message);
                    sendMessageToTemp(fullMsg, user2, user1);

                    //introducem mesajele si in istoricele conversatiilor
                    //celor doi utilizatori care comunica intre ei acum
                    historyFile1 << fullMsg << endl;
                    historyFile2 << fullMsg << endl;
                }
            }
        }

        //trimitem un raspuns in cazul in care s-a 
        //dat exit si s-a parasit conversatia
        strcpy(rsp, "\n Ati parasit conversatia! \n");
        sendCMD(clientSocketDescriptor, rsp);
    }
    
    //daca clientul cere sa vada istoricul conversatiei cu cineva
    else if (command == 4) 
    {
        //ii cerem sa ne spuna numele persoanei al carui istoric vrea sa il vada
        char rsp[100], persoana[100];
        strcpy(rsp, "Select someone you talked to: ");
        sendCMD(clientSocketDescriptor, rsp);

        recieveRSP(clientSocketDescriptor, persoana); //primim persoana de la client
        //facem o cautare sa vedem daca persoana exista
        ifstream fisier("credentials.txt");
        char s[100];
        int found = 0;
        while (fisier.getline(s, 100) && found == 0)
        {

            if (strstr(s, persoana) - s == 0)
                if (strstr(s, persoana) - s == 0)
                    if ((s[strlen(persoana)] == ' '))
                    {
                        fisier.close();
                        found = 1;
                    }
        }
        
        //trimitem catre client daca s-a gasit sau nu persoana
        sendCMD(clientSocketDescriptor, found);

        if (found == true) //daca persoana exista
        {
            char fileName[100];
            createChatConversationFile(user1, persoana, fileName);

            FILE *file1;
            char msg[100];
            int convExists;
            //verificam daca istoricul e gol sau nu
            if ((file1 = fopen(fileName, "r")) == NULL)
            {
                convExists = 0;
            }
            else
                convExists = 1;

            //ii spunem clientului daca istoricul e gol sau nu
            sendCMD(clientSocketDescriptor, convExists);

            //daca este gol
            if (convExists == false)
            {
                strcpy(rsp, "\n EMPTY\n\n");
                sendCMD(clientSocketDescriptor, rsp);
                //ii trimitem clientului un mesaj corespunzator
            }

            //daca istoricul conversatiei respective exista
            else
            {   
                //citim din fisier si trimitem mesaj 
                //cu mesaj inapoi la client afisandu-i
                //astfel tot istoricul conversatiei
                char msg[100];
                while (fgets(msg, 100, file1))
                {   
                    strcat(msg, "\n");
                    sendCMD(clientSocketDescriptor, msg);
                }
                strcpy(rsp, "EXIT");
                sendCMD(clientSocketDescriptor, rsp);
            }
            fclose(file1);
        }
    }

    //daca clientul vrea sa dea logout
    else if (command == 5) //logout
    {
        //clientul se va ocupa de asta
    }
}

//subprogram ce va realiza apelul listen si 
//va accepta clientii
int serverListen(int socketDescriptor)
{    
    //acceptam maxim 5 clienti
    int listened = listen(socketDescriptor, 5);
    if (listened == -1)//tratam posibile erori
    {
        cout << "[SERVER] Eroare la listen " << endl
             << endl;
        return -3;
    }

    cout << "**********ASTEPTAM CLIENTI SA SE CONECTEZE LA PORTUL " << PORT << "**********" << endl
         << endl;

    while (1)
    {
        //acceptarea si conectarea cu un nou client
        int clientSocketDescriptor = accept(socketDescriptor, NULL, NULL);

        if (clientSocketDescriptor < 0)//testam posibile erori la conectare
        {
            cout << "[SERVER] Eroare la lconectarea la client" << endl
                 << endl;
            continue;
        }

        cout << "*********CONEXIUNE REALIZATA CU CLIENTUL, SE ASTEAPTA COMANDA*********" << endl
             << endl;
        
        //folosim un fork pentru a crea un server concurent
        //asta ne ofera posibilitatea interactionarii cu mai 
        //multi utilizatori simultan, nefiind nevoiti sa asteptam
        //ca serverul sa termine treaba cu fiecare in parte
        int child = fork();
        //procesul copil va trata si executa fiecare comanda in parte primita de la client
        if (child == 0)
        {

            while (1)
            {
                int command;
                recieveRSP(clientSocketDescriptor, command); //receptionam comanda de la client

                cout << "[SERVER] Comanda receptionat de la client: " << command << endl
                     << endl;
                executeCommand(command, clientSocketDescriptor);//o executam
            }
            close(clientSocketDescriptor); //inchidem clientul
        }
    }
}

int main()
{
    sockaddr_in server;//adresa socketului
    int socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);//instantiem socketul
    startServer(server, socketDescriptor);//pornim serverul
    while (1)
    {
        serverListen(socketDescriptor);//ascultam de client intr-o bucla
    }
}