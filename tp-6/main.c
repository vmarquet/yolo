#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>  // pour isdigit(), isalpha()
#include <unistd.h>  // pour fork()
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/sem.h>


int sum();
char translate();
int dispatcher();

int somme = 0;
int* shared_memory = NULL;
struct sembuf operation;
int sem_id;


int main() {
	// we create the shared memory
	size_t shared_segment_size = sizeof(int);
	int segment_id = shmget(IPC_PRIVATE, shared_segment_size, IPC_CREAT | IPC_EXCL | 0600);
	if (segment_id == -1) {
		perror("ERROR: shmget failed ");
		return (EXIT_FAILURE);
	}

	// we attach it (montage en mémoire)
	shared_memory = (int*) shmat(segment_id, NULL, 0);
	if (shared_memory == (int*)0xFFFFFFFF) {
		perror("ERROR: shmat failed ");
		return (EXIT_FAILURE);
	}
	printf("INFO: shared memory attached at address %p \n", shared_memory);

	// to use the shared memory
	// shared_memory[0] = ...;  int ... = shared_memory[0];

	// we create the semaphore
	sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | 0600); // 1 semaphore, 0600 = droits
	if (sem_id < 0) {
		perror("ERROR: semget failed ");
		return (EXIT_FAILURE);
	}

	// to affect a value to the semaphore
	semctl(sem_id, 0, SETVAL, 1);
	// arg 2: on a créé 1 sémaphore donc son numéro est 0
	// arg 3: SETVAL <=> on veut changer une valeur, ça implique quelle sera de type int
	// arg 4: valeur que l'on veut affecter au semaphore
	// cf http://www.commentcamarche.net/faq/11267-utilisation-des-semaphores-systeme-v

	// les opérations dans les semaphores doivent être modélisées dans une structure
	operation.sem_num = 0;  // Numéro de notre sémaphore: le premier et le seul
	operation.sem_op = -1;  // Pour un P() (prendre le mutex) on décrémente
	operation.sem_flg = 0;  // paramètres optionnels, on ne s'en occupe pas

	// pour prendre le mutex
	// operation.sem_op = 1;
	// semop(sem_id, &operation, 1);

	// pour libérer le mutex
	// operation.sem_op = -1;
	// semop(sem_id, &operation, 1);


	// first fork: the translate function
	pid_t pid = fork();
	if (pid == -1) {
		perror("ERROR: fork failed ");
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
		perror("ERROR: fork failed ");
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

	// on libère proprement le semaphore
	semctl(sem_id, 0, IPC_RMID, 0);

	return EXIT_SUCCESS;
}

int sum() {
	// TODO: utiliser les semaphores
	char c = (char)shared_memory[0];

	if (isdigit(c) != 0)
		somme += c - '0';
	else
		somme = 0;
	return somme;
}

char translate() {
	// TODO: utiliser les semaphores
	char c = (char)shared_memory[0];

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

	// TODO: mettre C dans la mem partagée, utiliser un semaphore
	// TODO: récupérer c

	printf("%c ", c);
	fflush(stdout);

	return 1;
}
