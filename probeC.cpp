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
    
    // // prepare exit message to send
    // strncpy(msg.greeting, "?", size);	
    // cout << getpid() << ": killed" << endl;
    // msg.mtype = 113; 	// set message type mtype = 113
    // msgsnd(qid, (struct msgbuf *)&msg, size, 0); // sending
	// msg.mtype = 113;
    // msgsnd(qid, (struct msgbuf *)&msg, size, 0);

    // kill_patch(qid, (struct msgbuf *)&msg, size, msg.mtype);
    
    srand(time(NULL));  
    int magic_seed = 251;

    do {
        int rand_int = rand();   
        if (rand_int % magic_seed == 0) {
            // prepare my message to send

            // For sending C's PID
            strncpy(msg.greeting, to_string(getpid()).c_str(), size);
            // Delimiter between PID and Message from C	
            strncat(msg.greeting, " ", size);
            // For sending C's reading along with the PID
            strncat(msg.greeting, to_string(rand_int).c_str(), size);
            
            cout << getpid() << ": sends reading" << endl;
            msg.mtype = 113; 	// set message type mtype = 113
            msgsnd(qid, (struct msgbuf *)&msg, size, 0); // sending
        }
    } while (1);

    msg.mtype = 113;
    msgsnd(qid, (struct msgbuf *)&msg, size, 0);
    kill_patch(qid, (struct msgbuf *)&msg, size, msg.mtype);
}