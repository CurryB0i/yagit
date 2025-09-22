#pragma once
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

//.yagit folders
#define TOILET "toilet"
#define SNITCHES "snitches"
#define LANDMINES "landmines"
#define USELESS_TRIVIA "useless_trivia"
#define DIRT "dirt"
#define NO_OF_FOLDERS 5
extern const char* folders[];

//.yagit files
#define YESTERDAY "yesterday"
#define SETTINGS_YOULL_BREAK "settings_youll_break"
#define BRAG_SHEET "brag_sheet"
#define LIMBO "limbo"
#define STUFFED_SNITCHES "stuffed_snitches"
#define NO_OF_FILES 5
extern const char* files[];

// working dir
extern char YAGIT_SRC_DIR[PATH_MAX];
extern char CURRENT_DIR[PATH_MAX];
//yagit dir name
extern char YAGIT_DIR[7];

void init();
void build_path(char* buffer, int n, ...);
