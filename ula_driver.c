#include "ula_driver_internal.h"
#include "ezusb.h"

#define CHECK_ERR_VOID(cause, status)                                          \
  {                                                                            \
    if (status < 0) {                                                          \
      LIBUSB_PRINT_ERR(cause, status);                                         \
      return;                                                                  \
    }                                                                          \
  }

#define CHECK_ERR_BOOL(cause, status)                                          \
  {                                                                            \
    if (status < 0) {                                                          \
      LIBUSB_PRINT_ERR(cause, status);                                         \
      return false;                                                            \
    }                                                                          \
  }

bool check_libusb_init() {
  // ちゃんとするならちゃんとしたい
  if (libusb_init(NULL) < 0)
    return false;

  return true;
}

DllPublic bool ULA_CALLTYPE ula_open(ula_handle_t *device, uint16_t vid, uint16_t pid) {
  if (!check_libusb_init())
    return false;

  *device = (ula_handle_t)malloc(sizeof(ula_handle_internal));

  (*device)->bluk_in_endpoint = ULA_BULK_IN_EP;
  (*device)->bluk_out_endpoint = ULA_BULK_OUT_EP;

  return (ezusb_open(&((*device)->libusb_handle), vid, pid) < 0) ? false : true;
}

DllPublic bool ULA_CALLTYPE ula_close(ula_handle_t device) {
  ezusb_close(device->libusb_handle);
  free(device);
  return true;
}

DllPublic bool ULA_CALLTYPE ula_set_endpoint(ula_handle_t device, uint8_t bluk_in,
                                   uint8_t bluk_out) {
  device->bluk_in_endpoint = bluk_in;
  device->bluk_out_endpoint = bluk_out;
  return true;
}

DllPublic bool ULA_CALLTYPE ula_ezusb_firmware_download(uint16_t vid, uint16_t pid,
                                              uint8_t *fw, size_t len) {
  if (!check_libusb_init())
    return false;

  bool ret = (ezusb_firmware_download(vid, pid, fw, len) < 0) ? false : true;

  return ret;
}

DllPublic bool ULA_CALLTYPE ula_send_command(ula_handle_t device, uint8_t command,
                                   uint32_t param1, uint32_t param2,
                                   uint32_t param3) {
  uint8_t cmdbuf[13];
  cmdbuf[0] = command;
  cmdbuf[1] = (param1)&0xFF;
  cmdbuf[2] = (param1 >> 8) & 0xFF;
  cmdbuf[3] = (param1 >> 16) & 0xFF;
  cmdbuf[4] = (param1 >> 24) & 0xFF;

  cmdbuf[5] = (param2)&0xFF;
  cmdbuf[6] = (param2 >> 8) & 0xFF;
  cmdbuf[7] = (param2 >> 16) & 0xFF;
  cmdbuf[8] = (param2 >> 24) & 0xFF;

  cmdbuf[9] = (param3)&0xFF;
  cmdbuf[10] = (param3 >> 8) & 0xFF;
  cmdbuf[11] = (param3 >> 16) & 0xFF;
  cmdbuf[12] = (param3 >> 24) & 0xFF;

  int status;
  status = ula_data_out(device, cmdbuf, sizeof(cmdbuf));

  //ula_sleep_ms(ULA_CMD_WAIT_MS);

  if (status < 0) {
    fprintf(stderr, "ula_send_command failed cmd: %X, (%d)%s\n", command,
            status, libusb_error_name(status));
    return false;
  }

  return true;
}

DllPublic bool ULA_CALLTYPE ula_gba_init(ula_handle_t device) {
  bool ret = ula_send_command(device, ULA_EZFW_TARGET_INIT, 0, 0, 0);
  ula_sleep_ms(ULA_CMD_WAIT_MS);
  return ret;
}

DllPublic bool ULA_CALLTYPE ula_gba_firmware_download(ula_handle_t device,
                                            uint8_t *gba_fw, size_t len) {
  if (!ula_send_command(device, ULA_EZFW_TARGET_DOWNLOAD, 0x02010000, len, 0)) {
    printf("ULA_EZFW_TARGET_DOWNLOAD failed\n");
    return false;
  }

  int status;
#define GBA_FWDL_BLOCK_SIZE 1024
  for (int i = 0; i <= (len / GBA_FWDL_BLOCK_SIZE); i++) {
    int tr_size = GBA_FWDL_BLOCK_SIZE;
    int offset = i * GBA_FWDL_BLOCK_SIZE;
    status = ula_data_out(device, gba_fw + offset, tr_size);
    if (status < 0) {
      LIBUSB_PRINT_ERR("ula_data_out", status);
      return false;
    }
  }

  return true;
}

DllPublic void ULA_CALLTYPE ula_sleep_ms(uint32_t ms) {
#ifdef _WIN32
  Sleep(ms);
#else
  usleep(ms * 1000);
#endif
}

DllPublic bool ULA_CALLTYPE ula_data_out(ula_handle_t device, uint8_t *data, size_t len) {
  int ret =
      libusb_bulk_transfer(device->libusb_handle, device->bluk_out_endpoint,
                           data, (int)len, NULL, LIBUSB_TIMEOUT);

  if (ret < 0)
    LIBUSB_PRINT_ERR("ula_data_out", ret);

  return (ret < 0) ? false : true;
}

DllPublic bool ULA_CALLTYPE ula_data_in(ula_handle_t device, uint8_t *data, size_t len,
                              size_t *readsize) {
  int _readsize;
  int ret =
      libusb_bulk_transfer(device->libusb_handle, device->bluk_in_endpoint,
                           data, (int)len, &_readsize, LIBUSB_TIMEOUT);

  if (ret < 0)
    LIBUSB_PRINT_ERR("ula_data_in", ret);

  if (readsize)
    *readsize = (size_t)_readsize;
  return (ret < 0) ? false : true;
}

// ==========================================================================

DllPublic void ULA_CALLTYPE ula_enum_devices(ula_enum_devices_callback_t callback,
                                   void *userdata) {
  if (!check_libusb_init())
    return;

  int status;
  libusb_device **device_list;
  status = libusb_get_device_list(NULL, &device_list);
  CHECK_ERR_VOID("libusb_get_device_list", status);

  libusb_device *dev = NULL;
  for (int i = 0; (dev = device_list[i]) != NULL; i++) {

    struct libusb_device_descriptor desc;
    status = libusb_get_device_descriptor(dev, &desc);
    CHECK_ERR_VOID("libusb_get_device_descriptor", status);

    libusb_device_handle *handle;
    status = libusb_open(dev, &handle);
    if (status >= 0) {

      char manufacture[STRING_DESCRIPTOR_LEN] = {0},
           product[STRING_DESCRIPTOR_LEN] = {0};
      status = libusb_get_string_descriptor_ascii(
          handle, desc.iManufacturer, manufacture, STRING_DESCRIPTOR_LEN);
      if (status < 0) {
        strcpy_s(manufacture, sizeof(manufacture), "?");
      }

      status = libusb_get_string_descriptor_ascii(
          handle, desc.iProduct, product, STRING_DESCRIPTOR_LEN);
      if (status < 0) {
          strcpy_s(product, sizeof(product), "");
      }

      char desc_text[128];
      sprintf_s(desc_text, sizeof(desc_text), "%s %s", manufacture, product);
      callback(desc.idVendor, desc.idProduct, desc_text, userdata);

      libusb_close(handle);
    } else {
      callback(desc.idVendor, desc.idProduct, "?", userdata);
    }
  }
}