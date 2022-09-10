
int is_valid_file(const char *s);
int iscomment(char *line);
int mygetline(FILE *file, char *buf);
void code_writer(FILE *file, char *vm_inst);
void bootstrap(FILE *file);
