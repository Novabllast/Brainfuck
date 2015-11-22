//-----------------------------------------------------------------------------
// assa.c
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
#include <string.h>

typedef enum _Boolean_
{
	  FALSE = 0,
	  TRUE = 1
} Boolean;

char *get_filename_ext(char *filename);
int checkIfCharacterIsBrainfuckCommand(char character_to_check);
void loadBrainfuckFile(char *filename);
void runBrainfuckFile(char *memory_storage, int file_size, Boolean program_loaded);
void evalBrainfuckString(char* brainfuckstring);
void setBreakPoind(int program_counter);
void step(int number);
void memory(int number, char* type);
void show(int size);
void change(int number, char* hex_byte);
void quitProgram();
void quitViaEOF();

// -e
// /home/manfred/workspace/Brainfuck/src/hw.bf
// /home/manfred/workspace/Brainfuck/src/bottles.bf

int main (int argc, char *argv[])
{
  char character;
  char* program_memory = malloc(sizeof(char));
  char* action = malloc(sizeof(char));
  int some_counter = -1;
  Boolean is_program_loaded = TRUE;
  if(argc == 1)					//Interaktiver Debug Modus
    {
	  while (1) {//TODO
	    printf("esp>");
		for (some_counter = 0 ; character != EOF; some_counter++)
		{
		  character=getchar();
		  if(checkIfCharacterIsBrainfuckCommand(character) == 1)
		  {
			program_memory[some_counter]=character;
			program_memory=realloc(program_memory,(some_counter+2)*sizeof(char));
			if(program_memory == NULL)
			{
			  free(program_memory);
			  printf("Error: Out of memory!\n");
			  return 2;
			}
		  }
		}
	  }
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
		  //check if the file is a Brainfuck file
		  if (strcmp(get_filename_ext(filename), "bf") == 0)
		  {
	        FILE *file_to_read = fopen(filename, "r");
			if (file_to_read == 0)
			{
			  printf("[ERR] reading the file failed\n");
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
					return 2;
				  }
				}
			  }
			  is_program_loaded = TRUE;
			}
			fclose(file_to_read);
			runBrainfuckFile(program_memory, some_counter, is_program_loaded);
		  }
		}
		else
		{
		  printf("[ERR] usage: ./assa [-e brainfuck_filnename]\n");
		}
    }
	free(program_memory);
    return 0;
}

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it!
///
/// @param human The human to check.
/// @param number_to_return_if_not_married The number to return if the human
///        is not married. Should be 0 or 1 for example purposes.
///
/// @return int age of spouse
//
char *get_filename_ext(char *filename) {
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename)
      return "";
    return dot + 1;
}

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it!
///
/// @param human The human to check.
/// @param number_to_return_if_not_married The number to return if the human
///        is not married. Should be 0 or 1 for example purposes.
///
/// @return int age of spouse
//
void runBrainfuckFile(char *filename, int file_size, Boolean program_loaded) {
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
/// @return int 0 if no Brainfuck comman, 1 if Brainfuck Command
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

//-----------------------------------------------------------------------------
///
/// This is an example header comment. Copypaste and adapt it!
///
/// @return
//
void quitProgram()
{
  printf("Bye.\n");
}

