#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <stdio.h>
#include "driver/i2c_master.h"
#include "tdeck-lib.h"

const static char *TAG = "TDECKLIB";

esp_err_t td_board_spi_init(td_board_t *Board){
  spi_bus_config_t bus_config = {
    .miso_io_num = BOARD_SPI_MISO_PIN,
     .mosi_io_num = BOARD_SPI_MOSI_PIN,
     .sclk_io_num = BOARD_SPI_SCK_PIN,
     .quadwp_io_num = -1,
     .quadhd_io_num = -1,
     .max_transfer_sz = (BOARD_DISPLAY_WIDTH*BOARD_DISPLAY_HEIGHT*2)
  };
  return spi_bus_initialize(BOARD_DISPLAY_HOST, &bus_config, SPI_DMA_CH_AUTO);
}

esp_err_t td_board_i2c_init(td_board_t *Board){
  i2c_master_bus_config_t bus_config = {
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .i2c_port = BOARD_I2C,
      .scl_io_num = BOARD_I2C_SCL_PIN,
      .sda_io_num = BOARD_I2C_SDA_PIN,
      .glitch_ignore_cnt = 7,
  };
  return i2c_new_master_bus(&bus_config, &Board->proto.i2c.host);
}

esp_err_t td_board_init(td_board_t **Board) {
  *Board = (td_board_t *)malloc(sizeof(td_board_t));
  if(Board == NULL){
    return ESP_ERR_NO_MEM;
  }

  ESP_ERROR_CHECK(gpio_set_direction(BOARD_POWER_PIN, GPIO_MODE_OUTPUT));
  ESP_ERROR_CHECK(gpio_set_level(BOARD_POWER_PIN, 1));

  ESP_ERROR_CHECK(gpio_set_direction(BOARD_EN_PIN, GPIO_MODE_OUTPUT));
  ESP_ERROR_CHECK(gpio_set_level(BOARD_EN_PIN, 1));

  ESP_ERROR_CHECK(td_battery_init(*Board));
  ESP_ERROR_CHECK(td_board_spi_init(*Board));
  ESP_ERROR_CHECK(td_display_init(*Board));
  ESP_ERROR_CHECK(td_board_i2c_init(*Board));
  ESP_ERROR_CHECK(td_keyboard_init(*Board));
  ESP_ERROR_CHECK(td_speaker_init(*Board));
  ESP_ERROR_CHECK(td_sdcard_init(*Board));
  gpio_install_isr_service(0);
  ESP_ERROR_CHECK(td_trackball_init(*Board));

  return ESP_OK;
}
