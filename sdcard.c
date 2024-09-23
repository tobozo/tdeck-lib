#include <board.h>
#include <sdcard.h>

const static char *TAG = "TDECKSDCARD";

// courtesy of tobozo
// inspired by https://github.com/ByteWelder/Tactility/tree/main/boards/lilygo_tdeck

/**
 * Before we can initialize the sdcard's SPI communications, we have to set all
 * other SPI pins on the board high.
 * See https://github.com/espressif/esp-idf/issues/1597
 * See https://github.com/Xinyuan-LilyGO/T-Deck/blob/master/examples/UnitTest/UnitTest.ino
 * @return success result
 */
esp_err_t td_sdcard_init(void *ctx) {
    ESP_LOGD(TAG, "init");

    td_board_t* Board = (td_board_t*)ctx;
    td_sdcard_t* SDCard = &Board->SDCard;

    static gpio_config_t sd_gpio_config;

    sd_gpio_config.pin_bit_mask = BIT64(BOARD_SDCARD_CS_PIN) | BIT64(BOARD_RADIO_CS_PIN) | BIT64(BOARD_DISPLAY_CS_PIN);
    sd_gpio_config.mode         = GPIO_MODE_OUTPUT;
    sd_gpio_config.pull_up_en   = GPIO_PULLUP_DISABLE;
    sd_gpio_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    sd_gpio_config.intr_type    = GPIO_INTR_DISABLE;

    SDCard->gpio_config = &sd_gpio_config;


    if (gpio_config(&sd_gpio_config) != ESP_OK) {
        ESP_LOGE(TAG, "GPIO init failed");
        return ESP_FAIL;
    }

    if (gpio_set_level(BOARD_SDCARD_CS_PIN, 1) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set board CS pin high");
        return ESP_FAIL;
    }

    if (gpio_set_level(BOARD_RADIO_CS_PIN, 1) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set radio CS pin high");
        return ESP_FAIL;
    }

    if (gpio_set_level(BOARD_DISPLAY_CS_PIN, 1) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set TFT CS pin high");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "SDCard init OK");

    if( SDCard->behavior == SDCardAutoMount ) {
        ESP_LOGI(TAG, "Automounting filesystem");
        return td_sdcard_mount( ctx, SDCARD_MOUNT_POINT );
    }

    ESP_LOGE(TAG, "Behavior: %d", SDCard->behavior);

    return ESP_OK;
}


esp_err_t td_sdcard_mount(void *ctx, const char* mount_point)
{
    td_board_t* Board = (td_board_t*)ctx;
    td_sdcard_t* SDCard = &Board->SDCard;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 4,
        .allocation_unit_size = 16384,
        .disk_status_check_enable = false
    };

    // Init without card detect (CD) and write protect (WD)
    static sdspi_device_config_t sdcard_slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    SDCard->slot_config = &sdcard_slot_config;
    SDCard->slot_config->gpio_cs = BOARD_SDCARD_CS_PIN;
    SDCard->slot_config->host_id = BOARD_SPI;

    static sdmmc_host_t sdcard_host = SDSPI_HOST_DEFAULT();
    SDCard->host        = &sdcard_host;
    // The following value is from T-Deck repo's UnitTest.ino project:
    // https://github.com/Xinyuan-LilyGO/T-Deck/blob/master/examples/UnitTest/UnitTest.ino
    // Observation: Using this automatically sets the bus to 20MHz
    SDCard->host->max_freq_khz = 800000U;

    esp_err_t ret = esp_vfs_fat_sdspi_mount(mount_point, SDCard->host, SDCard->slot_config, &mount_config, &SDCard->card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Mounting failed. Ensure the card is formatted with FAT.");
        } else {
            ESP_LOGE(TAG, "Mounting failed (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    snprintf(SDCard->mount_point, 63, "%s", mount_point);

    ESP_LOGI(TAG, "Filesystem mounted");

    SDCard->mounted = true;

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, SDCard->card);

    // TODO: remove this block
    {
        // list the root directory
        DIR *rootdir = opendir(mount_point);
        struct dirent * dirent;
        while((dirent = readdir(rootdir)) != NULL) {
            ESP_LOGI(TAG, "Found %s, id %d, type %d", dirent->d_name, dirent->d_ino, dirent->d_type);
        }
        closedir(rootdir);
    }

    return ESP_OK;
}


esp_err_t td_sdcard_init_and_mount(void *ctx, const char* mount_point) {
    if (td_sdcard_init(ctx)!=ESP_OK) {
        ESP_LOGE(TAG, "Failed to set SPI CS pins high. This is a pre-requisite for mounting.");
        return ESP_FAIL;
    }
    return td_sdcard_mount(ctx, mount_point);
}


esp_err_t td_sdcard_unmount(void* ctx) {
    td_board_t* Board = (td_board_t*)ctx;
    td_sdcard_t* SDCard = &Board->SDCard;

    ESP_LOGI(TAG, "Unmounting %s", SDCard->mount_point);

    if (esp_vfs_fat_sdcard_unmount(SDCard->mount_point, SDCard->card) != ESP_OK) {
        ESP_LOGE(TAG, "Unmount failed for %s", SDCard->mount_point);
        return ESP_FAIL;
    }

    return ESP_OK;
}


// bool td_sdcard_is_mounted(void* ctx) {
//     td_board_t* Board = (td_board_t*)ctx;
//     td_sdcard_t* SDCard = &Board->SDCard;
//
//     if(SDCardMutex != NULL) {
//         if( ! xSemaphoreTake(SDCardMutex, 100) ) {
//             ESP_LOGE(TAG, "Failed to obtain lock");
//             return false;
//         }
//     }
//
//     bool result = (SDCard->card != NULL) && (sdmmc_get_status(SDCard->card) == ESP_OK);
//
//     if(SDCardMutex != NULL) {
//         xSemaphoreGive(SDCardMutex);
//     }
//
//     return result;
// }
