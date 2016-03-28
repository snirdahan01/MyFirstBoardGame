#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>


int sharedMemoArr(int size, char ch);
int sharedMemoDraws(int size, char ch);
int sharedMemoFlag(char ch);
int sharedMemoOutPlayer1(int size, char ch);
int sharedMemoOutPlayer2(int size, char ch);
void initializeBoard(int *arr);
void printArray(int *arr, int size);
void createSons();
int throwTheDice();
void my_handler(int nsig);
void play1(int draw);
void play2(int draw);
void isWinner();
void freeSharedMemo();

#define arrSIZE 20
#define out 0
#define eaten 1

int *arr;
int *draws;
int *gameStarted; //Tells if the game is started.
int *outPlayer1;
int *outPlayer2;
int shmidArray, shmidDraws, shmidFlag, shmidOutPlayer1, shmidOutPlayer2;

int main(int argc, char *argv[])
{
    arr = (int*)calloc(arrSIZE, sizeof(int));
    draws = (int*)calloc(2, sizeof(int));

    time_t t;
    srand((unsigned) time(&t));

    shmidArray = sharedMemoArr(arrSIZE, 'S');
    shmidDraws = sharedMemoDraws(2, 'T');
    shmidFlag = sharedMemoFlag('R');
    shmidOutPlayer1 = sharedMemoOutPlayer1(2, 'K');
    shmidOutPlayer2 = sharedMemoOutPlayer2(2, 'E');

//    freeSharedMemo();

    initializeBoard(arr);//Initialize board with players 1 and 2.
    printArray(arr, arrSIZE);
    createSons();

    freeSharedMemo();

    return 0;
}

int sharedMemoOutPlayer2(int size, char ch)
{
    key_t key;
    int shmid;
    key = ftok(".", ch);

    if((shmid = shmget(key, size, IPC_CREAT|0666)) == -1)
    {
        printf("Flag shared memory segment exists - opening as client\n");
        if((shmid = shmget(key, size, 0)) == -1)
        {
            perror("Bad shmget");
            exit(1);
        }
    }
    else
    {
        printf("Creating new gameStarted shared memory segment\n");
    }
    if((outPlayer2 = (int *) shmat(shmid, 0, 0)) == NULL)
    {
        perror("Shmat");
        exit(1);
    }

    return shmid;
}

int sharedMemoOutPlayer1(int size, char ch)
{
    key_t key;
    int shmid;
    key = ftok(".", ch);

    if((shmid = shmget(key, size, IPC_CREAT|0666)) == -1)
    {
        printf("Flag shared memory segment exists - opening as client\n");
        if((shmid = shmget(key, size, 0)) == -1)
        {
            perror("Bad shmget");
            exit(1);
        }
    }
    else
    {
        printf("Creating new gameStarted shared memory segment\n");
    }
    if((outPlayer1 = (int *) shmat(shmid, 0, 0)) == NULL)
    {
        perror("Shmat");
        exit(1);
    }

    return shmid;
}

int sharedMemoFlag(char ch)
{
    key_t key;
    int shmid;
    key = ftok(".", ch);

    if((shmid = shmget(key, sizeof(int), IPC_CREAT|0666)) == -1)
    {
        printf("Flag shared memory segment exists - opening as client\n");
        if((shmid = shmget(key, sizeof(int), 0)) == -1)
        {
            perror("Bad shmget");
            exit(1);
        }
    }
    else
    {
        printf("Creating new gameStarted shared memory segment\n");
    }
    if((gameStarted = (int *) shmat(shmid, 0, 0)) == NULL)
    {
        perror("Shmat");
        exit(1);
    }

    return shmid;
}

int sharedMemoDraws(int size, char ch)
{
    key_t key;
    int shmid;
    key = ftok(".", ch);

    if((shmid = shmget(key, size, IPC_CREAT|0666)) == -1)
    {
        printf("Draws shared memory segment exists - opening as client\n");
        if((shmid = shmget(key, size, 0)) == -1)
        {
            perror("Bad shmget");
            exit(1);
        }
    }
    else
    {
        printf("Creating new draws shared memory segment\n");
    }
    if((draws = (int *) shmat(shmid, 0, 0)) == NULL)
    {
        perror("Shmat");
        exit(1);
    }

    return shmid;
}

