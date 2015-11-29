//-----------------------------------------------------------------------------
// assa.c
//
// C Programm, welches Speicherverwaltung und Strings behandelt
//
// Group: 13031 study assistant Angela Promitzer
//
// Authors: Manfred Böck 1530598, Anna Haupt 14......, Patrick Struger 1530644
//
// Latest Changes: 28.11.2015 (by Manfred Böck)
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
int checkIfCharacterIsBrainfuckCommand(char character_to_check);
void runBrainfuckFile(unsigned char* memory_storage, int file_size,
                      Boolean program_loaded);
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
  unsigned char* program_memory = calloc(1, sizeof(char));
  char* user_input= calloc(1, sizeof(char));
  int action_counter = 0;
  Boolean is_program_loaded = TRUE;
  Boolean close_program = FALSE;
  if(argc == 1)                        //Interaktiver Debug Modus
    {
      while (!close_program) {
        printf("esp> ");
        while((character = getchar()) != '\n' && character != EOF)
        {
          user_input=realloc(user_input,2*sizeof(strlen(user_input)));
          if(user_input == NULL)
          {
            free(user_input);
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
          runBrainfuckFile(program_memory, 1, is_program_loaded);
        }
        else if (strcmp(action, "eval") == 0)
        {
          first_parameter = strtok(NULL, " ");
          printf("%s \n", first_parameter);
        }
        else if (strcmp(action, "break") == 0)
        {
          first_parameter = strtok(NULL, " ");
          printf("%s \n", first_parameter);
        }
        else if (strcmp(action, "step") == 0)
        {
          first_parameter = strtok(NULL, " ");
          printf("%s \n", first_parameter);
        }
        else if (strcmp(action, "memory") == 0)
        {
          first_parameter = strtok(NULL, " ");
          printf("%s \n", first_parameter);
          second_parameter = strtok(NULL, " ");
          printf("%s \n", second_parameter);
        }
        else if (strcmp(action, "show") == 0)
        {
          first_parameter = strtok(NULL, " ");
          printf("%s \n", first_parameter);
        }
        else if (strcmp(action, "change") == 0)
        {
          first_parameter = strtok(NULL, " ");
          printf("%s \n", first_parameter);
          second_parameter = strtok(NULL, " ");
          printf("%s \n", second_parameter);
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
    }
    else if(argc == 2)
    {
      printf("[ERR] wrong parameter count\n");
    }
    else if(argc >= 3)
    {
    	printf("%s", argv[1]);
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
      	  //runBrainfuckFile(program_memory, 1, is_program_loaded);
        }
        else
        {
          printf("[ERR] usage: ./assa [-e brainfuck_filnename]\n");
          return INCORRECT_PROGRAM_CALL;
        }
    }
    free(program_memory);
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
  int some_counter = 0;
  char character;
  int return_value = READING_THE_FILE_FAILED;
  //check if the file is a Brainfuck file
  printf("fd: %s \n", filename);
  if (strcmp(get_filename_ext(filename), "bf") == 0)
  {
	  printf("f: %s \n", filename);
    FILE *file_to_read = fopen(filename, "r");
    if (file_to_read == 0)
    {
      printf("[ERR] reading the file failed\n");
      return_value = READING_THE_FILE_FAILED;
    }
    else
    {
      for (some_counter = 0 ; (character = fgetc(file_to_read)) != EOF; some_counter++)
      {
        if(checkIfCharacterIsBrainfuckCommand(character) == 1) {
          program_memory[some_counter]=character;
          program_memory=realloc(program_memory,(some_counter+2)*sizeof(char));
          if(program_memory == NULL)
          {
			free(program_memory);
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
/// This is an example header comment. Copypaste and adapt it! //TODO
///
/// @param filename
/// @param file_size
/// @param program_loaded
//
void runBrainfuckFile(unsigned char* filename, int file_size, Boolean program_loaded) {
    if (program_loaded)
    {
      int another_counter = 0;
      for (another_counter = 0; another_counter < file_size; another_counter++)
      {
        putchar(filename[another_counter]);
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
int checkIfCharacterIsBrainfuckCommand(char character_to_check)
{
  int is_brainfuck_Command = 0;
  switch (character_to_check)
  {
    case '>':
      is_brainfuck_Command = 1;
      break;
    case '<':
      is_brainfuck_Command = 1;
      break;
    case '+':
      is_brainfuck_Command = 1;
      break;
    case '-':
      is_brainfuck_Command = 1;
      break;
    case '.':
      is_brainfuck_Command = 1;
      break;
    case ',':
      is_brainfuck_Command = 1;
      break;
    case '[':
      is_brainfuck_Command = 1;
      break;
    case ']':
      break;
    default:
      break;
  }
  return is_brainfuck_Command;
}
