#include "ezusb.h"
#include "ula_driver.h"
#include <stdio.h>
#include <sys/stat.h>


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

void ezusb_firm_dl(char *filename, uint16_t vid, uint16_t pid) {
  struct stat stbuf;
  stat(filename, &stbuf);

  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    printf("fw read err\n");
    return;
  }

  uint8_t *fw = (uint8_t *)malloc(stbuf.st_size);
  fread(fw, 1, stbuf.st_size, file);
  fclose(file);

  if (!ula_ezusb_firmware_download(vid, pid, fw, stbuf.st_size)) {
    printf("%s dl err\n", filename);
    return;
  }
  free(fw);
}

int main(void) {
  int status;
  status = libusb_init(NULL);
  CHECK_ERR_MAIN("libusb_init", status);

  ula_enum_devices(&devenum_callback, 0);

  ezusb_firm_dl("gba_boot.bix", 0x0590, 0x000f);

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

  FILE* file = fopen("ula_gp_gba.mb.gba", "rb");
  if (file == NULL) {
    printf("gba fw read err\n");
    return -2;
  }
  uint8_t gbafw[9044];
  fread(gbafw, 1, sizeof(gbafw), file);
  fclose(file);
  if (!ula_gba_firmware_download(ula_handle, gbafw, sizeof(gbafw))) {
    printf("gba fw dl err\n");
    return -2;
  }
  ula_close(ula_handle);

  ezusb_firm_dl("ula_gp.bix", ula_vid, ula_pid);
}