#pragma once

int alloc_shared_memory(key_t shmKey, size_t size);
void *get_shared_memory(int shmid, int shmflg);
void free_shared_memory(void *ptr_sh);
void remove_shared_memory(int shmid);

void write_ack(Acknowledgment *ptr, Acknowledgment ack);
void read_acks(Acknowledgment *ptr);
int acklist_contains(Acknowledgment *ptr, int message_id, pid_t pid_receiver);