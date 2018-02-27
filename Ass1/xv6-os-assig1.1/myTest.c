#include "types.h"
#include "user.h"
#include "date.h"

int main(void){
	int pid = fork();
	if(pid != 0){
		exit();		// Parent exits => Forked process is now orphan
	}
	ps();			// Outputs zombie! along with output of ps()
	exit();
}
