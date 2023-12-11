#pragma once

#include <span>
#include "driver/gpio.h"
#include "freertos/queue.h"

struct ButtonConfig
{
    gpio_num_t gpio;            // GPIO number of the button
    int pressed_level;          // GPIO level when button is pressed
    int long_press_duration;    // long press threshold, in ms
    int button_id;              // button id for this task
    QueueHandle_t queue;        // queue to post events to
};

struct ButtonEvent
{
    int button_id;              // button id for this event

    enum Type
    {
        ShortPress,
        LongPress,
    };

    Type type;
};

/**
 * Watches for button presses on the specified GPIO.  If button is pressed for at least
 * the long press duration, send a ButtonEvent with LongPress, otherwise if the button
 * is released before the long press duration, send a ButtonEvent with ShortPress.
 */
void ButtonTask(/*ButtonConfig*/void *config);
