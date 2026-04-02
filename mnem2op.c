#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef enum {
	OK = 0,
	ERR_USAGE = 1,
	ERR_IO = 2,
	ERR_PARSE = 3
} ExitCode;

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
	{"@", ""},
	{"K", ""}
};

int get_inst_index(const char *inst) {
	for (int i = 0; i < NUM_INST; i++) {
		if (strcmp(mnemtable[i].mnemonico, inst) == 0) {
			return i;
		}
	}

	return -1;
}

int get_symbol_index(const char *text, Symbol symbols[], int size) {
	for (int i = 0; i < size; i++) {
		if (strcmp(symbols[i].rotulo, text) == 0) {
			return i;
		}
	}

	return -1;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s arquivo.asm\n", argv[0]);
		return ERR_USAGE;
	}

	FILE *fp = fopen(argv[1], "r");
	if (fp == NULL) {
		perror("fopen");
		return ERR_IO;
	}

	char *line = NULL;
	size_t size = 0;
	ssize_t nread;

	char *token;
	uint16_t curr_address = 0;
	uint16_t inst_counter = 0;
	uint16_t symbol_counter = 0;
	MVN program[PROGRAM_SIZE] = {0};
	Symbol symbols[PROGRAM_SIZE] = {0};

	while ((nread = getline(&line, &size, fp)) != -1 && curr_address < 1000) {
		(void)nread;
		token = strtok(line, " \t\r\n");
		if (token == NULL) {
			continue;
		}
		if (token[0] == ';') {
			continue;
		}

		int inst_index = get_inst_index(token);
		int symbol_index = -1;
		uint16_t operando_addr = 0;
		char *symbol = token;

		token = strtok(NULL, " \t\r\n");
		if (inst_index < 0) {
			if (token == NULL) {
				fprintf(stderr, "Parse error: missing mnemonic after label\n");
				fclose(fp);
				free(line);
				return ERR_PARSE;
			}

			symbol_index = get_symbol_index(symbol, symbols, symbol_counter);
			inst_index = get_inst_index(token);
			if (inst_index < 0) {
				fprintf(stderr, "Parse error: invalid mnemonic '%s'\n", token);
				fclose(fp);
				free(line);
				return ERR_PARSE;
			}
			if (symbol_index >= 0) {
				symbols[symbol_index].address = curr_address;
				symbols[symbol_index].is_defined = 1;
			} else {
				snprintf(symbols[symbol_counter].rotulo, OPERANDO_SIZE, "%s", symbol);
				symbols[symbol_counter].address = curr_address;
				symbols[symbol_counter].is_defined = 1;
				symbol_counter++;
			}
			token = strtok(NULL, " \t\r\n");
		}

		if (token == NULL) {
			fprintf(stderr, "Parse error: missing operand\n");
			fclose(fp);
			free(line);
			return ERR_PARSE;
		}

		switch (inst_index) {
			case 16:
				switch (token[0]) {
					case '/':
						curr_address = strtoul(&token[1], NULL, 16);
						break;
					case '=':
						curr_address = strtoul(&token[1], NULL, 10);
						break;
					default:
						symbol_index = get_symbol_index(token, symbols, symbol_counter);
						if (symbol_index >= 0) {
							if (symbols[symbol_index].is_defined > 0) {
								snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%03X", symbols[symbol_index].address);
							} else {
								snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%s", token);
							}
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
						operando_addr = strtoul(&token[1], NULL, 16);
						snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%04X", operando_addr);
						break;
					case '=':
						operando_addr = strtoul(&token[1], NULL, 10);
						snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%04X", operando_addr);
						break;
					default:
						symbol_index = get_symbol_index(token, symbols, symbol_counter);
						if (symbol_index >= 0) {
							if (symbols[symbol_index].is_defined > 0) {
								snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%04X", symbols[symbol_index].address);
							} else {
								snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%s", token);
							}
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
						operando_addr = strtoul(&token[1], NULL, 16);
						snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%03X", operando_addr);
						break;
					case '=':
						operando_addr = strtoul(&token[1], NULL, 10);
						snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%03X", operando_addr);
						break;
					default:
						symbol_index = get_symbol_index(token, symbols, symbol_counter);
						if (symbol_index >= 0) {
							if (symbols[symbol_index].is_defined > 0) {
								snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%03X", symbols[symbol_index].address);
							} else {
								snprintf(program[inst_counter].operando, OPERANDO_SIZE, "%s", token);
							}
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

	fclose(fp);
	free(line);
	return OK;
}
