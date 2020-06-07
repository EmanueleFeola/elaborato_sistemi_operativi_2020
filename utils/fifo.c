#include <sys/stat.h>
#include "../defines.h"
#include "err_exit.h"
#include "fifo.h"

void create_fifo(char *fifoPath){
    int res = mkfifo(fifoPath, S_IRUSR | S_IWUSR);
    
    if(res == -1)
        ErrExit("failed mkfifo fifo\n");
}

int get_fifo(char *fifoPath, int flag){
    int fd = open(fifoPath, flag);
    
    if(fd == -1)
        ErrExit("failed open fifo\n");
    
    return fd;
}

void write_fifo(int fd, Message msg){
    int bW = write(fd, &msg, sizeof(msg));
    if(bW == -1)
        ErrExit("failed write fifo");

    if(close(fd) == -1)
        ErrExit("failed close fifo");
}
