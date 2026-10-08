#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define ITERATIONS 25

struct shared_data {
    int BankAccount;
    volatile int Turn;
};

void ParentProcess(struct shared_data *ShmPTR)
{
    int i, account, balance;

    srand((unsigned int)time(NULL) + (unsigned int)getpid());

    for (i = 0; i < ITERATIONS; i++) {
        sleep(rand() % 6);          /* 0 - 5 seconds */

        account = ShmPTR->BankAccount;

        while (ShmPTR->Turn != 0) {
            /* busy wait for parent's turn */
        }

        if (account <= 100) {
            balance = rand() % 101; /* 0 - 100 inclusive */

            if (balance % 2 == 0) {
                account += balance;
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n",
                       balance, account);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n",
                   account);
        }

        ShmPTR->BankAccount = account;
        ShmPTR->Turn = 1;
    }
}

void ChildProcess(struct shared_data *ShmPTR)
{
    int i, account, balance;

    srand((unsigned int)time(NULL) + (unsigned int)getpid());

    for (i = 0; i < ITERATIONS; i++) {
        sleep(rand() % 6);          /* 0 - 5 seconds */

        account = ShmPTR->BankAccount;

        while (ShmPTR->Turn != 1) {
            /* busy wait for child's turn */
        }

        balance = rand() % 51;      /* 0 - 50 inclusive */
        printf("Poor Student needs $%d\n", balance);

        if (balance <= account) {
            account -= balance;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n",
                   balance, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        ShmPTR->BankAccount = account;
        ShmPTR->Turn = 0;
    }
}

int main(void)
{
    int shmid;
    pid_t pid;
    struct shared_data *ShmPTR;

    shmid = shmget(IPC_PRIVATE, sizeof(struct shared_data),
                   IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    ShmPTR = (struct shared_data *)shmat(shmid, NULL, 0);
    if (ShmPTR == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    ShmPTR->BankAccount = 0;
    ShmPTR->Turn = 0;

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        ChildProcess(ShmPTR);
        shmdt(ShmPTR);
        exit(0);
    } else {
        ParentProcess(ShmPTR);
        wait(NULL);
        shmdt(ShmPTR);
        shmctl(shmid, IPC_RMID, NULL);
    }

    return 0;
}