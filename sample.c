#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

typedef enum
{
    false = 0, 
    true = 1
}bool;

void sendData(int power, int datalen, int ueid)
{

}

void reciveData()
{

}

void exitApp(int sign)
{
    printf("exit();");
    exit(0);
}

int main(int argc, char *argv[])
{
    int i = 1;
    bool send = false;
    bool recive = false;
    int power = 10;
    int ueid = 211;
    int datalen = 0;
    for(i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-s")) send = true;
        else if (strcmp(argv[i], "-r")) recive = true;
        else if (strcmp(argv[i], "-p")) // power
        {
            if(++i < argc ) power = atoi(argv[i]);
        }
        else if (strcmp(argv[i], "-u")) // ue_id
        {
            if (++i < argc) ueid = atoi(argv[i]);
        }
        else if (strcmp(argv[i], "-l")) // length
        {
            if (++i < argc) datalen = atoi(argv[i]);
        }
    }

    signal(SIGINT, exitApp);
    if(send == true)
    {
        sendData(power, datalen, ueid);
    }
    else if (recive == true)
    {
        reciveData();
    }

    while(1)
    {
        sleep(10);
    }

}
