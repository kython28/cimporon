#ifndef CIMPO_H
#define CIMPO_H

#include <stdint.h>

typedef struct {
	int fd;
	char *name;
	uint64_t size;
} cimpo;

cimpo *openFile(const char *name);

int64_t getValue(cimpo *file, uint64_t key);

uint8_t addValue(cimpo *file, uint64_t key, int64_t value);
uint8_t editValue(cimpo *file, uint64_t key, int64_t value);
uint8_t removeKey(cimpo *file, uint64_t key);

void clearCimpoFile(cimpo *file);
void closeCimpoFile(cimpo *file);

#endif