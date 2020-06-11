/// @file print_utils.h
/// @brief Contiene la definizione delle funzioni di supporto per stampare
///        la matrice e le strutture dati definite nel progetto

#pragma once

#include <stdio.h>
#include "../inc/defines.h"

void printMatrix(int *board_ptr, int iteration);
void printMessage(Message msg, char *who, char *mode);
void printAck(Acknowledgment ack, char *who, char *mode);
void printAllMessageId(Message *messages, int nMessages);