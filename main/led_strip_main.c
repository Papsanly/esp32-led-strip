#include "button_gpio.h"
#include "driver/rmt_tx.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h" // IWYU pragma: keep
#include "freertos/task.h"
#include "iot_button.h"
#include "led_strip_encoder.h"

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000
#define LED_BIRGHTNESS 0x02
#define RMT_LED_STRIP_GPIO_NUM 13
#define EXAMPLE_LED_NUMBERS 35
#define EXAMPLE_CHASE_SPEED_MS 100

static const char *TAG = "main";
static int current_led = 0;
static uint8_t led_strip_pixels[EXAMPLE_LED_NUMBERS * 3];

static void button_a_cb(void *arg, void *usr_data) {
  ESP_LOGI(TAG, "BUTTON A CLICKED");
  if (current_led == 0) {
    current_led = sizeof(led_strip_pixels) - 3;
  } else {
    current_led = (current_led - 3) % sizeof(led_strip_pixels);
  }
}

static void button_b_cb(void *arg, void *usr_data) {
  ESP_LOGI(TAG, "BUTTON B CLICKED");
  current_led = (current_led + 3) % sizeof(led_strip_pixels);
}

void app_main(void) {
  ESP_LOGI(TAG, "Create RMT TX channel");
  rmt_channel_handle_t led_chan = NULL;
  rmt_tx_channel_config_t tx_chan_config = {
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .gpio_num = RMT_LED_STRIP_GPIO_NUM,
      .mem_block_symbols = 64,
      .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
      .trans_queue_depth = 4,
  };
  ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

  ESP_LOGI(TAG, "Install led strip encoder");
  rmt_encoder_handle_t led_encoder = NULL;
  led_strip_encoder_config_t encoder_config = {
      .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
  };
  ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

  ESP_LOGI(TAG, "Enable RMT TX channel");
  ESP_ERROR_CHECK(rmt_enable(led_chan));

  rmt_transmit_config_t tx_config = {
      .loop_count = 0,
  };

  const button_config_t btn_cfg_a = {0};
  const button_gpio_config_t btn_gpio_cfg_a = {
      .gpio_num = 0,
      .active_level = 0,
  };
  button_handle_t gpio_btn_a = NULL;
  ESP_ERROR_CHECK(iot_button_new_gpio_device(&btn_cfg_a, &btn_gpio_cfg_a, &gpio_btn_a));
  iot_button_register_cb(gpio_btn_a, BUTTON_PRESS_DOWN, NULL, button_a_cb, NULL);

  const button_config_t btn_cfg_b = {0};
  const button_gpio_config_t btn_gpio_cfg_b = {
      .gpio_num = 4,
      .active_level = 0,
  };
  button_handle_t gpio_btn_b = NULL;
  ESP_ERROR_CHECK(iot_button_new_gpio_device(&btn_cfg_b, &btn_gpio_cfg_b, &gpio_btn_b));
  iot_button_register_cb(gpio_btn_b, BUTTON_PRESS_DOWN, NULL, button_b_cb, NULL);

  while (1) {
    led_strip_pixels[current_led] = LED_BIRGHTNESS;
    led_strip_pixels[current_led + 1] = LED_BIRGHTNESS;
    led_strip_pixels[current_led + 2] = LED_BIRGHTNESS;

    ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels),
                                 &tx_config));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));

    memset(led_strip_pixels, 0, sizeof(led_strip_pixels));
  }
}