int sharedMemoArr(int size, char ch) {
    key_t key;
    int shmid;

    key = ftok(".", ch);

    if ((shmid = shmget(key, size, IPC_CREAT | 0666)) == -1) {
        printf("Array shared memory segment exists - opening as client\n");
        if ((shmid = shmget(key, size, 0)) == -1) {
            perror("Bad shmget");
            exit(1);
        }
    }
    else {
        printf("Creating new array shared memory segment\n");
    }
    if ((arr = (int *) shmat(shmid, 0, 0)) == NULL) {
        perror("Shmat");
        exit(1);
    }

    return shmid;
}

void initializeBoard(int *arr)
{
    arr[0] = 1;
    arr[4] = 1;
    arr[5] = 1;
    arr[9] = 1;
    arr[10] = 2;
    arr[14] = 2;
    arr[15] = 2;
    arr[19] = 2;
}

void printArray(int *arr, int size)
{
    int i;

    printf("Array:");
    for(i=0; i<size; i++)
    {
        if(i == 0)
        {
            printf("{%d | ", arr[i]);
        }
        else if(i == arrSIZE - 1)
        {
            printf("%d}\n", arr[i]);
        }
        else
            printf("%d | ",arr[i]);
    }
}

void createSons()
{
    int pid1, pid2;
    int player1 = 0;
    int player2 = 10;

    time_t t;

    pid1 = fork();
    switch(pid1)
    {
        case -1:
            printf("No child created!\n");
            exit(1);
        case 0:
            signal(SIGINT, my_handler);
            while(1)
            {
                pause();
                srand((unsigned) time(&t));
                printf("Player1 Dice!\n");
                draws[0] = throwTheDice();
                if(*gameStarted == 1)
                {
                    play1(draws[0]);
                }
                sleep(4);
                kill(getppid(), SIGINT);
            }
            break;
        default:
            pid2 = fork();
            switch(pid2)
            {
                case -1:
                    printf("No child created!\n");
                    exit(1);
                case 0:
                    signal(SIGINT, my_handler);
                    while(1)
                    {
                        pause();
                        srand((unsigned) time(&t));
                        printf("Player2 Dice!\n");
                        draws[1] = throwTheDice();
                        if(*gameStarted == 1)
                        {
                            play2(draws[1]);
                        }
                        sleep(4);
                        kill(getppid(), SIGINT);
                    }
                    break;
                default:
                    signal(SIGINT,my_handler);
                    printf("I'm the father!\n");
                    sleep(4);
                    while(1)
                    {
                        kill(pid1, SIGINT);
                        pause();
                        printf("Player1 dice: %d\n", draws[0]);
                        printf("\n");
                        kill(pid2, SIGINT);
                        pause();
                        printf("Player2 dice: %d\n", draws[1]);
                        printf("\n");

                        if(draws[1] < draws[0])
                        {
                            *gameStarted = 1;
                            printf("Player1 win the dice and will start the game!\n");
                            printf("\n");
                            printArray(arr, arrSIZE);
                            while(1) {
                                kill(pid1, SIGINT);
                                pause();
                                printf("Player1 dice: %d\n", draws[0]);
                                isWinner();
                                printArray(arr, arrSIZE);

                                kill(pid2, SIGINT);
                                pause();
                                printf("Player1 dice: %d\n", draws[0]);
                                isWinner();
                                printArray(arr, arrSIZE);
                            }
                            break;
                        }
                        else if(draws[1] > draws[0])
                        {
                            *gameStarted = 1;
                            printf("Player2 win the dice and will start the game!\n");
                            printf("\n");
                            printArray(arr, arrSIZE);
                            while(1) {
                                kill(pid2, SIGINT);
                                pause();
                                printf("Player2 dice: %d\n", draws[1]);
                                isWinner();
                                printArray(arr, arrSIZE);

                                kill(pid1, SIGINT);
                                pause();
                                printf("Player1 dice: %d\n", draws[0]);
                                isWinner();
                                printArray(arr, arrSIZE);
                            }
                            break;
                        }
                        else
                            printf("Dice's are equals, try again please!\n");
                    }
            }
    }
}

void my_handler(int nsig)
{}

