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
  struct buf {
  		long mtype; // required
  		char greeting[50]; // mesg content
	};

	buf msg;  		// container of the message.
  int size = sizeof(msg)-sizeof(long);         // calculate the size of the message.
  const int beta = 514;                        // seed value to determine valid data

	int qid = msgget(ftok(".",'u'), 0);          // get locate the queue
    if(qid == -1){                             // Exit out of program if message queue is not created
        cout << strerror(errno) << endl;
        return -1;
    }

    int pass;                   // Holds whether the msgsnd succeeds
    int data;                   // holds data that will be sent to datahub
    const int Bmtype = 112;

    srand (time(NULL));         // seed random number

    bool firstmsg = false;      // Flag that displays if probeA has sent the first message

    char stringData[50];
    while(true){

      data = rand();            // generate data to send to hub

      if(data % beta == 0){     // send message when valid data is created
        if(firstmsg == false){
          sprintf(stringData, "%d", (int) getpid());           // send B pid as first message
          strcpy(msg.greeting, stringData);

          msg.mtype = 112;
          pass = msgsnd(qid, (struct msgbuf *)&msg, size, 0);		// sending the pid as 1st msg
          if(pass == 0){
            cout << "sent B's pid" << endl;
          }

          firstmsg = true;
        }
        else{
          msg.mtype = 112;

          sprintf(stringData, "%d", data);
          strcpy(msg.greeting, stringData);       // send the data generated to hub

          pass = msgsnd(qid, (struct msgbuf *)&msg, size, 0);		//sending the message.
          if(pass == 0){
            cout <<"B :" <<getpid() << ": sends reading :" << data << endl;
          }
        }
      }
    }

    cout << "ENDING "  << data << endl;

    return 0;
}
