#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define NUM_INST 18
#define SIZE_OPERANDO 18

typedef struct {
	char operando[SIZE_OPERANDO];
	uint16_t address;
	uint8_t opcode;
} mvn;

typedef struct {
	char mnemonico[3];
	char opcode[2];
} mnemonico;

const mnemonico mnemtable[NUM_INST] = {
	{"JP", "0"},
	{"JZ", "1"},
	{"JN", "2"},
	{"LV", "3"},
	{"AD", "4"},
	{"SB", "5"},
	{"ML", "6"},
	{"DV", "7"},
	{"LD", "8"},
	{"MM", "9"},
	{"SC", "A"},
	{"RS", "B"},
	{"HM", "C"},
	{"GD", "D"},
	{"PD", "E"},
	{"SO", "F"},
	{"K", ""},
	{"@", ""}
};

int is_blank_line(const char *line) {
    while (*line) {
        if (!isspace(*line)) {
            return 0;
        }
        line++;
    }
    return 1;
}

int get_inst_index(const char* inst) {
	printf("get index:\n");
	for (int i = 0; i < NUM_INST; i++) {
		if (strcmp(mnemtable[i].mnemonico, inst) == 0) {
			return i;
		}
	}
	return -1;
}

int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "r");
  char *line = NULL;
  size_t size = 0;
  ssize_t nread;

  char* token;
  int inst_counter = 0;
  mvn program[1000];
  // primeiro passo
  while ((nread = getline(&line, &size, fp)) != -1 && inst_counter < 1000) {
	  token = strtok(line, " \t\n");
	  if (is_blank_line(token)) continue;
	  if (token[0] == ';') continue;
	  if (strcmp(token, "\n") == 0) continue;
	  int inst_index = get_inst_index(token);
	  printf("token: %s; index: %d\n", token, inst_index);
	  if (inst_index > 0) {
		  program[inst_counter].address = inst_counter;
		  program[inst_counter].opcode = inst_index;
		  token = strtok(NULL, " \t\r\n");
		  printf("new token: %s\n", token);
		  if (inst_index < 16) {
			  snprintf(program[inst_counter].operando, SIZE_OPERANDO, "%s", token);
			  inst_counter += 2;
			  continue;
		  }
		  if (token[0] == '/') {
			  inst_counter = strtoul(&token[1], NULL, 16);
		  }
	  }
  }
  // segundo passo
  // back to the start of the file
  fseek(fp, 0L, SEEK_SET);
  inst_counter = 0;
  while ((nread = getline(&line, &size, fp)) != -1 && inst_counter < 1000) {
	  printf("%s", line);
  }
  printf("\n");
  printf("MVN(%d):\n", inst_counter);
  // for (int i = 0; i < symboltblcount; i++) {
  //  printf("(%d) %s -> %s\n", i, symboltbl[i].symbol, symboltbl[i].address);
  //  // printf("%d: {%s, %s, %s, %s}", 2*i, mvn[i][0], mvn[i][1], mvn[i][2], mvn[i][3]);
  // }
  //
  for (int i = 0; i < inst_counter; i++) {
	  printf("%d", program[i].opcode);
  }
  fclose(fp);
  free(line);
  return 0;
}
