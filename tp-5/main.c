#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include  <sys/types.h>
#include  <unistd.h>

#define bool  int
#define true  1
#define false 0

// les différents séparateurs possibles
#define SEP_END  0  // ';'
#define SEP_AND  1  // '&&'
#define SEP_OR   2  // '||'
#define SEP_ET   3  // '&'
#define SEP_NONE 4  // Pas de séparateur

int status;
int stop = false;

int main () {

	char* buffer;
	char* username = getenv("USER");  // OK on Linux, don't know for other OS

	while(true) {
		// afficher le prompt
		if (username != NULL) 
			printf("%s> ", username);
		else
			printf("> ");

		// récupérer la ligne
		buffer = malloc(sizeof(char)*1000);
		fgets(buffer, 1000, stdin);

		// debug: on printe la ligne
		// printf("%s", buffer);

		// séparer selon les espaces / caractères spéciaux
		// boucle pour chaque commande dans la ligne
		int i = 0;  // pointeur sur le caractère que l'on lit dans le buffer
		bool line_complete = false;
		int separator_before = SEP_NONE;
		while(line_complete == false) {
			// on récupère la prochaine commande
			char* buffer2 = malloc(sizeof(char)*100);
			int j = 0;  // pointeur sur buffer2 pour ajouter un caractère

			// boucle pour chaque caractère de la commande
			int separator_after = SEP_NONE;
			bool command_complete = false;
			while(j < 100 && i < 1000 && command_complete == false) {
				switch(buffer[i]) {
					case ';':
						separator_after = SEP_END;
						command_complete = true;
						break;
					case '&':
						if(buffer[++i] == '&')
							separator_after = SEP_AND;
						else
							separator_after = SEP_ET;
						command_complete = true;
						break;
					case '|':
						if(buffer[++i] == '|')
							separator_after = SEP_OR;
							command_complete = true;
						break;
					case '\n':
					case '\0':
						command_complete = true;
						line_complete = true;
						break;
					default:
						buffer2[j] = buffer[i];
						j++;
						break;
				}

				i++;
			}
			buffer2[j] = '\0';
			
			//on recupère la commande sous forme de tableau
			char **commande = malloc(sizeof(char*)*100);
			int nb_arg = 0;
			commande[nb_arg] = malloc(sizeof(char)*100);
			int a = 0;
			int t = 0;
			
			while (buffer2[t] == '\t' || buffer2[t] == ' ')
				t++;
				
			for (; t < j; t++)
			{
				if (buffer2[t] == '\t' || buffer2[t] == ' ') 
				{
					while (buffer2[t+1] == '\t' || buffer2[t+1] == ' ')
							t++;

					if ((t+1) != j) {
						commande[nb_arg][a] = '\0';
						nb_arg++;
						a = 0;
						commande[nb_arg] = malloc(sizeof(char)*100);
					}
				}
				else
				{
					commande[nb_arg][a] = buffer2[t];
					a++;
				}
			}
			commande[nb_arg][a] = '\0';

			//Derniere ligne NULL pour la fonction execvp
			nb_arg++;
			commande[nb_arg] = NULL;
			
		
			// le séparateur après la commande que l'on exécute
			// deviendra au prochain tour le séparateur d'avant commande
			separator_before = separator_after;
			
			pid_t child_pid;
			
			/* Duplique ce processus. */
			child_pid = fork ();

			if (child_pid != 0){
				//père, on attend que le fils se termine
				if (strcmp(commande[0], "exit") == 0){
					return(0);
				}

				if (separator_after != SEP_ET){
					waitpid(child_pid, &status, 0);
					if ((separator_after == SEP_OR && WEXITSTATUS(status) == 0) || (separator_after == SEP_AND && WEXITSTATUS(status) != 0))
						stop = true;
				}
			}
			else {
				// on est dans le processus fils
				if ((commande[0][0] != '\0' && strcmp(commande[0], "exit") != 0) && stop == false)
				{
					if (execvp (commande[0], commande) == -1);
					{
						printf("%s : commande introuvable\n", commande[0]);
						exit(EXIT_FAILURE);
					}
				}
			
				return(0);
			}
			

			free(buffer2);
			for (t = 0; t <= nb_arg; t++)
				free(commande[t]);
			free(commande);
		}

		free(buffer);
		stop = false;
	}
	return 0;
}
