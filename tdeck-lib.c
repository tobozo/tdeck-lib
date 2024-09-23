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

esp_err_t td_board_init(td_board_t **Board, ...) {
  *Board = (td_board_t *)malloc(sizeof(td_board_t));

  if(Board == NULL){
    return ESP_ERR_NO_MEM;
  }

  va_list ptr;
  va_start(ptr, **Board);
  for(;;) {
    td_board_option_t opt = va_arg(ptr, td_board_option_t);
    if( td_apply_board_option(*Board, &opt) != ESP_OK) break;
    ESP_LOGI(TAG, "Enabled Option (%d=%d)", opt.option, opt.value);
  }
  va_end(ptr);

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

  ESP_LOGI(TAG, "TBoard init OK");

  return ESP_OK;
}


esp_err_t td_apply_board_option(td_board_t* Board, td_board_option_t *opt) {
  if( opt==NULL ) return ESP_FAIL;
  switch(opt->option) {
    case OPTSDCard:
      // TODO: validate value
      Board->SDCard.behavior = opt->value;
      break;
    // TODO: implement more
    // case OPTKeyboard:
    //   Board->Keyboard.layout = opt->value;
    //   break;
    default:
      return ESP_FAIL;
  }
  return ESP_OK;
}

esp_err_t td_create_board_option(td_board_option_type_t type, int value, td_board_option_t* opt) {
  switch(type) {
    case OPTSDCard:
    case OPTKeyboard:
    case OPTBattery:
    case OPTDisplay:
    case OPTTrackball:
    case OPTSpeaker:
    case OPTRadio:
      opt->option   = type;
      opt->value    = value;
      opt->checksum = type+value;
      // ESP_LOGI(TAG, "Created board option#%d=%d (checksum=%d)", opt->option, opt->value, opt->checksum);
      break;
    default:
      return ESP_FAIL;
  }
  return ESP_OK;
}
