//-----------------------------------------------------------------------------
// assa.c
//
// C program which interprets and debugs "Brainfuck"-Code.
//
// Group: 13031 study assistant Angela Promitzer
//
// Authors: Manfred Böck 1530598, Anna Haupt 1432018, Patrick Struger 1530664
//
// Latest Changes: 16.12.2015 (by Struger Patrick)
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

int checkReturnValue(int return_value);
Boolean isBrainfuckCommand(char character_to_check);
int loadBrainfuckFile(char* filename, char* program_memory);
int runBrainfuckFile(char* program_memory, unsigned char* data_segment,
                     int* break_points, int startposition, int endposition,
                     int* segment_position, Boolean program_loaded);
int eval(char* brainfuckstring, unsigned char* data_segment,
         int current_position, int* seg_position, int* break_points);
void setBreakPoint(int program_counter, int* break_points,
                   Boolean program_loaded);
int step(int number, char* program_memory, unsigned char* data_segment,
         int* break_points, int current_position, int* segment_position,
         Boolean program_loaded);
int loadAndRunWithParameter(char *argv[]);
int interactiveDebugMode();
int handleUserInput(char* action, char* delimiter, char* program_memory, 
                    unsigned char** data_segment, int* break_points,
                    int* current_position, int* segment_position,
                    Boolean* run_instructions, Boolean* is_program_loaded,
                    Boolean* is_data_segment_loaded);

#define INCORRECT_PROGRAM_CALL 1
#define OUT_OF_MEMORY 2
#define PARSING_ON_INPUT_FAILED 3
#define READING_THE_FILE_FAILED 4
#define PROGRAM_SUCCESSFULLY_LOADED 100
#define NO_PROGRAM_LOADED "[ERR] no program loaded\n"
#define WRONG_PARAMETER_COUNT "[ERR] wrong parameter count\n"
#define READING_THE_FILE_FAILED_MESSAGE "[ERR] reading the file failed\n"
#define ERROR_OUT_OF_MEMORY "Error: Out of memory!\n"

int main(int argc, char *argv[])
{
  //Interactive Debug Mode
  if (argc == 1)
  {
    return interactiveDebugMode();
  }
  else if (argc == 2)
  {
    printf(WRONG_PARAMETER_COUNT);
  }
  // load and run via terminal
  else if (argc >= 3)
  {    
    int has_succeeded = -1;
    has_succeeded = loadAndRunWithParameter(argv);
    switch (has_succeeded)
    {
      case OUT_OF_MEMORY:
        return OUT_OF_MEMORY;
        break;
      case READING_THE_FILE_FAILED:
        return READING_THE_FILE_FAILED;
        break;
      default:
        break;
    }
  }
  return 0;
}

//Functions
//-----------------------------------------------------------------------------
///
/// The function checks whether the given character
/// is a valid Brainfuck command or not.
///
/// @param character_to_check The character to check.
/// @return int (0) - valid Brainfuck command
///             (1) - invalid Brainfuck Command
//
Boolean isBrainfuckCommand(char character_to_check)
{
  Boolean is_brainfuck_command = FALSE;
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
      is_brainfuck_command = TRUE;
      break;
    default:
      break;
  }
  return is_brainfuck_command;
}

//-----------------------------------------------------------------------------
///
/// Function to check what kind of return
///
/// @param int Return value to check.
/// @return int (2)   - out of memory
///         int (4)   - reading the file failed
///         int (100) - program was successfully loaded
//
int checkReturnValue(int return_value)
{
  switch (return_value)
  {
    case OUT_OF_MEMORY:
      printf(ERROR_OUT_OF_MEMORY);
      break;
    case READING_THE_FILE_FAILED:
      printf(READING_THE_FILE_FAILED_MESSAGE);
      break;
    default:
      break;
  }
  return return_value;
}

