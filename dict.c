/* Author: Mohamed M. A. Mordy */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dict.h"

#define arrlen(x) (sizeof(x)/sizeof(x[0]))

enum flags{
	MEM, ARITH, FLOW, FUNC,
};

extern char file_name[];
static char function_name[100];

static void func_name_setter(char *name)
{
	sprintf(function_name, "%s", name);
}

static void make_label(char *lbl)
{
	char buf[100];
	strcpy(buf, lbl);
	sprintf(lbl, "%s$%s", function_name, buf);
}

struct code {
	char *vm;
	char *assem;
};

struct code memcode[] = {
	{"push constant", "@%d\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"}, // push constant i
	{"push local", "@LCL\nD=M\n@R13\nM=D\n@%d\nD=A\n@R13\nM=D+M\n@R13"
		"\nA=M\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"}, // push local i
	{"pop local", "@LCL\nD=M\n@R13\nM=D\n@%d\nD=A\n@R13\nM=D+M\n@SP"
		"\nM=M-1\n@SP\nA=M\nD=M\n@R13\nA=M\nM=D\n"}, // pop local i
	{"push argument", "@ARG\nD=M\n@R13\nM=D\n@%d\nD=A\n@R13"
		"\nM=D+M\n@R13\nA=M\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"}, // push argument i
	{"pop argument", "@ARG\nD=M\n@R13\nM=D\n@%d\nD=A\n@R13"
		"\nM=D+M\n@SP\nM=M-1\n@SP\nA=M\nD=M\n@R13\nA=M\nM=D\n"}, // pop argument i
	{"push this", "@THIS\nD=M\n@R13\nM=D\n@%d\nD=A\n@R13"
		"\nM=D+M\n@R13\nA=M\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"}, // push this i
	{"pop this", "@THIS\nD=M\n@R13\nM=D\n@%d\nD=A\n@R13"
		"\nM=D+M\n@SP\nM=M-1\n@SP\nA=M\nD=M\n@R13\nA=M\nM=D\n"}, // pop this i
	{"push that", "@THAT\nD=M\n@R13\nM=D\n@%d\nD=A\n@R13"
		"\nM=D+M\n@R13\nA=M\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"}, // push that i
	{"pop that", "@THAT\nD=M\n@R13\nM=D\n@%d\nD=A\n@R13"
		"\nM=D+M\n@SP\nM=M-1\n@SP\nA=M\nD=M\n@R13\nA=M\nM=D\n"}, // pop that i
	{"push temp", "@5\nD=A\n@R13\nM=D\n@%d\nD=A\n@R13\nM=D+M\n@R13"
		"\nA=M\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"}, // push temp i
	{"pop temp", "@5\nD=A\n@R13\nM=D\n@%d\nD=A\n@R13\nM=D+M\n"
		"@SP\nM=M-1\n@SP\nA=M\nD=M\n@R13\nA=M\nM=D\n"}, // pop temp i
	{"push pointer", "@%s\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"}, // push pointer 0/1
	{"pop pointer", "@SP\nM=M-1\n@SP\nA=M\nD=M\n@%s\nM=D\n"}, // pop pointer 0/1
	{"pop static", "@SP\nM=M-1\n@SP\nA=M\nD=M\n@%s.%d\nM=D\n"}, // pop static i
	{"push static","@%s.%d\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"}, // push static i
};

struct code arithcode[] = {
	{"add", "@SP\nM=M-1\n@SP\nA=M\nD=M\n@R13\nM=D\n@SP\nM=M-1\n@SP\nA=M\nD=M"
		"\n@R13\nM=D+M\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"}, // add
	{"sub", "@SP\nM=M-1\n@SP\nA=M\nD=M\n@R13\nM=D\n@SP\nM=M-1\n@SP\nA=M\nD=M"
		"\n@R13\nM=D-M\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"}, // sub
	{"neg", "@SP\nM=M-1\n@SP\nA=M\nM=-M\n@SP\nM=M+1\n"}, // neg

