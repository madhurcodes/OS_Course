#include "types.h"
#include "user.h"
#include "stat.h" 

int main(int argc ,char* argv[]){
	int  pid = atoi(argv[1]);
	int  pr = atoi(argv[2]);
	setpriority(pid,pr);
	exit();
}
