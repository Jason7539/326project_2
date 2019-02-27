#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>


using namespace std;

int main(){
	int qid = msgget(ftok(".",'u'), 0);          // get locate the queue

    msgctl (qid, IPC_RMID, NULL);                           // this deallocates the queue.
    return 0;
}
