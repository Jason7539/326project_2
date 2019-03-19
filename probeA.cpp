/* Names: Jason Nguyen, Sophanna Ek, Chelsea Marfil

Assignment files: main.cpp (DataHub), probeA.cpp, probeB.cpp, force_patch.h, force_patch64.o, kill_patch.h, kill_patch64.o

Description: This file is ProbeA which executes concurrently with the DataHub, ProbeC, and ProbeB.
ProbeA will use a message queue created by the DataHub (main.cpp). ProbeA expects
an acknowledge message for every message it sends to the datahub.

Pre-conditions: Datahub is running in separate terminal
Post-conditions: ProbeA sends messages to the queue and DataHub outputs them if valid.
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <time.h>
#include <errno.h>

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
  const int alpha = 1994;                      // seed value to determine valid data

  int qid = msgget(ftok(".",'u'), 0);          // locate the queue
  if(qid == -1){                               // Exit out of program if the message queue is not succesful
    cout << strerror(errno) << endl;
    return -1;
  }

  int data;                   // holds data that will be sent to datahub
  srand (time(NULL));         // seed random number

  bool firstmsg = false;      // Flag that displays if probeA has sent the first message

  char stringData[50];

  do{
    data = rand();            // Generate the value

    if(data % alpha ==  0){   // Send message when a valid value is made
      if(firstmsg == false){  // send Pid on first message
        sprintf(stringData, "%d", (int) getpid()); // convert pid to string
        strncpy(msg.greeting, stringData, 20);

        msg.mtype = 111;
        if(msgsnd(qid, (struct msgbuf *)&msg, size, 0) == -1){ // sending the pid
          perror("msgsnd");
        }
        cout << "sent:" << msg.greeting << endl;
        firstmsg = true;
      }

      else{ // send the data from every message thats not the first message
        msg.mtype = 111;
        sprintf(stringData, "%d", data);        // convert integer to string

        strncpy(msg.greeting, stringData, 20);
        if(msgsnd(qid, (struct msgbuf *)&msg, size, 0) == -1){ // sending the message.
          perror("msgsnd");
        }
      }
      cout <<"A :" << getpid() << ": sends reading :" << data << endl;

      // wait for acknowledgement after sending message from hub
      if(msgrcv (qid, (struct msgbuf *)&msg, size, 114, 0) == -1){
        perror("msgrcv:");
        cout << errno << endl;
        cout << strerror(errno) << endl;
        cin >> data;
      }
      cout << "recieved: acknowledge " << endl;
    }
  }
  while(data > 100);

  // send goodbye message to hub
  msg.mtype = 110;
  strncpy(msg.greeting, to_string(data).c_str(), 20);
  if(msgsnd(qid, (struct msgbuf *)&msg, size, 0) == -1){		// sending the message.
    perror("msgsnd");
  }
  cout << "ENDING "  << data << endl;

  return 0;
}
