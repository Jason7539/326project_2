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

/*
 * This files executes the datahub which takes input from
 * Probe A, Probe B, and Probe C. It then ouputs the Process Id of the probe
 * and the data that the probe sent.
*/
using namespace std;

int main()
{
  struct buf {// the ordering matters in this struct.
  		long mtype; // required
  		char greeting[50]; // mesg content
	};

    int qid = msgget(ftok(".",'u'), IPC_EXCL|IPC_CREAT|0606);   // Create the fresh message queue
    if(qid == -1){                             // Exit out of program if the message queue is not succesful
        cout << strerror(errno) << endl;
        return -1;
    }

    /////////////////////////////
    // changing the number of bytes allowed in the message queue
    struct msqid_ds msgds;
    msgctl(qid, IPC_STAT, &msgds);          // getting the info of the message queue

    // cout << "bytes of msgQ:" << (int) msgds.msg_qbytes << endl;
    cout << "The size of the queue is: " << msgds.msg_qbytes << endl;

    msgds.msg_qbytes = 4194304;

    msgctl(qid, IPC_SET, &msgds);           // increase the size of the message queue

    cout << "The size of the queue is now : " << msgds.msg_qbytes << endl;
    /////////////////////////////////



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

    // constants to hold the qid of each probe messages
    const int Amtype = 111;
    const int Bmtype = 112;
    const int Cmtype = 113;

    buf msg;        // declare container for the message
    int size = sizeof(msg)-sizeof(long);         // calculate the size of the message.

    int pass = 0;       // variable to track whether a msgrcv is succesful

    int Bcounter = 0;                   // holds numbers of msg received. terminate B after 10,000 messages

  //  char ack[];

    while(flag_a || flag_b || flag_c)             // keeps checking messages while probeA and probeB are still active
    {
      pass = msgrcv(qid, (struct msgbuf *)&msg, size, -113, 0); // reading the messages of probeA, probeB, or probeC

      cout << "mtype: " << msg.mtype << "and the pass is " << pass << endl;
      if(pass != -1){               // process messsage if it msgrcv succeeds

        Bcounter++;                 // increase probeB message counter

        if(Bcounter == 10000){      // terminate b when we recieve the 10000 msg
          cout << "REACHED THE END OF B ********************************************************************" << endl;
          cout << Bpid << "the b id is " << endl;
          int id = stoi(Bpid);        // get the pid of B
          force_patch(id);                          // send the signal the end B
          flag_b = false;                           // set probeB as off
        }

        switch(msg.mtype)
        {
          case 109: // recieved a goodbye message from probeC
            cout << "ended C " << endl;
            flag_c = false;
            break;
          case 110: // recieved a goodbye message from probeA
            cout << "ended A " << endl;
            flag_a = false;
            break;
          case 111:                   // recieving probeA message
            if(firstA == false){      // store the pid if it's the first message
              Apid = msg.greeting;
              firstA = true;         // no process message from A that aren't the pid
            }
            else{   // Process ProbeA message that are not the first
              cout << "from A " << Apid << ": " << msg.greeting << endl;

            }

            // sending acknowledge to A
            msg.mtype = 114;
            strcpy(msg.greeting, "acknowledge");
            if(msgsnd(qid, (struct msgbuf *)&msg, size, IPC_NOWAIT) == -1){  // sending  acknowledge mesg to A
              perror("sending");
              cout << errno << endl;
              cout << strerror(errno) << endl;

              msgctl(qid, IPC_STAT, &msgds);          // getting the info of the message queue
              cout << "message size is "  << msgds.msg_qbytes << endl;

              cin >> firstA;
            }
            cout << "sent acknowledge " << endl;
            break;

          case 112:

            if(firstB == false){        // process the pid of B if it's the 1st msg
              Bpid = msg.greeting;      // save the pid
              firstB = true;
            }
            else{                       // prints B's messages that are not the first
              cout << "from B " << Bpid << ": " << msg.greeting << endl;
            }
            break;

          case 113:                     // process message from probeC
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
            // datahub read a message that it wasn't suppose to( sent from itself)
            cout << "got a wierd one type:" << msg.mtype << "mesg:" << msg.greeting << endl;
            cin >> msg.greeting;
            pass = msgsnd(qid, (struct msgbuf *)&msg, size, 0);  // place the read message back
            break;
        }
      }
      else{
        printf("got an error \n");
        break;
      }
    }
    // Close Probe C after probe A and B have ended

    msgctl (qid, IPC_RMID, NULL);                           // deallocate the queue.
    cout << "pogram ended " << endl;
    return 0;
}