//-----------------------------------------------------------------------------
///
/// Loads the given file into the program memory.
///
/// @param filename filename The Path of the file which should be loaded.
/// @return int (2)   - out of memory
///         int (4)   - reading the file failed
///         int (100) - file successfully loaded
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
    int program_memory_size_limit = 1023;
    int program_memory_size = sizeof(char) / sizeof(program_memory[0]);
    
    while ((character = fgetc(file_to_read)) != EOF)
    {
      if (isBrainfuckCommand(character))
      {
        program_memory[character_counter++]=character;
        if (program_memory_size == program_memory_size_limit)
        {
          program_memory_size_limit *= 2;
          program_memory=realloc(program_memory, 2 * program_memory_size);
        }
        if (program_memory == NULL)
        {
          free(program_memory);
          program_memory = NULL;
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
/// Executes the Brainfuck commands which have been loaded in the program memory
///
/// @param program_memory Char Array containing Brainfuck commands.
/// @param data_segment Unsigned Char Array containing the data pointers.
/// @param break_points Int Array with all breakpoints.
/// @param startposition Position of the first command to execute.
/// @param endposition Position of the last command to execute.
/// @param segment_position Current position of the data pointer.
/// @param run_instructions Boolean to check if program should be executed.
/// @return int The current position of the last executed command.      
//
int runBrainfuckFile(char* program_memory, unsigned char* data_segment,
                     int* break_points, int startposition, int endposition,
                     int* segment_position, Boolean run_instructions)
{
  int currrent_position = 0;
  data_segment = data_segment + *segment_position;
  if (run_instructions)
  {
    char brainfuck_character = 0;
    Boolean break_point_detected = FALSE;
    int current_cell_index = *segment_position;
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
            *segment_position = *segment_position + 1;
            ++data_segment;
            break;
          case '<': // decrement pointer
            //if i < 0 ERROR: tried to access invalid => just ignore it
            if(current_cell_index > 0)
            {
              current_cell_index--;
              *segment_position = *segment_position - 1;
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
        break_point_detected = TRUE;
        currrent_position--;
      }
    }
  }
  else
  {
    printf(NO_PROGRAM_LOADED);
  }
  printf("Segment int Row 2 %d", *segment_position);
  return currrent_position;
}

//-----------------------------------------------------------------------------
///
/// Executes the given Brainfuck commands.
///
/// @param brainfuckstring The commands to be executed.
/// @param eval_program_memory //TODO
/// @param data_segment Unsigned Char Array containing the data pointers.
/// @param current_position The current position in the Brainfuck source code.
/// @param segment_position The current position in the data segment.
/// @param break_points Int Array with all breakpoints.
/// @return int The current position of the datasegment.
//
int eval(char* brainfuckstring, unsigned char* data_segment,
         int current_position, int* seg_position, int* break_points)
{
  if (strlen(brainfuckstring) < 80)
  {
    int bf_index = 0;
    int endposition = 0;
    int string_index = 0;
    Boolean program_loaded = TRUE;
    char* eval_program_memory = malloc(1 * sizeof(char));
    
    //check if it is a brainfuck command
    for (string_index = 0; string_index < strlen(brainfuckstring);
         string_index++)
    {
      if (isBrainfuckCommand(brainfuckstring[string_index]))
      {
        eval_program_memory[bf_index] = brainfuckstring[string_index];
        bf_index++;
        eval_program_memory = realloc(eval_program_memory,
                                      (bf_index + 1) * sizeof(char));
      }
    }
    eval_program_memory[bf_index] = '\0';
    endposition = strlen(eval_program_memory);
    runBrainfuckFile(eval_program_memory, data_segment,
                     break_points, current_position, endposition,
                     seg_position, program_loaded);
    free(eval_program_memory);
    eval_program_memory = NULL;
  }
  else
  {
    return -1;
  }
  return *seg_position;
}

//-----------------------------------------------------------------------------
///
/// Sets a breakpoint in the loaded program.
///
/// @param program_counter The position where the breakpoint should be set.
/// @param break_points Int Array where the breakpoint should be set.
/// @param program_loaded Boolean whether a Brainfuck program has been loaded
///        or not.
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
/// @param data_segment Unsigned Char Array containing the data pointers.
/// @param break_points Int Array with all breakpoints.
/// @param current_position The current position in the Brainfuck source code.
/// @param program_loaded Boolean whether a Brainfuck program has been loaded
///        or not.
/// @return int The current position of the last executed command.
//
int step(int number, char* program_memory, unsigned char* data_segment,
         int* break_points, int current_position, int* segment_position,
         Boolean program_loaded)
{
  if (program_loaded)
  {
    current_position = runBrainfuckFile(program_memory, data_segment,
                       break_points, current_position, number,
                       segment_position, program_loaded);
  }
  else
  {
    printf(NO_PROGRAM_LOADED);
  }
  return current_position;
}

//-----------------------------------------------------------------------------
///
/// Function to load and execute via terminal if the second parameter is -e and
/// the third parameter is a valid filename.
///
/// @param argv char Array which contains all parameters from the terminal.
/// @return int (2)   - out of memory
///         int (4)   - reading the file failed
///         int (100) - file successfully loaded 
//
int loadAndRunWithParameter(char* argv[])
{
  if(strcmp(argv[1], "-e") == 0)
  {
    int endposition = 0;
    int has_succeeded = -1;
    char* filename = argv[2];
    Boolean is_program_loaded = FALSE;
    int* break_points = calloc(1024, sizeof(int));
    char* program_memory = calloc(1024, sizeof(char));
    has_succeeded = loadBrainfuckFile(filename, program_memory);
    unsigned char* data_segment = calloc(1024, sizeof(char));
    
    if(checkReturnValue(has_succeeded) == PROGRAM_SUCCESSFULLY_LOADED)
    {
      is_program_loaded = TRUE;
      endposition = strlen(program_memory);
      runBrainfuckFile(program_memory, data_segment, break_points,
                       0, endposition, 0,
                       is_program_loaded);
    }
                     
    free(program_memory);
    program_memory = NULL;
    free(data_segment);
    data_segment = NULL;
    free(break_points);
    break_points = NULL;
    return has_succeeded;
  }
  else
  {
    printf("[ERR] usage: ./assa [-e brainfuck_filnename]\n");
    return INCORRECT_PROGRAM_CALL;
  }
}

//-----------------------------------------------------------------------------
///
/// Function to start interactive debug mode reacting to user inputs
///
/// @return int (0) - no errors
///         int (2) - out of memory
///         int (4) - reading the file failed
//
int interactiveDebugMode()
{
  char character = 0;
  
  int return_value = 0;
  int current_position = 0;
  int segment_position = 0;
  int action_input_counter = 0;
  
  Boolean close_program = FALSE;
  Boolean run_instructions = FALSE;
  Boolean is_program_loaded = FALSE;
  Boolean is_data_segment_loaded = FALSE;
  
  char* action = NULL;
  char* delimiter = " ";
  char* user_input= calloc(1, sizeof(char));
  
  int* break_points = calloc(1024, sizeof(int));
  char* program_memory = calloc(1024, sizeof(char));
  unsigned char* data_segment = calloc(1024, sizeof(char));
  
  while (!close_program)
  {
    printf("esp> ");
    while((character = getchar()) != '\n' && character != EOF)
    {
      user_input[action_input_counter] = character;
      action_input_counter++;
      user_input = realloc(user_input, 
                           (action_input_counter + 1) * sizeof(char));
      
      if(user_input == NULL)
      {
        free(user_input);
        user_input = NULL;
        printf(ERROR_OUT_OF_MEMORY);
        return_value = OUT_OF_MEMORY;
      }
    }
    
    user_input[action_input_counter] = '\0';
    action = strtok(user_input, delimiter);
    
    //Exits the program with a message
    if (strcmp(action, "quit") == 0)
    {
      printf("Bye.\n");
      close_program = TRUE;
    }
    //Exits the program without a message
    else if (strcmp(user_input, "EOF") == 0)
    {
      close_program = TRUE;
    }
    else 
    {
      //int* segment_pos = &segment_position;
      unsigned char** data = &data_segment;
      return_value = handleUserInput(action, delimiter, program_memory,
                                     data, break_points,
                                     &current_position,
                                     &segment_position, &run_instructions,
                                     &is_program_loaded,
                                     &is_data_segment_loaded);
       printf("Segment %d\n", segment_position);               
    }
    //Resets the user input string
    action_input_counter = 0;
    user_input = realloc(user_input, 2 * sizeof(char));
  }
  
  free(user_input);
  user_input = NULL;
  free(program_memory);
  program_memory = NULL;
  free(data_segment);
  data_segment = NULL;
  free(break_points);
  break_points = NULL;
  
  return return_value;
}

//-----------------------------------------------------------------------------
///
/// Function to start interactive debug mode reacting to user inputs
///
/// @param argv char Array which contains all parameters from the terminal.
/// @return int (2)   - out of memory
///         int (4)   - reading the file failed
//
int handleUserInput(char* action, char* delimiter, char* program_memory, 
                    unsigned char** data_segment, int* break_points,
                    int* current_position, int* segment_position,
                    Boolean* run_instructions, Boolean* is_program_loaded,
                    Boolean* is_data_segment_loaded)
{
  int return_value = 0;
  int instruction_length = strlen(program_memory);
  
  char* first_parameter = NULL;
  char* second_parameter = NULL;
  first_parameter = strtok(NULL, delimiter);
  second_parameter = strtok(NULL, delimiter);
  
  if (strcmp(action, "load") == 0)
  {
    if (first_parameter)
    {
      int has_succeeded = -1;
      has_succeeded = loadBrainfuckFile(first_parameter, program_memory);
      if (checkReturnValue(has_succeeded) == PROGRAM_SUCCESSFULLY_LOADED)
      {
        *current_position = 0;
        *run_instructions = TRUE;
        *is_program_loaded = TRUE;
        *is_data_segment_loaded = TRUE;
        //Reset data_segment for new program
        if (data_segment != NULL)
        {
          free(*data_segment);
          *data_segment = NULL;
          *segment_position = 0;
        }
        *data_segment = calloc(1024, sizeof(char));
      }
    }
  }
  else if (strcmp(action, "run") == 0)
  {
    *current_position = runBrainfuckFile(program_memory, *data_segment,
                                        break_points, *current_position,
                                        instruction_length, segment_position, 
                                        *run_instructions);
    if (*current_position > instruction_length && *current_position != 0)
    {
      *run_instructions = FALSE;
    }
  }
  else if (strcmp(action, "eval") == 0)
  {
    if (first_parameter)
    {
      eval(first_parameter, *data_segment, *current_position,
           segment_position, break_points);
      printf("Segment 1: %d", *segment_position);
      if(*segment_position >= 0)
      {
        *is_data_segment_loaded = TRUE;
      } 
    }
  }
  else if (strcmp(action, "break") == 0)
  {
    if (first_parameter)
    {
      setBreakPoint(atoi(first_parameter), break_points, *is_program_loaded);
    }
  } 
  else if(strcmp(action, "step") == 0)
  {
    if(first_parameter == NULL)
    {
      //Default-value
      first_parameter = "1";
    }
    int steps = atoi(first_parameter);
    steps = *current_position + steps - 1;
    if (steps <= instruction_length)
    {
      *current_position = step(steps, program_memory,
                              *data_segment, break_points, *current_position,
                              segment_position, *is_program_loaded);
    }
    if (*current_position > instruction_length && *current_position != 0)
    {
      *run_instructions = FALSE;
    }
  }
  return return_value;
}
