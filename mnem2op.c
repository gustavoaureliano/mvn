#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_INST 18
#define OPERANDO_SIZE 64
#define PROGRAM_SIZE 1024
#define MAX_ADDRESS 1000
#define AT_PSEUDO_IDX 16
#define K_PSEUDO_IDX 17

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

static int get_inst_index(const char *inst) {
	for (int i = 0; i < NUM_INST; i++) {
		if (strcmp(mnemtable[i].mnemonico, inst) == 0) {
			return i;
		}
	}

	return -1;
}

static int get_symbol_index(const char *text, Symbol symbols[], uint16_t size) {
	for (uint16_t i = 0; i < size; i++) {
		if (strcmp(symbols[i].rotulo, text) == 0) {
			return i;
		}
	}

	return -1;
}

static int parse_number_token(const char *token, uint16_t *out_value) {
	char *endptr = NULL;
	unsigned long parsed = 0;

	if (token[0] == '/') {
		errno = 0;
		parsed = strtoul(&token[1], &endptr, 16);
	} else if (token[0] == '=') {
		errno = 0;
		parsed = strtoul(&token[1], &endptr, 10);
	} else {
		return 0;
	}

	if (errno != 0 || *endptr != '\0' || parsed > UINT16_MAX) {
		return -1;
	}

	*out_value = (uint16_t)parsed;
	return 1;
}

static int find_or_add_symbol(const char *label, Symbol symbols[], uint16_t *symbol_counter) {
	int symbol_index = get_symbol_index(label, symbols, *symbol_counter);

	if (symbol_index >= 0) {
		return symbol_index;
	}
	if (*symbol_counter >= PROGRAM_SIZE) {
		return -1;
	}

	snprintf(symbols[*symbol_counter].rotulo, OPERANDO_SIZE, "%s", label);
	symbols[*symbol_counter].address = 0;
	symbols[*symbol_counter].is_defined = 0;
	(*symbol_counter)++;

	return *symbol_counter - 1;
}

static int resolve_or_store_operand(const char *token, Symbol symbols[], uint16_t *symbol_counter,
		char *dst, size_t dst_size, int width) {
	uint16_t operand_value = 0;
	int parsed = parse_number_token(token, &operand_value);

	if (parsed < 0) {
		return -1;
	}
	if (parsed > 0) {
		if (width == 4) {
			snprintf(dst, dst_size, "%04X", operand_value);
		} else {
			snprintf(dst, dst_size, "%03X", operand_value);
		}
		return 0;
	}

	int symbol_index = find_or_add_symbol(token, symbols, symbol_counter);
	if (symbol_index < 0) {
		return -1;
	}
	if (symbols[symbol_index].is_defined > 0) {
		if (width == 4) {
			snprintf(dst, dst_size, "%04X", symbols[symbol_index].address);
		} else {
			snprintf(dst, dst_size, "%03X", symbols[symbol_index].address);
		}
	} else {
		snprintf(dst, dst_size, "%s", token);
	}

	return 0;
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

	while ((nread = getline(&line, &size, fp)) != -1 && curr_address < MAX_ADDRESS) {
		(void)nread;
		token = strtok(line, " \t\r\n");
		if (token == NULL) {
			continue;
		}
		if (token[0] == ';') {
			continue;
		}

		int inst_index = get_inst_index(token);
		if (inst_index < 0) {
			int symbol_index = find_or_add_symbol(token, symbols, &symbol_counter);
			if (symbol_index < 0) {
				fprintf(stderr, "Symbol table overflow\n");
				fclose(fp);
				free(line);
				return ERR_PARSE;
			}

			symbols[symbol_index].address = curr_address;
			symbols[symbol_index].is_defined = 1;

			token = strtok(NULL, " \t\r\n");
			if (token == NULL) {
				fprintf(stderr, "Parse error: missing mnemonic after label\n");
				fclose(fp);
				free(line);
				return ERR_PARSE;
			}

			inst_index = get_inst_index(token);
			if (inst_index < 0) {
				fprintf(stderr, "Parse error: invalid mnemonic '%s'\n", token);
				fclose(fp);
				free(line);
				return ERR_PARSE;
			}
		}

		token = strtok(NULL, " \t\r\n");
		if (token == NULL) {
			fprintf(stderr, "Parse error: missing operand\n");
			fclose(fp);
			free(line);
			return ERR_PARSE;
		}

		switch (inst_index) {
			case AT_PSEUDO_IDX: {
				uint16_t next_address = 0;
				int parsed = parse_number_token(token, &next_address);
				if (parsed < 0) {
					fprintf(stderr, "Parse error: invalid address operand '%s'\n", token);
					fclose(fp);
					free(line);
					return ERR_PARSE;
				}
				if (parsed > 0) {
					curr_address = next_address;
					break;
				}

				int symbol_index = find_or_add_symbol(token, symbols, &symbol_counter);
				if (symbol_index < 0) {
					fprintf(stderr, "Symbol table overflow\n");
					fclose(fp);
					free(line);
					return ERR_PARSE;
				}
				if (symbols[symbol_index].is_defined > 0) {
					curr_address = symbols[symbol_index].address;
				}
				break;
			}
			case K_PSEUDO_IDX:
				if (inst_counter >= PROGRAM_SIZE) {
					fprintf(stderr, "Program table overflow\n");
					fclose(fp);
					free(line);
					return ERR_PARSE;
				}
				program[inst_counter].address = curr_address;
				program[inst_counter].opcode = -1;
				if (resolve_or_store_operand(token, symbols, &symbol_counter,
							program[inst_counter].operando, OPERANDO_SIZE, 4) != 0) {
					fprintf(stderr, "Parse error: invalid operand '%s'\n", token);
					fclose(fp);
					free(line);
					return ERR_PARSE;
				}
				curr_address += 2;
				inst_counter += 1;
				break;
			default:
				if (inst_counter >= PROGRAM_SIZE) {
					fprintf(stderr, "Program table overflow\n");
					fclose(fp);
					free(line);
					return ERR_PARSE;
				}
				program[inst_counter].address = curr_address;
				program[inst_counter].opcode = inst_index;
				if (resolve_or_store_operand(token, symbols, &symbol_counter,
							program[inst_counter].operando, OPERANDO_SIZE, 3) != 0) {
					fprintf(stderr, "Parse error: invalid operand '%s'\n", token);
					fclose(fp);
					free(line);
					return ERR_PARSE;
				}
				curr_address += 2;
				inst_counter += 1;
				break;
		}
	}

	for (uint16_t i = 0; i < inst_counter; i++) {
		printf("%04X ", program[i].address);
		if (program[i].opcode < 0) {
			printf("%s\n", program[i].operando);
			continue;
		}

		int symbol_index = get_symbol_index(program[i].operando, symbols, symbol_counter);
		if (symbol_index < 0) {
			printf("%01X%s\n", program[i].opcode, program[i].operando);
			continue;
		}

		printf("%01X%03X\n", program[i].opcode, symbols[symbol_index].address);
	}

	fclose(fp);
	free(line);
	return OK;
}
