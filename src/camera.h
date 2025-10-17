#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>
#include <stdint.h>

void loadRoom(uint8_t roomID);
void loadStatics(uint8_t roomID, bool redraw_all);

#endif