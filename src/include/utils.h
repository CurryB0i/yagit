#pragma once

#include <stddef.h>
#include <stdint.h>

#define RED "\e[0;91m"
#define GREEN "\e[0;92m"
#define MAGENTA "\e[0;95m"
#define BLUE "\e[0;94m"
#define CYAN "\e[0;96m"
#define RESET "\e[0m"

int is_yagit_repo();
void crlf_to_lf(char *buffer, size_t *buffer_len);
void write_into_toilet(uint8_t[], char*, size_t);
void* read_from_toilet(uint8_t[], size_t*);
