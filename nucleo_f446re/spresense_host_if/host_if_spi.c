#include <main.h>
#include <stdbool.h> // bool

extern SPI_HandleTypeDef hspi2;

/* ICMD defnitions */
#define ICMD_AVAILABLE_SIZE(n)  (0x10 + (n))
#define ICMD_FIXLEN_TRANS(n)    (0x80 + (n))
#define ICMD_VARLEN_TRANS(n)    (0xa0 + (n))

int hostif_get_bufsize(uint8_t bufid, size_t* p_buf_size) {
    uint8_t tx_buf[5] = {0xff, 0xff, 0xff, 0xff, 0xff};
    uint8_t rx_buf[5] = {0xff, 0xff, 0xff, 0xff, 0xff};

    tx_buf[0] = ICMD_AVAILABLE_SIZE(bufid);
    if (HAL_SPI_TransmitReceive(&hspi2, tx_buf, rx_buf, sizeof(rx_buf), 1000) != HAL_OK) {
    	return -1;
    }

    if (rx_buf[2] != 0) {
      return -1;
    }

    *p_buf_size = rx_buf[3] | (rx_buf[4] << 8);

    return 0;
}


int host_receive(int bufid, uint8_t* buffer, size_t len, bool lock) {
  size_t tx_size = len - 3;

  buffer[0] = ICMD_VARLEN_TRANS(bufid);
  buffer[1] = tx_size & 0xff;
  buffer[2] = ((tx_size >> 8) & 0x3f);
  if (lock) buffer[2] |= 0x40;

//  spi5_write_and_read(buffer, len);

  if (buffer[2] != 0) {
    return -1;
  }

  return 0;
}


int host_send(int bufid, uint8_t* buffer, size_t len) {
  size_t send_data_size = len - 4;

  buffer[0] = ICMD_VARLEN_TRANS(bufid);
  buffer[1] = send_data_size & 0xff;
  buffer[2] = (send_data_size >> 8) & 0x3f;

//  spi5_write_and_read(buffer, len);

  if (buffer[2] != 0) {
    return -1;
  }

  return 0;
}
