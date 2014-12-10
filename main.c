#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define bool  int
#define true  1
#define false 0


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
		while(line_complete == false) {
			// on récupère la prochaine commande
			char* buffer2 = malloc(sizeof(char)*100);
			int j = 0;  // pointeur sur buffer2 pour ajouter un caractère

			// boucle pour chaque caractère de la commande
			bool command_complete = false;
			while(j < 100 && i < 1000 && command_complete == false) {
				switch(buffer[i]) {
					case ';':
						command_complete = true;
						break;
					case '&':
						if(buffer[++i] == '&')
							command_complete = true;
						break;
					case '|':
						if(buffer[++i] == '|')
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

			printf("  %s \n", buffer2);
			free(buffer2);
		}

		// "exit" => break;
		// sinon, exec et c°
	}


	free(buffer);
	return 0;
}