int throwTheDice()
{
    int randInt;
    randInt = rand() % 6 + 1;
    return randInt;
}

void play1(int draw)
{
    int i;
    int counterP1 = 0;

    for(i=15; i<arrSIZE; i++)
    {
        if(arr[i] == 1)
            counterP1++;
    }

    if(counterP1 == 4) {
        for (i = 15; i < arrSIZE; i++) {
            if (arr[i] == 1 && (i + draw) >= arrSIZE) {
                arr[i] = 0;
                outPlayer1[out]++;
                printf("player 1 moved a piece out from position %d", i + 1);
                return;
            }
        }
    }

    if(outPlayer1[eaten] > 0)
    {
        if(arr[draw] == 2)
        {
            arr[draw] = 1;
            outPlayer2[eaten]++;
            outPlayer1[eaten]--;
            printf("player 1 moved a piece in to positions %d", draw);
            return;
        }
        else if(arr[draw] == 0)
        {
            arr[draw] = 1;
            outPlayer1[eaten]--;
            printf("player 1 moved a piece in to positions %d", draw);
            return;
        }
    }

    for(i=0; i<arrSIZE; i++)
    {
        if(arr[i] == 1 && arr[i + draw] == 2)
        {
            arr[i] = 0;
            arr[i + draw] = 1;
            outPlayer2[eaten]++;
            printf("player 1 ate a piece from positions %d to %d", i+1, (i+1 + draw));
            return;
        }
    }

    for(i=0; i<arrSIZE; i++)
    {
        if(arr[i] == 1 && arr[i + draw] == 0)
        {
            arr[i] = 0;
            arr[i + draw] = 1;
            printf("player 1 moved a piece from positions %d to %d", i+1, (i+1 + draw));
            return;
        }
    }
}

void play2(int draw)
{
    int i;
    int counterP2 = 0;

    for(i=0; i<4; i++)
    {
        if(arr[i] == 2)
            counterP2++;
    }

    if(counterP2 == 4) {
        for (i = 4; i >= 0; i--) {
            if (arr[i] == 2 && (i + draw) < 0) {
                arr[i] = 0;
                outPlayer2[out]++;
                printf("player 2 moved a piece out from position %d", i + 1);
                return;
            }
        }
    }

    if(outPlayer2[eaten] > 0)
    {
        if(arr[arrSIZE - draw] == 1)
        {
            arr[arrSIZE - draw] = 2;
            outPlayer1[eaten]++;
            outPlayer2[eaten]--;
            printf("player 2 moved a piece in to positions %d", (arrSIZE - draw) + 1);
            return;
        }
        else if(arr[arrSIZE - draw] == 0)
        {
            arr[arrSIZE - draw] = 2;
            outPlayer2[eaten]--;
            printf("player 2 moved a piece in to positions %d", (arrSIZE - draw) + 1);
            return;
        }
    }

    for(i=arrSIZE - 1; i>=0; i--)
    {
        if(arr[i] == 2 && arr[i - draw] == 1)
        {
            arr[i] = 0;
            arr[i - draw] = 2;
            outPlayer1[eaten]++;
            printf("player 2 ate a piece from positions %d to %d", i+1, (i+1 - draw));
            return;
        }
    }

    for(i=arrSIZE - 1; i>=0; i--)
    {
        if(arr[i] == 2 && arr[i - draw] == 0)
        {
            arr[i] = 0;
            arr[i - draw] = 2;
            printf("player 2 moved a piece from positions %d to %d", i+1, (i+1 - draw));
            return;
        }
    }
}

void isWinner()
{
    if(outPlayer1[out] == 4)
    {
        printf("Player 1 is the WINNER!!!!!!!!\n");
        freeSharedMemo();
        exit(1);
    }
    if(outPlayer2[out] == 4)
    {
        printf("Player 2 is the WINNER!!!!!!!!\n");
        freeSharedMemo();
        exit(1);
    }
}

void freeSharedMemo()
{
    shmctl(shmidArray, IPC_RMID, 0);
    shmctl(shmidDraws, IPC_RMID, 0);
    shmctl(shmidFlag, IPC_RMID, 0);
    shmctl(shmidOutPlayer1, IPC_RMID, 0);
    shmctl(shmidOutPlayer2, IPC_RMID, 0);
}