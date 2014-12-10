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
#define SEP_NONE 3  // '&' n'impose pas de condition sur le résultat de la commande


int main () {

	char* buffer = malloc(sizeof(char)*1000);
	char* username = getenv("USER");  // OK on Linux, don't know for other OS

	while(true) {
		// afficher le prompt
		if (username != NULL) 
			printf("%s> ", username);
		else
			printf("> ");

		// récupérer la ligne
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
							separator_after = SEP_NONE;
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

			switch(separator_before) {
				case SEP_NONE:
				case SEP_END:
					
					break;
				case SEP_AND:
					
					break;
				case SEP_OR:
					
					break;
			}
			
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
				commande[nb_arg][a] = '\0';

				if ((t+1) != j){
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
			nb_arg++;
			commande[nb_arg] = NULL;
			
			printf("%s\n", buffer2);
			for (t = 0; t < nb_arg; t++)
				printf("%s\n",commande[t]);
		
			// le séparateur après la commande que l'on exécute
			// deviendra au prochain tour le séparateur d'avant commande
			separator_before = separator_after;
			
			pid_t child_pid;
			
			/* Duplique ce processus. */
			child_pid = fork ();
			
			if (child_pid != 0)
				// on est dans le processus père
				wait(NULL);
				// on attend que le processus fils se termine
			else {
				// on est dans le processus fils
				execvp (commande[0], commande);
				
				free(buffer2);
				free(buffer);
				free(username);
				for (t = 0; t <= nb_arg; t++)
					free(commande[t]);
				free(commande);
					   
				return(0);
			 }
			

			free(buffer2);
			for (t = 0; t <= nb_arg; t++)
			  free(commande[t]);
			free(commande);
		}

		// "exit" => break;
		// sinon, exec et c°
	}


	free(buffer);
	free(username);
	return 0;
}
