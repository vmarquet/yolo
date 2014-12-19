#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>  // pour isdigit(), isalpha()
#include <unistd.h>  // pour fork()
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h>


int sum(char);
char translate(char);
int dispatcher();

int somme = 0;


int main() {
	// we create the shared memory
	size_t shared_segment_size = sizeof(int);
	int segment_id = shmget(IPC_PRIVATE, shared_segment_size, IPC_CREAT | IPC_EXCL | 0600);
	if (segment_id == -1) {
		perror("ERROR: shmget failed ");
		return (EXIT_FAILURE);
	}

	// we attach it (montage en mémoire)
	int* shared_memory = (int*) shmat(segment_id, NULL, 0);
	if (shared_memory == (int*)0xFFFFFFFF) {
		perror("ERROR: shmat failed ");
		return (EXIT_FAILURE);
	}
	printf("shared memory attached at address %p \n", shared_memory);

	// we access it
	shared_memory[0] = 0;

	// first fork: the translate function
	pid_t pid = fork();
	if (pid == -1) {
		printf("ERROR: fork failed \n");
		return EXIT_FAILURE;
	}
	else if (pid == 0) {
		// we are in the child process
		// while(1)
		// 	translate(c);
		return EXIT_SUCCESS;
	}

	// second fork: the sum function
	pid = fork();
	if (pid == -1) {
		printf("ERROR: fork failed \n");
		return EXIT_FAILURE;
	}
	else if (pid == 0) {
		// we are in the child process
		// while(1)
		// 	sum(c);
		return EXIT_SUCCESS;
	}

	// program's main loop
	while(1)
		if(dispatcher() == 0)
			break;

	return EXIT_SUCCESS;
}

int sum(char c) {
	if (isdigit(c) != 0)
		somme += c - '0';
	else
		somme = 0;
	return somme;
}

char translate(char c) {
	if (isalpha(c) == 0)
		return c;
	else {
		if (isupper(c) != 0)
			return tolower(c);
		else
			return toupper(c);
	}
}

int dispatcher() {
	int c = fgetc(stdin);
	if (c == EOF)
		return 0;
	else if (isdigit(c) != 0) {
		printf("%d ", sum(c));
		fflush(stdout);
	}
	else {
		printf("%c", translate(c));
		fflush(stdout);
	}
	return 1;
}
