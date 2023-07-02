#ifndef HOST_IF_SPI_H
#define HOST_IF_SPI_H
#include <stdbool.h>

int hostif_get_bufsize(uint8_t bufid, size_t* p_buf_size);
int host_receive(int bufid, uint8_t* buffer, size_t len, bool lock);
int host_send(int bufid, uint8_t* buffer, size_t len);

#endif
