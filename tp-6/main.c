#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


int sum(char);
char translate(char);
int dispatcher();

int somme = 0;


int main() {
	// first fork: the translate function
	pid_t pid = fork();
	if (pid == -1) {
		printf("ERROR: fork failed \n");
		return EXIT_FAILURE;
	}
	else if (pid > 0) {
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
	else if (pid > 0) {
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
	char c = fgetc(stdin);
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
