#include <map>
#include "patterns/Player.h"
#include "esp_log.h"
#include "playback_task.h"


void PlaybackTask(/*PlaybackConfig*/void *_config)
{
    auto config(*static_cast<PlaybackConfig *>(_config));
    ESP_LOGI("playback", "strip size %d, refresh rate %d", config.strip->numPixels(), config.refresh_rate);

    std::map<PlaybackEvent::Source, PlaybackEvent> stack;

    Pattern::Player player;
    auto now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    player.UpdatePattern( now, Pattern::PlayerControl(), config.strip );
    config.strip->transmit();
    while (1)
    {
        auto now = xTaskGetTickCount() * portTICK_PERIOD_MS;

        // process any events that came in
        PlaybackEvent event;
        while ( xQueueReceive(config.queue, &event, 0))
        {
            ESP_LOGI("playback", "Received source %d, command %d, pattern %d", (int)event.source, (int)event.command, (int)event.control.pattern);
            if (config.max_intensity) {
                event.control.intensity = config.max_intensity;
            }
            if (event.command == PlaybackEvent::Command::Play) {
                stack[event.source] = event;
            } else {
                stack.erase(event.source);
            }
            if (stack.begin() != stack.end()) {
                player.UpdatePattern( now, stack.begin()->second.control, config.strip );
            }
        }

        // render the pattern to the strip, in case there were no events
        player.UpdateStrip( now, config.strip );

        // output the strip to hardware
        config.strip->transmit();

        // wait the refresh period
        vTaskDelay(pdMS_TO_TICKS(1000 / config.refresh_rate));
    }
}
