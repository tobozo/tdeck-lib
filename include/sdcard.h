#ifndef _SDCARD_H_
#define _SDCARD_H_

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

#include <dirent.h>

typedef struct td_mountdata_t {
    const char* mount_point;
    sdmmc_card_t* card;
} td_mountdata_t;


typedef struct td_sdcard_t {
    const char mount_point[64];
    sdspi_device_config_t *slot_config;
    gpio_config_t *gpio_config;
    sdmmc_host_t *host;
    sdmmc_card_t* card;
    bool mounted;
} td_sdcard_t;



#define SDCARD_MOUNT_POINT "/sdcard"


esp_err_t td_sdcard_init(void *ctx);

esp_err_t sdcard_init(void *ctx);
esp_err_t sdcard_mount(void *ctx, const char* mount_point);
void* sdcard_mount_impl(void *ctx, const char* mount_point);
void* sdcard_init_and_mount(void *ctx, const char* mount_point);
void sdcard_unmount(void *ctx);
bool sdcard_is_mounted(void *ctx);




#endif
