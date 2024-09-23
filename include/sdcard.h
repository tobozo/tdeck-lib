#ifndef _SDCARD_H_
#define _SDCARD_H_

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include <dirent.h> // TODO: remove this after removing the dir-listing from td_sdcard_mount()

typedef enum td_sdcard_beharior_t {
    SDCardAutoMount,
    SDCardManualMount,
    SDCardNeverMount
} td_sdcard_beharior_t;


typedef struct td_sdcard_t {
    char mount_point[64];
    sdspi_device_config_t *slot_config;
    gpio_config_t *gpio_config;
    sdmmc_host_t *host;
    sdmmc_card_t* card;
    td_sdcard_beharior_t behavior;
    bool mounted;
} td_sdcard_t;


[[maybe_unused]] static SemaphoreHandle_t SDCardMutex = NULL;

esp_err_t td_sdcard_init(void *ctx);
esp_err_t td_sdcard_mount(void *ctx, const char* mount_point);
esp_err_t td_sdcard_init_and_mount(void *ctx, const char* mount_point);
esp_err_t td_sdcard_unmount(void *ctx);

//bool td_sdcard_is_mounted(void *ctx);

#endif
