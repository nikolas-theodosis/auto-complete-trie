#include <sys/ioctl.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include "trie.h"

#define LINESIZE 128
#define BACKSPACE 0x7f

#define N 8

int getchar_silent()
{
    int ch;
    struct termios oldt, newt;

    /* Retrieve old terminal settings */
    tcgetattr(STDIN_FILENO, &oldt);

    /* Disable canonical input mode, and input character echoing. */
    newt = oldt;
    newt.c_lflag &= ~( ICANON | ECHO );

    /* Set new terminal settings */
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    /* Read next character, and then switch to old terminal settings. */
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}

int get_input (TrieNode *root, char *path)
{
    int next, i = 0, length, j=0, k, choice, found=0;
	char word[25] = {0};
	char result[100] = "";
	char input[LINESIZE] = {0};
	char buffer[LINESIZE]={0};
	char temp[2], temp2[2], newword[25];
	char **options;
	char selection;
	char *args[6];

    struct winsize w;

    /* Keep reading one character at a time */
    while ((next = getchar_silent()) != EOF)
    {
        /* Print normal characters */
        if (isalpha(next) || ispunct(next) || next == ' ' || next == '\n')
        {
			if (next != '\t') {			//kataxwrw kathe xaraktira sto input gia na mporw na tupwsw olokliri tin protasi
                input[i] = next;
                i++;
            }
            if (isalpha(next)) {			//otan pairnw gramma to vazw sto word gia na elegxw pithano tab se auti ti leksi
                word[j] = next;
                j++;
            }
            if (next == ' ' || ispunct(next)) { 		//an parw keno i simeio stiksis, termatizw to word giati den patithike tab
				temp2[0] = '\n';					//opote tha ksekinisw ap tin arxi.prosthetw kai to \n giati kata tin prosthiki neas leksis
				temp2[1] = '\0';					//sto trie pairnw to length-1 logw tou oti oi lekseis sto leksiko exoun kai \n sto telos
				strcat(word, temp2);
				if (j > 0) { 							// an j=0, tote o proigoumenos xaraktiras itan pali keno i simeio stiksis
					if (searchWord(root, word) == 0) {	// an den vrw tin leksi sto trie
						root = addToTrie(root, word);	//tin prosthetw
						addToDict(word, path);			//kai sto leksiko
					}
					else
						increaseCounter(root, word, 1); //alliws auksanw ton counter tis leksis
		            memset(word, 0, sizeof(word));
		            j = 0;
				}
            }
            putchar(next);
            continue;
    	  }

        /* Treat special characters differently */
        switch(next) {

        case '\t':              /* Just read a tab */

			
			word[j] = '\0';
            /* Get terminal window size */
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
			putchar('\n');
			input[i] = '\0';
			strcat(buffer, input);
			length = strlen(word);
			
			options = search(root, word, result);				//psaxnw tis pithanes lekseis gia to prothema word
			//options = sort(options);

			for (i=0 ; i<N ; i++) {						
            	if (options != NULL) {
            	    if (options[i] != NULL) {
            	        fprintf(stderr, "%s\t", options[i]);	//kai tupwnw tis N lekseis
					}
				}
			}		
            selection = getchar_silent(); 
			choice = selection - '0';
            putchar('\n');
			
			if (choice <= N) { 				// an i epilogi tou xristi einai mexri N
				strcat(buffer, options[choice - 1] + length); //tupwnw tin arxiki protasi me ti leksi pou epelekse
				increaseCounter(root, options[choice - 1], 0);	//kai auksanw ton counter tis leksis
			}
			else {
				temp[0] = selection;	//alliws sinexizei na grafei se nea seira
				temp[1] = '\0';
				strcat(buffer, temp);
			}
			printf("%s", buffer);
			i = 0;
			j = 0;
			memset(word, 0, sizeof(word));
            memset(input, 0, sizeof(input));
			memset(result, 0, sizeof(result));
            options[0] = '\0';
            free(options);

            break;

        case BACKSPACE:         /* Just read a backspace */

            /* Move the cursor one step back, print space, and move
             * cursor back once more to imitate backward character
             * deletion.
             */

            printf("\b \b");
            break;

        case VEOF:              /* Just read EOF (Ctrl-D) */

            /* The EOF character is recognized only in canonical input
             * mode. In noncanonical input mode, all input control
             * characters such as Ctrl-D are passed to the application
             * program exactly as typed. Thus, Ctrl-D now produces
             * this character instead.
             */
			
            printf("\n\nExiting. Bye...");
            goto THE_END;
        default:
            continue;
        }
    }
THE_END:
    putchar('\n');
    return EXIT_SUCCESS;
}

char** sort(char **options) {
	int i, j;
	char temp[20];
	if (options != NULL) {
			for (j=1 ; j<N ; j++) {
				if (strcmp(options[j-1], options[j]) > 0) {
					strcpy(temp, options[j-1]);
					strcpy(options[j-1],options[j]);
					strcpy(options[j],temp);
				}
			}
		
	}
	return options;
}

void addToDict(char *word, char *path) {				//prosthetw tis nees lekseis sto leksiko
	FILE *file;

	if ((file = fopen(path, "a+")) == NULL) {
		perror("source-file not found");
		return;
	}

	fprintf(file, "%s", word);

	fclose(file);
}
