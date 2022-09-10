#ifndef MYSTRING_INCLUDED
#define MYSTRING_INCLUDED

#include <string.h>

void mystrcat(char *s, const char *t);
void mystrncat(char *s, const char *t, int n);
void mystrcpy(char *dst, const char *src);
void mystrncpy(char *dst, const char *src, int n);
int mystrcmp(const char *s, const char *t);
int mystrncmp(const char *s, const char *t, int n);
int mystrlen(char *s);
int mystrend(const char *s, const char *t);
int strindex(const char *s, const char *t);
int strrindex(const char *s, const char *t); 
void reverse(char *s);
void mysubstring(char *dst, const char *src, int from, int to);
void mysplit(const char *str, char *s1, char *s2, char dlm);
void mystrcat_dlm(char *s1, char *s2, char dlm);

#endif
