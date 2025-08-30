#ifndef OPCODE_H
#define OPCODE_H

#include <stdint.h>
#include <stdbool.h>

#include "parse.h"
#include "hashmap.h"


#define BIN2(a, b) (((a&1) << 1) | ((b&1) << 0))
#define BIN4(a, b, c, d) ((BIN2(a, b) << 2) | (BIN2(c, d)))
#define BIN5(a, b, c, d, e) (((BIN4(a, b, c, d)) << 1) | (e&1))
#define BIN6(a, b, c, d, e, f) ((BIN4(a, b, c, d) << 2) | BIN2(e, f))
#define REG_ADDR(REG_FROM_MAP) ((uint8_t)((uintptr_t)REG_FROM_MAP & BIN4(1,1,1,1)))
#define OPC_BIN(insw) ((insw).binword.opcode << 11 | (insw).binword.reg1 << 7 | (insw).binword.flag1 << 6 | (insw).binword.flag2 << 5 | (insw).binword.flag3 << 4 | (insw).binword.reg2)


#define OP_ADD   (uint8_t)0x00
#define OP_SUB   (uint8_t)0x01
#define OP_MUL   (uint8_t)0x02
#define OP_DIV   (uint8_t)0x03
#define OP_SHA   (uint8_t)0x04
#define OP_MOD   (uint8_t)0x05
#define OP_AND   (uint8_t)0x06
#define OP_OR    (uint8_t)0x07
#define OP_XOR   (uint8_t)0x08
#define OP_NOT   (uint8_t)0x09
#define OP_SHL   (uint8_t)0x0a
#define OP_ROL   (uint8_t)OP_SHL
#define OP_SHR   (uint8_t)0x0b
#define OP_ROR   (uint8_t)OP_SHR
#define OP_JMP   (uint8_t)0x0c
#define OP_CALL  (uint8_t)OP_JMP
#define OP_RET   (uint8_t)0x0d
#define OP_RETI  (uint8_t)OP_RET
#define OP_INT   (uint8_t)0x0e
#define OP_MOV   (uint8_t)0x0f
#define OP_LD    (uint8_t)0x10
#define OP_ST    (uint8_t)0x11
#define OP_LDFLG (uint8_t)0x12
#define OP_STFLG (uint8_t)0x13


enum {
	DIR_ORIGIN,
	DIR_WORD,
	DIR_ZWORDS,
};

enum argtype {
	ARG_NON = 0,
	ARG_REG = 1 << 0,
	ARG_LBL = 1 << 1,
	ARG_NUM = 1 << 2,
#define ARG_IMM  (ARG_LBL | ARG_NUM)
	ARG_INV = 1 << 3,
};


typedef struct {
	uint8_t opcode;
	int n_args;
	char flags[6];

	enum argtype arg1type;
	enum argtype arg2type;

	/* immediate argument is in the instruction word */
	bool inplace;
} opinfo;

typedef struct {
	int dircode;
	int n_args;
	enum argtype argtype;
} dirinfo;

typedef struct {
	bool bad;
	bool imm;
	char label[16];
	struct {
		uint8_t opcode : 5;
		uint8_t reg1   : 4;
		uint8_t flag1  : 1;
		uint8_t flag2  : 1;
		uint8_t flag3  : 1;
		uint8_t reg2   : 4;
	} binword;
} insword;


extern opinfo op_ADD_info;
extern opinfo op_SUB_info;
extern opinfo op_MUL_info;
extern opinfo op_DIV_info;
extern opinfo op_SHA_info;
extern opinfo op_MOD_info;
extern opinfo op_AND_info;
extern opinfo op_OR_info;
extern opinfo op_XOR_info;
extern opinfo op_NOT_info;
extern opinfo op_SHL_info;
extern opinfo op_ROL_info;
extern opinfo op_SHR_info;
extern opinfo op_ROR_info;
extern opinfo op_JMP_info;
extern opinfo op_CALL_info;
extern opinfo op_RET_info;
extern opinfo op_RETI_info;
extern opinfo op_INT_info;
extern opinfo op_MOV_info;
extern opinfo op_LD_info;
extern opinfo op_ST_info;
extern opinfo op_LDFLG_info;
extern opinfo op_STFLG_info;

extern dirinfo dir_ORIGIN_info;
extern dirinfo dir_WORD_info;
extern dirinfo dir_ZWORDS_info;

hashmap * generate_opmap(void);
hashmap * generate_regmap(void);
hashmap * generate_dirmap(void);
enum argtype getargtype(hashmap *, char *);
bool isint(char *);
void get_insbin(hashmap *, hashmap *, line_content, insword *);

#endif /* OPCODE_H */
