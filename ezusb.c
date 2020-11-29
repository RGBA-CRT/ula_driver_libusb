#include "ezusb.h"
#include <string.h>

int ezusb_open(libusb_device_handle **device, uint16_t vid, uint16_t pid) {
  *device = NULL;

  libusb_device_handle *handle;
  handle = libusb_open_device_with_vid_pid(NULL, vid, pid);
  if (handle == NULL) {
    return LIBUSB_ERROR_OTHER;
  }

  int status;
  libusb_set_auto_detach_kernel_driver(handle, 1);
  status = libusb_claim_interface(handle, 0);
  if (status < 0)
    goto EO_ERR_ERTURN;

  *device = handle;
  return LIBUSB_SUCCESS;

EO_ERR_ERTURN:
  libusb_close(handle);
  return status;
}

int ezusb_close(libusb_device_handle *device) {
  libusb_release_interface(device, 0);
  libusb_close(device);
  libusb_exit(NULL);
  return LIBUSB_SUCCESS;
}

int ezusb_firmware_write(libusb_device_handle *device, uint16_t address,
                         uint8_t *data, size_t len) {
  int status;

  status = libusb_control_transfer(device, LIBUSB_ENDPOINT_OUT |
                                               LIBUSB_REQUEST_TYPE_VENDOR |
                                               LIBUSB_RECIPIENT_DEVICE,
                                   EZUSB_VENDOR_REQEST_FW_LOAD, address, 0x00,
                                   data, (uint16_t)len, LIBUSB_TIMEOUT);

  if (status < 0) {
    fprintf(stderr,
            "note: ezusb_firmware_write: adr:%04X data:%02X ... len:%d\n",
            address, data[0], len);
  }

  return status;
}

int ezusb_cpucs(libusb_device_handle *device, uint8_t cpucs) {
  int status =
      ezusb_firmware_write(device, AN21XX_CPUCS_ADDR, &cpucs, sizeof(cpucs));

  // リセットされるのでエラーになる
  if (cpucs == AN21XX_CPUCS_RUN)
    status = 0;
  return status;
}

int ezusb_firmware_download(uint16_t vid, uint16_t pid, uint8_t *fw,
                            size_t len) {
  int status;
  libusb_device_handle *handle;

  status = ezusb_open(&handle, vid, pid);
  CHECK_ERR_FUNC("[fwdl] ezusb_open", status);

  status = ezusb_cpucs(handle, AN21XX_CPUCS_RESET_HALT);
  CHECK_ERR_FUNC("[fwdl] AN21XX_CPUCS_RESET_HALT", status);

  for (int i = 0; i < len / EZUSB_FWDL_BLOCK_SIZE; i++) {
    uint16_t offset = i * EZUSB_FWDL_BLOCK_SIZE;
    uint16_t address = EZUSB_FWDL_ADDRESS_OFFSET + offset;
    uint16_t transfer_len = EZUSB_FWDL_BLOCK_SIZE;

    // printf("fw dl: %04X-%04X len:%d\n", address, address + transfer_len - 1,
    //        transfer_len);

    status = ezusb_firmware_write(handle, address, fw + offset, transfer_len);
    CHECK_ERR_FUNC("[fwdl] ezusb_firmware_write", status);
  }

  status = ezusb_cpucs(handle, AN21XX_CPUCS_RUN);
  CHECK_ERR_FUNC("[fwdl] AN21XX_CPUCS_RUN", status);

  status = ezusb_close(handle);
  CHECK_ERR_FUNC("[fwdl] ezusb_close", status);
}