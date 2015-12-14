//-----------------------------------------------------------------------------
// assa.c
//
// C program which interprets and debugs "Brainfuck"-Code.
//
// Group: 13031 study assistant Angela Promitzer
//
// Authors: Manfred Böck 1530598, Anna Haupt 1432018, Patrick Struger 1530664
//
// Latest Changes: 14.12.2015 (by Manfred Böck)
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

int loadBrainfuckFile(char* filename, char* program_memory);
Boolean isBrainfuckCommand(char character_to_check);
int runBrainfuckFile(char* program_memory, unsigned char* data_segment,
                     int* break_points, int startposition, int endposition,
                     Boolean program_loaded);
int eval(char* brainfuckstring, char* eval_program_memory,
                        unsigned char* data_segment, int current_position,
                        int segment_position, int* break_points);
void setBreakPoint(int program_counter, int* break_points,
                   Boolean program_loaded);
int step(int number, char* program_memory, unsigned char* data_segment,
         int* break_points, int current_position, Boolean program_loaded);
void memory(int number, char* type, Boolean is_data_segment_loaded,
            unsigned char* data_segment);
void show(int size, char* program_memory, int current_position,
          Boolean program_loaded);
void change(int number, char* hex_byte, Boolean is_data_segment_loaded,
            unsigned char* data_segment);

#define INCORRECT_PROGRAM_CALL 1
#define OUT_OF_MEMORY 2
#define PARSING_ON_INPUT_FAILED 3
#define READING_THE_FILE_FAILED 4
#define PROGRAM_SUCCESSFULLY_LOADED 100
#define NO_PROGRAM_LOADED "[ERR] no program loaded\n"
#define WRONG_PARAMETER_COUNT "[ERR] wrong parameter count\n"
#define READING_THE_FILE_FAILED_MESSAGE "[ERR] reading the file failed\n"
#define ERROR_OUT_OF_MEMORY "Error: Out of memory!\n"
// -e
// /home/manfred/workspace/git/Brainfuck/Brainfuck/src/hw.bf
// /home/manfred/workspace/git/Brainfuck/Brainfuck/src/bottles.bf

// /home/anonymus/C_projects/Hauptbeispiel/Brainfuck/Brainfuck_v2/src/hw.bf
// /home/anonymus/C_projects/Hauptbeispiel/Brainfuck/Brainfuck_v2/src/bottles.bf

