#include <stdint.h>
#include <stdio.h>
#include "main.h"

char *jal5_head_jlvm(void) {
	return GEN_jlvm;
}

uint32_t jal5_head_size(void) {
	return sizeof(GEN_jlvm);
}