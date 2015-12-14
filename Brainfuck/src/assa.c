//-----------------------------------------------------------------------------
// assa.c
//
// C Programm, welches Speicherverwaltung und Strings behandelt
//
// Group: 13031 study assistant Angela Promitzer
//
// Authors: Manfred Böck 1530598, Anna Haupt 1432018, Patrick Struger 1530664
//
// Latest Changes: 12.12.2015 (by Patrick Struger)
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
int runBrainfuckFile(unsigned char* program_memory, unsigned char* data_segment, int* break_points, int startposition, int endposition,Boolean program_loaded);
void evalBrainfuckString(char* brainfuckstring);
void setBreakPoint(int program_counter, int* break_points, Boolean program_loaded);
int step(int number, unsigned char* program_memory, unsigned char* data_segment, int* break_points, int current_position, Boolean program_loaded);
void memory(int number, char* type, Boolean program_loaded);
void show(int size, unsigned char* program_memory, int current_position, Boolean program_loaded);
void change(int number, char* hex_byte, Boolean program_loaded);

#define INCORRECT_PROGRAM_CALL 1
#define OUT_OF_MEMORY 2
#define PARSING_ON_INPUT_FAILED 3
#define READING_THE_FILE_FAILED 4
#define PROGRAM_SUCCESSFULLY_LOADED 100

// -e
// /home/manfred/workspace/git/Brainfuck/Brainfuck/src/hw.bf
// /home/manfred/workspace/git/Brainfuck/Brainfuck/src/bottles.bf

// C:/Users/Patrick/Documents/GitHub/Brainfuck/Brainfuck/src/hw.bf
// C:/Users/Patrick/Documents/GitHub/Brainfuck/Brainfuck/src/bottles.bf

