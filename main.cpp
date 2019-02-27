#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <string>


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
    if(qid == -1){                             // Exit out of program if the message queue is not succesful
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
    string Apid = "";
    string Bpid = "";
    string Cpid = "";

    // constants to hold the qid of each probe messages
    const int Amtype = 111;
    const int Bmtype = 112;
    const int Cmtype = 113;

    buf msg;        // declare container for the message
    int size = sizeof(msg)-sizeof(long);         // calculate the size of the message.

    int pass = 0;       // variable to track whether a msgrcv is succesful
    // recieve messages from the probes
    //
    // while(flag_a){
    //   pass = msgrcv(qid, (struct msgbuf *)&msg, size, 0, 0); // reading the message
    //   cout << "recieved msg " << endl;
    //
    //   msgsnd(qid, (struct msgbuf *)&msg, size, 0);  // sending acknowledge mesg to A
    //   cout << "sent: " <<  msg.greeting << endl;
    // }

    while(flag_a || flag_b || flag_c)   // keeps check messages while the probes are on
    {


      pass = msgrcv(qid, (struct msgbuf *)&msg, size, 0, 0); // reading the message
      cout << "message recieved " << endl;
      cout << "mtype: " << msg.mtype << endl;
      if(pass != -1){    // process messsage if it msgcrcv succeeds
        switch(msg.mtype)
        {
          case 111: // prints probeA's pid and data
            if(firstA == false){        // If we haven't got the first message from A
                                        // capture it and get pid
              cout << "FIRSTTTTTTTTTTTTTTTTTT " << endl;
              Apid = msg.greeting;    // store the pid we get from A
              cout << Apid << endl;

              msg.mtype = 114;
              strcat(msg.greeting, "acknowledge");
              msgsnd(qid, (struct msgbuf *)&msg, size, 0);  // sending acknowledge mesg to A
              cout << "sending message 114" << endl;
              firstA = true;         // no process message from A that aren't the pid
            }
            else{ // Process all messages that aren't the first
              cout << "from" << Apid << ": " << msg.greeting << endl;
              msg.mtype = 114;
              strcat(msg.greeting, "");
              msgsnd(qid, (struct msgbuf *)&msg, size, 0);  // sending  acknowledge mesg to A
              cout << "sending message 114" <<endl;
            }
            break;
          case 112:
            break;
          case 113:
            break;
          default:
            break;
        }
      }
    }



    msgctl (qid, IPC_RMID, NULL);                           // this deallocates the queue.

    return 0;
}
