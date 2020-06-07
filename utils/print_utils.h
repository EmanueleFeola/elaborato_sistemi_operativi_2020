#pragma once

#include <stdio.h>
#include "../defines.h"

void printMatrix(int *board_ptr, int iteration);
void printMessage(Message msg, char *who, char *mode);
void printAck(Acknowledgment ack, char *who, char *mode);