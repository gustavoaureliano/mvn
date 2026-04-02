#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_SIZE 1024
#define LINE_SIZE 256
#define LABEL_SIZE 16
#define OPERAND_SIZE 64
#define DATA_START 0x0300
#define DATA_END 0x03FF
#define CODE_END 0x02FF
#define JP_OPCODE 0x0
#define JZ_OPCODE 0x1
#define JN_OPCODE 0x2
#define SC_OPCODE 0xA

typedef struct {
	uint16_t address;
	uint16_t word;
} MvnLine;

typedef struct {
	uint16_t address;
	uint8_t kind;
	char name[LABEL_SIZE];
} Label;

typedef enum {
	OK = 0,
	ERR_USAGE = 1,
	ERR_IO = 2,
	ERR_PARSE = 3
} ExitCode;

typedef enum {
	LABEL_JUMP = 0,
	LABEL_SUB = 1,
	LABEL_ROT = 2
} LabelKind;

static const char *opcodes[16] = {
	"JP", "JZ", "JN", "LV", "AD", "SB", "ML", "DV",
	"LD", "MM", "SC", "RS", "HM", "GD", "PD", "SO"
};

static int is_data_address(uint16_t address) {
	return address >= DATA_START && address <= DATA_END;
}

static int is_code_address(uint16_t address) {
	return address <= CODE_END;
}

static int find_label_by_address(Label labels[], uint16_t label_count, uint16_t address) {
	for (uint16_t i = 0; i < label_count; i++) {
		if (labels[i].address == address) {
			return i;
		}
	}

	return -1;
}

static int add_label(Label labels[], uint16_t *label_count, uint16_t address,
		uint8_t kind, uint16_t *jump_counter, uint16_t *sub_counter, uint16_t *rot_counter) {
	int index = find_label_by_address(labels, *label_count, address);
	if (index >= 0) {
		return index;
	}
	if (*label_count >= PROGRAM_SIZE) {
		return -1;
	}

	labels[*label_count].address = address;
	labels[*label_count].kind = kind;

	if (kind == LABEL_JUMP) {
		snprintf(labels[*label_count].name, LABEL_SIZE, "JUMP%02u", *jump_counter);
		(*jump_counter)++;
	} else if (kind == LABEL_SUB) {
		snprintf(labels[*label_count].name, LABEL_SIZE, "SUB%02u", *sub_counter);
		(*sub_counter)++;
	} else {
		snprintf(labels[*label_count].name, LABEL_SIZE, "ROT%02u", *rot_counter);
		(*rot_counter)++;
	}

	(*label_count)++;
	return *label_count - 1;
}

static int parse_mvn_line(const char *line, uint16_t *address, uint16_t *word) {
	char addr_token[8] = {0};
	char word_token[8] = {0};
	char *endptr = NULL;
	unsigned long parsed_addr = 0;
	unsigned long parsed_word = 0;

	if (sscanf(line, " %7s %7s", addr_token, word_token) != 2) {
		return 0;
	}

	errno = 0;
	parsed_addr = strtoul(addr_token, &endptr, 16);
	if (errno != 0 || *endptr != '\0' || parsed_addr > UINT16_MAX) {
		return -1;
	}

	errno = 0;
	parsed_word = strtoul(word_token, &endptr, 16);
	if (errno != 0 || *endptr != '\0' || parsed_word > UINT16_MAX) {
		return -1;
	}

	*address = (uint16_t)parsed_addr;
	*word = (uint16_t)parsed_word;
	return 1;
}

static int should_emit_org(const MvnLine lines[], uint16_t index) {
	if (index == 0) {
		return 1;
	}

	uint16_t curr = lines[index].address;
	uint16_t prev = lines[index - 1].address;
	if (curr != (uint16_t)(prev + 2)) {
		return 1;
	}
	if (is_data_address(curr) != is_data_address(prev)) {
		return 1;
	}

	return 0;
}

static void format_operand(char *buffer, size_t buffer_size, uint16_t operand,
		Label labels[], uint16_t label_count) {
	int index = find_label_by_address(labels, label_count, operand);

	if (index >= 0) {
		snprintf(buffer, buffer_size, "%s", labels[index].name);
		return;
	}

	snprintf(buffer, buffer_size, "/%03X", operand);
}

