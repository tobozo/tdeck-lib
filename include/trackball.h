#ifndef _TRACKBALL_H_
#define _TRACKBALL_H_

#include <esp_log.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

extern QueueHandle_t td_trackball_queue;

typedef enum {
  EVT_TRACKBALL_UP,
  EVT_TRACKBALL_DOWN,
  EVT_TRACKBALL_LEFT,
  EVT_TRACKBALL_RIGHT,
} td_trackball_evt_t ;

typedef struct td_trackball_t {
  void *test;
} td_trackball_t;


esp_err_t td_trackball_init(void *ctx);

#endif /* end of include guard: _TRACKBALL_H_ */
