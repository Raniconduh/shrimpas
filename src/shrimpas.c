#include <fcntl.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "parse.h"
#include "opcode.h"
#include "hashmap.h"


typedef struct assembled_i {
	struct assembled_i * next;
	insword bin;
} assembled_i;


hashmap * collect_labels(FILE *);
bool islabel(char *);
void print_ascii(FILE *, uint16_t);
void destroy_list(assembled_i *);
void shrimp_quit(int);
void shrimp_help();

 
static uint16_t origin = 0x0;
static FILE * inputf = NULL;
static hashmap * opmap = NULL;
static hashmap * regmap = NULL;
static hashmap * dirmap = NULL;
static hashmap * labels = NULL;
static assembled_i * assembly_list = NULL;
static char * line = NULL;
static bool ascii = false;


int main(int argc, char ** argv) {
	if (argc > 1 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) {
		shrimp_help();
		return 0;
	}

	if (argc < 2) {
		fputs("Invalid argument count\n", stderr);
		return 1;
	}

	if (argc > 2 && !strcmp(argv[2], "--ascii")) {
		ascii = true;
	}

	inputf = fopen(argv[1], "r");
	if (!inputf) {
		fputs("Invalid input file\n", stderr);
		return 1;
	}

	opmap = generate_opmap();
	regmap = generate_regmap();
	dirmap = generate_dirmap();
	labels = collect_labels(inputf);
	
	if (!labels) {
		fclose(inputf);
		destroy_hashmap(opmap);
		destroy_hashmap(regmap);
		return 1;
	}

	assembly_list = NULL;

	ssize_t line_number = 1;
	ssize_t loffset = 0;
	line = NULL;
	size_t len = 0;

	while (getline(&line, &len, inputf) != -1) {
		line_content c = parse(line);
		switch (c.type) {
			case CONTENT_INSTRUCTION:;
				opinfo * opcode_info = (opinfo*)hashmap_get(opmap, c.opcode);
				/* determine if valid opcode */
				if (!opcode_info && c.opcode[0]) {
					fprintf(stderr, "l%ld: Unknown opcode '%s'\n",
					        line_number, c.opcode);
					shrimp_quit(1);
				} else if (!opcode_info) {
					fprintf(stderr, "l%ld: Unknown opcode '.%s'\n",
					        line_number, c.flags);
					shrimp_quit(1);
				}
				/* check number of arguments */
				if ((opcode_info->n_args == 0 && c.arg1)
				 || (opcode_info->n_args == 1 && c.arg2)
				 || (opcode_info->n_args == 2 && !c.arg2)) {
					fprintf(stderr, "l%ld: Invalid number of arguments for '%s'\n",
					        line_number, c.opcode);
					shrimp_quit(1);
				}
				/* validate flags */
				if (c.flags) for (char * f = c.flags; *f; f++) {
					if (!strchr(opcode_info->flags, *f)) {
						fprintf(stderr, "l%ld: Invalid flag '%c' for '%s'\n",
						        line_number, *f, c.opcode);
						shrimp_quit(1);
					}
				}
				/* validate arguments */
				enum argtype arg1type = getargtype(regmap, c.arg1);
				enum argtype arg2type = getargtype(regmap, c.arg2);
				if (c.arg1 && !(opcode_info->arg1type & arg1type)) {
					fprintf(stderr, "l%ld: Invalid type of first argument for '%s'\n",
					        line_number, c.opcode);
					shrimp_quit(1);
				}
				if (c.arg2 && !(opcode_info->arg2type & arg2type)) {
					fprintf(stderr, "l%ld: Invalid type of second argument for '%s'\n",
					        line_number, c.opcode);
					shrimp_quit(1);
				}

				/* validate labels */
				if (arg1type & ARG_LBL && !hashmap_get(labels, c.arg1)) {
					fprintf(stderr, "l%ld: Unknown label '%s' for '%s'\n",
							line_number, c.arg1, c.opcode);
					shrimp_quit(1);
				}
				if (arg2type & ARG_LBL && !hashmap_get(labels, c.arg2)) {
					fprintf(stderr, "l%ld: Unknown label '%s' for '%s'\n",
							line_number, c.arg2, c.opcode);
					shrimp_quit(1);
				}
				
				/* fix label offsets */
				if (((c.arg1 && (arg1type & ARG_IMM)) || (c.arg2
						&& (arg2type & ARG_IMM))) && !opcode_info->inplace) {
					hashmap_walk_state s = {0};
					while (hashmap_walk(labels, &s)) {
						if ((uintptr_t)s.val - 1 > loffset) {
							hashmap_insert(labels, s.key, (void*)((uintptr_t)s.val + 1));
						}
					}

					loffset++;
				}

				/* assemble instruction into binary */
				insword insbin;
				get_insbin(opmap, regmap, c, &insbin);
				if (insbin.bad) {
					fprintf(stderr, "l%ld: Bad argument type for '%s'\n",
					        line_number, c.opcode);
					shrimp_quit(1);
					return 1;
				}

				/* save this assembled instruction
				 * label offsets will be finalized at the end of assembly */
				assembled_i * a = malloc(sizeof(assembled_i));
				a->next = NULL;
				a->bin = insbin;
				if (!assembly_list) {
					assembly_list = a;
				} else {
					assembled_i * i;
					for (i = assembly_list; i->next; i = i->next)
						;
					i->next = a;
				}

				loffset++;
				break;
			case CONTENT_DIRECTIVE:;
				dirinfo * directive_info = hashmap_get(dirmap, c.label);
				/* validate directive */
				if (!directive_info) {
					fprintf(stderr, "l%ld: Unknown directive '%s'\n",
					        line_number, c.label);
					shrimp_quit(1);
				}
				/* validate arguments */
				if (!c.arg1 && directive_info->n_args == 1) {
					fprintf(stderr, "l%ld: Invalid argument count for directive '%s'\n",
					        line_number, c.label);
					shrimp_quit(1);
				}
				enum argtype dargtype = getargtype(regmap, c.arg1);
				if (!(dargtype & directive_info->argtype)) {
					fprintf(stderr, "l%ld: Invalid argument type for directive '%s'\n",
					        line_number, c.label);
					shrimp_quit(1);
				}

				uint16_t dnum = strtol(c.arg1, NULL, 0);
				switch (directive_info->dircode) {
				case DIR_ORIGIN:
					origin = dnum;
					break;
				case DIR_WORD:;
					insword winsbin;
					winsbin.bad = false;
					winsbin.imm = false;
					winsbin.binword.opcode = (dnum >> 11) & BIN5(1,1,1,1,1);
					winsbin.binword.reg1   = (dnum >> 7) & BIN4(1,1,1,1);
					winsbin.binword.flag1  = (dnum >> 6) & 1;
					winsbin.binword.flag2  = (dnum >> 5) & 1;
					winsbin.binword.flag3  = (dnum >> 4) & 1;
					winsbin.binword.reg2   = dnum & BIN4(1,1,1,1);

					/* fix label offsets */
					hashmap_walk_state ws = {0};
					while (hashmap_walk(labels, &ws)) {
						if ((uintptr_t)ws.val - 1 >= loffset) {
							hashmap_insert(labels, ws.key, (void*)((uintptr_t)ws.val + 1));
						}
					}

					/* place word in assembly list */
					assembled_i * wa = malloc(sizeof(assembled_i));
					wa->next = NULL;
					wa->bin = winsbin;
					if (!assembly_list) {
						assembly_list = wa;
					} else {
						assembled_i * wi;
						for (wi = assembly_list; wi->next; wi = wi->next)
							;
						wi->next = wa;
					}

					loffset++;
					break;
				case DIR_ZWORDS:
					if (dnum <= 0) {
						fprintf(stderr, "l%ld: Invalid argument for directive '%s'\n",
						        line_number, c.label);
						shrimp_quit(1);
					}

					insword zinsbin = {0};

					/* fix label offsets */
					hashmap_walk_state zs = {0};
					while (hashmap_walk(labels, &zs)) {
						if ((uintptr_t)zs.val - 1 >= loffset) {
							hashmap_insert(labels, zs.key, (void*)((uintptr_t)zs.val + dnum));
						}
					}

					/* place wmpty words in assembly list */
					assembled_i * zlast = assembly_list;
					if (zlast) while (zlast->next) zlast = zlast->next;
					for (uint16_t i = 0; i < dnum; i++) {
						assembled_i * za = malloc(sizeof(assembled_i));
						za->next = NULL;
						za->bin = zinsbin;
						if (!assembly_list) {
							assembly_list = za;
						} else {
							zlast->next = za;
							zlast = zlast->next;
						}
					}

					loffset += dnum;
					break;
				}
				break;
			case CONTENT_LABEL:
			case CONTENT_NOTHING:
			default:
				break;
		}
		free(line);
		line = NULL;
		line_number++;
	}

	/* assembly has been completed, now write to file */
	FILE * outf = fopen("out.bin", "wb");
	for (assembled_i * i = assembly_list; i; i = i->next) {
		uint16_t bin = OPC_BIN(i->bin);
		if (ascii) {
			print_ascii(outf, bin);
		} else {
			fputc(((uint8_t*)(&bin))[1], outf);
			fputc(((uint8_t*)(&bin))[0], outf);
		}
		if (!i->bin.imm) continue;
		uint16_t imm;
		/* output integer value */
		if (isint(i->bin.label)) {
			imm = (uint16_t)strtol(i->bin.label, NULL, 0);
		/* output label offset */
		} else {
			imm = (uintptr_t)hashmap_get(labels, i->bin.label) - 1 + origin;
		}
		if (ascii) {
			print_ascii(outf, imm);
		} else {
			fputc(((uint8_t*)(&imm))[1], outf);
			fputc(((uint8_t*)(&imm))[0], outf);
		}
	}
	fclose(outf);

	shrimp_quit(0);
}


