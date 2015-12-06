//-----------------------------------------------------------------------------
// assa.c
//
// C Programm, welches Speicherverwaltung und Strings behandelt
//
// Group: 13031 study assistant Angela Promitzer
//
// Authors: Manfred Böck 1530598, Anna Haupt 14......, Patrick Struger 1530644
//
// Latest Changes: 29.11.2015 (by Manfred Böck)
//-----------------------------------------------------------------------------
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum _Boolean_
{
      FALSE = 0,
      TRUE = 1
} Boolean;

char *get_filename_ext(char* filename);
int loadBrainfuckFile(char* filename, unsigned char* program_memory);
Boolean isBrainfuckCommand(char character_to_check);
void runBrainfuckFile(unsigned char* memory_storage, Boolean program_loaded);
void interpret(unsigned char* brainfuck_character);
void evalBrainfuckString(char* brainfuckstring);
void setBreakPoind(int program_counter);
void step(int number);
void memory(int number, char* type);
void show(int size);
void change(int number, char* hex_byte);

#define INCORRECT_PROGRAM_CALL 1
#define OUT_OF_MEMORY 2
#define PARSING_ON_INPUT_FAILED 3
#define READING_THE_FILE_FAILED 4
#define PROGRAM_SUCCESSFULLY_LOADED 100

// -e
// /home/manfred/workspace/Brainfuck/src/hw.bf
// /home/manfred/workspace/Brainfuck/src/bottles.bf

