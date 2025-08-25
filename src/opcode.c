#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "parse.h"
#include "opcode.h"
#include "hashmap.h"

#include <stdio.h>

opinfo op_ADD_info   = {OP_ADD,   2, "B",    ARG_REG, ARG_REG | ARG_IMM};
opinfo op_SUB_info   = {OP_SUB,   2, "B",    ARG_REG, ARG_REG | ARG_IMM};
opinfo op_MUL_info   = {OP_MUL,   2, "B",    ARG_REG, ARG_REG | ARG_IMM};
opinfo op_DIV_info   = {OP_DIV,   2, "B",    ARG_REG, ARG_REG | ARG_IMM};
opinfo op_SHA_info   = {OP_SHA,   2, "",     ARG_REG, ARG_REG | ARG_IMM};
opinfo op_MOD_info   = {OP_MOD,   2, "B",    ARG_REG, ARG_REG | ARG_IMM};
opinfo op_AND_info   = {OP_AND,   2, "B",    ARG_REG, ARG_REG | ARG_IMM};
opinfo op_OR_info    = {OP_OR,    2, "B",    ARG_REG, ARG_REG | ARG_IMM};
opinfo op_XOR_info   = {OP_XOR,   2, "B",    ARG_REG, ARG_REG | ARG_IMM};
opinfo op_NOT_info   = {OP_NOT,   2, "B",    ARG_REG, ARG_REG | ARG_IMM};
opinfo op_SHL_info   = {OP_SHL,   2, "",     ARG_REG, ARG_REG | ARG_IMM};
opinfo op_ROL_info   = {OP_ROL,   2, "",     ARG_REG, ARG_REG | ARG_IMM};
opinfo op_SHR_info   = {OP_SHR,   2, "",     ARG_REG, ARG_REG | ARG_IMM};
opinfo op_ROR_info   = {OP_ROR,   2, "",     ARG_REG, ARG_REG | ARG_IMM};
opinfo op_JMP_info   = {OP_JMP,   1, "NZPCO",ARG_REG | ARG_IMM, ARG_NON};
opinfo op_CALL_info  = {OP_CALL,  1, "NZPCO",ARG_REG | ARG_IMM, ARG_NON};
opinfo op_RET_info   = {OP_RET,   0, "",     ARG_NON, ARG_NON};
opinfo op_RETI_info  = {OP_RETI,  0, "",     ARG_NON, ARG_NON};
opinfo op_INT_info   = {OP_INT,   1, "",     ARG_REG | ARG_IMM, ARG_NON};
opinfo op_MOV_info   = {OP_MOV,   2, "B",    ARG_REG, ARG_REG | ARG_IMM};
opinfo op_LD_info    = {OP_LD,    2, "",     ARG_REG, ARG_REG | ARG_IMM};
opinfo op_ST_info    = {OP_ST,    2, "",     ARG_REG, ARG_REG | ARG_IMM};
opinfo op_LDFLG_info = {OP_LDFLG, 1, "",     ARG_REG, ARG_NON};
opinfo op_STFLG_info = {OP_STFLG, 1, "",     ARG_REG, ARG_NON};

dirinfo dir_ORIGIN_info = {DIR_ORIGIN, 1, ARG_NUM};
dirinfo dir_WORD_info   = {DIR_WORD,   1, ARG_NUM};
dirinfo dir_ZWORDS_info = {DIR_ZWORDS, 1, ARG_NUM};

hashmap * generate_opmap(void) {
	hashmap * opmap = new_hashmap(NULL);
	hashmap_insert(opmap, "ADD",   (void*)&op_ADD_info);
	hashmap_insert(opmap, "SUB",   (void*)&op_SUB_info);
	hashmap_insert(opmap, "UMUL",  (void*)&op_MUL_info);
	hashmap_insert(opmap, "SMUL",  (void*)&op_MUL_info);
	hashmap_insert(opmap, "UDIV",  (void*)&op_DIV_info);
	hashmap_insert(opmap, "SDIV",  (void*)&op_DIV_info);
	hashmap_insert(opmap, "SHA",   (void*)&op_SHA_info);
	hashmap_insert(opmap, "UMOD",  (void*)&op_MOD_info);
	hashmap_insert(opmap, "SMOD",  (void*)&op_MOD_info);
	hashmap_insert(opmap, "AND",   (void*)&op_AND_info);
	hashmap_insert(opmap, "OR",    (void*)&op_OR_info);
	hashmap_insert(opmap, "XOR",   (void*)&op_XOR_info);
	hashmap_insert(opmap, "NOT",   (void*)&op_NOT_info);
	hashmap_insert(opmap, "SHL",   (void*)&op_SHL_info);
	hashmap_insert(opmap, "ROL",   (void*)&op_ROL_info);
	hashmap_insert(opmap, "SHR",   (void*)&op_SHR_info);
	hashmap_insert(opmap, "ROR",   (void*)&op_ROR_info);
	hashmap_insert(opmap, "JMP",   (void*)&op_JMP_info);
	hashmap_insert(opmap, "CALL",  (void*)&op_CALL_info);
	hashmap_insert(opmap, "RET",   (void*)&op_RET_info);
	hashmap_insert(opmap, "RETI",  (void*)&op_RETI_info);
	hashmap_insert(opmap, "INT",   (void*)&op_INT_info);
	hashmap_insert(opmap, "MOV",   (void*)&op_MOV_info);
	hashmap_insert(opmap, "LD",    (void*)&op_LD_info);
	hashmap_insert(opmap, "ST",    (void*)&op_ST_info);
	hashmap_insert(opmap, "LDFLG", (void*)&op_LDFLG_info);
	hashmap_insert(opmap, "STFLG", (void*)&op_STFLG_info);
	return opmap;
}