int main (int argc, char *argv[])
{
  char character = NULL;
  unsigned char* program_memory = calloc(1024, 1024 * sizeof(char)); //TODO
  unsigned char* data_segment = calloc(1024, 1024 * sizeof(char)); //TODO
  int* break_points = calloc(1024, 1024 * sizeof(int)); //TODO
  char* user_input= calloc(1, 1024 * sizeof(char));
  int action_input_counter = 0;
  int current_position = 0;
  Boolean is_program_loaded = FALSE;
  Boolean close_program = FALSE;

  //Interactive Debug Mode
  if(argc == 1)
  {
    while (!close_program)
    {
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
        if (!first_parameter)
        {
          printf("[ERR] usage: load brainfuck_filnename\n");
          //TODO ist diese Ausgabe erlaubt?
        }
        else
        {
          int has_succeeded = -1;
          has_succeeded = loadBrainfuckFile(first_parameter, program_memory);
          switch (has_succeeded)
          {
            case OUT_OF_MEMORY:
              return OUT_OF_MEMORY;
              break;
            case READING_THE_FILE_FAILED:
              break;
            case PROGRAM_SUCCESSFULLY_LOADED:
              is_program_loaded = TRUE;
              data_segment = calloc(1024, 1024 * sizeof(char)); //TODO
              break;
            default:
              break;
          }
        }
      }
      else if (strcmp(action, "run") == 0)
      {
        int endposition = strlen(program_memory);
        runBrainfuckFile(program_memory, data_segment, break_points, current_position, endposition, is_program_loaded);
        is_program_loaded = FALSE;
      }
      else if (strcmp(action, "eval") == 0)
      {
        first_parameter = strtok(NULL, " ");
        if (first_parameter != NULL)
        {
          evalBrainfuckString(first_parameter);
        }
      }
      else if (strcmp(action, "break") == 0)
      {
        first_parameter = strtok(NULL, " ");
        if (first_parameter != NULL)
        {
          setBreakPoint(atoi(first_parameter), break_points, is_program_loaded);
        }
      }
      else if(strcmp(action, "step") == 0)
      {
        first_parameter = strtok(NULL, " ");
        if(first_parameter == NULL)
        {
          first_parameter = "1";
        }
        current_position = step(atoi(first_parameter),program_memory, data_segment, break_points, current_position, is_program_loaded);
      }
      else if (strcmp(action, "memory") == 0)
      {
        first_parameter = strtok(NULL, " ");
        second_parameter = strtok(NULL, " ");
        if (first_parameter != NULL && second_parameter != NULL)
        {
          memory(atoi(first_parameter), second_parameter, is_program_loaded);
        }
      }
      else if (strcmp(action, "show") == 0)
      {
        first_parameter = strtok(NULL, " ");
        if (first_parameter == NULL)
        {
          first_parameter = "10";
        }
        show(atoi(first_parameter), program_memory , current_position, is_program_loaded);
      }
      else if (strcmp(action, "change") == 0)
      {
        first_parameter = strtok(NULL, " ");
        second_parameter = strtok(NULL, " ");
        if (first_parameter != NULL && second_parameter != NULL)
        {
          change(atoi(first_parameter), second_parameter, is_program_loaded);
        }
      }
      else if (strcmp(action, "quit") == 0)
      {
        printf("Bye.\n");
        close_program = TRUE;
      }
      else if(strcmp(user_input, "EOF") == 0)
      {
        close_program = TRUE;
      }
      else
      {
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
        switch (has_succeeded)
        {
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
  		  int endposition = strlen(program_memory);
      	current_position = runBrainfuckFile(program_memory, data_segment, break_points, current_position, endposition, is_program_loaded);
      }
      else
      {
        printf("[ERR] usage: ./assa [-e brainfuck_filnename]\n");
        return INCORRECT_PROGRAM_CALL;
      }
  }
  free(program_memory);
  program_memory = NULL;
  free(data_segment);
  data_segment = NULL;
  free(break_points);
  break_points = NULL;
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
char* get_filename_ext(char *filename)
{
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
/// @return int 4, 2, 100
//
int loadBrainfuckFile(char *filename, unsigned char* program_memory)
{
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
      int program_memory_size_limit = 1023;
      while((character = fgetc(file_to_read)) != EOF)
      {
        if(isBrainfuckCommand(character))
        {
          program_memory[character_counter++]=character;
          if (program_memory_size == program_memory_size_limit)
          { //TODO keine ahnung wie die abfrage lauten soll
            //printf("i: %i D: %i", program_memory_size, strlen((const char*)program_memory));
            program_memory_size_limit *= 2;
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
int runBrainfuckFile(unsigned char* program_memory,
                     unsigned char* data_segment,
                     int* break_points,
                     int startposition, int endposition,
                     Boolean program_loaded)
{
	printf("%d: %d: \n", startposition, endposition);
	int currrent_position = 0;
	if (program_loaded)
  {
	  // current char to be working on
    unsigned char brainfuck_character = NULL;
	  int current_cell_index = 0;
	  // to find paired brackets
    int bracket_counter = 0;
	  for(currrent_position = startposition;
		  currrent_position <= endposition; currrent_position++)
	  {
      if (break_points[currrent_position] != 1)
      {
        brainfuck_character = program_memory[currrent_position];
        //interpret brainfuck
        switch (brainfuck_character)
        {
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
                currrent_position++;
                if(program_memory[currrent_position] == ']')
                  bracket_counter--;
                else if(program_memory[currrent_position] == '[')
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
                currrent_position--;
                if(program_memory[currrent_position] == '[')
                  bracket_counter--;
                else if(program_memory[currrent_position] == ']')
                  bracket_counter++;
              }
            }
            break;
          default:
            break;
        }
      }
      else
      {
        break_points[currrent_position] = 0;
      }
    }
  }
  else
  {
    printf("[ERR] no program loaded\n");
  }
	return currrent_position;
}

//-----------------------------------------------------------------------------
///
/// The function checks whether the given character
/// is a valid Brainfuck command or not.
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
/// @param program_counter
///
/// @param break_points
///
/// @param program_loaded
//
void setBreakPoint(int program_counter, int* break_points, Boolean program_loaded)
{
  if (program_loaded)
  {
	  break_points[program_counter] = 1;
  }
  else
  {
    printf("[ERR] no program loaded\n");
  }
}

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it!//TODO
///
/// @param
//
int step(int number, unsigned char* program_memory, unsigned char* data_segment, int* break_points, int current_position, Boolean program_loaded)
{
  if (program_loaded)
  {
    current_position = runBrainfuckFile(program_memory, data_segment, break_points, current_position, number, program_loaded);
  }
  else
  {
    printf("[ERR] no program loaded\n");
  }
  return current_position;
}

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it!//TODO
///
/// @param
//

void memory(int number, char* type, Boolean program_loaded)
{
	  if (program_loaded)
	  {

	  }
	  else
	  {
      printf("[ERR] no program loaded\n");
	  }
}

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it!//TODO
///
/// @param
//

void show(int size, unsigned char* program_memory,  int current_position, Boolean program_loaded)
{
  int step_counter = 0;
  if (program_loaded)
  {
    int endposition = size + current_position -1;
    for (step_counter =  current_position; step_counter < endposition; step_counter++)
    {
      putchar(program_memory[step_counter]);
    }
    printf("\n");
  }
  else
  {
    printf("[ERR] no program loaded\n");
  }
}

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it!//TODO
///
/// @param
//
void change(int number, char* hex_byte, Boolean program_loaded)
{
	  if (program_loaded)
	  {

	  }
	  else
	  {
      printf("[ERR] no program loaded\n");
	  }
}
