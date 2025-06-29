#ifndef STRING_H
#define STRING_H

#include <common/common.h>

unsigned long strlen(const char *str);
bool isEqual (const uint8_t* a, const uint8_t* b, size_t size);
char* itoa(int value, char str, int base);
void* memset (void* ptr, int value, size_t num);
void* memcpy (void* dest, const void* src, size_t num);
char* strstr(const char* haystack, const char* needle);
int strcmp(const char* str1, const char* str2);
int strncmp (const char* stri, const char* str2, size_t num);
int stremp(const char* stri, const char* str2);
bool isset (void *pointer);
char *strncpy(char *dest, const char *src, uintptr_t n);
bool isint(char c); 
int isspace(char c); 
int isalpha(char c);
char upper(char c); 
char lower(char c);

#endif // STRING_H
