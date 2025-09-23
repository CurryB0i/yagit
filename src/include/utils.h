#pragma once

#include <stddef.h>

#define RED "\e[0;31m"
#define GREEN "\e[0;32m"
#define MAGENTA "\e[0;35m"
#define RESET "\e[0m"

int is_yagit_repo();
void crlf_to_lf(char *buffer, size_t *buffer_len);