	{"eq", "@SP\nM=M-1\n@SP\nA=M\nD=M\n@SP\nM=M-1\n@SP\nA=M\nD=M-D\n@IF%d\nD;JEQ\n@0\nD=A\n@SP\nA=M"
		"\nM=D\n@SP\nM=M+1\n@EIF%d\n0;JMP\n(IF%d)\n@SP\nA=M\nM=-1\n@SP\nM=M+1\n(EIF%d)\n"}, // eq
	{"gt", "@SP\nM=M-1\n@SP\nA=M\nD=M\n@SP\nM=M-1\n@SP\nA=M\nD=M-D\n@IF%d\nD;JGT\n@0\nD=A\n@SP\nA=M\nM=D\n"
		"@SP\nM=M+1\n@EIF%d\n0;JMP\n(IF%d)\n@SP\nA=M\nM=-1\n@SP\nM=M+1\n(EIF%d)\n"}, // gt
	{"lt", "@SP\nM=M-1\n@SP\nA=M\nD=M\n@SP\nM=M-1\n@SP\nA=M\nD=M-D\n@IF%d\nD;JLT\n@0\nD=A\n@SP\nA=M\nM=D\n"
		"@SP\nM=M+1\n@EIF%d\n0;JMP\n(IF%d)\n@SP\nA=M\nM=-1\n@SP\nM=M+1\n(EIF%d)\n"}, // lt

	{"and", "@SP\nM=M-1\n@SP\nA=M\nD=M\n@SP\nM=M-1\n@SP\nA=M\nM=D&M\n@SP\nM=M+1\n"}, // and
	{"or", "@SP\nM=M-1\n@SP\nA=M\nD=M\n@SP\nM=M-1\n@SP\nA=M\nM=D|M\n@SP\nM=M+1\n"}, // or
	{"not", "@SP\nM=M-1\nA=M\nM=!M\n@SP\nM=M+1\n"}, // not
};

struct code flowcode[] = {
	{"label", "(%s)\n"},
	{"goto", "@%s\n0;JMP\n"},
	{"if-goto", "@SP\nM=M-1\nA=M\nD=M\n@%s\nD;JNE\n"},
};

struct code funccode[] = {
	{"call", "@%s$return.%d\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"
		"@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"
		"@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"
		"@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"
		"@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"
		"@SP\nD=M\n@ARG\nM=D\n@%d\nD=A\n@ARG\nM=M-D\n@5\nD=A\n@ARG\nM=M-D\n"
		"@SP\nD=M\n@LCL\nM=D\n"
		"@%s\n0;JMP\n"
		"(%s$return.%d)\n"
		},

	{"function", "(%s)\n"
		"@%d\nD=A\n@R15\nM=D\n(%s$repeat)\n@R15\nD=M\n@%s$end\nD;JEQ\n@R15\nM=M-1\n@SP\nA=M\nM=0\n@SP\nM=M+1\n@%s$repeat\n0;JMP\n(%s$end)\n"
		},

	{"return", "@LCL\nD=M\n@R13\nM=D\n"
		"@R15\nM=D\n@5\nD=A\n@R15\nM=M-D\n@R15\nA=M\nD=M\n@R14\nM=D\n"
		"@SP\nM=M-1\nA=M\nD=M\n@ARG\nA=M\nM=D\n"
		"@ARG\nD=M\nD=D+1\n@SP\nM=D\n"
		"@R13\nD=M\nA=D-1\nD=M\n@THAT\nM=D\n"
		"@R13\nD=M\n@2\nA=D-A\nD=M\n@THIS\nM=D\n"
		"@R13\nD=M\n@3\nA=D-A\nD=M\n@ARG\nM=D\n"
		"@R13\nD=M\n@4\nA=D-A\nD=M\n@LCL\nM=D\n"
		"@R14\nA=M\n0;JMP\n"
		},
};


int memlen = arrlen(memcode);
int arithlen = arrlen(arithcode);
int flowlen = arrlen(flowcode);

