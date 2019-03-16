#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <string>
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

    int qid = msgget(ftok(".",'u'), IPC_EXCL|IPC_CREAT|0600);   // Create the fresh message queue
    if(qid == -1){                             // Exit out of program if the message queue is not successful
        cout << strerror(errno) << endl;
        return -1;
    }

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

    int pass = 0;       // variable to track whether a msgrcv is successful

    int Bcounter = 0;                   // holds numbers of msg received. terminate B after 10,000 messages

    while(flag_a || flag_b || flag_c)   // keeps check messages while the probes are on
    {
      pass = msgrcv(qid, (struct msgbuf *)&msg, size, -113, 0); // reading the message

      cout << "mtype: " << msg.mtype << "and the pass is " << pass << endl;
      if(pass != -1){    // process messsage if it msgcrcv succeeds

        switch(msg.mtype)
        {
          case 111: // prints probeA's pid and data
            if(firstA == false){        // If we haven't got the first message from A                                        // capture it and get pid
              Apid = msg.greeting;    // store the pid we get from A
              cout << "trying to send " << endl;

              msg.mtype = 114;
              strncpy(msg.greeting, "sending", size);
              if(msgsnd(qid, (struct msgbuf *)&msg, size, 0) == -1) {  // sending acknowledge mesg to A
                perror("msgsnd");
              }

              cout << "sending message 114" << endl;
              firstA = true;         // no process message from A that aren't the pid
            }
            else{ // Process all messages that aren't the first
              cout << "from A " << Apid << ": " << msg.greeting << endl;

              msg.mtype = 114;
              strncpy(msg.greeting, "acknowledge", size);
              if(msgsnd(qid, (struct msgbuf *)&msg, size, 0) == -1){  // sending  acknowledge mesg to A
                perror("sending");
              }
              cout << "sent acknowledge " << endl;
            }
            break;
          case 112:
            Bcounter++;                 // increase probeB message B counter

            if(Bcounter == 10000){      // terminate b when we recieve the 10000 msg
              // terminate B
              cout << "REACHED THE END OF B ********************************************************************" << endl;
              cout << Bpid << "the b id is " << endl;
              int id = stoi(Bpid.substr(2, -1));
              force_patch(id);
              break;
            }

            if(firstB == false){        // process the pid of B if it's the 1st msg
              Bpid = msg.greeting;      // save the pid
              firstB = true;
            }
            else { // prints b message
              cout << "from B " << Bpid << ": " << msg.greeting << endl;

            }
            break;
          case 113:
            char PID [10];
            char message [10];
            strcpy(PID, strtok(msg.greeting , " "));
            strcpy(message, strtok(NULL, "\0"));
            
            if(firstC == false){        // process the pid of C if it's the 1st msg
              
              //string token = msg.greeting[0, msg.greeting.find(delimiter)];
              Cpid = PID;      // save the pid
              firstC = true;
            }
            else { // prints b message
              // if(strcmp(msg.greeting, "?")) {
              //   break;
              // }
              cout << "C PID " << Cpid << ": " << message << endl;

            }
            break;
          default:
            // datahub read a message that it wasn't suppose to( sent from itself)
            cout << "got a wierd one " << endl;
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
    msgctl (qid, IPC_RMID, NULL);                           // deallocate the queue.
    cout << "pogram ended " << endl;
    exit(0);
}
