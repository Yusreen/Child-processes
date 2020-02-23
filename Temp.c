#include <stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<math.h>
#include <stdlib.h>
#include<sys/wait.h>
#include <stdbool.h> 
#include <sys/mman.h>
#include <pthread.h> 

#define a 0
#define b 1
#define c 2

#define READ 0
#define WRITE 1

int const NUMBER_OF_CHILDREN = 3;
int INITIAL_NUMBER_OF_TRAPEZOIDS = 32;
static int *value = 0; //number of iterations
static int *index=0;//index of each array
static int *trap_left ; //number of trapezoids left
static float *sum = 0; //sum of all areas
float h= (float)1/(float)32; //height of the trapezoid
int  PARENT_CONTROL = 0;
int SIZE_OF_FLOAT = sizeof(float);

//calculate area function
void calculate_area (int fd_ctp[][2], int fd_ptc[][2], int id);

//Evaluate the function at each x
float f(float x){
	float temp= (float)1/((float)1+(x*x));
	return temp;
}


int main(){
	
	value = mmap(NULL, sizeof *value, PROT_READ | PROT_WRITE, 
	MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	sum = mmap(NULL, sizeof *sum, PROT_READ | PROT_WRITE, 
	MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	trap_left = mmap(NULL, sizeof *trap_left, PROT_READ | PROT_WRITE, 
	MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	index = mmap(NULL, sizeof *index, PROT_READ | PROT_WRITE, 
	MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	//Creating N pairs of pipes using 2d array
	int fd_ctp [NUMBER_OF_CHILDREN+1][2];
	int fd_ptc [NUMBER_OF_CHILDREN+1][2];
	
	//Create an array of all trapezoids
	float areas [INITIAL_NUMBER_OF_TRAPEZOIDS];
	

	//Piping parent to child processes
	for(int i=1;i<NUMBER_OF_CHILDREN+1;i++)
	{
		if (pipe(fd_ptc[i]) <0)
		{ 
			perror("Failed to allocate pipes");
			exit(EXIT_FAILURE);
		}
	}
	
	printf("\n");
	
	//Piping child to parent
	for(int i=1;i<NUMBER_OF_CHILDREN+1;i++)
	{
		if (pipe(fd_ctp[i]) <0)
		{ 
			perror("Failed to allocate pipes");
			exit(EXIT_FAILURE);
		}
	}
	
	
	//Creating the parent pipes
	pipe(fd_ptc[0]);

	float area= f(a)+f(b);

	//number of trapezoids left
	*trap_left=32;

	//for keeping track if we are in the parent of child for each fork
	int pid;

	//create the child processes

	for (int i = 1; i < NUMBER_OF_CHILDREN+1; i++){

		pid = fork();



		if (pid < 0){

			printf("OH SNAP! Child %d failed ",i);
			return -1;

		}

		if (pid == 0 ) {
			//output to show when children are created
			printf("Child %d created with pid %d \n",i,getpid());
			calculate_area(fd_ctp,fd_ptc,i);
			break;

		}

	} //end child creation for loop

	
	if(pid >0) { //parent and control function

		bool loop = true;
		
		while (loop){
			
			for (int k=1;k <=NUMBER_OF_CHILDREN;k++)
			{
				//Increase the number of iteration
				*value+=1;
				//printf("The updated value is %d \n",*value);
				//send the calculated area to the child process
				write(fd_ptc[k][WRITE],&area,SIZE_OF_FLOAT);
				//read the calculated area from child process
				read(fd_ctp[k][READ],&area,SIZE_OF_FLOAT);
				//add area of each trapezoid to the array
				areas[*index]=area;
				//printf("This is the area %d from parent %f \n",*index,areas[*index]);	
				//increase the index of the array of areas
				*index+=1;
				
				if (*trap_left ==PARENT_CONTROL ){
					
					loop = false;
					*sum = *sum+area;
					
					for (int j=0;j<32;j++)
					{
						//printf("The array element is %f\n",areas[j]);
						*sum=*sum +areas[j];
					}
					
					printf("The total is %6.4f\n",*sum);
					float result =*sum * h;
					printf("The result is %6.4f\n",result);
				}
				
			}
			
			
			//if there are trapezoids left, output how many are left

			if (*trap_left > 0){

				printf("We have %d trapezoids left. \n",*trap_left);
			}

			//when *trap_left < PARENT_CONTROL, all children have finished their loops

			
		}
		exit(0);
	}
}				

void calculate_area (int fd_ctp[][2], int fd_ptc[][2], int id){

	bool loop = true;
	//do a loop
	
	while (loop) {            

		//declare area as a local variable

		float area;

		//wait for parent to write, then process trapezoids left
		close(fd_ptc[id][WRITE]);
		read(fd_ptc[id][READ], &area, SIZE_OF_FLOAT);
		float j=(h* (*value));
		area=f(j);



		//if there are trapezoids left, write out how many

		if (*trap_left > 0){

			printf("I am child %d .There are %d trapezoids. The number of iterations is %d and area is %f.\n",id,*trap_left,*value,area);

			*trap_left-=1;


			if (*trap_left == 0)
			{
				loop=false;
				
			}

			
			
			//tell the parent how many trapezoids are left 
			write(fd_ctp[id][WRITE], &area, SIZE_OF_FLOAT);

			usleep(100);

		}



		//if there are no trapezoids left

		else{

			loop = false;

			*trap_left-=1;

			write(fd_ctp[id][WRITE], &area, SIZE_OF_FLOAT);

		}
		
	}
}
