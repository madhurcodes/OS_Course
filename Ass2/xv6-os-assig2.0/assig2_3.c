#include "types.h"
#include "user.h"
#include "stat.h"
#include "fcntl.h"
int main(int argc , char *argv[]){
	long x=0;
	float z;
	int pid = fork();
	if(pid < 0)
	{
		
	}
	else if(pid == 0)
	{

		setpriority(getpid(), 18);
		for(z=0;z<900000;z=z+0.01){
			printf(1, "b\n");
			x = x + 3.15*z;
			if(x==0){
				x=x;
			}
		}	
	}
	else
	{
		for(z=0;z<90;z=z+1){
			printf(1, "a\n");
			x = x + 3.15*z;
			if(x==0){
				x=x;
			}
		}
		int pid1 = fork();
		if(pid1 < 0)
		{
			
		}
		else if(pid1 == 0)
		{
			setpriority(getpid(), 19);
			for(z=0;z<900000;z=z+0.01){
				printf(1,"c\n");
				x = x + 3.15*z;
				if(x==0){
					x=x;
				}
			}	
		}
		else
		{
			for(z=0;z<900000;z=z+0.01){
				x = x + 3.15*z;
				printf(1,"a\n");
				if(x==0){
					x=x;
				}
			}			
			wait();
			wait();
		}
	}
exit();
}