hashmap * collect_labels(FILE * inputf) {
	hashmap * labels = new_hashmap(NULL);

	ssize_t line_number = 1;
	ssize_t loffset = 0;
	char * line = NULL;
	size_t len = 0;

	while (getline(&line, &len, inputf) != -1) {
		line_content c = parse(line);

		switch (c.type) {
			case CONTENT_LABEL:
				if (hashmap_get(labels, c.label) != NULL) {
					fprintf(stderr, "l%ld: Duplicate label '%s'\n",
					        line_number, c.label);
					free(line);
					destroy_hashmap(labels);
					return NULL;
				}
				hashmap_insert(labels, c.label, (void*)(loffset + 1));
				break;
			case CONTENT_INVALID_LABEL_NAME:
				fprintf(stderr, "l%ld: Invalid label name '%s'\n",
				        line_number, c.label);
				free(line);
				destroy_hashmap(labels);
				return NULL;
			case CONTENT_INVALID_LABEL_LENGTH:
				fprintf(stderr, "l%ld: Label '%s' name too long\n",
				        line_number, c.label);
				free(line);
				destroy_hashmap(labels);
				return NULL;
			case CONTENT_INVALID_ARG1:
				fprintf(stderr, "l%ld: Invalid first argument for '%s'\n",
				        line_number, c.opcode);
				free(line);
				destroy_hashmap(labels);
				return NULL;
			case CONTENT_INVALID_ARG2:
				fprintf(stderr, "l%ld: Invalid second argument for '%s'\n",
				        line_number, c.opcode);
				free(line);
				destroy_hashmap(labels);
				return NULL;
			case CONTENT_NOTHING:
			case CONTENT_DIRECTIVE:
				break;
			default:
				loffset++;
				break;
		}
		
		free(line);
		line = NULL;
		line_number++;
	}

	free(line);
	rewind(inputf);
	return labels;
}


void print_ascii(FILE * f, uint16_t word) {
	for (int i = 0; i < 16; i++) {
		if (word & (1 << 15)) fputc('1', f);
		else fputc('0', f);
		word <<= 1;
	}
	fputc('\n', f);
}


void destroy_list(assembled_i * l) {
	assembled_i * i = l;
	while (i) {
		assembled_i * next = i->next;	
		free(i);
		i = next;
	}
}


void shrimp_quit(int ret) {
	free(line);
	fclose(inputf);
	destroy_hashmap(labels);
	destroy_hashmap(opmap);
	destroy_hashmap(regmap);
	destroy_hashmap(dirmap);
	destroy_list(assembly_list);
	exit(ret);
}

void shrimp_help(void) {
	puts(
		"Usage: shrimpas <FILE> [--ascii]\n"
		"Compile a SHRIMP assembly file into binary\n"
		"Outputs file `out.bin'\n"
		"\n"
		"Options:\n"
		"\t--ascii\tOutput is in ASCII instead of a binary"
	);
}
