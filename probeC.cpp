/* Names: Jason Nguyen, Sophanna Ek, Chelsea Marfil

Assignment files: main.cpp (DataHub), probeA.cpp, probeB.cpp, force_patch.h, force_patch64.o, kill_patch.h, kill_patch64.o

Description: This file is ProbeC which executes concurrently with the DataHub, ProbeA, and ProbeB. 
ProbeC will use a message queue created by the DataHub (main.cpp). 
The DataHub will receive and output this probe's PID and valid data readings until a kill command is issued to terminate ProbeC.

ProbeC uses a software patch that uses Linux SIGUSR1 signal handling. It lingers in the background while
ProbeC executes. To compile the code, use the command 'g++ probeC.cpp kill_patch64.o' which compiles the two files, 
and links and fuses them to produce a single executable. 

The command 'kill -10 <pid>' can be entered in a separate terminal to terminate ProbeC. 

Pre-conditions: DataHub is running in a separate terminal.
Post-conditions: ProbeC sends messages to the queue and DataHub outputs them if valid.
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
#include "kill_patch.h"

using namespace std;

int main()
{
  // Declare my message buffer
	struct buf {
		long mtype; // Number attached to the message
		char greeting[50]; // Message
	};
	buf msg; // Will hold the mtype and message.
	int size = sizeof(msg)-sizeof(long); // Length of the message

  /* Makes the message queue available. Locates the queue created by DataHub (main.cpp).
  The ftok function generates a system-wide key associated with the queue. 
  The 0 parameter gets the queue. 
  */
  int qid = msgget(ftok(".",'u'), 0); 
  
  msg.mtype = 109; // Numbering C's exit message in the queue. We chose 109.

  strncpy(msg.greeting, "bye bye from C", size); // Copies our message "bye bye..." into msg.greeting.

  /* Uses kill_patch from kill_patch.h to send the exit message to the Hub. 
  Notifies the Hub that ProbeC terminated. 
  Called once after the msgget function for best result.
  */
  kill_patch(qid, (struct msgbuf *)&msg, size, msg.mtype); 

  char stringData[50]; // Will hold ProbeC's PID.
  srand(time(NULL)); // Ensures true randomness. 

  /* "rho": Satisfies the mathematical condition alpha > 3 * beta > rho.
  This is the magic seed that the random integer will have to be divisible by in order to be considered
  a "valid" reading to be sent to the Hub.
  */
  int rho = 502; 

  do {
      /* The random integer that will have to be divisible by rho in order to be considered
      a valid reading to be sent to the Hub
      */
      int rand_int = rand(); 

      if (rand_int % rho == 0) { // Checks that the random int is divisible by rho.
          
          // Prepare my message to send - For sending C's PID and reading.
          sprintf(stringData, "%d", (int) getpid()); // Stores PID as a C string.
          strncpy(msg.greeting, stringData, size); // Copies the PID to buffer.
          strncat(msg.greeting, " ", size); // Concatenates a delimiter after the PID
          strncat(msg.greeting, to_string(rand_int).c_str(), size); // Concatenates the random int (the reading/message) after the delimeter

          cout << "C :" << getpid() << ": sends reading :" << rand_int << endl; // Prints C's PID and reading.
          msg.mtype = 113; 	// Numbering C's message in the queue. We chose 113.
          msgsnd(qid, (struct msgbuf *)&msg, size, 0); // Sends msg to the queue.
      }
  } while (1); // Loops until user enteres kill command.

  msgsnd(qid, (struct msgbuf *)&msg, size, 0); // Sends our exit message to the queue.
}
