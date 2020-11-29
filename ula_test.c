#include "ezusb.h"
#include "ula_driver.h"
#include <stdio.h>

#define FWLIB_ROM_PROBE 0x01
#define FWLIB_WRAM_LOADEXEC 0x02
#define FWLIB_ROM_BERASE 0x03
#define FWLIB_ROM_WRITE 0x04
#define FWLIB_ROM_BWRITE 0x05
#define FWLIB_READ 0x06
#define FWLIB_WRITE 0x07
#define FWLIB_FIND 0x08
#define FWLIB_BOOT_ROM 0x09
#define FWLIB_SRAM2EWRAM 0x0a
#define FWLIB_EWRAM2SRAM 0x0b
#define FWLIB_BU_PROBE 0x0c
#define FWLIB_SRAM_READ 0x0d
#define FWLIB_EEP_READ 0x0e
#define FWLIB_EEP_WRITE 0x0f
#define FWLIB_SRAM_WRITE 0x10
#define FWLIB_FLASH_WRITE 0x11
#define FWLIB_BLANK 0x20
#define FWLIB_IS_PRO 0x21

#define CHECK_ERR_MAIN(cause, status)                                          \
  {                                                                            \
    if (status < 0) {                                                          \
      LIBUSB_PRINT_ERR(cause, status);                                         \
      return -1;                                                               \
    }                                                                          \
  }

void ULA_CALLTYPE devenum_callback(uint16_t vid, uint16_t pid, const char *desc,
                                   void *userdata) {
  printf("VID:%04X PID:%04X: %s\n", vid, pid, desc);
}

int main(void) {
  int status;
  status = libusb_init(NULL);
  CHECK_ERR_MAIN("libusb_init", status);

  ula_enum_devices(&devenum_callback, 0);

  FILE *file = fopen(
      "H:\\My-File\\Data\\Programs\\ActiveBasic\\ULA\\gba_boot.bix", "rb");
  //   "H:\\My-File\\Data\\Programs\\ActiveBasic\\ULA\\2020\\textimpl\\gba_boot_"
  //   "min.bix",
  //   "rb");
  if (file == NULL) {
    printf("fw read err\n");
    return -2;
  }
  uint8_t fw[0x2000];
  fread(fw, 1, sizeof(fw), file);
  fclose(file);

  uint16_t vid = 0x0590, pid = 0x000f;
  if (!ula_ezusb_firmware_download(vid, pid, fw, sizeof(fw))) {
    printf("fw dl err\n");
  }

  ula_sleep_ms(ULA_CMD_WAIT_MS);

  uint16_t ula_vid = 0x0547, ula_pid = 0x1002;
  ula_handle_t ula_handle;
  if (!ula_open(&ula_handle, ula_vid, ula_pid)) {
    printf("ula open err\n");
    return -1;
  }

  printf("Turn on GBA with START+SELECT!\n");
  if (!ula_gba_init(ula_handle)) {
    printf("ula init err\n");
    return -1;
  }

  file = fopen(
      "H:\\My-File\\Data\\Programs\\ActiveBasic\\ULA\\fwlib20030312.bin", "rb");
  if (file == NULL) {
    printf("gba fw read err\n");
    return -2;
  }
  uint8_t gbafw[0x2134];
  fread(gbafw, 1, sizeof(gbafw), file);
  fclose(file);
  if (!ula_gba_firmware_download(ula_handle, gbafw, sizeof(gbafw))) {
    printf("gba fw dl err\n");
    return -2;
  }

  ula_sleep_ms(ULA_CMD_WAIT_MS);

  ula_send_command(ula_handle, FWLIB_ROM_PROBE, 0, 0, 0);

  uint8_t data[512];
  int readlen;
  status = ula_data_in(ula_handle, data, 12, &readlen);
  LIBUSB_PRINT_ERR("bulk in", status);

#define GBA_CART_ROM_ADDRESS 0x08000000
  ula_send_command(ula_handle, FWLIB_READ, GBA_CART_ROM_ADDRESS, 512, 0);

  status = ula_data_in(ula_handle, data, sizeof(data), &readlen);
  LIBUSB_PRINT_ERR("bulk in", status);

  printf("readlen=%d\n", readlen);

  for (int i = 0; i < 512; i++)
    printf("[%02X %c] ", data[i],
           data[i] >= 0x20 ? (data[i] > 0x7f ? '.' : data[i]) : '.');

  ula_close(ula_handle);
}