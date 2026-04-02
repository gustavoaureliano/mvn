#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define NUM_INST 18
#define OPERANDO_SIZE 64
#define PROGRAM_SIZE 1024

typedef struct {
	char operando[OPERANDO_SIZE];
	uint16_t address;
	int8_t opcode;
} MVN;

typedef struct {
	char mnemonico[3];
	char opcode[2];
} Mnemonico;

typedef struct {
	uint16_t address;
	uint8_t is_defined;
	char rotulo[OPERANDO_SIZE];
} Symbol;

const Mnemonico mnemtable[NUM_INST] = {
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
	{"@" , "" },
	{"K" , "" }
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

int get_inst_index(const char *inst) {
	// printf("get index:\n");
	for (int i = 0; i < NUM_INST; i++) {
		if (strcmp(mnemtable[i].mnemonico, inst) == 0) {
			return i;
		}
	}
	return -1;
}

int get_symbol_index(const char *text, Symbol symbols[], int size) {
	// printf("get index symbol:\n");
	for (int i = 0; i < size; i++) {
		// printf("%s == %s ?\n", symbol, symbols[i].rotulo);
		if (strcmp(symbols[i].rotulo, text) == 0) {
			// printf("yes!\n");
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

	char *token;
	uint16_t curr_address = 0;
	uint16_t inst_counter = 0;
	uint16_t symbol_counter = 0;
	MVN program[PROGRAM_SIZE] = {0};
	Symbol symbols[PROGRAM_SIZE] = {0};

	// primeiro passo
	while ((nread = getline(&line, &size, fp)) != -1 && curr_address < 1000) {
		token = strtok(line, " \t\n");
		if (is_blank_line(token))
			continue;
		if (token[0] == ';')
			continue;
		if (strcmp(token, "\n") == 0)
			continue;
		int inst_index = get_inst_index(token);
		int symbol_index = -1;
		uint16_t operando_addr = 0;
		char *symbol = token;
		// printf("token: %s; index: %d\n", symbol, inst_index);
		token = strtok(NULL, " \t\r\n");
		// printf("new token: %s\n", token);
		if (inst_index < 0) {
			symbol_index = get_symbol_index(symbol, symbols, symbol_counter);
			inst_index = get_inst_index(token);
			if (symbol_index >= 0) {
				// printf("updating symbol address: %s -> %03X\n", symbol, curr_address);
				symbols[symbol_index].address = curr_address;
				symbols[symbol_index].is_defined = 1;

			} else {
				snprintf(symbols[symbol_counter].rotulo, OPERANDO_SIZE, "%s", symbol);
				symbols[symbol_counter].address = curr_address; // shouldn't be needed
				symbols[symbol_counter].is_defined = 1;
				symbol_counter++;
			}
			symbol = token;
			token = strtok(NULL, " \t\r\n");
		}
		switch (inst_index) {
			case 16:
				switch (token[0]) {
					case '/':
						// printf("hex: %s\n", token);
						curr_address = strtoul(&token[1], NULL, 16);
						break;
					case '=':
						// printf("decimal: %s\n", token);
						curr_address = strtoul(&token[1], NULL, 10);
						break;
					default:
						// label
						symbol_index = get_symbol_index(token, symbols, symbol_counter);
						if (symbol_index >= 0) {
							if (symbols[symbol_index].is_defined > 0)
								snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%03X", symbols[symbol_index].address);
							else
								snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%s", token);
							break;
						}
						snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%s", token);
						snprintf(symbols[symbol_counter].rotulo, OPERANDO_SIZE, "%s", token);
						symbol_counter++;
						break;
				}
				break;
			case 17:
				program[inst_counter].address = curr_address;
				program[inst_counter].opcode = -1;
				operando_addr = 0;
				switch (token[0]) {
					case '/':
						// printf("hex: %s\n", token);
						operando_addr = strtoul(&token[1], NULL, 16);
						snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%04X", operando_addr);
						break;
					case '=':
						// printf("decimal: %s\n", token);
						operando_addr = strtoul(&token[1], NULL, 10);
						snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%04X", operando_addr);
						break;
					default:
						// label
						symbol_index = get_symbol_index(token, symbols, symbol_counter);
						if (symbol_index >= 0) {
							if (symbols[symbol_index].is_defined > 0)
								snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%04X", symbols[symbol_index].address);
							else
								snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%s", token);
							break;
						}
						snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%s", token);
						snprintf(symbols[symbol_counter].rotulo, OPERANDO_SIZE, "%s", token);
						symbol_counter++;
						break;
				}
				curr_address += 2;
				inst_counter += 1;
				break;
			default:
				program[inst_counter].address = curr_address;
				program[inst_counter].opcode = inst_index;
				switch (token[0]) {
					case '/':
						// printf("hex: %s\n", token);
						operando_addr = strtoul(&token[1], NULL, 16);
						snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%03X", operando_addr);
						break;
					case '=':
						// printf("decimal: %s\n", token);
						operando_addr = strtoul(&token[1], NULL, 10);
						snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%03X", operando_addr);
						break;
					default:
						// label
						symbol_index = get_symbol_index(token, symbols, symbol_counter);
						if (symbol_index >= 0) {
							if (symbols[symbol_index].is_defined > 0)
								snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%03X", symbols[symbol_index].address);
							else
								snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%s", token);
							break;
						}
						snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%s", token);
						snprintf(symbols[symbol_counter].rotulo, OPERANDO_SIZE, "%s", token);
						symbol_counter++;
						break;
				}
				curr_address += 2;
				inst_counter += 1;
				break;
		}
	}
	// segundo passo
	// back to the start of the file
	fseek(fp, 0L, SEEK_SET);
	// while ((nread = getline(&line, &size, fp)) != -1 && curr_address < 1000) {
	// 	printf("%s", line);
	// }
	// printf("\n");
	// printf("MVN(%d):\n", inst_counter);
	// for (int i = 0; i < symboltblcount; i++) {
	//  printf("(%d) %s -> %s\n", i, symboltbl[i].symbol, symboltbl[i].address);
	//  // printf("%d: {%s, %s, %s, %s}", 2*i, mvn[i][0], mvn[i][1], mvn[i][2],
	//  mvn[i][3])
	// }
	//
	// for (int i = 0; i < inst_counter; i++) {
	// 	printf("%04X %d %s\n", program[i].address, program[i].opcode, program[i].operando);
	// }
	// printf("memory:\n");
	for (int i = 0; i < inst_counter; i++) {
		printf("%04X ", program[i].address);
		if (program[i].opcode < 0) {
			printf("%s\n", program[i].operando);
			continue;
		}
		int idx = get_symbol_index(program[i].operando, symbols, symbol_counter);
		if (idx < 0) {
			printf("%01X%s\n", program[i].opcode, program[i].operando);
			continue;
		}
		printf("%01X%03X\n", program[i].opcode, symbols[idx].address);
	}
	// printf("symbols(%d):\n", symbol_counter);
	// for (int i = 0; i < symbol_counter; i++) {
	// 	printf("%04X %s\n", symbols[i].address, symbols[i].rotulo);
	// }
	fclose(fp);
	free(line);
	return 0;
}
