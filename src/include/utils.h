#pragma once

#include <stddef.h>

#define RED "\e[0;31m"
#define GREEN "\e[0;32m"
#define RESET "\e[0m"

void init();
void populate_limbo();
void populate_dir_entries(const char* folder_path);
int is_yagit_repo();
void crlf_to_lf(char *buffer, size_t *buffer_len);
bool starts_with(const char* str, const char* prefix);
