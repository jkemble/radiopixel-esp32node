/*
RadioPixel esp32 node

Copyright (c) 2023 Jonathan Kemble
*/
#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "patterns/Sequence.h"
#include "button_task.h"
#include "controller_task.h"
#include "playback_task.h"
#include "esp_strip.h"


#define RADIOPIXEL2_2 1

#if defined( DEVKIT)

const auto LED_GPIO = GPIO_NUM_8;
const int LED_COUNT = 60;
const int LED_REFRESH_RATE = 40; // Hz
const int LED_MAX_INTENSITY = 192;

const auto BUTTON_1_GPIO = GPIO_NUM_9;
const auto BUTTON_LONGPRESS_MS = 1500;

const uint8_t WIFI_CHANNEL = 6;

#elif defined(RADIOPIXEL2_0)

const auto LED_GPIO = GPIO_NUM_7;
const int LED_COUNT = 60;
const int LED_REFRESH_RATE = 40; // Hz
const int LED_MAX_INTENSITY = 128;

const auto BUTTON_1_GPIO = GPIO_NUM_2;
const auto BUTTON_LONGPRESS_MS = 1500;

const uint8_t WIFI_CHANNEL = 6;

#elif defined(RADIOPIXEL2_2)

const auto LED_GPIO = GPIO_NUM_7;
const int LED_COUNT = 60;
const int LED_REFRESH_RATE = 40; // Hz
const int LED_MAX_INTENSITY = 128;

const auto BUTTON_1_GPIO = GPIO_NUM_1;
const auto BUTTON_2_GPIO = GPIO_NUM_2;
const auto BUTTON_LONGPRESS_MS = 1500;

const uint8_t WIFI_CHANNEL = 6;

#endif

/*
TODO
[x] playback keep prioritized set of playbacks, play highest priority
[x] playback remove control on release
[x] when sequence is complete, return step -1 to release the sequence
[x] when advancing ordered sequence, jump to next step with a manual trigger (zero wait time)
[x] mechanism to determine master vs independent mode
[ ] use received RSSI?
*/

static void app_wifi_init()
{
    esp_event_loop_create_default();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE));
}

QueueHandle_t _playbackQueue;

void OnDataRecv(const esp_now_recv_info_t * esp_now_info, const uint8_t *data, int data_len) 
{
    auto src_addr(esp_now_info->src_addr);
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
        src_addr[0], src_addr[1], src_addr[2], src_addr[3], src_addr[4], src_addr[5]);
    ESP_LOGI("radiopixel", "recv from: %s, size %d", macStr, data_len);
    if(data_len == sizeof(PlaybackEvent))
    {
        xQueueSendToBack(_playbackQueue, data, 0);
    }
}    

extern "C" void app_main(void)
{
    // Dan said do this .. ?
    gpio_set_level(GPIO_NUM_6, 1);

    // the event queues
    QueueHandle_t buttonQueue = xQueueCreate(10, sizeof( ButtonEvent ));
    QueueHandle_t playbackQueue = xQueueCreate(10, sizeof( PlaybackEvent ));

    // start the button task
    ButtonConfig button_1_cfg{BUTTON_1_GPIO, 0, BUTTON_LONGPRESS_MS, 1, buttonQueue};
    TaskHandle_t button_task;
    xTaskCreate(ButtonTask, "buttons", 32*1024, &button_1_cfg, 5, &button_task);

    // start the local control task
    bool master(gpio_get_level(button_1_cfg.gpio) == button_1_cfg.pressed_level);
    ControllerConfig local{master, buttonQueue, playbackQueue};
    TaskHandle_t local_task;
    xTaskCreate(ControllerTask, "local", 32*1024, &local, 5, &local_task);

    // setup wifi
    nvs_flash_init();
    app_wifi_init();
    esp_now_init();
    esp_now_peer_info_t broadcast;
    memset(&broadcast, 0, sizeof(broadcast));
    for (auto& byte : broadcast.peer_addr)
    {
        byte = 0xff;
    }
    //broadcast.channel = WIFI_CHANNEL;
    esp_now_add_peer(&broadcast);
    _playbackQueue = playbackQueue;
    esp_now_register_recv_cb(OnDataRecv);

    // start the playback task
    auto strip(new EspStrip(LED_GPIO, LED_COUNT));
    PlaybackConfig playbackConfig{playbackQueue, strip, LED_REFRESH_RATE, LED_MAX_INTENSITY};
    TaskHandle_t playback_task;
    xTaskCreate(PlaybackTask, "playback", 32*1024, &playbackConfig, 5, &playback_task);

    // put idle step in the background
    PlaybackEvent idle{PlaybackEvent::Source::Background, 
        PlaybackEvent::Command::Play, Pattern::idleStep().command};
    xQueueSendToBack(playbackQueue, &idle, 0);

    ESP_LOGI("main", "started, master: %s", master?"true":"false");
}
