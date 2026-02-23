#include "config.h"
#include <string.h>

User user;

void config_init() {
  strncpy(user.email, "unknown", sizeof("unknown"));
  strncpy(user.name, "unknown", sizeof("unkown"));
}