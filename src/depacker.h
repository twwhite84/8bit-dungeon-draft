#ifndef DEPACKER_H
#define DEPACKER_H

#include "shared.h"
#include <stdint.h>

extern void inflate(uint16_t src_addr, uint16_t dest_addr);
extern uint16_t mapIDtoAddr(uint8_t id);

#endif