int main (int argc, char *argv[])
{
  char character = NULL;
  char* eval_program_memory = NULL;
  unsigned char* data_segment = calloc(1024, 1024 * sizeof(char));
  char* program_memory = calloc(1024, 1024 * sizeof(char));
  int* break_points = calloc(1024, 1024 * sizeof(int));
  char* user_input= malloc(2 * sizeof(char));
  int action_input_counter = 0;
  int current_position = 0;
  int segment_position = 0;
  Boolean is_program_loaded = FALSE;
  Boolean close_program = FALSE;
  Boolean is_data_segment_loaded = FALSE;
  Boolean run_instructions = FALSE;

  //Interactive Debug Mode
  if(argc == 1)
  {
    while (!close_program)
    {
      printf("esp> ");
      while((character = getchar()) != '\n' && character != EOF)
      {
        user_input[action_input_counter] = character;
        action_input_counter++;
        user_input=realloc(user_input, strlen(user_input) + 1 * sizeof(char));
        if(user_input == NULL)
        {
          free(user_input);
          user_input = NULL;
          printf(ERROR_OUT_OF_MEMORY);
          return OUT_OF_MEMORY;
        }
      }
      action_input_counter++;
      user_input[action_input_counter]='\0';
      int user_input_length = strlen(user_input);
      char delimiter[] = " ";
      char* action = strtok(user_input, delimiter);
      char* first_parameter = NULL;
      char* second_parameter = NULL;

      if (strcmp(action, "load") == 0)
      {
        first_parameter = strtok(NULL, delimiter);
        if (first_parameter)
        {
          int has_succeeded = -1;
          has_succeeded = loadBrainfuckFile(first_parameter, program_memory);
          switch (has_succeeded)
          {
            case OUT_OF_MEMORY:
              return OUT_OF_MEMORY;
              break;
            case READING_THE_FILE_FAILED:
              printf(READING_THE_FILE_FAILED_MESSAGE);
              break;
            case PROGRAM_SUCCESSFULLY_LOADED:
              current_position = 0;
              run_instructions = TRUE;
              is_program_loaded = TRUE;
              is_data_segment_loaded = TRUE;
              //Reset data_segment for new program
              if(data_segment != NULL)
              {
                free(data_segment);
                data_segment = NULL;
              }
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
        current_position = runBrainfuckFile(program_memory, data_segment,
                                            break_points, current_position,
                                            endposition, run_instructions);
    	int instructions_length = strlen(program_memory);
    	if (current_position > instructions_length &&
        		current_position != 0) {
            run_instructions = FALSE;
    	}
      }
      else if (strcmp(action, "eval") == 0)
      {
        first_parameter = strtok(NULL, " ");
        if (first_parameter != NULL)
        {
          segment_position = eval(first_parameter,
                                                 eval_program_memory,
                                                 data_segment,
                                                 current_position,
                                                 segment_position,
                                                 break_points);
          if(segment_position >= 0)
          {
            is_data_segment_loaded = TRUE;
          }
        }
      }
      else if (strcmp(action, "break") == 0)
      {
        first_parameter = strtok(NULL, " ");
        if (first_parameter != NULL)
        {
          setBreakPoint(atoi(first_parameter), break_points,
                        is_program_loaded);
        }
      }
      else if(strcmp(action, "step") == 0)
      {
        first_parameter = strtok(NULL, " ");
        if(first_parameter == NULL)
        {
          //Default-value
          first_parameter = "1";
        }
        current_position = step(atoi(first_parameter),program_memory,
                                data_segment, break_points, current_position,
                                is_program_loaded);
    	int instructions_length = strlen(program_memory);
    	if (current_position > instructions_length &&
    		current_position != 0) {
            run_instructions = FALSE;
    	}
      }
      else if (strcmp(action, "memory") == 0)
      {
        first_parameter = strtok(NULL, " ");
        second_parameter = strtok(NULL, " ");
        if (first_parameter != NULL && second_parameter == NULL)
        {
          //Default-values
          second_parameter = "hex";
          memory(atoi(first_parameter), second_parameter,
                 is_data_segment_loaded, data_segment);
        }
        else if (first_parameter == NULL && second_parameter == NULL)
        {
          //Default-values
          second_parameter = "hex";
          memory(segment_position, second_parameter,
                 is_data_segment_loaded, data_segment);
        }
        else if (first_parameter != NULL && second_parameter != NULL)
        {
          memory(atoi(first_parameter), second_parameter,
                 is_data_segment_loaded, data_segment);
        }
      }
      else if (strcmp(action, "show") == 0)
      {
        first_parameter = strtok(NULL, " ");
        if (first_parameter == NULL)
        {
          //Default-value
          first_parameter = "10";
        }
        show(atoi(first_parameter), program_memory,
                  current_position, is_program_loaded);
      }
      else if (strcmp(action, "change") == 0)
      {
        first_parameter = strtok(NULL, " ");
        second_parameter = strtok(NULL, " ");
        if (first_parameter == NULL && second_parameter == NULL)
        {
          change(segment_position, "00", is_data_segment_loaded, data_segment);
        }
        else if (first_parameter != NULL && second_parameter != NULL)
        {
          char compare_string[] = "0x";
          //compare the first two characters
          if(strncmp(second_parameter, compare_string, 2) != 0)
          {
            change(atoi(first_parameter), second_parameter,
                   is_data_segment_loaded, data_segment);
          }
        }
      }
      //Exits the program with a message
      else if (strcmp(action, "quit") == 0)
      {
        printf("Bye.\n");
        close_program = TRUE;
      }
      //Exits the program without a message
      else if (strcmp(user_input, "EOF") == 0)
      {
        close_program = TRUE;
      }
      //Resets the user input string
      memset(user_input,'\0',user_input_length);
      action_input_counter = 0;
    }
    free(user_input);
    user_input = NULL;
  }
  else if(argc == 2)
  {
    printf(WRONG_PARAMETER_COUNT);
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
      current_position = runBrainfuckFile(program_memory, data_segment,
                                          break_points, current_position,
                                          endposition, is_program_loaded);
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
/// Loads the given file into the program memory.
///
/// @param filename filename The Path of the file which should be loaded.
/// @return int 4, 2, 100 //TODO
//
int loadBrainfuckFile(char *filename, char* program_memory)
{
  char character;
  int character_counter = 0;
  int return_value = READING_THE_FILE_FAILED;

  FILE *file_to_read = fopen(filename, "r");
  if (file_to_read == 0)
  {
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
        {
          program_memory_size_limit *= 2;
          program_memory=realloc(program_memory, 2*program_memory_size);
        }

        if(program_memory == NULL)
        {
          free(program_memory);
          program_memory = NULL;
          printf(ERROR_OUT_OF_MEMORY);
          return_value =  OUT_OF_MEMORY;
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
/// Execudes the Brainfuck commands which has been loaded in the program memory
///
/// @param program_memory Int Array with the Brainfuck instructions.
/// @param data_segment //TODO
/// @param break_points Int Array with all breakpoints.
/// @param startposition //TODO
/// @param endposition //TODO
/// @param run_instructions
/// @return int The current position // TODO
//
int runBrainfuckFile(char* program_memory,
                     unsigned char* data_segment,
                     int* break_points,
                     int startposition, int endposition,
                     Boolean run_instructions)
{
  int currrent_position = 0;
  if (run_instructions)
  {
	Boolean break_point_detected = FALSE;
	unsigned char brainfuck_character = NULL;
	int current_cell_index = 0;
	// to find paired brackets
    int bracket_counter = 0;
	for(currrent_position = startposition; !break_point_detected &&
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
        currrent_position--;
        break_points[currrent_position] = 0;
        break_point_detected = TRUE;
      }
    }
  }
  else
  {
    printf(NO_PROGRAM_LOADED);
  }
  return currrent_position;
}

//-----------------------------------------------------------------------------
///
/// The function checks whether the given character
/// is a valid Brainfuck command or not.
///
/// @param character_to_check The character to check.
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
/// Executes the given Brainfuck commands.
///
/// @param brainfuckstring The commands to be executed.
/// @param eval_program_memory //TODO
/// @param data_segment //TODO
/// @param current_position The current position in the Brainfuck source code.
/// @param segment_position The current position in the data segment.
/// @param break_points Int Array with all breakpoints.
/// @return int The current position //TODO
//
int eval(char* brainfuckstring, char* eval_program_memory,
                        unsigned char* data_segment, int current_position,
                        int segment_position, int* break_points)
{
  if(strlen(brainfuckstring) < 80)
  {
    int bf_size = 2;
    int bf_index = 0;
    int string_index = 0;
    if(eval_program_memory == NULL)
    {
      eval_program_memory = malloc(bf_size * sizeof(char));
    }
    //check if it is a brainfuck command
    for(string_index = 0;
        string_index < strlen(brainfuckstring); string_index++)
    {
      if(isBrainfuckCommand(brainfuckstring[string_index]))
      {
        eval_program_memory[bf_index] = brainfuckstring[string_index];
        bf_size++;
        bf_index++;
        eval_program_memory = realloc(eval_program_memory, bf_size * sizeof(char));
        switch(brainfuckstring[string_index])
        {
          case '>':
            seg_position++;
            break;
          case '<':
            if(seg_position > 0)
            {
              seg_position--;
            }
            break;
          default:
            break;
        }
      }
    }
    int endposition = strlen(eval_program_memory);
    runBrainfuckFile(eval_program_memory, data_segment,
                     break_points, current_position, endposition, TRUE);
    free(eval_program_memory);
    eval_program_memory = NULL;
  }
  else
  {
    return -1;
  }
  return seg_position;
}

//-----------------------------------------------------------------------------
///
/// Sets a breakpoint in the loaded program.
///
/// @param program_counter The position where the breakpoint should be set.
/// @param break_points Int Array where the breakpoint should be set.///
/// @param program_loaded Boolean whether a Brainfuck program has been loaded
//         or not.
//
void setBreakPoint(int program_counter, int* break_points,
                   Boolean program_loaded)
{
  if (program_loaded)
  {
    break_points[program_counter] = 1;
  }
  else
  {
    printf(NO_PROGRAM_LOADED);
  }
}

//-----------------------------------------------------------------------------
///
/// Executes @number steps of the loaded program.
///
/// @param number The number of the steps to be executed.
/// @param program_memory Int Array with the Brainfuck instructions.
/// @param data_segment //TODO
/// @param break_points Int Array with all breakpoints.
/// @param current_position The current position in the Brainfuck source code.
/// @param program_loaded Boolean whether a Brainfuck program has been loaded
//         or not.
/// @return int The current position //TODO
//
int step(int number, char* program_memory, unsigned char* data_segment,
         int* break_points, int current_position, Boolean program_loaded)
{
  if (program_loaded)
  {
    number--;
    current_position = runBrainfuckFile(program_memory, data_segment,
                       break_points, current_position, number, program_loaded);
  }
  else
  {
    printf(NO_PROGRAM_LOADED);
  }
  return current_position;
}

//-----------------------------------------------------------------------------
///
/// Shows the memory of the data segment at the given position.
///
/// @param number Position of the memory to be shown.
/// @param type Possible types are hex, int bin and char.
/// @param is_data_segment_loaded Boolean whether
//         data_segment is initialized or not.
/// @param data_segment Contains the data which should be shown.
//
void memory(int number, char* type,
            Boolean is_data_segment_loaded, unsigned char* data_segment)
{
  if (is_data_segment_loaded)
  {
    if (strcmp(type, "hex") == 0)
    {
      printf("Hex at %d: %x\n", number, data_segment[number]);
    }
    else if (strcmp(type, "int") == 0)
    {
      printf("Integer at %d: %d\n", number, data_segment[number]);
    }
    else if (strcmp(type, "char") == 0)
    {
      printf("Character at %d: %c\n", number, data_segment[number]);
    }
    else if (strcmp(type, "bin") == 0)
    {
      int bin_index;
      int bin_size = 8;
      int bin[8] = {0};
      int decimal = data_segment[number];
      for (bin_index = 0; decimal > 0; bin_index++)
      {
        bin[bin_index] = decimal % 2;
        decimal = decimal / 2;
      }
      printf("Binary at %d: ", number);
      for (bin_index = (bin_size-1); bin_index >= 0; bin_index--)
      {
        printf("%d", bin[bin_index]);
      }
      printf("\n");
    }
  }
  else
  {
    printf(NO_PROGRAM_LOADED);
  }
}

//-----------------------------------------------------------------------------
///
/// Shows the next @size instructions from the program memory,
//  or till the program ends.
///
/// @param size The number of instructions to be shown.
/// @param program_memory Int Array with the Brainfuck instructions.
/// @param current_position The current position in the Brainfuck source code.
/// @param program_loaded Boolean whether a Brainfuck program has been loaded
//         or not.
//
void show(int size, char* program_memory,
          int current_position, Boolean program_loaded)
{
  int step_counter = 0;
  if (program_loaded)
  {
    int end_position = size + current_position - 1;
    for (step_counter = current_position;
         step_counter <= end_position; step_counter++)
    {
      putchar(program_memory[step_counter]);
    }
    printf("\n");
  }
  else
  {
    printf(NO_PROGRAM_LOADED);
  }
}

//-----------------------------------------------------------------------------
///
/// Changes the byte to a hex byte at the given position.
///
/// @param number Position of the byte to change.
/// @param hex_byte The new Value at the given position.
/// @param is_data_segment_loaded Boolean whether
//         data_segment is initialized or not.
/// @param data_segment Contains the data which should be changed.
//
void change(int number, char* hex_byte, Boolean is_data_segment_loaded,
            unsigned char* data_segment)
{
  if (is_data_segment_loaded)
  {
    char hex[sizeof(hex_byte)+3] = "0x";
    strcat(hex, hex_byte);
    int hex_to_int = (int)strtol(hex, NULL, 0);
    data_segment[number] = hex_to_int;
  }
  else
  {
    printf(NO_PROGRAM_LOADED);
  }
}
