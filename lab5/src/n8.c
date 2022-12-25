#include "server.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int msqid = -1;

void sig_handler(int sig)
{
    if ((msgctl(msqid, IPC_RMID, 0)) == -1)
    {
        perror("msgctl error");
    }
}

int main(int argc, char *argv[0])
{
    struct sigaction act;
    act.sa_handler = sig_handler;
    sigaction(SIGINT, &act, 0);

    key_t key = ftok(argv[0], 7);
    if (key == -1)
    {
        perror("ftok error");
        return 1;
    }

    msqid = msgget(key, 0666 | IPC_CREAT);
    if (msqid == -1)
    {
        perror("msgget error");
        return 2;
    }

    printf("server %d started...\n", msqid);

    while (1)
    {
        struct server_msg received_msg;

        if ((msgrcv(msqid, &received_msg, sizeof(received_msg.mtext), 0, IPC_NOWAIT)) == -1)
        {
            if (errno == ENOMSG)
            {
                continue;
            }

            perror("msgrcv error");
            return 3;
        }

        printf("message from client %d to client %ld: %s\n",
               received_msg.sender_msqid, received_msg.mtype, received_msg.mtext);

        if ((send(received_msg.mtype, received_msg.sender_msqid, received_msg.mtype, received_msg.mtext)) == -1)
        {
            perror("send error");
            return 4;
        }
    }

    return 0;
}