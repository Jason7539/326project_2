/* Names: Jason Nguyen, Sophanna Ek, Chelsea Marfil

Assignment files: main.cpp (DataHub), probeA.cpp, probeC.cpp, force_patch.h, force_patch64.o, kill_patch.h, kill_patch64.o

Description: This file is ProbeB which executes concurrently with the DataHub, ProbeA, and ProbeC. 
ProbeB will use a message queue created by the DataHub (main.cpp).

The DataHub will receive and output this probe's PID and valid data readings until the Hub has received 10,000 messages
which will force ProbeB to exit. This requires using the force_patch files. force_patch from force_patch.h is used by the
Hub to signal and terminate ProbeB after receiving those 10,000 messages. 

Pre-conditions: DataHub is running in a separate terminal.
Post-conditions: ProbeB sends messages to the queue and DataHub outputs them if valid.
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

using namespace std;

int main()
{
  // Declare my message buffer
  struct buf {
  		long mtype; // Number attached to the message
  		char greeting[50]; // Message
	};

	buf msg; // Will hold the mtype and message.
  int size = sizeof(msg)-sizeof(long); // Size of the message

  /* "beta": Satisfies the mathematical condition alpha > 3 * beta > rho.
  This is the magic seed that the random integer will have to be divisible by in order to be considered
  a "valid" reading to be sent to the Hub.
  */
  const int beta = 514;             

  /* Makes the message queue available. Locates the queue created by DataHub (main.cpp).
  The ftok function generates a system-wide key associated with the queue. 
  The 0 parameter gets the queue. 
  */
	int qid = msgget(ftok(".",'u'), 0);  
  if(qid == -1) { // Exit out of program if message queue is not created
      cout << strerror(errno) << endl;
      return -1;
  }

  int pass;                   // Holds whether the msgsnd succeeds
  int data;                   // holds data that will be sent to datahub
  const int Bmtype = 112;

  srand (time(NULL));         // Ensures true randomness. 

  bool firstmsg = false;      // Flag that displays if probeA has sent the first message

  char stringData[50];        // Will hold ProbeB's PID.
  
  while(true){

    data = rand();            // Generate data to send to hub

    if(data % beta == 0){     // Send message when valid data is created
      if(firstmsg == false){
        sprintf(stringData, "%d", (int) getpid());            // Send B pid as first message
        strncpy(msg.greeting, stringData, size);              // Copies the PID to buffer.

        msg.mtype = 112;                                      // Numbering B's message in the queue. We chose 112.
        
        pass = msgsnd(qid, (struct msgbuf *)&msg, size, 0);   // Sending the pid as 1st msg
        if(pass == 0){
          cout << "sent B's pid" << endl;
        }

        firstmsg = true;
      }
      else{
        msg.mtype = 112; // Numbering B's message in the queue. We chose 112.


        sprintf(stringData, "%d", data);                      // Stores the data as a C string.
        strncpy(msg.greeting, stringData, size);              // Send the data generated to the hub

        pass = msgsnd(qid, (struct msgbuf *)&msg, size, 0);		// Sending the message.
        
        if(pass == 0){
          cout <<"B :" <<getpid() << ": sends reading :" << data << endl;
        }
      }
    }
  }

  cout << "ENDING "  << data << endl;

  return 0;
}
