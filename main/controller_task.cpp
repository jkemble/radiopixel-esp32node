#include "patterns/Sequence.h"
#include "playback_task.h"
#include "button_task.h"
#include "esp_log.h"
#include "esp_now.h"
#include "controller_task.h"


void ControllerTask(/*ControllerConfig*/void *_config)
{
    auto config(*static_cast<ControllerConfig *>(_config));

    Pattern::RandomSequence random;
    random.AddSteps(Pattern::randomSteps());

    Pattern::OrderedSequence alert;
    alert.AddSteps(Pattern::alertSteps());

    Pattern::Sequence *sequence = nullptr;
    int step = -1;
    while (1)
    {
        // wait the current step duration for a button to come in
        ButtonEvent btEvent;
        auto duration = (step != -1) ? sequence->GetDuration(step) : 0;
        bool button = xQueueReceive(config.buttonQueue, &btEvent, pdMS_TO_TICKS(duration ? duration : 100));

        // if a button event came in then process it
        if (button)
        {
            // restart the sequence if needed
            if (config.master && (btEvent.type == ButtonEvent::LongPress))
            {
                if (sequence != &alert)
                {
                    sequence = &alert;
                    step = -1;
                }
            }
            else
            {
                if (sequence != &random)
                {
                    sequence = &random;
                    step = -1;
                }
            }
        }

        // if a button was pressed or a step expired then advance to the next step
        if ( button || duration)
        {
            // start or advance the sequence as needed
            if ( step == -1 )
            {
                step = sequence->Reset();
            }
            else
            {
                step = sequence->Advance( step, !button );
            }
            ESP_LOGI("controller", "%s advance to step %d", button?"button":"timeout", step);

            // create the playback event

            // transmit the new step, if master
            PlaybackEvent plEvent{PlaybackEvent::Source::Master, 
                (step == -1) ? PlaybackEvent::Command::Release : PlaybackEvent::Command::Play, 
                (step == -1) ? Pattern::PlayerControl() : sequence->GetCommand(step)};
            if (config.master)
            {
                uint8_t broadcast[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
                esp_now_send( broadcast, reinterpret_cast<uint8_t *>(&plEvent), sizeof(plEvent));
            }

            // playback the new step locally
            plEvent.source = PlaybackEvent::Source::Local;
            xQueueSendToBack(config.playbackQueue, &plEvent, 0);
        }
    }
}