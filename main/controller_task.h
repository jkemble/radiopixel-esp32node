#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

struct ControllerConfig
{
    bool master;
    QueueHandle_t buttonQueue;
    QueueHandle_t playbackQueue;
};

void ControllerTask(/*ControllerConfig*/void *config);
