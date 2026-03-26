#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_INST 16

typedef struct {
	char address[100];
	int operando[100];
} mvn;

typedef struct {
	char mnemonico[3];
	char opcode[2];
} mnemonico;

typedef struct {
	char symbol[100];
	char address[100];
} symbol;

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
	{"SO", "F"}
};

const char pseudo[2][2] = {
	"K",
	"@"
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
	for (int i = 0; i < sizeof(mnemtable); i++) {
		printf("%d\n", i);
		// if (strcmp(mnemtable[i].mnemonico, inst) == 0) {
		// 	return i;
		// }
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
  // primeiro passo
  while ((nread = getline(&line, &size, fp)) != -1 && inst_counter < 1000) {
	  token = strtok(line, " \t\n");
	  if (token[0] == ';' || is_blank_line(token)) continue;
	  if (strcmp(token, "\n") == 0) continue;
	  printf("token: %s; index: %d\n", token, get_inst_index(token));
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
  fclose(fp);
  free(line);
  return 0;
}
