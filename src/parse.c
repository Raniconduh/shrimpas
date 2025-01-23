#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "parse.h"


static void strncpy_upper(char * dest, char * source, size_t n) {
	size_t i;
	for (i = 0; i < n; i++) {
		dest[i] = toupper(source[i]);
	}
	dest[i] = '\0';
}


line_content parse(char * line) {
	static line_content content;

	content.arg1 = NULL;
	content.arg2 = NULL;
	content.flags = NULL;

	/* skip whitespace */
	char * l = line;
	while (*l && *l != ';' && isspace(*l)) l++;

	/* skip comma */
	char * e = strchr(l, ';');
	if (!e) {
		e = l;
		while (*e) e++;
	}

	/* this line is empty */
	if (e == l) {
		content.type = CONTENT_NOTHING;
		return content;
	}

	e--;

	/* skip whitespace at end */
	while (isspace(*e)) e--;
	*(e + 1) = '\0';

	/* check if label */
	char * colon = strchr(l, ':');
	if (colon && *l != '.') {
		strncpy_upper(content.label, l, e - l);

		if (colon != e) {
			content.type = CONTENT_INVALID_LABEL_NAME;
			return content;
		}

		if (e - l > 15) {
			content.type = CONTENT_INVALID_LABEL_LENGTH;
			return content;
		}

		bool valid_label = true;

		if (!isalpha(content.label[0]) && content.label[0] != '_') {
			valid_label = false;
		} else for (char * p = &content.label[1]; *p; p++) {
			if (!isalnum(*p) && *p != '_') {
				valid_label = false;
				break;
			}
		}

		if (!valid_label) {
			content.type = CONTENT_INVALID_LABEL_NAME;
			return content;
		}

		content.type = CONTENT_LABEL;
	/* directive */
	} else if (colon) {
		content.arg1 = NULL;
		char * directive_end = colon;
		strncpy_upper(content.label, l, directive_end - l);

		char * arg_start = directive_end + 1;
		while (*arg_start && isspace(*arg_start)) arg_start++;

		/* there is an argument */
		if (arg_start != e + 1) {
			content.arg1 = content.arg1content;
			strncpy_upper(content.arg1, arg_start, e + 1 - arg_start);
		}

		content.type = CONTENT_DIRECTIVE;
	/* this must be an instruction now */
	} else {
		content.opcode[0] = '\0';

		char * opcode_end = l;
		while (*opcode_end && !isspace(*opcode_end)) opcode_end++;

		/* determine if flags exist */
		char * flag_start = NULL;
		char * flag_end = opcode_end;
		for (char * p = l; p <= opcode_end; p++) {
			if (*p == '.') {
				flag_start = p + 1;
				opcode_end = p;
				break;
			}
		}

		content.type = CONTENT_INSTRUCTION;
		strncpy_upper(content.opcode, l, opcode_end - l);

		if (flag_start) {
			content.flags = content.flagscontent;
			strncpy_upper(content.flags, flag_start, flag_end - flag_start);
		} else {
			content.flags = NULL;
		}


		/* figure out arguments */
		char * arg1_start = flag_end;
		char * arg1_end = e + 1; /* null character at end of string */
		while (*arg1_start && isspace(*arg1_start))	arg1_start++;
		/* there exists at least 1 argument */
		if (arg1_start != opcode_end) {
			content.arg1 = content.arg1content;

			char * arg2_start = strchr(arg1_start, ',');
			char * arg2_end = e + 1;
			/* there is a second argument */
			if (arg2_start) {
				arg1_end = arg2_start;
				arg2_start++;
				while (isspace(*arg1_end)) arg1_end--;
				while (isspace(*arg2_start)) arg2_start++;

				content.arg2 = content.arg2content;
				strncpy_upper(content.arg2, arg2_start, arg2_end - arg2_start);

				for (char * p = content.arg2; *p; p++) {
					if (isspace(*p)) {
						content.type = CONTENT_INVALID_ARG2;
						return content;
					}
				}
			}

			strncpy_upper(content.arg1, arg1_start, arg1_end - arg1_start);
			
			for (char * p = content.arg1; *p; p++) {
				if (isspace(*p)) {
					content.type = CONTENT_INVALID_ARG1;
					return content;
				}
			}
		}
	}

	return content;
}
