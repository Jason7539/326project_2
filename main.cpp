// Jason Nguyen
// Chelsea Marfil
// Sophanna Ek
// Main.cpp
// This file servers a a hub to recieve and print out messages
// produced by probeA.cpp, probeB.cpp, and probeC.cpp. Main.cpp is also
// responsible for terminating probeB after 10,000 messages have been recieved.
// The message queue is also created and deleted in main.cpp
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <string>
#include <errno.h>
#include "force_patch.h"

using namespace std;

int main()
{
  struct buf {// the ordering matters in this struct.
  		long mtype; // required
  		char greeting[50]; // mesg content
	};

    int qid = msgget(ftok(".",'u'), IPC_EXCL|IPC_CREAT|0606);   // Create the fresh message queue
    if(qid == -1){                                              // Exit out of program if the message queue is not succesful
        cout << strerror(errno) << endl;
        return -1;
    }

    // changing the number of bytes allowed in the message queue
    struct msqid_ds msgds;
    msgctl(qid, IPC_STAT, &msgds);          // getting the info of the message queue
    cout << "The size of the queue is: " << msgds.msg_qbytes << endl;
    msgds.msg_qbytes = 4194304;
    msgctl(qid, IPC_SET, &msgds);           // increase the size of the message queue
    cout << "The size of the queue is now : " << msgds.msg_qbytes << endl;

    // while any of the probes are on keep checking the messages
    bool flag_a = true;
    bool flag_b = true;
    bool flag_c = true;

    // flags that specify whether the hub has receive the first messages
    bool firstA = false;
    bool firstB = false;
    bool firstC = false;

    // variables that store the process id of each probe
    string Apid;
    string Bpid;
    string Cpid;

    buf msg;        // declare container for the message
    int size = sizeof(msg)-sizeof(long);// calculate the size of the message.

    int pass = 0;                       // variable to track whether a msgrcv is succesful

    int Bcounter = 0;                   // holds numbers of msg received. terminate B after 10,000 messages

    while(flag_a || flag_b || flag_c)   // keep checking message while probes are on
    {
      pass = msgrcv(qid, (struct msgbuf *)&msg, size, -113, 0); // recieve message from A, B or C

      cout << "mtype: " << msg.mtype << "and the pass is " << pass << endl;
      if(pass != -1){               // process messsage if it msgrcv succeeds

        Bcounter++;                 // increase probeB message counter

        if(Bcounter == 10000){      // terminate b when we recieve the 10000 msg
          cout << "REACHED THE END OF B ********************************************************************" << endl;
          int id = stoi(Bpid);                      // get the pid of B
          force_patch(id);                          // send the signal the end B
          flag_b = false;                           // set probeB as off
        }

        switch(msg.mtype)
        {
          case 109: // recieved a goodbye message from probeC
            cout << "ended C " << endl;
            flag_c = false;       // Turn C flag off
            break;
          case 110: // recieved a goodbye message from probeA
            cout << "ended A " << endl;
            flag_a = false;
            break;
          case 111: // recieve probeA message
            if(firstA == false){      // store the pid if it's the first message
              Apid = msg.greeting;
              firstA = true;
            }
            else{   // Process ProbeA message that are not the first
              cout << "from A " << Apid << ": " << msg.greeting << endl;

            }

            // sending acknowledge to A
            msg.mtype = 114;
            strncpy(msg.greeting, "acknowledge", 20);
            msgsnd(qid, (struct msgbuf *)&msg, size, 0);  // sending  acknowledge mesg to A

            cout << "sent acknowledge " << endl;
            break;

          case 112: // recieve probeB message
            if(firstB == false){        // process the pid of B if it's the 1st msg
              Bpid = msg.greeting;      // save the pid
              firstB = true;
            }
            else{ // prints B's messages that are not the first
              cout << "from B " << Bpid << ": " << msg.greeting << endl;
            }
            break;

          case 113: // process message from probeC
            char PID [10];
            char message [10];
            // Gets the PID from msg.greeting which is separated from the reading by delimiter " "
            strcpy(PID, strtok(msg.greeting , " "));
            // Gets the reading (C's reading which is a random int) from msg.greeting which comes after the delimeter " "
            strcpy(message, strtok(NULL, "\0"));

            if(firstC == false){        // process the pid of C if it's the 1st msg
              Cpid = PID;               // save the pid
              firstC = true;
            }
            else {
              cout << "C's PID is: " << Cpid << " Reading received: " << message << endl;
            }
            break;
          default:
            // datahub read a message that it wasn't suppose to
            cout << "got a wierd one type:" << msg.mtype << "mesg:" << msg.greeting << endl;
            return -1;
            break;
        }
      }
      else{
        printf("got an error \n");
        return -1;
        break;
      }
    }

    msgctl (qid, IPC_RMID, NULL); // deallocate the queue.
    cout << "pogram ended " << endl;
    return 0;
}
