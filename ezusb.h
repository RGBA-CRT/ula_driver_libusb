#ifndef _EZUSB_H
#define _EZUSB_H

#include <libusb-1.0/libusb.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define LIBUSB_PRINT_ERR(cause, status)                                        \
  {                                                                            \
    fprintf(stderr, cause " failed: (%d)%s\n", status,                         \
            libusb_error_name(status));                                        \
  }

#define CHECK_ERR_FUNC(cause, status)                                          \
  {                                                                            \
    if (status < 0) {                                                          \
      LIBUSB_PRINT_ERR(cause, status);                                         \
      return status;                                                           \
    }                                                                          \
  }

#define EZUSB_VENDOR_REQEST_FW_LOAD 0xA0
#define AN21XX_CPUCS_ADDR 0x7F92
#define AN21XX_CPUCS_RUN 0x00
#define AN21XX_CPUCS_RESET_HALT 0x01
#define EZUSB_FWDL_BLOCK_SIZE 512
#define EZUSB_FWDL_ADDRESS_OFFSET 0

#define LIBUSB_TIMEOUT 99000

int ezusb_open(libusb_device_handle **device, uint16_t vid, uint16_t pid);
int ezusb_close(libusb_device_handle *device);

int ezusb_firmware_write(libusb_device_handle *device, uint16_t address,
                         uint8_t *data, size_t len);

int ezusb_cpucs(libusb_device_handle *device, uint8_t cpucs);

int ezusb_firmware_download(uint16_t vid, uint16_t pid, uint8_t *fw,
                            size_t len);

#endif