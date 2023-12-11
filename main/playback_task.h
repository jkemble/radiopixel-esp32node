#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "patterns/Player.h"

struct PlaybackConfig
{
    QueueHandle_t queue;
    Pattern::Strip *strip;
    int refresh_rate; // Hz
    uint8_t max_intensity = 0;
};

struct PlaybackEvent
{
    enum class Source : char
    {
        Master,
        Local,
        Background,
    };
    enum class Command : char
    {
        Play,
        Release,
    };

    Source source;
    Command command;
    Pattern::PlayerControl control;
};

void PlaybackTask(/*PlaybackConfig*/void *config);
