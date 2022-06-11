#ifndef SANDGLASS_CONFIG_H
#define SANDGLASS_CONFIG_H
#include <M5StickCPlus.h>
#include "type.h"
namespace config
{
    const preset_entry preset[] =
    {
        preset_entry(30 *1000, BLUE),
        preset_entry(60 *1000, RED),
        preset_entry(90 *1000, GREEN),
        preset_entry(120 *1000, CYAN),
        preset_entry(180 *1000, MAGENTA),
        preset_entry(300 *1000, YELLOW),
        preset_entry(600 *1000, ORANGE),
        preset_entry(900 *1000, GREENYELLOW),
        preset_entry(1800 *1000, PINK),
        preset_entry(3600 *1000, WHITE),
    };
    const preset_entry default_entry = preset[4];
    const float max_voltage = 4.2f;
    const float min_voltage = 3.0f;
    const int brisk_cpu_frequency_mhz = 40;
    const int brisk_screen_breath = 9;
    const int lazy_cpu_frequency_bhz = 20;
    const int lazy_screen_breath = 8;
    const tick_type sleep_suspend_tick = 60 *1000;
    const tick_type lazy_suspend_tick = 30 *1000;
    const float move_threshold = 0.1;
    const tick_type falling_sand_step_unit = 25;
    const auto falling_sand_pattern = ". .. . ... .. ... . ... .. .. . ... .. ... . ... .. . . ... .. ... . .. . ... .. .. . . .. . ... ";
}
#endif
