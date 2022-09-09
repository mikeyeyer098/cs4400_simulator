/*
 * Author: Daniel Kopta
 * Updated by: Erin Parker
 * CS 4400, University of Utah
 *
 * Simulator handout
 * A simple x86-like processor simulator.
 * Read in a binary file that encodes instructions to execute.
 * Simulate a processor by executing instructions one at a time and appropriately 
 * updating register and memory contents.
 *
 * Some code and pseudo code has been provided as a starting point.
 *
 * Completed by: Michael Eyer, u1245499
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "instruction.h"

// Forward declarations for helper functions
unsigned int get_file_size(int file_descriptor);
unsigned int* load_file(int file_descriptor, unsigned int size);
instruction_t* decode_instructions(unsigned int* bytes, unsigned int num_instructions);
unsigned int execute_instruction(unsigned int program_counter, instruction_t* instructions, 
				 int* registers, unsigned char* memory);
void print_instructions(instruction_t* instructions, unsigned int num_instructions);
void error_exit(const char* message);

// 17 registers
#define NUM_REGS 17
// 1024-byte stack
#define STACK_SIZE 1024

int main(int argc, char** argv)
{
  // Make sure we have enough arguments
  if(argc < 2)
    error_exit("must provide an argument specifying a binary file to execute");

  // Open the binary file
  int file_descriptor = open(argv[1], O_RDONLY);
  if (file_descriptor == -1) 
    error_exit("unable to open input file");

  // Get the size of the file
  unsigned int file_size = get_file_size(file_descriptor);
  // Make sure the file size is a multiple of 4 bytes
  // since machine code instructions are 4 bytes each
  if(file_size % 4 != 0)
    error_exit("invalid input file");

  // Load the file into memory
  // We use an unsigned int array to represent the raw bytes
  // We could use any 4-byte integer type
  unsigned int* instruction_bytes = load_file(file_descriptor, file_size);
  close(file_descriptor);

  unsigned int num_instructions = file_size / 4;

  /****************************************/
  /**** Begin code to modify/implement ****/
  /****************************************/

  // Allocate and decode instructions (left for you to fill in)
  instruction_t* instructions = decode_instructions(instruction_bytes, num_instructions);

  // Optionally print the decoded instructions for debugging
  // Will not work until you implement decode_instructions
  // Do not call this function in your submitted final version
  //print_instructions(instructions, num_instructions);

  // Once you have completed Part 1 (decoding instructions), uncomment the below block

  // Allocate and initialize registers
  int* registers = (int*)malloc(sizeof(int) * NUM_REGS);
  // Set all registers values to 0
  int i;
  for (i = 0; i < NUM_REGS; i++) {
    registers[i] = 0x0;
  }

  // Set stack pointer register to be 1024 (0x400)
  registers[6] = 0x400;

  // Stack memory is byte-addressed, so it must be a 1-byte type 
  unsigned char* memory = malloc(STACK_SIZE);
  // Set all values in the stack to 0
  for (i = 0; i < STACK_SIZE; i++) {
    memory[i] = 0x0;
  }

  // Run the simulation
  unsigned int program_counter = 0;

  // program_counter is a byte address, so we must multiply num_instructions by 4 
  // to get the address past the last instruction
  while(program_counter != num_instructions * 4)
  {
    program_counter = execute_instruction(program_counter, instructions, registers, memory);
  }
  
  return 0;
}

/*
 * Decodes the array of raw instruction bytes into an array of instruction_t
 * Each raw instruction is encoded as a 4-byte unsigned int
*/
instruction_t* decode_instructions(unsigned int* bytes, unsigned int num_instructions)
{
  instruction_t* retval;
  retval = malloc(sizeof(instruction_t) * num_instructions);

  int i;
  for(i = 0; i < num_instructions; i++) {
    retval[i].opcode = ((bytes[i] >> 27) & 0x1F);
    retval[i].first_register = ((bytes[i] >> 22) & 0x1F);
    retval[i].second_register = ((bytes[i] >> 17) & 0x1F);
    retval[i].immediate = bytes[i] & 0xFFFF;
  }

  /*
  printf("opcode: %x\nf reg: %x\ns reg: %x\nimm: %x",     retval[i].opcode = ((bytes[i] >> 27) & 0x1F), 
    retval[i].first_register = ((bytes[i] >> 22) & 0x1F),
    retval[i].second_register = ((bytes[i] >> 17) & 0x1F),
    retval[i].immediate = bytes[i] & 0xFFFF);
    */

  return retval;
}


