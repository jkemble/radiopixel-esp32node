#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "button_task.h"


struct ButtonState
{
    bool pressed = false;
    size_t ticksPressed = 0;
};

void ButtonTask(/*ButtonConfig*/void *_config)
{
    // make a copy of the config
    ButtonConfig config = *static_cast<ButtonConfig *>(_config);
    ESP_LOGI("button", "ButtonTask start GPIO %d", config.gpio);

    // setup the GPIO
    gpio_set_direction(config.gpio, GPIO_MODE_INPUT);

    // setup the state
    ButtonState state;
    state.pressed = gpio_get_level(config.gpio) == config.pressed_level;

    // process the button
    const size_t waitTicks = 1; // the tick count wait time, 1 tick = 10ms
    const size_t debounceTickCount = pdMS_TO_TICKS(10);
    const size_t longTickCount = pdMS_TO_TICKS(config.long_press_duration);
    while (1)
    {
        bool pressed = gpio_get_level(config.gpio) == config.pressed_level;
        if (pressed && !state.pressed)
        {
            // just got pressed - reset the count
            state.ticksPressed = 0;
            state.pressed = pressed;
        }
        else if (pressed && state.pressed)
        {
            // remains pressed
            // see if we've gone over the long press threshold
            if ((state.ticksPressed < longTickCount) &&
                (state.ticksPressed + waitTicks ) >= longTickCount)
            {
                ESP_LOGI("button", "id %d, long press", config.button_id);
                ButtonEvent longEvent{config.button_id, ButtonEvent::LongPress};
                xQueueSendToBack(config.queue, &longEvent, 0);
            }
            // increment the counter
            state.ticksPressed += waitTicks;
        }
        else if (!pressed && state.pressed)
        {
            // just got released
            // if it was short enough, but not a bounce, then it's a short press
            if ((state.ticksPressed < longTickCount) &&
                (state.ticksPressed >= debounceTickCount))
            {
                ESP_LOGI("button", "id %d, short press", config.button_id);
                ButtonEvent shortEvent{config.button_id, ButtonEvent::ShortPress};
                xQueueSendToBack(config.queue, &shortEvent, 0);
            }
            state.pressed = pressed;
        }

        vTaskDelay(waitTicks);
    }
}