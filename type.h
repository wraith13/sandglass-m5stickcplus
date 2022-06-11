#ifndef SANDGLASS_TYPE_H
#define SANDGLASS_TYPE_H
#include <M5StickCPlus.h>
typedef int color_type;
typedef unsigned long tick_type;
struct preset_entry
{
    int size;
    color_type color;
    preset_entry(int a_size, color_type a_color)
        :size(a_size), color(a_color) { }
};
#endif