/*
 * Executes a single instruction and returns the next program counter
*/
unsigned int execute_instruction(unsigned int program_counter, instruction_t* instructions, int* registers, unsigned char* memory)
{
  // program_counter is a byte address, but instructions are 4 bytes each
  // divide by 4 to get the index into the instructions array
  instruction_t instr = instructions[program_counter / 4];
  //printf("program counter: %x, %d, %d\nopcode: %d\nfr: %x\nsr: %x\nimm: %x\n", program_counter, program_counter, program_counter / 4, instr.opcode, instr.first_register, instr.second_register, instr.immediate);
    //printf("esp?: %d (0x%x)\n", registers[sizeof(int) * 6], registers[sizeof(int) * 6]);

  int cmplResult = registers[instr.second_register] - registers[instr.first_register];
  int cmplSwitch = 0x0;
  int cf = (registers[16] & 0x1) >> 0;
  int zf = (registers[16] & 0x40) >> 6;
  int sf = (registers[16] & 0x80) >> 7;
  int of = (registers[16] & 0x800) >> 11;
  //printf("flags: %d, %d, %d, %d\n", cf, zf, sf, of);

  switch(instr.opcode)
  {
  //opcode 0, clear
  case subl:
    registers[instr.first_register] = registers[instr.first_register] - instr.immediate;
    break;
  //opcode 1, clear
  case addl_reg_reg:
    registers[instr.second_register] = registers[instr.first_register] + registers[instr.second_register];
    break;
  //opcode 2, clear
  case addl_imm_reg:
    registers[instr.first_register] = registers[instr.first_register] + instr.immediate;
    break;
  //opcode 3, clear
  case imull:
    registers[instr.second_register] = registers[instr.first_register] * registers[instr.second_register];
    break;
  //opcode 4, clear
  case shrl:
    registers[instr.first_register] = (unsigned int) registers[instr.first_register] >> 1;
    break;
  //opcode 5, clear
  case movl_reg_reg:
    registers[instr.second_register] = registers[instr.first_register];
    break;
  //opcode 6 (not implemented)
  case movl_deref_reg:
    registers[instr.second_register] = memory[registers[instr.first_register] + instr.immediate];
    break;
  //opcode 7 (not implemented)
  case movl_reg_deref:
    memory[registers[instr.second_register] + instr.immediate] = registers[instr.first_register];
    break;
  //opcode 8, clear
  case movl_imm_reg:
    registers[instr.first_register] = (unsigned char) instr.immediate;
    break;
  //opcode 9, (not implemented)
  case cmpl:
    //cf
    if ((unsigned int) registers[instr.second_register] < (unsigned int) registers[instr.first_register]) {
      cmplSwitch = cmplSwitch | (0x1 << 0);
    }
    //zf
    if (cmplResult == 0) {
      cmplSwitch = cmplSwitch | (0x1 << 6);
    }
    //sf
    if (cmplResult >> 31 & 0x1) {
      cmplSwitch = cmplSwitch | (0x1 << 7);
    }
    //of
    if (registers[instr.second_register] > 0 && registers[instr.first_register] < 0 && cmplResult < 0) {
      cmplSwitch = cmplSwitch | (0x1 << 11);
    } else if (registers[instr.second_register] < 0 && registers[instr.first_register] > 0 && cmplResult > 0) {
      cmplSwitch = cmplSwitch | (0x1 << 11);
    }

    //printf("cmpl called, switch: 0x%x, result: 0x%x\n", cmplSwitch, cmplResult);
    registers[16] = cmplSwitch;
    break;
  //opcode 10
  case je:
    if (zf) {
      return program_counter += instr.immediate + 4;
    }
    break;
  //opcode 11
  case jl:
    if (sf ^ of) {
      return program_counter += instr.immediate + 4;
    }
    break;
  //opcode 12
  case jle:
    if ((sf ^ of) | zf) {
      return program_counter += instr.immediate + 4;
    }
    break;
  //opcode 13
  case jge:
    if (~(sf ^ of) & 0x1) {
      return program_counter += instr.immediate + 4;
    }
    break;
  //opcode 14
  case jbe:
    if (cf | zf) {
      return program_counter += instr.immediate + 4;
    }
    break;
  //opcode 15
  case jmp:
    program_counter += instr.immediate + 4;
    return program_counter;
  //opcode 16
  case call:
    registers[6] = registers[6] - 4;
    memory[registers[6]] = program_counter + 4;
    program_counter += instr.immediate + 4;
    return program_counter;
  //opcode 17
  case ret:
    //printf("ret has been called!!!! esp?: %d (0x%x)\n", registers[sizeof(int) * 6], registers[sizeof(int) * 6]);
    //sleep(1);
    if (registers[6] == 0x400) {
      exit(0);
    }
    else {
      program_counter = memory[registers[6]];
      registers[6] = registers[6] + 4;
      return program_counter;
    }
    break;
  //opcode 18
  case pushl:
    //printf("pushl called\n");
    registers[6] -= 4;
    //printf("esp?: %d (0x%x)\n", registers[6], registers[6]);
    memory[registers[6]] = registers[instr.first_register];
    //printf("mem[esp]: %d (0x%x)\n", memory[registers[6]], memory[registers[6]]); 
    break;
  //opcode 19
  case popl:
    //printf("popl called\n");
    registers[instr.first_register] = memory[registers[6]];
    registers[6] += 4;
    break;
  //opcode 20
  case printr: 
    printf("%d (0x%x)\n", registers[instr.first_register], registers[instr.first_register]);
    break;
  //opcode 21
  case readr:
    scanf("%d", &(registers[instr.first_register]));
    break;
    

  }

  // TODO: Do not always return program_counter + 4
  //       Some instructions jump elsewhere

  // program_counter + 4 represents the subsequent instruction
  return program_counter + 4;
}


