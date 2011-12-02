/**
 * memory.c: Handles the memory management.
 *
 * @author: Kyle Hardgrave <kyleh>
 * @package: trace
 */

#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

#define I_OP(i)   (i >> 12) // For opcodes
#define I_2_0(i)  (i & 0x7) //For Rt
#define I_8_6(i)  ((i >> 6) & 0x7) // For Rs
#define I_11_9(i) ((i >> 9) & 0x7) // For Rd, and NZP codes

#define I_3_0(i)  (i & 0xF) // For IMM4 (AND w/ 15)
#define I_4_0(i)  (i & 0x1F) // For IMM5 (AND w/ 31)
#define I_5_0(i)  (i & 0x3F) // For IMM6 (AND w/ 63)
#define I_6_0(i)  (i & 0x7F) // FOR IMM7 (AND w/ 127)
#define I_7_0(i)  (i & 0xFF) // FOR IMM8 (AND w/ 255)
#define I_8_0(i)  (i * 0x1FF) // FOR IMM9 (AND w/ 511)
#define I_10_0(i) (i * 0x7FF) // FOR IMM11 (AND w/ 2047)

#define I_5(i)    ((i >> 5) & 0x1) // For 1-bit secondary opcodes
#define I_5_4(i)  ((i >> 4) & 0x3) // For 2-bit secondary opcodes
#define I_5_3(i)  ((i >> 3) & 0x7) // For 3-bit secondary opcodes
#define I_8_7(i)  ((i >> 7) & 0x3) // For 2-bit secondary opcodes a I[8:7]
#define I_11(i)   ((i >> 11) & 0x1) // For 1-bit secondary opcodes in I[11]

#define REVERSE(x) (((x & 0x00ff) << 8) + (x >> 8))

#define D 1

int read_word(unsigned short *buffer, FILE *f) {
  int size;
  size = fread(buffer, 2, 1, f);
  *buffer = REVERSE(*buffer);
  return 1 == size;
}

void parse_instruction(unsigned short word) {
  switch (I_OP(word)) {

  case 0x0: do_br(I_8_0(word), I_11_9(word)); break;

  case 0x1:
    switch (I_5_3(word)) {
    case 0x0: do_add(I_11_9(word), I_8_6(word), I_2_0(word)); break;
    case 0x1: do_mul(I_11_9(word), I_8_6(word), I_2_0(word)); break;
    case 0x2: do_sub(I_11_9(word), I_8_6(word), I_2_0(word)); break;
    case 0x3: do_div(I_11_9(word), I_8_6(word), I_2_0(word)); break;
    default:
      if (I_5(word)) { do_addi(I_11_9(word), I_4_0(word)) }
      else printf("ARITH ERROR");
    }
    break;

  case 0x2:
    switch(I_8_7(word)) {
    case 0x0:
      printf("CMP\n");
      break;
    case 0x1:
      printf("CMPU\n");
      break;
    case 0x2:
      printf("CMPI\n");
      break;
    case 0x3:
      printf("CMPIU\n");
      break;
    }


    break;

  case 0x4:
    if (I_11(word) == 0) {
      printf("JSRR");
    } else {
      printf("JSR");
    }
    break;

  case 0x5:
    switch (I_5_3(word)) {
    case 0x0:
      printf("AND\n");
      break;
    case 0x1:
      printf("NOT\n");
      break;
    case 0x2:
      printf("OR\n");
      break;
    case 0x3:
      printf("XOR\n");
      break;
    default:
      if (I_5(word))
        printf("ANDI\n");
      else
        printf("LOGIC ERROR");
    }
    break;

  case 0x6:
    printf("LDR\n");
    break;

  case 0x7:
    printf("STR\n");
    break;

  case 0x8:
    printf("RTI\n");
    break;

  case 0x9:
    printf("CONST\n");
    break;

  case 0xA:
    switch (I_5_4(word)) {
    case 0x0:
      printf("SLL\n");
      break;
    case 0x1:
      printf("SRA\n");
      break;
    case 0x2:
      printf("SRL\n");
      break;
    case 0x3:
      printf("MOD\n");
      break;
    }
    break;

  case 0xC:
    if (I_11(word) == 0) {
      printf("JMPR\n");
    } else {
      printf("JMP\n");
    }
    break;

  case 0xD:
    printf("HICONST\n");
    break;

  case 0xF:
    printf("TRAP\n");
    break;

  default:
    printf("OPCODE ERROR");
  }

}

void parse_code(FILE *f, int addr, int n) {
  int m;
  unsigned short word;
  printf("parse_code: <%4x> <%d>\n", addr, n);

  for (m = 0; m < n; m++) {
    read_word(&word, f);
    mem[addr + m] = word;
    parse_instruction(word);
 
  }


}

void parse_data(FILE *f, int addr, int n) {}
void parse_symbol(FILE *f, int addr, int n) {}
void parse_filename(FILE *f, int addr, int n) {}
void parse_linenumber(FILE *f, int addr, int line, 
                      int file_index) {}


int main(int argc, char *argv[]) {
  // TODO: Check for arg length;

  FILE *output_file, *input_file;
  unsigned short word, addr, n, letter, mode, f;
  int size;

  // For each input file
  for (f = 2; f < argc; f++) {
    input_file = fopen(argv[2], "r");

    // Read word-by-word
    do {
      //      fread(&word, 2, 1, input_file);
      size = read_word(&word, input_file);
      if (D) printf("%4x %d\n", word, size);
      switch (word) {
      case 0xcade:
        if (D) printf("CODE\n");
        read_word(&addr, input_file);
        read_word(&n, input_file);
        parse_code(input_file, addr, n);
        break;
      case 0xdada:
        printf("CODE\n");
        break;
      case 0xc3b7:
        printf("SYMBOL\n");
        break;
      case 0xf17e:
        printf("FILE\n");
        break;
      case 0x715e:
        printf("LINE\n");
        break;
      default:
        printf("DEFAULT\n");
      }

    } while (!feof(input_file));


    fclose(input_file);
  }
  
  output_file = fopen(argv[1], "w");
  print_lc4_state(reg, mem, REG_LEN, MEM_LEN, output_file);
  return 0;

}
