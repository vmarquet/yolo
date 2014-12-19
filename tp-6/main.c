#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>  // pour isdigit(), isalpha()
#include <unistd.h>  // pour fork()
#include <signal.h>
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/sem.h>

#define FLAG_USE_TRANSLATE 0
#define FLAG_USE_SUM       1
#define FLAG_JOB_DONE      2

void sum();
void translate();
int dispatcher();
void kill_childs(int);

int* shared_memory = NULL;
struct sembuf operation;
int sem_id;
pid_t pid_translate;
pid_t pid_sum;
int somme = 0;


int main() {
	// we create the shared memory
	size_t shared_segment_size = sizeof(int)*2;
	int segment_id = shmget(IPC_PRIVATE, shared_segment_size, IPC_CREAT | IPC_EXCL | 0600);
	if (segment_id == -1) {
		perror("ERROR: shmget failed ");
		return (EXIT_FAILURE);
	}
	// CONVENTIONS:
	// shared_memory[0] = le caractère à envoyer à translate() ou sum()
	// shared_memory[1] = un flag pour savoir s'il faut appeler translate() ou sum()

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

	shared_memory[1] = FLAG_JOB_DONE;  // truc tordu

	// first fork: the translate function
	pid_translate = fork();
	if (pid_translate == -1) {
		perror("ERROR: fork failed ");
		return EXIT_FAILURE;
	}
	else if (pid_translate == 0) {
		// we are in the child process
		translate();
		return EXIT_SUCCESS;
	}

	// second fork: the sum function
	pid_sum = fork();
	if (pid_sum == -1) {
		perror("ERROR: fork failed ");
		return EXIT_FAILURE;
	}
	else if (pid_sum == 0) {
		// we are in the child process
		sum();
		return EXIT_SUCCESS;
	}

	// we override the SIGINT signal (CTRL+C) to kill child processes before exiting
	signal(SIGINT, kill_childs);


	// program's main loop
	while(1)
		if(dispatcher() == 0)
			break;

	// on libère proprement le semaphore
	semctl(sem_id, 0, IPC_RMID, 0);

	// we kill the two subprocesses
	kill(pid_translate, SIGTERM);
	kill(pid_sum, SIGTERM);

	return EXIT_SUCCESS;
}

void kill_childs(int sig) {
	kill(pid_translate, SIGTERM);
	kill(pid_sum, SIGTERM);
	// now we kill the parent process
	signal(sig, SIG_DFL);  // we handle the signal with default handler
	kill(getpid(), SIGINT);  // we send the signal to default handler
	return;
}

void sum() {
	while(1) {
		// on prend le mutex
		operation.sem_op = 1;
		semop(sem_id, &operation, 1);

		if (shared_memory[1] == FLAG_USE_SUM) {
			char c = (char)shared_memory[0];

			if (isdigit(c) != 0)
				somme += c - '0';
			else
				somme = 0;
			shared_memory[0] = somme;

			shared_memory[1] = FLAG_JOB_DONE;
		}

		// on libère le mutex
		operation.sem_op = -1;
		semop(sem_id, &operation, 1);
	}

	return;
}

void translate() {
	while(1) {
		// on prend le mutex
		operation.sem_op = 1;
		semop(sem_id, &operation, 1);

		if (shared_memory[1] == FLAG_USE_TRANSLATE) {
			char c = (char)shared_memory[0];

			if (isalpha(c) == 0)
				shared_memory[0] = c;
			else {
				if (isupper(c) != 0)
					shared_memory[0] = tolower(c);
				else
					shared_memory[0] = toupper(c);
			}

			shared_memory[1] = FLAG_JOB_DONE;
		}

		// on libère le mutex
		operation.sem_op = -1;
		semop(sem_id, &operation, 1);
	}
	return;
}

int dispatcher() {
	int c = fgetc(stdin);

	if (c == EOF)
		return 0;

	// on met c dans la mémoire partagée
	shared_memory[0] = c;

	// on met le bon flag pour savoir quelle fonction doit être appelée
	int task;
	if (isdigit(c) != 0) {
		shared_memory[1] = FLAG_USE_SUM;
		task = FLAG_USE_SUM;
	}
	else {
		shared_memory[1] = FLAG_USE_TRANSLATE;
		task = FLAG_USE_TRANSLATE;
	}

	// on libère le mutex
	operation.sem_op = -1;
	semop(sem_id, &operation, 1);

	// tant que le job n'est pas fait, on laisse le mutex libre
	while(1) {
		// on reprend le mutex
		operation.sem_op = 1;
		semop(sem_id, &operation, 1);

		if (shared_memory[1] != FLAG_JOB_DONE) {
			// on libère le mutex
			operation.sem_op = -1;
			semop(sem_id, &operation, 1);
		}
		else
			break;
	}

	// on récupère la nouvelle valeur du caractère
	c = shared_memory[0];

	if (task == FLAG_USE_TRANSLATE)
		printf("%c ", c);
	else if (task == FLAG_USE_SUM)
		printf("%d ", c);

	fflush(stdout);

	return 1;
}
