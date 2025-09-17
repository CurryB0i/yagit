#pragma once

#include <stdint.h>
#include <stddef.h>

#define SHA256_DIGEST_SIZE 32

int SHA256(const uint8_t* msg, size_t msgLen, uint8_t* digest);
