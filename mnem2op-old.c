#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	char address[100];
	int operando[100];
} mvn;

typedef struct {
	char mnemonico[100];
	char opcode[100];
} mnemonico;

typedef struct {
	char symbol[100];
	char address[100];
} symbol;

const mnemonico mnemtable[16] = {
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



int is_mnem(const char* token, unsigned long size) {
	if (size > 1) {
		int i;
		for (i = 0; i < sizeof(mnemtable); i++) {
			if (strcmp(mnemtable[i].mnemonico, token) == 0)
				return 0;
		}
		return 1;
	}
	return 0;
}

int is_pseudo(const char* token, unsigned int size) {
	if (size > 1)
		return 1;
	for (int i = 0; i < sizeof(pseudo); i++) {
		if (strcmp(pseudo[i], token) == 0)
			return 0;
	}
	return 1;
}

int is_label(const char* token) {
	unsigned long size = strlen(token);
	if (size > 2)
		return 1;
	if (size > 1) {
		return is_mnem(token, size);
	}
	return is_pseudo(token, size);
}

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
	for (int i = 0; i < sizeof(mnemtable); i++) {
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

  mvn mvn[1000];
  symbol symboltbl[1000];

  char* token;
  int inst_counter = 0;
  int symboltblcount = 0;
  // primeiro passo
  while ((nread = getline(&line, &size, fp)) != -1 && inst_counter < 1000) {
	  token = strtok(line, " \t\n");
	  if (token[0] == ';' || is_blank_line(token)) continue;
	  if (strcmp(token, "\n") == 0) continue;
	  printf("token: %s\n", token);
	  if (is_label(token)) {
		  printf("(%d) label: %s\n", symboltblcount, token);
		  snprintf(symboltbl[symboltblcount].symbol, 100, "%s", token);
		  snprintf(symboltbl[symboltblcount].address, 100, "%03d", inst_counter);
		  symboltblcount++;
	  }
	  if (token[0] == pseudo[1][0]) {
		  token = strtok(NULL, " \t\n");
		  if (token[0] == '/') {
			  inst_counter = strtoul(&token[1], NULL, 10);
			  printf("changed symboltblcount to: %d", inst_counter);
		  }
	  }
	  inst_counter += 2;
  }
  // segundo passo
  // back to the start of the file
  fseek(fp, 0L, SEEK_SET);
  inst_counter = 0;
  while ((nread = getline(&line, &size, fp)) != -1 && inst_counter < 1000) {
	  printf("%s", line);
	  // token = strtok(line, " \t\n");
	  // if (token[0] == ';' || is_blank_line(token)) continue;
	  // if (strcmp(token, "\n") == 0) continue;
	  // if (is_label(token)) {
	  //  snprintf(symboltbl[linecount][0], 100, "%s", token);
	  //  symboltblcount++;
	  // }
	  // linecount++;
  }
  printf("\n");
  printf("MVN(%d):\n", inst_counter);
  for (int i = 0; i < symboltblcount; i++) {
	  printf("(%d) %s -> %s\n", i, symboltbl[i].symbol, symboltbl[i].address);
	  // printf("%d: {%s, %s, %s, %s}", 2*i, mvn[i][0], mvn[i][1], mvn[i][2], mvn[i][3]);
  }

  fclose(fp);
  free(line);
  return 0;
}
