#pragma once

#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include "object.h"

#define RED "\e[1;91m"
#define GREEN "\e[1;92m"
#define MAGENTA "\e[1;95m"
#define BLUE "\e[1;94m"
#define CYAN "\e[1;96m"
#define YELLOW "\e[1;33m"
#define RESET "\e[0m"

void crlf_to_lf(char*, size_t*);
void print_error(const char* msg);
int get_timezone_offset_minutes(time_t);
void print_tz_offset(int);
void print_localtime(time_t);
int is_yagit_repo();
void build_path(char*, int, ...);
int hex_to_uint8t(char*, size_t , uint8_t (*)[]);
void print_hash(const uint8_t*);
int calculate_blob_hash(FILE*, long long, char**, size_t*, uint8_t*);
void write_into_toilet(const uint8_t*, char*, size_t);
void* read_from_toilet(const uint8_t*, size_t*);
void destruct();