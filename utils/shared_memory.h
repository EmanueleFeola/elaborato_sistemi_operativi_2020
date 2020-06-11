/// @file shared_memory.h
/// @brief Contiene la definizione delle funzioni
///         specifiche per la gestione della memoria condivisa.

#pragma once

int alloc_shared_memory(key_t shmKey, size_t size);
void *get_shared_memory(int shmid, int shmflg);
void free_shared_memory(void *ptr_sh);
void remove_shared_memory(int shmid);