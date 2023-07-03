/**
 * @file usrcmd.c
 * @author CuBeatSystems
 * @author Shinichiro Nakamura
 * @copyright
 * ===============================================================
 * Natural Tiny Shell (NT-Shell) Version 0.3.1
 * ===============================================================
 * Copyright (c) 2010-2016 Shinichiro Nakamura
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "ntopt.h"
#include "ntlibc.h"

#include <stdio.h>
#include "main.h"
#include "host_if_spi.h"
#include <stdlib.h>	// atoi

#define uart_puts printf

typedef int (*USRCMDFUNC)(int argc, char **argv);

static int usrcmd_ntopt_callback(int argc, char **argv, void *extobj);
static int usrcmd_help(int argc, char **argv);
static int usrcmd_info(int argc, char **argv);
static int read_user_button_b1(int argc, char **argv);
static int write_led_ld2(int argc, char **argv);
static int usrcmd_hostif_get_bufsize(int argc, char **argv);
static int usrcmd_hostif_get_version(int argc, char **argv);
static int usrcmd_hostif_echo10(int argc, char **argv);

typedef struct {
    char *cmd;
    char *desc;
    USRCMDFUNC func;
} cmd_table_t;

static const cmd_table_t cmdlist[] = {
    { "help", "This is a description text string for help command.", usrcmd_help },
    { "info", "This is a description text string for info command.", usrcmd_info },
    { "read_user_button", "User button B1 reads.", read_user_button_b1 },
    { "write_led", "Write to LED LD2.", write_led_ld2 },
    { "hostif_get_bufsize",  "hostif_get_bufsize [buffer ID = 0..31]\r\nex) hostif_get_bufsize 0", usrcmd_hostif_get_bufsize },
    { "hostif_get_version",  "hostif_get_version", usrcmd_hostif_get_version },
    { "hostif_echo10",  "hostif send, recv echo * 10 count", usrcmd_hostif_echo10 },
};

enum {
  COMMAND_HELP,
  COMMAND_INFO,
  COMMAND_READ_USER_BUTTON,
  COMMAND_WRITE_LED,
  COMMAND_HOSTIF_GET_BUFSIZE,
  COMMAND_HOSTIF_GET_VERSION,
  COMMAND_HOSTIF_ECHO10,
  COMMAND_MAX
};


int usrcmd_execute(const char *text)
{
    return ntopt_parse(text, usrcmd_ntopt_callback, 0);
}

static int usrcmd_ntopt_callback(int argc, char **argv, void *extobj)
{
    if (argc == 0) {
        return 0;
    }
    const cmd_table_t *p = &cmdlist[0];
    for (int i = 0; i < sizeof(cmdlist) / sizeof(cmdlist[0]); i++) {
        if (ntlibc_strcmp((const char *)argv[0], p->cmd) == 0) {
            return p->func(argc, argv);
        }
        p++;
    }
    uart_puts("Unknown command found.\r\n");
    return 0;
}

static int print_cmd_description(const cmd_table_t* msg)
{
  uart_puts(msg->cmd);
  uart_puts("\t:");
  uart_puts(msg->desc);
  uart_puts("\r\n");

  return 0;
}

static int usrcmd_help(int argc, char **argv)
{
    const cmd_table_t *p = &cmdlist[0];
    for (int i = 0; i < sizeof(cmdlist) / sizeof(cmdlist[0]); i++) {
        uart_puts(p->cmd);
        uart_puts("\t:");
        uart_puts(p->desc);
        uart_puts("\r\n");
        p++;
    }
    return 0;
}

static int usrcmd_info(int argc, char **argv)
{
    if (argc != 2) {
        uart_puts("info sys\r\n");
        uart_puts("info ver\r\n");
        return 0;
    }
    if (ntlibc_strcmp(argv[1], "sys") == 0) {
        uart_puts("ST NUCLEO-F446RE Monitor\r\n");
        return 0;
    }
    if (ntlibc_strcmp(argv[1], "ver") == 0) {
        uart_puts("Version 0.0.0\r\n");
        return 0;
    }
    uart_puts("Unknown sub command found\r\n");
    return -1;
}

static int read_user_button_b1(int argc, char **argv){
    if (argc != 1) {
        uart_puts("read_user_button\r\n");
        return -1;
    }
	GPIO_PinState b1_pin = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
	uart_puts("USER BUTTON B1 = %d\r\n", b1_pin);

	return 0;
}

static int write_led_ld2(int argc, char **argv){
    if (argc != 2) {
        uart_puts("write_led_ld2 1 or write_led_ld2 0\r\n");
        return -1;
    }
    GPIO_PinState PinState = GPIO_PIN_RESET;

    if (ntlibc_strcmp(argv[1], "1") == 0) PinState = GPIO_PIN_SET;
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, PinState);

	return 0;
}

static int usrcmd_hostif_get_bufsize(int argc, char **argv)
{
  uint8_t bufid = 0;
  size_t buf_size;

  if (argc != 2) {
    print_cmd_description(&cmdlist[COMMAND_HOSTIF_GET_BUFSIZE]);
    return -1;
  }

  bufid = atoi(argv[1]);
  if (!(0 <= bufid && bufid <= 31)) {
    uart_puts("Error: Buffer ID is 0..31.\r\n");
    return -1;
  }

  if (hostif_get_bufsize(bufid, &buf_size) != 0) {
    uart_puts("Error: status.\r\n");
    return -1;
  }

  uart_puts("bufid[%d] size = %d\r\n", bufid, buf_size);

  return 0;
}

static int usrcmd_hostif_get_version(int argc, char **argv)
{
  uint8_t bufid = 2;
  size_t buf_size;
  size_t malloc_len;
  uint8_t* buffer;

  if (argc != 1) {
    print_cmd_description(&cmdlist[COMMAND_HOSTIF_GET_VERSION]);
    return -1;
  }

  if (hostif_get_bufsize(bufid, &buf_size) != 0) {
    uart_puts("Error: status.\r\n");
    return -1;
  }

  if (buf_size <= 0) {
    uart_puts("Error: failed to get the size of buffer.\r\n");
    return -1;
  }

  // Allocate memory with 3 byte of 2 dummy and status
  malloc_len = buf_size + 3;

  buffer = (uint8_t*)malloc(malloc_len);
  if (!buffer) {
    uart_puts("Error: failed to allocate memory.\r\n");
    return -1;
  }

  if (host_receive(bufid, buffer, malloc_len, true) == 0) {
    uart_puts("version=%s (buff size=%d)\r\n", (char*)&buffer[3], buf_size);
  }

  free(buffer);

  return 0;
}

int send_data(void) {
  static int base_data = 0;
  size_t send_data_len;
  size_t buffer_size;
  size_t data_size;
  uint8_t* buffer;
  int i;

  if (hostif_get_bufsize(0, &buffer_size) != 0) {
    uart_puts("Error: get_bufsize status.\r\n");
    return -1;
  }

  data_size = 16;
  send_data_len = data_size + 4;

  buffer = (uint8_t*)malloc(send_data_len);
  if (!buffer) {
    uart_puts("Error: failed to allocate memory.\r\n");
    return -1;
  }

  for (i = 0; i < data_size; i++) {
    buffer[i + 3] = (i + base_data) & 0xff;
  }

  if (host_send(0, buffer, send_data_len) == 0) {
      uart_puts("Send done.\r\n");
  }

  free(buffer);
  base_data++;

  return 0;
}


int receive_data(void) {
  size_t receive_data_len;
  size_t buffer_size;
  uint8_t* buffer;
  int i;
  char out[32];

  if (hostif_get_bufsize(1, &buffer_size) != 0) {
    uart_puts("Error: get_bufsize status.\r\n");
    return -1;
  }

  receive_data_len = buffer_size + 3;

  buffer = (uint8_t*)malloc(receive_data_len);
  if (!buffer) {
    uart_puts("Error: failed to allocate memory.\r\n");
    return -1;
  }

  if (host_receive(1, buffer, receive_data_len, false) == 0) {
    for (i = 3; i < receive_data_len; i++) {
      sprintf(out, " %02x", buffer[i]);
      uart_puts(out);
    }
    uart_puts("\r\n");
  }

  free(buffer);
  return 0;
}


static int usrcmd_hostif_echo10(int argc, char **argv) {
  int i;

  if (argc != 1) {
    print_cmd_description(&cmdlist[COMMAND_HOSTIF_ECHO10]);
    return -1;
  }

  for (i = 0; i < 10; i++) {
    // send
    if (send_data() != 0) {
      uart_puts("Error: send data failed.\r\n");
      return -1;
    }

    // Wait 10ms
    HAL_Delay(10);

    // receive
    if (receive_data() != 0) {
      uart_puts("Error: receive data failed.\r\n");
      return -1;
    }
  }

  return 0;
}
