#ifndef PARSE_H
#define PARSE_H

#include <stdint.h>


enum content_type {
	CONTENT_NOTHING,
	CONTENT_DIRECTIVE,
	CONTENT_INSTRUCTION,
	CONTENT_LABEL,
	CONTENT_INVALID_LABEL_NAME,
	CONTENT_INVALID_LABEL_LENGTH,
	CONTENT_INVALID_ARG1,
	CONTENT_INVALID_ARG2,
};

typedef struct {
	enum content_type type;
	char opcode[10];
	char label[10];
	char * arg1;
	char * arg2;
	char * flags;

	/* parser use only */
	char arg1content[16];
	char arg2content[16];
	char flagscontent[10];
} line_content;


line_content parse(char *);

#endif /* PARSE_H */
