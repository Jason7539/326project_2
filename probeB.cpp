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
  const int beta = 257;                        // seed value to determine valid data

	int qid = msgget(ftok(".",'u'), 0);          // get locate the queue
    if(qid == -1){                             // Exit out of program if the message queue is not succesful
        cout << strerror(errno) << endl;
        return -1;
    }

    int pass;                   // Holds whether the msgsnd succeeds
    int data;                   // holds data that will be sent to datahub
    const int Bmtype = 112;

    srand (time(NULL));         // seed random number

    bool firstmsg = false;      // Flag that displays if probeA has sent the first message

    while(true){
      data = rand();

      if(data % beta == 0){
        if(firstmsg == false){
          const string spid = to_string((int)getpid());
          strncat(msg.greeting, spid.c_str(), 50);// use strncat.

          msg.mtype = 112;            // we are numbering our messages.
          pass = msgsnd(qid, (struct msgbuf *)&msg, size, 0);		// sending the pid as 1st msg
          if(pass == 0){
            cout << "msg sent from B" << endl;
          }

          firstmsg = true;
        }
        else{
          msg.mtype = 112;
          strcpy(msg.greeting, to_string(data).c_str());

          pass = msgsnd(qid, (struct msgbuf *)&msg, size, 0);		// now we are sending the message.
          if(pass == 0){
            cout << "msg sent from B " << msg.mtype << " " << msg.greeting << endl;
          }
        }
      }
    }

    cout << "ENDING "  << data << endl;

    return 0;
}
