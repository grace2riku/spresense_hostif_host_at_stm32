#include <main.h>
#include <stdio.h> // printf
#include <stdbool.h> // bool
#include <stdlib.h> // malloc

extern SPI_HandleTypeDef hspi2;

/* ICMD defnitions */
#define ICMD_AVAILABLE_SIZE(n)  (0x10 + (n))
#define ICMD_FIXLEN_TRANS(n)    (0x80 + (n))
#define ICMD_VARLEN_TRANS(n)    (0xa0 + (n))

int hostif_get_bufsize(uint8_t bufid, size_t* p_buf_size) {
    uint8_t tx_buf[5] = {0xff, 0xff, 0xff, 0xff, 0xff};
    uint8_t rx_buf[5] = {0xff, 0xff, 0xff, 0xff, 0xff};

    tx_buf[0] = ICMD_AVAILABLE_SIZE(bufid);
	HAL_GPIO_WritePin(CS_HOSTIF_GPIO_Port, CS_HOSTIF_Pin, GPIO_PIN_RESET);
	if (HAL_SPI_TransmitReceive(&hspi2, tx_buf, rx_buf, sizeof(rx_buf), 1000) != HAL_OK) {
		HAL_GPIO_WritePin(CS_HOSTIF_GPIO_Port, CS_HOSTIF_Pin, GPIO_PIN_SET);
    	return -1;
    }
	HAL_GPIO_WritePin(CS_HOSTIF_GPIO_Port, CS_HOSTIF_Pin, GPIO_PIN_SET);

    if (rx_buf[2] != 0) {
      return -1;
    }

    *p_buf_size = rx_buf[3] | (rx_buf[4] << 8);

    return 0;
}


int host_receive(int bufid, uint8_t* buffer, size_t len, bool lock) {
  uint8_t* txbuf;
  size_t tx_size = len - 3;

  // Allocate memory for tx buffer */
  txbuf = (uint8_t*)malloc(len);
  if (!txbuf) {
	  printf("Error: failed to allocate memory.\r\n");
	  return -1;
  }

  txbuf[0] = ICMD_VARLEN_TRANS(bufid);
  txbuf[1] = tx_size & 0xff;
  txbuf[2] = ((tx_size >> 8) & 0x3f);
  if (lock) txbuf[2] |= 0x40;

  HAL_GPIO_WritePin(CS_HOSTIF_GPIO_Port, CS_HOSTIF_Pin, GPIO_PIN_RESET);
  if (HAL_SPI_TransmitReceive(&hspi2, txbuf, buffer, len, 1000) != HAL_OK) {
	HAL_GPIO_WritePin(CS_HOSTIF_GPIO_Port, CS_HOSTIF_Pin, GPIO_PIN_SET);
	free(txbuf);
  	return -1;
  }
  HAL_GPIO_WritePin(CS_HOSTIF_GPIO_Port, CS_HOSTIF_Pin, GPIO_PIN_SET);

  if (buffer[2] != 0) {
	free(txbuf);
    return -1;
  }

  free(txbuf);

  return 0;
}


int host_send(int bufid, uint8_t* buffer, size_t len) {
  size_t send_data_size = len - 4;
  uint8_t* rxbuf;

  /* Allocate memory for rx buffer */
  rxbuf = (uint8_t*)malloc(len);
  if (!rxbuf) {
	  printf("Error: failed to allocate memory.\r\n");
	  return -1;
  }

  buffer[0] = ICMD_VARLEN_TRANS(bufid);
  buffer[1] = send_data_size & 0xff;
  buffer[2] = (send_data_size >> 8) & 0x3f;

  HAL_GPIO_WritePin(CS_HOSTIF_GPIO_Port, CS_HOSTIF_Pin, GPIO_PIN_RESET);
  if (HAL_SPI_TransmitReceive(&hspi2, buffer, rxbuf, len, 1000) != HAL_OK) {
	HAL_GPIO_WritePin(CS_HOSTIF_GPIO_Port, CS_HOSTIF_Pin, GPIO_PIN_SET);
	free(rxbuf);
  	return -1;
  }
  HAL_GPIO_WritePin(CS_HOSTIF_GPIO_Port, CS_HOSTIF_Pin, GPIO_PIN_SET);

  if (rxbuf[2] != 0) {
	free(rxbuf);
    return -1;
  }

  free(rxbuf);
  return 0;
}