hashmap * generate_regmap(void) {
	hashmap * regmap = new_hashmap(NULL);
	hashmap_insert(regmap, "R0",  (void*)0x10);
	hashmap_insert(regmap, "RZ",  (void*)0x10);
	hashmap_insert(regmap, "R1",  (void*)0x11);
	hashmap_insert(regmap, "R2",  (void*)0x12);
	hashmap_insert(regmap, "R3",  (void*)0x13);
	hashmap_insert(regmap, "R4",  (void*)0x14);
	hashmap_insert(regmap, "R5",  (void*)0x15);
	hashmap_insert(regmap, "R6",  (void*)0x16);
	hashmap_insert(regmap, "R7",  (void*)0x17);
	hashmap_insert(regmap, "R8",  (void*)0x18);
	hashmap_insert(regmap, "R9",  (void*)0x19);
	hashmap_insert(regmap, "R10", (void*)0x1a);
	hashmap_insert(regmap, "R11", (void*)0x1b);
	hashmap_insert(regmap, "R12", (void*)0x1c);
	hashmap_insert(regmap, "R13", (void*)0x1d);
	hashmap_insert(regmap, "R14", (void*)0x1e);
	hashmap_insert(regmap, "R15", (void*)0x1f);
	hashmap_insert(regmap, "SP",  (void*)0x1f);
	return regmap;
}


hashmap * generate_dirmap(void) {
	hashmap * dirmap = new_hashmap(NULL);
	hashmap_insert(dirmap, ".ORIGIN", (void*)&dir_ORIGIN_info);
	hashmap_insert(dirmap, ".WORD",   (void*)&dir_WORD_info);
	hashmap_insert(dirmap, ".ZWORDS", (void*)&dir_ZWORDS_info);
	return dirmap;
}


bool isint(char * s) {
	char * p = s;
	int base = 10;
	if (*p == '+' || *p == '-') p++;
	if (!strncmp(s, "0X", 2)) {
		base = 16;
		p += 2;
	} else if (s[0] == '0') {
		base = 8;
	}

	for (; *p; p++) {
		char c = *p;
		if (c >= '0' && c <= '7') continue;
		if (base >= 8 && c >= '8' && c <= '9') continue;
		if (base >= 16 && c >= 'A' && c <= 'F') continue;
		return false;
	}

	return true;
}


enum argtype getargtype(hashmap * regmap, char * arg) {
	if (!arg) return ARG_NON;
	if (hashmap_get(regmap, arg)) return ARG_REG;
	if (isint(arg)) return ARG_NUM;
	return ARG_LBL;
}


