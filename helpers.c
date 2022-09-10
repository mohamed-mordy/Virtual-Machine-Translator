#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "dict.h"
#include "helpers.h"
#include "mystring.h"

extern char file_name[];

// static functions
static void cleanup(char *vm_line)
{
	char *vm_inst = vm_line; 

	while (isspace(*vm_line))
		vm_line++;
	while (*vm_line != '\n' && *vm_line != '/' && *(vm_line + 1) != '/') {
		if (isspace(*vm_line) && isspace(*(vm_line - 1))) {
			*vm_line++ = ' ';
			continue;
		}
		*vm_inst++ = *vm_line++;
	}
	if (isspace(*(vm_inst - 1)))
		*(vm_inst - 1) = '\0';
	*vm_inst = '\0';
}

/*
static void parseline(char *vm_inst, char *str, int *val)
{
	char buf[10];
	char *ptr = buf;
	while (*vm_inst && (isalpha(*vm_inst) || isspace(*vm_inst)))
		*str++ = *vm_inst++;
	str--;
	*str = '\0';
	while ((*ptr++ = *vm_inst++))
		;
	*val = atoi(buf);
}
*/

static int count_space(char *ptr)
{
	int i = 0;
	while (*ptr) {
		if (isspace(*ptr))
			i++;
		ptr++;	
	}
	return i;
}

// functions
int is_valid_file(const char *s)
{
	int n;
	char buf[5];

	n = strlen(s);
	mysubstring(buf, s, n- 3, n - 1);
	if (!strcmp(buf, ".vm")) {
		mysubstring(file_name, s, 0, n - 4 );
		return 1;
	}
	return 0;
}

int iscomment(char *line)
{
	if (*line == '\n')
		return 1;
	while (*line == ' ')
		line++;
	if (*line == '/' && *(line + 1) == '/')
		return 1;
	return 0;
}

int mygetline(FILE *file, char *buf)
{
	int c;
	char *s = buf;

	while ((c = fgetc(file)) != EOF && c != '\n')
		*buf++ = c;
	if (c == '\n')
		*buf++ = '\n';
	*buf = '\0';
	return (int) (buf - s);
}

void code_writer(FILE *file, char *vm_inst)
{
	char asm_code[1000];
	int val;
	char str[50];
	char s1[50];
	char s2[50];
	char s3[50];

	cleanup(vm_inst);
	int cspace = count_space(vm_inst);
	mysplit(vm_inst, s1, str, ' ');
	mysplit(str, s2, s3, ' ');
	val = atoi(s3);
	fprintf(file, "//%s\n", vm_inst);
	if (cspace == 0 && strcmp(s1, "return")) {
		get_arith_code(s1, asm_code);
		fprintf(file, "%s", asm_code);
		return;
	} else if (cspace == 1) {
		get_flow_code(s1, s2, asm_code);
		fprintf(file, "%s", asm_code);
		return;
	} else if (cspace == 2 || !strcmp(s1, "return")) {
		if (! strcmp(s1, "call") || ! strcmp(s1, "function") || ! strcmp(s1, "return")) {
			get_func_code(s1, s2, val, asm_code);
			fprintf(file, "%s", asm_code);
			return;
		} else {
			mystrcat_dlm(s1, s2, ' ');
			get_mem_code(s1, val, asm_code);
			fprintf(file, "%s", asm_code);
			return;
		}
	}
	fprintf(stderr, "something unexpected happened\nin the function code_writer()\n");
}

