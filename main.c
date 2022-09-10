#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "helpers.h"

char file_name[32];

void do_stuff(FILE *outfile, char *file_name, char *line);

int main(int argc, char **argv)
{
	FILE *outfile;
	DIR *dir;
	struct dirent *entry;
	char infile_path[261];
	char outfile_name[50];
	char line[1000];

	if (argc != 2) {
		fprintf(stderr, "Usage: VMTranslator [fileName.vm] [dirName]\n");
		exit(1);
	}

	if (is_valid_file(argv[1])) {
		sprintf(outfile_name, "%s.asm", file_name);
	} else {
		sprintf(outfile_name, "./%s/%s.asm", argv[1], argv[1]);
	}

	if ((outfile = fopen(outfile_name, "w"))) {
		if (is_valid_file(argv[1])) {
			do_stuff(outfile, argv[1], line);
		} else if ((dir = opendir(argv[1]))) {
			bootstrap(outfile);
			while ((entry = readdir(dir))) {
				if (is_valid_file(entry->d_name)) {
					sprintf(infile_path, "%s/%s", argv[1], entry->d_name);
					do_stuff(outfile, infile_path, line);
				}
			}
		} else {
			fprintf(stderr, "%s: error, invalid input(%s)\n", argv[0], argv[1]);
			exit(1);
		}
	} else {
		fprintf(stderr, "error: can't open outfile(%s)\n", outfile_name);
		exit(1);
	}

	fclose(outfile);
	fprintf(stderr, "done successfully\ncheck output\n");
	return 0;
}

void do_stuff(FILE *outfile, char *infile_name, char *line)
{
	FILE *infile;
	if ((infile = fopen(infile_name, "r"))) {
		while (mygetline(infile, line)) {
			if (iscomment(line))
				continue;
			code_writer(outfile, line);
		}
	} else {
		fprintf(stderr, "error: con't open file(%s)\n", infile_name);
		exit(1);
	}
	fclose(infile);
}