void get_insbin(hashmap * opmap, hashmap * regmap, line_content c, insword * insbin) {
	insbin->bad = false;
	insbin->imm = false;
	insbin->label[0] = '\0';

	opinfo * opcode_info = (opinfo*)hashmap_get(opmap, c.opcode);
	uint8_t op_bin = opcode_info->opcode & 0x1F;
	insbin->binword.opcode = op_bin;

	enum argtype arg1type = getargtype(regmap, c.arg1);
	enum argtype arg2type = getargtype(regmap, c.arg2);

	/* reg1 */
	switch (op_bin) {
		case OP_RET:
			if (strcmp(c.opcode, "RET"))
				insbin->binword.reg1 = BIN4(1,0,0,0);
			else /* RETI */
				insbin->binword.reg1 = BIN4(0,0,0,0);
			break;
		case OP_JMP:
			/* register argument */
			if (arg1type & ARG_REG) {
				insbin->binword.reg1 = REG_ADDR(hashmap_get(regmap, c.arg1));
			} else /* immediate argument */ {
				insbin->imm = true;
				strcpy(insbin->label, c.arg1);
				insbin->binword.reg1 = BIN4(0,0,0,0);
			}
			break;
		default:
			insbin->binword.reg1 = REG_ADDR(hashmap_get(regmap, c.arg1));
			break;
	}

	/* flags */
	switch (op_bin) {
		case OP_ADD:
		case OP_SUB:
		case OP_AND:
		case OP_OR :
		case OP_XOR:
		case OP_NOT:
		case OP_MOV:
			/* check immediate */
			if (arg2type & ARG_IMM) {
				insbin->binword.flag1 = 1;
				insbin->imm = true;
				strcpy(insbin->label, c.arg2);
			} else /* register argument */
				insbin->binword.flag1 = 0;

			/* check word or byte */
			if (c.flags && c.flags[0] == 'B')
				insbin->binword.flag2 = 0;
			else
				insbin->binword.flag2 = 1;

			insbin->binword.flag3 = 0;
			break;
		case OP_MUL:
		case OP_DIV:
		case OP_MOD:
			/* immediate */
			if (arg2type & ARG_IMM) {
				insbin->binword.flag1 = 1;
				insbin->imm = true;
				strcpy(insbin->label, c.arg2);
			} else insbin->binword.flag1 = 0;
			if (c.flags && c.flags[0] == 'B') insbin->binword.flag2 = 0;
			else insbin->binword.flag2 = 1;
			if (c.opcode[0] == 'S') insbin->binword.flag3 = 1;
			else insbin->binword.flag3 = 0;
			break;
		case OP_SHL:
		case OP_SHR:
			if (arg2type & ARG_IMM) insbin->binword.flag1 = 1;
			else insbin->binword.flag1 = 0;
			insbin->binword.flag2 = 0;
			if (c.opcode[0] == 'R') /* rotate */
				insbin->binword.flag3 = 1;
			else /* linear shift */
				insbin->binword.flag3 = 0;
			break;
		case OP_SHA:
		case OP_ST:
		case OP_LD:
			if (arg2type & ARG_IMM) insbin->binword.flag1 = 1;
			else insbin->binword.flag1 = 0;
			insbin->binword.flag2 = 0;
			insbin->binword.flag3 = 0;
			break;
		/* finish the entire word here */
		case OP_JMP:
			if (arg1type & ARG_IMM) {
				insbin->binword.flag1 = 1;
				insbin->imm = true;
				strcpy(insbin->label, c.arg1);
			} else insbin->binword.flag1 = 0;
			if (c.flags && strchr(c.flags, 'N')) insbin->binword.flag2 = 1;
			else insbin->binword.flag2 = 0;
			if (c.flags && strchr(c.flags, 'Z')) insbin->binword.flag3 = 1;
			else insbin->binword.flag3 = 0;
			int flag4;
			int flag5;
			int flag6;
			int flag7;
			if (c.flags && strchr(c.flags, 'P')) flag4 = 1;
			else flag4 = 0;
			if (c.flags && strchr(c.flags, 'C')) flag5 = 1;
			else flag5 = 0;
			if (c.flags && strchr(c.flags, 'O')) flag6 = 1;
			else flag6 = 0;
			if (c.opcode[0] == 'C')   flag7 = 1;
			else flag7 = 0;
			insbin->binword.reg2 = BIN4(flag4, flag5, flag6, flag7);
			break;
		/* finish the entire word here */
		case OP_RET:
		case OP_LDFLG:
		case OP_STFLG:
			insbin->binword.flag1 = 0;
			insbin->binword.flag2 = 0;
			insbin->binword.flag2 = 0;
			insbin->binword.reg2 = BIN4(0,0,0,0);
			break;
		case OP_INT:
			if (arg1type & ARG_LBL) {
				insbin->bad = true;
				break;
			}
			if (arg1type & ARG_NUM) {
				insbin->binword.flag1 = 1;
				uint8_t num = strtol(c.arg1, NULL, 0);
				insbin->binword.flag2 = (num >> 5) & 1;
				insbin->binword.flag3 = (num >> 4) & 1;
				insbin->binword.reg2 = num & BIN4(1,1,1,1);
			}
			break;
	}

	/* reg2 */
	switch (op_bin) {
		case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV:
		case OP_MOD:
		case OP_AND:
		case OP_OR:
		case OP_XOR:
		case OP_NOT:
		case OP_MOV:
		case OP_LD:
		case OP_ST:
			if (arg2type & ARG_IMM) {
				insbin->imm = true;
				strcpy(insbin->label, c.arg2);
				insbin->binword.reg2 = BIN4(0,0,0,0);
			} else
				insbin->binword.reg2 = REG_ADDR(hashmap_get(regmap, c.arg2));
			break;
		case OP_SHA:
		case OP_SHL:
		case OP_SHR:
			if (arg2type & ARG_LBL) {
				insbin->bad = true;
				break;
			}
			if (arg2type & ARG_NUM) {
				uint8_t num = strtol(c.arg2, NULL, 0);
				insbin->binword.reg2 = num & BIN4(1,1,1,1);
			} else {
				insbin->binword.reg2 = REG_ADDR(hashmap_get(regmap, c.arg2));
			}
			break;
	}
}
