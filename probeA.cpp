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

  int data;                   // holds data that will be sent to datahub
  srand (time(NULL));         // seed random number

  bool firstmsg = false;      // Flag that displays if probeA has sent the first message

  do{
    data = rand();            // Generate the value


    if(data % alpha ==  0){   // Send message when a valid value is made
      if(firstmsg == false){
        const string spid = to_string((int)getpid());   // get the pid
        strncpy(msg.greeting, spid.c_str(), size);  // sending pid to hub

        msg.mtype = 111;            // we are numbering our messages.
        if(msgsnd(qid, (struct msgbuf *)&msg, size, 0) == -1){		// now we are sending the message.
          perror("msgsnd");
        }
        cout << getpid() << "sent:" << msg.greeting << endl;

        if(msgrcv (qid, (struct msgbuf *)&msg, size, 114, 0) == -1){  // recieve acknowledgement from hub
          perror("msgrcv");
        }
        cout << "1st acknowledge receive:" << msg.greeting << endl;

        firstmsg = true;
      }
      else{
        msg.mtype = 111;
        strncpy(msg.greeting, to_string(data).c_str(), size);
        if(msgsnd(qid, (struct msgbuf *)&msg, size, 0) == -1){		// now we are sending the message.
          perror("msgsnd2");
        }

        cout << getpid() << " " << "sending: " << msg.greeting << endl;

        if(msgrcv (qid, (struct msgbuf *)&msg, size, 114, 0) == -1){ // recieve acknowledgement
          perror("msgrcv2:");
        }
        cout << "recieved: " << msg.greeting << endl;
      }
    }
  }
  while(data > 100);

  cout << "ENDING "  << data << endl;

  return 0;
}
