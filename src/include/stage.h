#pragma once

#include <limits.h>

extern size_t staged_count;
extern size_t unstaged_count;
extern size_t committed_count;
extern size_t untracked_count;
extern size_t visited_count;
extern size_t untracked_cap;
extern char (*staged)[PATH_MAX];
extern char (*unstaged)[PATH_MAX];
extern char (*committed)[PATH_MAX];
extern char (*untracked)[PATH_MAX];

void set_stage();
void destroy_stage();