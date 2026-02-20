#pragma once

#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#define RED "\e[0;91m"
#define GREEN "\e[0;92m"
#define MAGENTA "\e[0;95m"
#define BLUE "\e[0;94m"
#define CYAN "\e[0;96m"
#define RESET "\e[0m"

int is_yagit_repo();
void build_path(char*, int, ...);
int hex_to_uint8t(char*, size_t , uint8_t (*)[]);
void print_hash(const uint8_t*);
int calculate_blob_hash(FILE*, long long, char**, size_t*, uint8_t*);
void crlf_to_lf(char *buffer, size_t *buffer_len);
void write_into_toilet(uint8_t[], char*, size_t);
void* read_from_toilet(uint8_t[], size_t*);
void read_entry_obj(char (*)[], size_t*, char*, uint8_t*, size_t, char*, size_t*);
void read_tree_entries(char (*)[], size_t*, char*, const uint8_t*, size_t);