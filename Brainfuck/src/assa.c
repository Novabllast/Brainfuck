//-----------------------------------------------------------------------------
// assa.c
//
// C Programm, welches Speicherverwaltung und Strings behandelt
//
// Group: 13031 study assistant Angela Promitzer
//
// Authors: Manfred Böck 1530598, Anna Haupt 14......, Patrick Struger 1530644
//
// Latest Changes: 07.12.2015 (by Manfred Böck)
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
void runBrainfuckFile(unsigned char* memory_storage, unsigned char* data_segment, Boolean program_loaded);
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
// /home/manfred/workspace/git/Brainfuck/Brainfuck/src/hw.bf
// /home/manfred/workspace/git/Brainfuck/Brainfuck/src/bottles.bf

int main (int argc, char *argv[])
{
  char character = NULL;
  unsigned char* program_memory = calloc(1024, 1024 * sizeof(char)); //TODO
  unsigned char* data_segment = calloc(1024, 1024 * sizeof(char)); //TODO
  char* user_input= calloc(1, 1024 * sizeof(char));
  int action_input_counter = 0;
  Boolean is_program_loaded = FALSE;
  Boolean close_program = FALSE;

  if(argc == 1)                        //Interactiv Debug Mode
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
          user_input[action_input_counter++]=character;
        }
        user_input[action_input_counter++]='\0';
        int user_input_length = strlen(user_input);
        char delimiter[] = " ";
        char* action = strtok(user_input, delimiter);
        char* first_parameter = NULL;
        char* second_parameter = NULL;

        if (strcmp(action, "load") == 0)
        {
          first_parameter = strtok(NULL, delimiter);
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
          runBrainfuckFile(program_memory, data_segment, is_program_loaded);
        }
        else if (strcmp(action, "eval") == 0)
        {
          first_parameter = strtok(NULL, " ");
          evalBrainfuckString(first_parameter);
        }
        else if (strcmp(action, "break") == 0)
        {
          first_parameter = strtok(NULL, " ");
          setBreakPoind(atoi(first_parameter));
        }
        else if (strcmp(action, "step") == 0)
        {
          first_parameter = strtok(NULL, " ");
          step(atoi(first_parameter));
        }
        else if (strcmp(action, "memory") == 0)
        {
          first_parameter = strtok(NULL, " ");
          second_parameter = strtok(NULL, " ");
          memory(atoi(first_parameter), second_parameter);
        }
        else if (strcmp(action, "show") == 0)
        {
          first_parameter = strtok(NULL, " ");
          show(atoi(first_parameter));
        }
        else if (strcmp(action, "change") == 0)
        {
          first_parameter = strtok(NULL, " ");
          second_parameter = strtok(NULL, " ");
          change(atoi(first_parameter), second_parameter);
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
        action_input_counter = 0;
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
      	  runBrainfuckFile(program_memory, data_segment, is_program_loaded);
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
/// This is an example header comment. Copypaste and adapt it! //TODO
///
/// @param filename
/// @param program_loaded
//
void runBrainfuckFile(unsigned char* memory_storage, unsigned char* data_segment, Boolean program_loaded) {
	if (program_loaded)
    {
      unsigned char ch = NULL;      // current char to be working on
      int run_counter = 0;
      int current_cell_index = 0;
      int bracket_counter = 0;     // to find paired brackets
      for(run_counter = 0; memory_storage[run_counter]; run_counter++)
        {
          ch = memory_storage[run_counter];
          //interpret brainfuck
          switch (ch){
            case '>': // increment pointer
            	current_cell_index++;
                ++data_segment;
              break;
            case '<': // decrement pointer
              //if i < 0 ERROR: tried to access invalid => just ignore it
              if(current_cell_index > 0)
              {
            	  current_cell_index--;
                --data_segment;
              }
              break;
            case '+': // increment pointer value
            	++*data_segment;
              break;
            case '-': // decrement pointer value
            	--*data_segment;
              break;
            case '.': // output pointer value
              putchar(*data_segment);
              break;
            case ',': // read in value
            	*data_segment = getchar();
              break;
            case '[': // start bracket loop
              if(!*data_segment)
              {
                bracket_counter++;
                while(bracket_counter)
                {
                  run_counter++;
                  if(memory_storage[run_counter] == ']')
                    bracket_counter--;
                  else if(memory_storage[run_counter] == '[')
                    bracket_counter++;
                }
              }
              break;
            case ']': // end bracket loop
              if(*data_segment)
              {
                bracket_counter++;
                while(bracket_counter)
                {
                  run_counter--;
                  if(memory_storage[run_counter] == '[')
                    bracket_counter--;
                  else if(memory_storage[run_counter] == ']')
                    bracket_counter++;
                }
              }
              break;
            default:
            	break;
          }
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