/*********************************************/
/****  DO NOT MODIFY THE FUNCTIONS BELOW  ****/
/*********************************************/

/*
 * Returns the file size in bytes of the file referred to by the given descriptor
*/
unsigned int get_file_size(int file_descriptor)
{
  struct stat file_stat;
  fstat(file_descriptor, &file_stat);
  return file_stat.st_size;
}

/*
 * Loads the raw bytes of a file into an array of 4-byte units
*/
unsigned int* load_file(int file_descriptor, unsigned int size)
{
  unsigned int* raw_instruction_bytes = (unsigned int*)malloc(size);
  if(raw_instruction_bytes == NULL)
    error_exit("unable to allocate memory for instruction bytes (something went really wrong)");

  int num_read = read(file_descriptor, raw_instruction_bytes, size);

  if(num_read != size)
    error_exit("unable to read file (something went really wrong)");

  return raw_instruction_bytes;
}

/*
 * Prints the opcode, register IDs, and immediate of every instruction, 
 * assuming they have been decoded into the instructions array
*/
void print_instructions(instruction_t* instructions, unsigned int num_instructions)
{
  printf("instructions: \n");
  unsigned int i;
  for(i = 0; i < num_instructions; i++)
  {
    printf("op: %d, reg1: %d, reg2: %d, imm: %d\n", 
	   instructions[i].opcode,
	   instructions[i].first_register,
	   instructions[i].second_register,
	   instructions[i].immediate);
  }
  printf("--------------\n");
}

/*
 * Prints an error and then exits the program with status 1
*/
void error_exit(const char* message)
{
  printf("Error: %s\n", message);
  exit(1);
}
