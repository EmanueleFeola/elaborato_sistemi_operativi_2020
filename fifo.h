/// @file fifo.h
/// @brief Contiene la definizioni di variabili e
///         funzioni specifiche per la gestione delle FIFO.

#pragma once

void create_fifo(char *fifoPath);
int get_fifo(char *fifoPath, int flag);
void write_fifo(int fd, Message msg);