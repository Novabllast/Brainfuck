//-----------------------------------------------------------------------------
// AssA WS15.c
//
// C Programm, welches Speicherverwaltung und Strings behandelt
//
// Group: 13031 study assistant Angela Promitzer
//
// Authors: Manfred Böck 1530598, Anna Haupt 14......, Patrick Struger 1530644
//
// Latest Changes: 20.11.2015 (by Manfred Böck)
//-----------------------------------------------------------------------------
//

#include <stdio.h>
#include <stdlib.h>

int main ( int argc, char *argv[] )
{
    char character;
    char* input_text = malloc(sizeof(char));
    int i = -1;
    if (argc == 1)
    {
    	for (i =0 ; character != EOF; i++) {
    	  character=getchar();
    	  input_text[i]=character;
    	  input_text=realloc(input_text,(i+2)*sizeof(char));
    	  if(input_text == NULL) {
    	    free(input_text);
    	    printf("Error: Out of memory!\n");
    	    return 1;
    	  }
    	}
    }
    else if (argc == 2)
    {
        FILE *file_to_read = fopen(argv[1], "r");
        if (file_to_read == 0)
        {
          printf("Could not open file: %s\n", argv[1]);
        }
        else
        {
            for (i = 0 ; (character = fgetc(file_to_read)) != EOF; i++) {
			  input_text[i]=character;
			  input_text=realloc(input_text,(i+2)*sizeof(char));
			  if(input_text == NULL) {
				free(input_text);
				printf("Error: Out of memory!\n");
				return 1;
			  }
            }
            fclose(file_to_read);
        }
    }
	free(input_text);
    return 0;
}