int main (int argc, char *argv[])
{
  char character = NULL;
  unsigned char* program_memory = calloc(1024, 1024 * sizeof(char)); //TODO
  char* user_input= calloc(1, 1024 * sizeof(char));
  int action_counter = 0;
  Boolean is_program_loaded = TRUE;
  Boolean close_program = FALSE;

  if(argc == 1)                        //Interaktiver Debug Modus
    {
      while (!close_program) {
        printf("esp> ");
        while((character = getchar()) != '\n' && character != EOF)
        {
          user_input=realloc(user_input, 1024 * sizeof(char)); //TODO
          if(user_input == NULL)
          {
            free(user_input);
            user_input = NULL;
            printf("Error: Out of memory!\n");
            return OUT_OF_MEMORY;
          }
          user_input[action_counter++]=character;
        }
        user_input[action_counter++]='\0';
        int user_input_length = strlen(user_input);
        char delimiter[] = " ";
        char* action = strtok(user_input, delimiter);
        char* first_parameter = NULL;
        char* second_parameter = NULL;

        if (strcmp(action, "load") == 0)
        {
          first_parameter = strtok(NULL, delimiter);
          printf("first: %s\n", first_parameter);
		  if (!first_parameter) {
	          printf("[ERR] usage: load brainfuck_filnename\n");//TODO ist diese Ausgabe erlaubt?
		  } else {
            int has_succeeded = -1;
            has_succeeded = loadBrainfuckFile(first_parameter, program_memory);
            switch (has_succeeded) {
			  case OUT_OF_MEMORY:
				  return OUT_OF_MEMORY;
				  break;
			  case READING_THE_FILE_FAILED:
				  return READING_THE_FILE_FAILED;
				  break;
			  case PROGRAM_SUCCESSFULLY_LOADED:
				  is_program_loaded = TRUE;
				  break;
			  default:
				  break;
		    }
		  }
        }
        else if (strcmp(action, "run") == 0)
        {
          runBrainfuckFile(program_memory, is_program_loaded);
        }
        else if (strcmp(action, "eval") == 0)
        {
          first_parameter = strtok(NULL, " ");
          evalBrainfuckString(first_parameter);
        }
        else if (strcmp(action, "break") == 0)
        {
          first_parameter = strtok(NULL, " ");
          setBreakPoind(first_parameter);
        }
        else if (strcmp(action, "step") == 0)
        {
          first_parameter = strtok(NULL, " ");
          step(first_parameter);
        }
        else if (strcmp(action, "memory") == 0)
        {
          first_parameter = strtok(NULL, " ");
          second_parameter = strtok(NULL, " ");
          memory(first_parameter, second_parameter);
        }
        else if (strcmp(action, "show") == 0)
        {
          first_parameter = strtok(NULL, " ");
          show(first_parameter);
        }
        else if (strcmp(action, "change") == 0)
        {
          first_parameter = strtok(NULL, " ");
          second_parameter = strtok(NULL, " ");
          change(first_parameter, second_parameter);
        }
        else if (strcmp(action, "quit") == 0)
        {
          printf("Bye.\n");
          close_program = TRUE;
    }
        else if(strcmp(user_input, "EOF") == 0){
          close_program = TRUE;
        } else{
          printf("Command: %s is not available.\n"
                 "Available Commands: "
                 "load, run, eval, break, step, "
                 "memory, show, change, quit & EOF\n", user_input);
        }
        memset(user_input,'\0',user_input_length);
        action_counter = 0;
      }
      free(user_input);
      user_input = NULL;
    }
    else if(argc == 2)
    {
      printf("[ERR] wrong parameter count\n");
    }
    else if(argc >= 3)
    {
        if(strcmp(argv[1], "-e") == 0)
        {
          char* filename = argv[2];
          int has_succeeded = -1;
		  has_succeeded = loadBrainfuckFile(filename, program_memory);
		  switch (has_succeeded) {
		    case OUT_OF_MEMORY:
			  return OUT_OF_MEMORY;
			  break;
		    case READING_THE_FILE_FAILED:
			  return READING_THE_FILE_FAILED;
			  break;
		    case PROGRAM_SUCCESSFULLY_LOADED:
			  is_program_loaded = TRUE;
			  break;
		    default:
			  break;
		  }
      	  runBrainfuckFile(program_memory, is_program_loaded);
        }
        else
        {
          printf("[ERR] usage: ./assa [-e brainfuck_filnename]\n");
          return INCORRECT_PROGRAM_CALL;
        }
    }
    free(program_memory);
    program_memory = NULL;
    return 0;
}

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it! //TODO
///
/// @param filename The name of the file whose extension you want to know.
///
/// @return char* filename extension
//
char* get_filename_ext(char *filename) {
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename)
      return "";
    return dot + 1;
}

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it! //TODO
///
/// @param filename
///
/// @return int filename extension 4, 2, 100
//
int loadBrainfuckFile(char *filename, unsigned char* program_memory) {
  int character_counter = 0;
  char character;
  int return_value = READING_THE_FILE_FAILED;

  //check if the file is a Brainfuck file
  if (strcmp(get_filename_ext(filename), "bf") == 0)
  {
    FILE *file_to_read = fopen(filename, "r");
    if (file_to_read == 0)
    {
      printf("[ERR] reading the file failed\n");
      return_value = READING_THE_FILE_FAILED;
    }
    else
    {
      int program_memory_size = sizeof(char)/sizeof(program_memory[0]);
      while((character = fgetc(file_to_read)) != EOF)
      {
        if(isBrainfuckCommand(character)) {
          program_memory[character_counter++]=character;
		  if (program_memory_size <= 1024) { //TODO keine ahnung wie die abfrage lauten soll
          //printf("i: %i D: %i", program_memory_size, strlen((const char*)program_memory));
			  program_memory=realloc(program_memory, 2*program_memory_size);
		  }
          if(program_memory == NULL)
          {
			free(program_memory);
			program_memory = NULL;
			printf("Error: Out of memory!\n");
			return_value =  OUT_OF_MEMORY;
		  }
		}
	  }
	}
	fclose(file_to_read);
	return_value =  PROGRAM_SUCCESSFULLY_LOADED;
  }
  return return_value;
}

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it!
///
/// @param brainfuck_character
//
void interpret(unsigned char* brainfuck_character)
{
    putchar(brainfuck_character);
	/*char *d;
	int  p, r, q;
	char a[5000], f[5000], b, o;
	r++;
	while(*brainfuck_character) {
		switch(o=1,*brainfuck_character++) {
		case '<': p--;        break;
		case '>': p++;        break;
		case '+': a[p]++;     break;
		case '-': a[p]--;     break;
		case '.': putchar(a[p]); fflush(stdout); break;
		case ',': a[p]=getchar();fflush(stdout); break;
		case '[':
			for( b=1,d=brainfuck_character; b && *brainfuck_character; brainfuck_character++ )
				b+=*brainfuck_character=='[', b-=*brainfuck_character==']';
			if(!b) {
				brainfuck_character[-1]=0;
				while( a[p] )
					interpret(d);
				brainfuck_character[-1]=']';
				break;
			}
			break;
		case ']':
			puts("UNBALANCED BRACKETS"), exit(0);
			break;
		case '#':
			if(q>2)
				printf("%2d %2d %2d %2d %2d %2d %2d %2d %2d %2d\n%*s\n",
				       *a,a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9],3*p+2,"^");
			break;
		default: o=0;
		}
		if( p<0 || p>100)
			puts("RANGE ERROR"), exit(0);
	}
	r--;*/
}

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it! //TODO
///
/// @param filename
/// @param program_loaded
//
void runBrainfuckFile(unsigned char* memory_storage, Boolean program_loaded) {
    if (program_loaded)
    {
      int program_size = strlen((const char*)memory_storage);
      int run_counter = 0;
      for (run_counter = 0; run_counter < program_size; run_counter++)
      {
        interpret(memory_storage[run_counter]);
      }
    }
    else
    {
      printf("[ERR] no program loaded\n");
    }
}

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it!
///
/// @param character_to_check The character to check.
///
/// @return int 0 if no Brainfuck command, 1 if Brainfuck Command
//
Boolean isBrainfuckCommand(char character_to_check)
{
  Boolean is_brainfuck_Command = FALSE;
  switch (character_to_check)
  {
    case '>':
    case '<':
    case '+':
    case '-':
    case '.':
    case ',':
    case '[':
    case ']':
      is_brainfuck_Command = TRUE;
      break;
    default:
      break;
  }
  return is_brainfuck_Command;
}

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it!//TODO
///
/// @param
//
void evalBrainfuckString(char* brainfuckstring)
{
  printf("%s \n", brainfuckstring);
}

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it!//TODO
///
/// @param
//
void setBreakPoind(int program_counter)
{
  printf("%i \n", program_counter);
}

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it!//TODO
///
/// @param
//
void step(int number)
{
	  printf("%i \n", number);
}

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it!//TODO
///
/// @param
//

void memory(int number, char* type)
{
	  printf("%i    %s \n", number, type);
}

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it!//TODO
///
/// @param
//

void show(int size)
{
	  printf("%i \n", size);
}

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it!//TODO
///
/// @param
//
void change(int number, char* hex_byte)
{
	  printf("%i  %s\n", number, hex_byte);
}
