#include "stdio.h"
#include "stdlib.h"
#include "string.h"

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

	while(true) {
		// afficher le prompt
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

			printf("  %s  ", buffer2);

			switch(separator_before) {
				case SEP_NONE:
				case SEP_END:
					printf("[exécuter tout le temps]");
					break;
				case SEP_AND:
					printf("[n'exécuter que si la commande précédente a réussi]");
					break;
				case SEP_OR:
					printf("[n'exécuter que si la commande précédente a échoué]");
					break;
			}
			printf("\n");

			// le séparateur après la commande que l'on exécute
			// deviendra au prochain tour le séparateur d'avant commande
			separator_before = separator_after;

			free(buffer2);
		}

		// "exit" => break;
		// sinon, exec et c°
	}


	free(buffer);
	return 0;
}
