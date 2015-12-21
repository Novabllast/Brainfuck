//-----------------------------------------------------------------------------
// assa.c
//
// C program which interprets and debugs "Brainfuck"-Code.
//
// Group: 13031 study assistant Angela Promitzer
//
// Authors: Manfred Böck 1530598, Anna Haupt 1432018, Patrick Struger 1530664
//
// Latest Changes: 21.12.2015 (by Böck Manfred and Patrick Struger)
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
int loadBrainfuckFile(char* filename, char** program_memory);
int runBrainfuckFile(char* program_memory, unsigned char* data_segment,
                     int* break_points, int startposition, int endposition,
                     int* segment_position, Boolean program_loaded);
void eval(char* brainfuckstring, unsigned char* data_segment,
          int current_position, int* seg_position, int* break_points);
void setBreakPoint(int program_counter, int* break_points,
                   Boolean program_loaded);
int step(int number, char* program_memory, unsigned char* data_segment,
         int* break_points, int current_position, int* segment_position,
         Boolean program_loaded);
void show(int size, char* program_memory, int current_position,
          Boolean program_loaded);
void change(int number, char* hex_byte, Boolean is_data_segment_loaded,
            unsigned char* data_segment);
void memory(int number, char* type, Boolean is_data_segment_loaded,
            unsigned char* data_segment);
int loadAndRunWithParameter(char *argv[]);
int interactiveDebugMode();
int handleUserInput(char* action, char* delimiter, char* program_memory,
                    unsigned char** data_segment, int* break_points,
                    int* current_position, int* segment_position,
                    Boolean* run_instructions, Boolean* is_program_loaded,
                    Boolean* is_data_segment_loaded);
int check_in_memory_range(int value_to_check, int segment_position);

#define INCORRECT_PROGRAM_CALL 1
#define OUT_OF_MEMORY 2
#define PARSING_ON_INPUT_FAILED 3
#define READING_THE_FILE_FAILED 4
#define PROGRAM_SUCCESSFULLY_LOADED 100
#define NO_PROGRAM_LOADED "[ERR] no program loaded\n"
#define ERROR_OUT_OF_MEMORY "Error: Out of memory!\n"
#define WRONG_PARAMETER_COUNT "[ERR] wrong parameter count\n"
#define READING_THE_FILE_FAILED_MESSAGE "[ERR] reading the file failed\n"

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
    has_succeeded = checkReturnValue(has_succeeded);
    if(has_succeeded == 100)
      has_succeeded = 0;
    return has_succeeded;
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
/// @param program_memory Char Array to store Brainfuck commands.
/// @return int (2)   - out of memory
///         int (4)   - reading the file failed
///         int (100) - file successfully loaded
//
int loadBrainfuckFile(char *filename, char** program_memory)
{
  char character = 0;
  int character_counter = 0;
  int return_value = READING_THE_FILE_FAILED;

  FILE *file_to_read = fopen(filename, "r");
  if (file_to_read == 0)
  {
    return_value = READING_THE_FILE_FAILED;
  }
  else
  {
    int program_memory_size = 0;
    int program_memory_size_limit = 1023;

    while ((character = fgetc(file_to_read)) != EOF)
    {
      if (isBrainfuckCommand(character))
      {
        (*program_memory)[program_memory_size] = character;
        program_memory_size++;
        if (program_memory_size >= program_memory_size_limit)
        {
          program_memory_size_limit *= 2;
          *program_memory = realloc(*program_memory,
        		                    2 * program_memory_size + 1);

          if (*program_memory == NULL)
          {
            free(*program_memory);
            *program_memory = NULL;
            fclose(file_to_read);
            return OUT_OF_MEMORY;
          }
        }
      }
    }

    (*program_memory)[program_memory_size] = '\0';
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
  int current_position = 0;
  data_segment = data_segment + *segment_position;
  if (run_instructions)
  {
    char brainfuck_character = 0;
    Boolean break_point_detected = FALSE;
    int current_cell_index = *segment_position;
    // to find paired brackets
    int bracket_counter = 0;
    for(current_position = startposition; !break_point_detected &&
      current_position <= endposition; current_position++)
    {
      if (break_points[current_position] != 1)
      {
        brainfuck_character = program_memory[current_position];
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
                current_position++;
                if(program_memory[current_position] == ']')
                  bracket_counter--;
                else if(program_memory[current_position] == '[')
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
                current_position--;
                if(program_memory[current_position] == '[')
                  bracket_counter--;
                else if(program_memory[current_position] == ']')
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
        break_points[current_position] = 0;
        break_point_detected = TRUE;
        current_position--;
      }
    }
  }
  else
  {
    printf(NO_PROGRAM_LOADED);
  }
<<<<<<< HEAD
  return currrent_position;
=======
  return current_position;
>>>>>>> origin/master
}

//-----------------------------------------------------------------------------
///
/// Executes the given Brainfuck commands.
///
/// @param brainfuckstring The commands to be executed.
/// @param eval_program_memory Char Array containing Brainfuck commands.
/// @param data_segment Unsigned Char Array containing the data pointers.
/// @param current_position The current position in the Brainfuck source code.
/// @param segment_position The current position in the data segment.
/// @param break_points Int Array with all breakpoints.
/// @return int The current position of the datasegment.
//
void eval(char* brainfuckstring, unsigned char* data_segment,
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
                     break_points, 0, endposition,
                     seg_position, program_loaded);
    free(eval_program_memory);
    eval_program_memory = NULL;
  }
}