void bootstrap(FILE *file)
{
	const char *boot = 
		"//bootstrap start\n"
		// sp = 256
		"@256\nD=A\n@SP\nM=D\n"
		// set LCL, ARG, THIS, THAT -> -1, -2, -3, -4
		"@1\nD=A\n@LCL\nM=-D\n@2\nD=A\n@ARG\nM=-D\n@3\nD=A\n@THIS\nM=-D\n@4\nD=A\n@THAT\nM=-D\n"
		// push return address, which is NULL
		"@SP\nM=M+1\n"
		// save the current function frame
		"@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"
		"@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"
		"@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"
		"@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"
		// ARG = SP - 0 - 5
		"@SP\nD=M\n@ARG\nM=D\n@5\nD=A\n@ARG\nM=M-D\n"
		// LCL = SP
		"@SP\nD=M\n@LCL\nM=D\n"
		// call Sys.init()
		"@Sys.init\n0;JMP\n"
		"//bootstrap finish\n";

	fprintf(file, "%s", boot);
}

static int get_index(char *str, int len, int flag)
{
	int i = 0;
	if (flag == ARITH)
	{
		for ( ; i < len; i++)
			if (!strcmp(arithcode[i].vm, str))
				return i;
	} else if (flag == MEM){
		for ( ; i < len; i++)
			if (!strcmp(memcode[i].vm, str))
				return i;
	} else if (flag == FLOW) {
		for ( ; i < len; i++)
			if (!strcmp(flowcode[i].vm, str))
				return i;
	} else if (flag == FUNC) {
		for ( ; i < len; i++)
			if (!strcmp(funccode[i].vm, str))
				return i;
	}
	fprintf(stderr, "error: something unexpected happened\n"
		"\tfrom the function get_index()"
		"\the current flag is: %d\n", flag);
	exit(1);
}

void get_arith_code(char *vm_inst, char *asm_code)
{
	static int i = 0;
	if (!strcmp(vm_inst, "eq") || !strcmp(vm_inst, "gt") || !strcmp(vm_inst, "lt")) {
		sprintf(asm_code, arithcode[get_index(vm_inst, arithlen, ARITH)].assem, i, i, i, i);
		i++;
	} else {
		strcpy(asm_code, arithcode[get_index(vm_inst, arithlen, ARITH)].assem);
	}
}

void get_mem_code(char *str, int val, char *asm_code)
{
	if (!strcmp(str, "pop static")) {
		sprintf(asm_code, memcode[get_index(str, memlen, MEM)].assem, file_name, val);
	} else if (!strcmp(str, "push static")) {
		sprintf(asm_code, memcode[get_index(str, memlen, MEM)].assem, file_name, val);
	} else if (!strcmp(str, "push pointer")) {
		sprintf(asm_code, memcode[get_index(str, memlen, MEM)].assem, val ? "THAT" : "THIS");
	} else if (!strcmp(str, "pop pointer")) {
		sprintf(asm_code, memcode[get_index(str, memlen, MEM)].assem, val ? "THAT" : "THIS");
	} else {
		sprintf(asm_code, memcode[get_index(str, memlen, MEM)].assem, val);
	}
}

void get_flow_code(char *cmd, char *lbl, char *asm_code)
{
	make_label(lbl);
	sprintf(asm_code, flowcode[get_index(cmd, flowlen, FLOW)].assem, lbl);
}

void get_func_code(char *cmd, char *lbl, int val, char *asm_code)
{
	static int i = 0;
	if (!strcmp(cmd, "call")) {
		sprintf(asm_code, funccode[0].assem,lbl, i, val, lbl, lbl, i);
		i++;
		return;
	} else if (!strcmp(cmd, "function")) {
		func_name_setter(lbl);
		sprintf(asm_code, funccode[1].assem, lbl, val, lbl, lbl, lbl, lbl);
		return;
	} else if (!strcmp(cmd, "return")) {
		sprintf(asm_code, funccode[2].assem);
		return;
	}
	fprintf(stderr, "something unexpected happened\n"
		"from the function get_func_code()\n");
}