static void discover_labels(const MvnLine lines[], uint16_t line_count, Label labels[],
		uint16_t *label_count) {
	uint16_t jump_counter = 0;
	uint16_t sub_counter = 0;
	uint16_t rot_counter = 0;

	for (uint16_t i = 0; i < line_count; i++) {
		if (!is_code_address(lines[i].address)) {
			continue;
		}

		uint8_t opcode = (uint8_t)((lines[i].word >> 12) & 0xF);
		uint16_t operand = lines[i].word & 0x0FFF;

		if ((opcode == JP_OPCODE || opcode == JZ_OPCODE || opcode == JN_OPCODE) &&
				is_code_address(operand)) {
			if (add_label(labels, label_count, operand, LABEL_JUMP,
						&jump_counter, &sub_counter, &rot_counter) < 0) {
				return;
			}
			continue;
		}

		if (opcode == SC_OPCODE && is_code_address(operand)) {
			if (add_label(labels, label_count, operand, LABEL_SUB,
						&jump_counter, &sub_counter, &rot_counter) < 0) {
				return;
			}
			continue;
		}

		if (is_data_address(operand)) {
			if (add_label(labels, label_count, operand, LABEL_ROT,
						&jump_counter, &sub_counter, &rot_counter) < 0) {
				return;
			}
		}
	}
}

static void emit_asm(const MvnLine lines[], uint16_t line_count, Label labels[], uint16_t label_count) {
	for (uint16_t i = 0; i < line_count; i++) {
		if (should_emit_org(lines, i)) {
			if (i > 0) {
				printf("\n");
			}
			printf("@ /%04X\n", lines[i].address);
		}

		int line_label = find_label_by_address(labels, label_count, lines[i].address);
		if (is_data_address(lines[i].address)) {
			if (line_label >= 0) {
				printf("%-8sK /%04X\n", labels[line_label].name, lines[i].word);
			} else {
				printf("\tK /%04X\n", lines[i].word);
			}
			continue;
		}

		uint8_t opcode = (uint8_t)((lines[i].word >> 12) & 0xF);
		uint16_t operand = lines[i].word & 0x0FFF;
		char operand_text[OPERAND_SIZE] = {0};

		format_operand(operand_text, sizeof(operand_text), operand, labels, label_count);

		if (line_label >= 0) {
			printf("%-8s%s %s\n", labels[line_label].name, opcodes[opcode], operand_text);
		} else {
			printf("\t%s %s\n", opcodes[opcode], operand_text);
		}
	}
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s arquivo.mvn\n", argv[0]);
		return ERR_USAGE;
	}

	FILE *fp = fopen(argv[1], "r");
	if (fp == NULL) {
		perror("fopen");
		return ERR_IO;
	}

	MvnLine lines[PROGRAM_SIZE] = {0};
	Label labels[PROGRAM_SIZE] = {0};
	uint16_t line_count = 0;
	uint16_t label_count = 0;
	char line[LINE_SIZE] = {0};

	while (fgets(line, sizeof(line), fp) != NULL) {
		uint16_t address = 0;
		uint16_t word = 0;
		int parsed = parse_mvn_line(line, &address, &word);
		if (parsed == 0) {
			continue;
		}
		if (parsed < 0) {
			fprintf(stderr, "Parse error: invalid .mvn line '%s'", line);
			fclose(fp);
			return ERR_PARSE;
		}
		if (line_count >= PROGRAM_SIZE) {
			fprintf(stderr, "Parse error: input too large\n");
			fclose(fp);
			return ERR_PARSE;
		}

		lines[line_count].address = address;
		lines[line_count].word = word;
		line_count++;
	}

	if (ferror(fp)) {
		fprintf(stderr, "I/O error while reading input\n");
		fclose(fp);
		return ERR_IO;
	}

	fclose(fp);

	discover_labels(lines, line_count, labels, &label_count);
	emit_asm(lines, line_count, labels, label_count);

	return OK;
}
