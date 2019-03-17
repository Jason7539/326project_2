#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <time.h>
#include "kill_patch.h"

using namespace std;

int main()
{
    // declare my message buffer
	struct buf {
		long mtype; // required
		char greeting[50]; // mesg content
	};
	buf msg;
	int size = sizeof(msg)-sizeof(long);

    int qid = msgget(ftok(".",'u'), 0);

    msg.mtype = 109;
    strncpy(msg.greeting, "bye bye from C", size);

    kill_patch(qid, (struct msgbuf *)&msg, size, msg.mtype);

		char stringData[50];


    srand(time(NULL));
    int magic_seed = 502;

    do {
        int rand_int = rand();

        if (rand_int % magic_seed == 0) {
            // prepare my message to send

						sprintf(stringData, "%d", (int) getpid());
          	strcpy(msg.greeting, stringData);										 // copy pid to buffer

            strncat(msg.greeting, " ", size);										 // Delimiter between PID and Message from C
            // For sending C's reading along with the PID
            strncat(msg.greeting, to_string(rand_int).c_str(), size);

            cout <<"C :" <<getpid() << ": sends reading :" << rand_int << endl;
            msg.mtype = 113; 	// set message type mtype = 113
            msgsnd(qid, (struct msgbuf *)&msg, size, 0); // sending
        }
    } while (1);

    msgsnd(qid, (struct msgbuf *)&msg, size, 0);
}
