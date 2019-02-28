#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <time.h>


using namespace std;

int main()
{
  // Message buffer
  struct buf {// the ordering matters in this struct.
  		long mtype; // required
  		char greeting[50]; // mesg content
	};

	buf msg;  		// this is the container of the message.
  int size = sizeof(msg)-sizeof(long);         // calculate the size of the message.
  const int alpha = 997;                       // seed value to determine valid data

	int qid = msgget(ftok(".",'u'), 0);          // get locate the queue
    if(qid == -1){                             // Exit out of program if the message queue is not succesful
        cout << strerror(errno) << endl;
        return -1;
    }

    int pass;                   // Holds whether the msgsnd succeeds
    int data;                   // holds data that will be sent to datahub
    srand (time(NULL));         // seed random number

    bool firstmsg = false;      // Flag that displays if probeA has sent the first message

    do{
      data = rand();            // Generate the value


      if(data % alpha ==  0){   // Send message when a valid value is made
        if(firstmsg == false){
          const string spid = to_string((long)getpid());
          strncat(msg.greeting, spid.c_str(), 50);// use strncat.

          msg.mtype = 111;            // we are numbering our messages.
          pass = msgsnd(qid, (struct msgbuf *)&msg, size, 0);		// now we are sending the message.
          if(pass == 0){
            cout << "msg sent from A" << endl;
          }
          pass = msgrcv (qid, (struct msgbuf *)&msg, size, 114, 0);

          cout << "1st acknowledge" << msg.greeting << endl;

          firstmsg = true;
        }
        else{
          msg.mtype = 111;
          strcpy(msg.greeting, to_string(data).c_str());

          pass = msgsnd(qid, (struct msgbuf *)&msg, size, 0);		// now we are sending the message.
          if(pass == 0){
            cout << "msg sent from A " << msg.mtype << " " << msg.greeting << endl;
          }

          pass = msgrcv (qid, (struct msgbuf *)&msg, size, 114, 0);

          cout << "got a message back " << msg.mtype << endl;
        }
      }
    }
    while(data > 100);

    cout << "ENDING "  << data << endl;

    return 0;
}