//-----------------------------------------------------------------------------
///
/// Shows the next @size instructions from the program memory,
/// or till the program ends.
///
/// @param size The number of instructions to be shown.
/// @param program_memory Int Array with the Brainfuck instructions.
/// @param current_position The current position in the Brainfuck source code.
/// @param program_loaded Boolean whether a Brainfuck program has been loaded
///        or not.
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
/// Shows the memory of the data segment at the given position.
///
/// @param number Position of the memory to be shown.
/// @param type Possible types are hex, int bin and char.
/// @param is_data_segment_loaded Boolean whether
///        data_segment is initialized or not.
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
/// Changes the byte to a hex byte at the given position.
///
/// @param number Position of the byte to change.
/// @param hex_byte The new Value at the given position.
/// @param is_data_segment_loaded Boolean whether
///        data_segment is initialized or not.
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
    int segment_position = 0;
    int* break_points = calloc(1024, sizeof(int));
    char* program_memory = calloc(1024, sizeof(char));
    unsigned char* data_segment = calloc(1024, sizeof(unsigned char));

    has_succeeded = loadBrainfuckFile(filename, &program_memory);
    if(has_succeeded == PROGRAM_SUCCESSFULLY_LOADED)
    {
      has_succeeded = 0;
      is_program_loaded = TRUE;
      endposition = strlen(program_memory);
      break_points = realloc(break_points, 5 + endposition * sizeof(int));
      memset(break_points, 0, 5 + endposition);
      runBrainfuckFile(program_memory, data_segment, break_points,
                       0, endposition, &segment_position,
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
  int action_input_limit = 50;
  int action_input_counter = 0;

  Boolean close_program = FALSE;
  Boolean run_instructions = FALSE;
  Boolean is_program_loaded = FALSE;
  Boolean is_data_segment_loaded = FALSE;

  char* action = NULL;
  char* delimiter = " ";
  char* user_input= calloc(50, sizeof(char));

  int* break_points = calloc(1024, sizeof(int));
  char* program_memory = calloc(2048, sizeof(char));
  unsigned char* data_segment = calloc(1024, sizeof(unsigned char));

  while (!close_program)
  {
    printf("esp> ");
    while((character = getchar()) != '\n' && character != EOF)
    {
      user_input[action_input_counter] = character;
      action_input_counter++;
      if (action_input_counter >= action_input_limit)
      {printf("%i\n", action_input_counter);
    	  action_input_limit *= 2;
          user_input = realloc(user_input,
                               (action_input_limit + 1) * sizeof(char));
	  }

      if(user_input == NULL)
      {
        free(user_input);
        user_input = NULL;
        printf(ERROR_OUT_OF_MEMORY);
        return_value = OUT_OF_MEMORY;
      }
    }

    //check if there is a user input
    if (user_input[0] != 0)
    {
      user_input[action_input_counter] = '\0';
      action = strtok(user_input, delimiter);
      //Exits the program with a message
      if (strcmp(action, "quit") == 0)
      {
        printf("Bye.\n");
        close_program = TRUE;
      }
      //Exits the program without a message
      else if (strcmp(user_input, "EOF") == 0 || character == EOF)
      {
        close_program = TRUE;
      }
      else
      {
        unsigned char** data = &data_segment;
        return_value = handleUserInput(action, delimiter, program_memory,
                                       data, break_points,
                                       &current_position,
                                       &segment_position, &run_instructions,
                                       &is_program_loaded,
                                       &is_data_segment_loaded);
      }
    }

    //Resets the user input string
    action_input_counter = 0;
    user_input = realloc(user_input, 50 * sizeof(char));
    user_input[0] = '\0';
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
      has_succeeded = loadBrainfuckFile(first_parameter, &program_memory);
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
        *data_segment = calloc(1024, sizeof(unsigned char));
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
      *run_instructions = FALSE;
  }
  else if (strcmp(action, "eval") == 0)
  {
    if (first_parameter)
    {
      eval(first_parameter, *data_segment, *current_position,
           segment_position, break_points);
      if(*segment_position >= 0)
        *is_data_segment_loaded = TRUE;
    }
  }
  else if (strcmp(action, "break") == 0)
  {
    if (first_parameter)
      setBreakPoint(atoi(first_parameter), break_points, *is_program_loaded);
  }
  else if(strcmp(action, "step") == 0)
  {
    if(!first_parameter)
    {
      //Default-value
      first_parameter = "1";
    }
    int steps = atoi(first_parameter);
    steps = *current_position + steps - 1;

    if (steps <= instruction_length)
      *current_position = step(steps, program_memory,
                               *data_segment, break_points, *current_position,
                               segment_position, *is_program_loaded);

    if (*current_position > instruction_length && *current_position != 0)
      *run_instructions = FALSE;
  }
  else if (strcmp(action, "show") == 0)
  {
    int show_count = 0;
    if (!first_parameter)
      //Default-value
      show_count = 10;
    else if(atoi(first_parameter) > (instruction_length - *current_position))
      show_count = (instruction_length - *current_position);
    else
      show_count = atoi(first_parameter);

    show(show_count, program_memory, *current_position, *is_program_loaded);
  }
  else if (strcmp(action, "memory") == 0)
  {
    int memory_position = 0;
    if(first_parameter)
      memory_position = check_in_memory_range(atoi(first_parameter), *segment_position);
    if (first_parameter && !second_parameter)
    {
      //Default-values
      second_parameter = "hex";
      memory(memory_position, second_parameter,
             *is_data_segment_loaded, *data_segment);
    }
    else if (!first_parameter && !second_parameter)
    {
      //Default-values
      second_parameter = "hex";
      memory(*segment_position, second_parameter,
             *is_data_segment_loaded, *data_segment);
    }
    else if (first_parameter && second_parameter)
      memory(memory_position, second_parameter,
             *is_data_segment_loaded, *data_segment);
  }
  else if (strcmp(action, "change") == 0)
  {
    int change_position = 0;
    if(first_parameter)
      change_position = check_in_memory_range(atoi(first_parameter), *segment_position);
    if (!first_parameter && !second_parameter)
      change(*segment_position, "00", *is_data_segment_loaded, *data_segment);
    else if (first_parameter && !second_parameter)
      change(change_position, "00",*is_data_segment_loaded, *data_segment);
    else if (first_parameter && second_parameter)
    {
      char compare_string[] = "0x";
      //compare the first two characters
      if(strncmp(second_parameter, compare_string, 2) != 0)
        change(change_position, second_parameter,
               *is_data_segment_loaded, *data_segment);
    }
  }

  return return_value;
}

//-----------------------------------------------------------------------------
///
/// Function to check if the value_to_check is within the data segment range.
///
/// @param value_to_check Wanted position of the user for change or memory.
/// @param segment_position current position of the data segment.
/// @return int value_to_check   - value is within range and
///                                gets wanted position
///         int segment_position - value is not within range and
///                                gets endposition
//
int check_in_memory_range(int value_to_check, int segment_position)
{
  if(value_to_check > segment_position)
    return segment_position;
  else
    return value_to_check;
}
