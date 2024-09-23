#ifndef _TDECK_LIB_H_
#define _TDECK_LIB_H_

#include <board.h>
#include <battery.h>
#include <display.h>
#include <keyboard.h>
#include <trackball.h>
#include <speaker.h>
#include <sdcard.h>

esp_err_t td_board_init(td_board_t **Board, ...);

esp_err_t td_apply_board_option(td_board_t* Board, td_board_option_t *opt);
esp_err_t td_create_board_option(td_board_option_type_t type, int value, td_board_option_t* opt);

#endif
/* end of include guard:  _TDECK_LIB_H_ */
