#pragma once

typedef struct {
  char email[256];
  char name[256];
} User;

extern User user;

void config_init();