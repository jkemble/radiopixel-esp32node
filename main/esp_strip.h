#pragma once

#include <vector>
#include "led_strip.h"
#include "patterns/Strip.h"

class EspStrip : public Pattern::Strip
{
public:
    EspStrip( int gpio, size_t size)
        : m_leds(size)
    {
        //configure_led();
        //ESP_LOGI(TAG, "Example configured to blink addressable LED!");
        /* LED strip initialization with the GPIO and pixels number*/
        led_strip_config_t strip_config = {
            .strip_gpio_num = gpio,
            .max_leds = size, // at least one LED on board
            .led_pixel_format = LED_PIXEL_FORMAT_GRB,
            .led_model = LED_MODEL_WS2812,
            .flags = 0
        };
        led_strip_rmt_config_t rmt_config = {
            .resolution_hz = 10 * 1000 * 1000, // 10MHz
        };
        ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &m_led_strip));
        /* Set all LED off to clear all pixels */
        led_strip_clear(m_led_strip);
    }

    // returns the number of pixels
    virtual uint16_t numPixels( ) const override
    {
        return m_leds.size();
    }

    // get a pixel color value
    virtual Color::rgb32_t getPixelColor( uint16_t pixel ) const override
    {
        return m_leds[pixel];
    }

    // set a single pixel to a color
    virtual void setPixelColor( uint16_t pixel, Color::rgb32_t color ) override
    {
        m_leds[pixel] = color;
    }

    // set all pixels to a color
    virtual void setAllColor( Color::rgb32_t color ) override
    {
        for ( size_t pixel = 0; pixel < m_leds.size(); ++pixel )
        {
            m_leds[pixel] = color;
        }
    }

    // independent control of brightness
    virtual uint8_t getBrightness( ) const override
    {
        return m_brightness;
    }

    // independent control of brightness
    virtual void setBrightness( uint8_t bright ) override
    {
        m_brightness = bright;
    }

    void transmit() const override
    {
        for ( size_t pixel = 0; pixel < m_leds.size(); ++pixel )
        {
            led_strip_set_pixel(m_led_strip, pixel, 
                m_leds[pixel].r() * m_brightness / 255, 
                m_leds[pixel].g() * m_brightness / 255, 
                m_leds[pixel].b() * m_brightness / 255);
        }
        /* Refresh the strip to send data */
        led_strip_refresh(m_led_strip);
    }

protected:
    led_strip_handle_t m_led_strip;
    std::vector<Color::rgb32_t> m_leds;
    uint8_t m_brightness = 255;
};
