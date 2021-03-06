#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/wait.h>

//True and false values since there are no boolean typs
#define TRUE 1
#define FALSE 0

//Number of fibonacci numbers each child prints out
#define ITERATIONS 10

//Number of iterations of loop to cause non-zero time values
//Child 1 loop does LOOPCOUNT iterations
//Child 2 loop does 10*LOOPCOUNT iterations
//Parent loop does 100*LOOPCOUNT iterations
#define LOOPCOUNT 1000000

int info_printout(char *process_title, int isParent);
int time_printout(char *process_title, double begin, double end);
int cuserid(char *p);

int main(){
    /*** Variables for calculating runtime later ***/
	time_t begin;
	time(&begin);
	time_t end;
	
	pid_t pid;
	int f1;
	int f2;
	f1 = 1;
	f2 = 1;
	
	//Make first child
	pid=fork();
	
	//Check if there was a fork error
	if( pid<0 ){
		perror("Error in forking first child:");
    }
    //If this is the first child
	else if(pid == 0){
	    sleep(2);
	    time(&begin);
	    int i;
	    
	    /*** Child 1 Printouts ***/
	    info_printout("first child", FALSE);
	    
	    //Sleep before loop so that the info messages print before the
	    //Fibonacci printouts start
	    sleep(2);
	    printf("Beginning Fibonacci Sequencing\n");
	    for( i = 1; i <= ITERATIONS*2; i += 2){
	        f2 = f1 + f2;
	        f1 = f1 + f2;
            printf("The process id %d (first child) produced the Fibonnaci number f%d as %d\n", getpid(), i+2, f2);  
            fflush(stdout);
            
            sleep(2);
	    }
	    for(i = 0; i < LOOPCOUNT; i++ )
	        ;  //Loop for nonzero time values
	    time(&end);
	    
	    time_printout("first child", begin, end);
	    
	}
	//If this is the parent
	else{
	    //Make second child
	    pid = fork();
	    //Check for errors
	    if( pid<0 ){
		    perror("Error in forking second child:");
        }
        //If this is the second child
	    else if(pid == 0){
	        sleep(3);
	        time(&begin);
            /*** Child 2 Printouts ***/
	        info_printout("second child", FALSE);
	        printf("Ending Info Printouts\n\n");
	        fflush(stdout);
	        
	        int i;
	        //Sleep before loop so that the info messages print before the
	        //Fibonacci printouts start and so the second child prints second
	        f1 = f1 + f2;
	        sleep(2);
	        for( i = 2; i <= ITERATIONS*2; i += 2){
	            f2 = f1 + f2;
	            f1 = f1 + f2;
                printf("The process id %d (second child) produced the Fibonnaci number f%d as %d\n", getpid(), i+2, f2);
                fflush(stdout);
                
                sleep(2);
            }
            for(i = 0; i < 10*LOOPCOUNT; i++ )
	            ;  //Loop for nonzero time values
            time(&end);
            
            time_printout("second child", begin, end);
            
	    }else{
	        /*** Parent Printouts ***/ 
	        printf("Beginning Info Printouts\n");
	        info_printout("parent", TRUE);
	        
	        //Print status of children when they finish
	        int status;
	        if( wait(&status) ){
	            printf("\nProcess id %d (parent) says the first child terminated with status %d\n", getpid(), status);
	        }else{
	            printf("\nProcess id %d (parent) says the first child had a termination error\n", getpid());
	            perror("Child 1 termination error:");
	        }
	        fflush(stdout);
	        if( wait(&status) ){
	            printf("\nProcess id %d (parent) says the second child terminated with status %d\n", getpid(), status);
	        }else{
	            printf("\nProcess id %d (parent) says the second child had a termination error\n", getpid());
	            perror("Child 2 termination error:");
	        }
	        fflush(stdout);
	        int i;
	        for(i = 0; i < 100*LOOPCOUNT; i++ )
	            ;  //Loop for nonzero time values
	        time(&end);
            
            time_printout("parent", begin, end);
	    }
	}
	//I call wait again to make sure the parent is the last to terminate
	wait(NULL);
	exit(0);
}

//Prints out process information, real and effective gid and uid
//Calls the current process process_title, and there is slightly different
//behavior when it is the parent
int info_printout(char *process_title, int isParent){
    /*** 1) Get username, parent and child processes' real and effective ***
	 ***    user ids, and their group ids                                ***/
	//Allocate a string of size 20
	char *buf = (char *) malloc(20*sizeof(char));
	//Save username to string
	cuserid(buf);
	//Print username, parent process id, r and e group id, euid and uid
	printf("Process id %d (%s) is being run by user %s\n", getpid(), process_title, buf);
	free(buf);
	//If this is the parent process
	if( isParent ){
	    printf("Process id %d (%s) is the parent process\n", getpid(), process_title );
	    //Get and print the cwd
	    char cwd[1024];
	    getcwd(cwd, sizeof(cwd) );
	    printf("Current directory is %s\n", cwd);
	}
	//If this is either child, print the parent process id
	else{
	    printf("Process id %d (%s) has parent process id %d\n", getpid(), process_title, getppid() ); 
	}
	//Print real and effective group id and user ids
	printf("Process id %d (%s) has real group id %d\n", getpid(), process_title, getgid() );
	printf("Process id %d (%s) has effective group id %d\n", getpid(), process_title, getegid() );
	printf("Process id %d (%s) has real user id %d\n", getpid(), process_title, getuid() );
	printf("Process id %d (%s) has effective user id %d\n\n", getpid(), process_title, geteuid() );
	fflush(stdout);
	return(0);
}


//Prints out runtime information, given the begin and endtime
//Refers to the current proccess as process_title
int time_printout(char *process_title, double begin, double end){
    struct rusage RUsage;
    getrusage(RUSAGE_SELF, &RUsage);
    
    //Print wall clock time
    printf("\nProcess id %d (%s) has wall clock time %fs\n", getpid(), process_title, end - begin);
    //Print user time
    printf("Process id %d (%s) has user time %fs\n", 
            getpid(), process_title, (RUsage.ru_utime).tv_sec + (float) (RUsage.ru_utime).tv_usec / 1000000000);
    //Print system time
    printf("Process id %d (%s) has system time %fs\n", 
            getpid(), process_title, (RUsage.ru_stime).tv_sec + (float) (RUsage.ru_utime).tv_usec / 1000000000);
    
    fflush(stdout);
    return(0);
}
