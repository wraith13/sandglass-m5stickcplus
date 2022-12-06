#include <M5StickCPlus.h> // Arduino/libraries/M5StickCPlus/src
#include <Preferences.h> // Arduino15/packages/esp32/hardware/esp32/2.0.2/libraries/Preferences/src
#include <string>
#include <algorithm>
#include <vector>
#include "type.h"
#include "config.h"
#define ARRAY_SIZE(X) (sizeof(X) / sizeof(X[0]))
#define ARRAY_LAST(X) X[ARRAY_SIZE(X) -1]
template<class minT, class valueT, class maxT> inline valueT clip(minT min, valueT value, maxT max)
{
    return (std::min)((std::max)(value, (valueT)min), (valueT)max);
}
template<class valueT> valueT square(valueT value)
{
    return value * value;
}
typedef int rotate_type;
namespace notification
{
    void (*updator)(tick_type, bool) = nullptr;
    tick_type start_at = 0;
    bool beeped = false;
    bool flashed = false;
    bool is_active()
    {
        return nullptr != updator;
    }
    void initialize()
    {
        pinMode(config::led_pin_number, OUTPUT);
        digitalWrite(config::led_pin_number, HIGH);
    }
    void set(void (*a_updator)(tick_type, bool))
    {
        updator = a_updator;
        start_at = millis();
    }
    void update(bool mute)
    {
        if (is_active())
        {
            updator(millis() -start_at, mute);
        }
    }
    void beep_on()
    {
        if ( ! beeped)
        {
            beeped = true;
            M5.Beep.beep();
        }
    }
    void beep_on(bool mute)
    {
        if ( ! mute)
        {
            beep_on();
        }
    }
    void beep_off()
    {
        if (beeped)
        {
            beeped = false;
            M5.Beep.mute();
        }
    }
    void led_on()
    {
        if ( ! flashed)
        {
            flashed = true;
            digitalWrite(config::led_pin_number, LOW);
        }
    }
    void led_off()
    {
        if (flashed)
        {
            flashed = false;
            digitalWrite(config::led_pin_number, HIGH);
        }
    }
    void off()
    {
        beep_off();
        led_off();
    }
    void clear()
    {
        updator = nullptr;
        off();
    }
    void first_updator(tick_type tick, bool mute)
    {
        int step = tick /100;
        switch(step)
        {
        case 0:
            // 最初の一発目は絶対に鳴ってくれないっぽいので、ダミーの Beep を１回挟む。
            beep_on(mute);
            break;
        case 1:
            off();
            break;
        case 2:
            beep_on(mute);
            led_on();
            break;
        default:
            clear();
            break;
        }
    }
    void single_updator(tick_type tick, bool mute)
    {
        int step = tick /100;
        switch(step)
        {
        case 0:
            beep_on(mute);
            // led_on(); ここで光ってもウザいだけなので、光らせない。
            break;
        default:
            clear();
            break;
        }
    }
    void tri_updator(tick_type tick, bool mute)
    {
        int step = tick /100;
        switch(step)
        {
        case 0:
        case 2:
        case 4:
            beep_on(mute);
            led_on();
            break;
        case 1:
        case 3:
            off();
            break;
        default:
            clear();
            break;
        }
    }
    void first()
    {
        M5.Beep.setVolume(3);
        set(first_updator);
    }
    void single()
    {
        M5.Beep.setVolume(3);
        set(single_updator);
    }
    void tri()
    {
        M5.Beep.setVolume(6);
        set(tri_updator);
    }
}
namespace battery_state
{
    bool is_charging()
    {
        return 0 < M5.Axp.GetBatCurrent();
    }
    bool is_low_battery()
    {
        return 0 != M5.Axp.GetWarningLeve();
    }
    float get_percentage()
    {
        return (M5.Axp.GetBatVoltage() -config::min_voltage) / (config::max_voltage -config::min_voltage);
    }
    color_type get_color()
    {
        return
            is_charging() ? BLUE:
            is_low_battery() ? RED:
            GREEN;
    }
};
namespace rotate
{
    struct accel_data
    {
        float x;
        float y;
        float z;
        typedef accel_data this_type;
        this_type & get()
        {
            M5.Imu.getAccelData(&x, &y, &z);
            return *this;
        }
        rotate_type get_rotate(rotate_type last_rotate)
        {
            if (battery_state::is_charging())
            {
                return 0;
            }
            const float delta = 0.2;
            if (-delta < x && x <= delta && -delta < y && y <= delta)
            {
                return last_rotate;
            }
            else
            {
                const float angle_unit = PI / 2;
                const float rawArea = (atan2f(x, y) /angle_unit) +4;
                const float area = roundf(rawArea);
                const float diff = rawArea -area;
                const float delta = 0.4;
                if (-delta < diff && diff <= delta)
                {
                    return ((int)area) %4;
                }
                else
                {
                    return last_rotate;
                }
            }
        }
        float abs(const this_type & other)
        {
            return sqrtf(square(x -other.x) +square(y -other.y) +square(z -other.z));
        }
    };
}
namespace save_battery
{
    enum power_mode
    {
        power_mode_none,
        power_mode_lazy,
        power_mode_brisk,
        power_mode_turbo,
    };
    power_mode current_power_mode = power_mode_none;
    void set(power_mode new_power_mode)
    {
        if (new_power_mode != current_power_mode)
        {
            current_power_mode = new_power_mode;
            switch(current_power_mode)
            {
            case power_mode_turbo:
                setCpuFrequencyMhz(config::max_cpu_frequency_mhz);
                break;
            case power_mode_brisk:
                setCpuFrequencyMhz(config::brisk_cpu_frequency_mhz);
                M5.Axp.ScreenBreath(config::brisk_screen_breath);
                break;
            case power_mode_lazy:
                setCpuFrequencyMhz(config::lazy_cpu_frequency_bhz);
                M5.Axp.ScreenBreath(config::lazy_screen_breath);
                break;
            }
        }
    }
    void initialize()
    {
        set(power_mode_brisk);
    }
    tick_type last_moving_at;
    tick_type last_falling_at;
    void update(bool is_moving, bool is_falling, bool turbo)
    {
        tick_type now = millis();
        if (is_moving)
        {
            last_moving_at = now;
        }
        if (is_falling)
        {
            last_falling_at = now;
        }
        if (config::sleep_suspend_tick < now -(std::max)(last_moving_at, last_falling_at))
        {
            M5.Axp.PowerOff();
        }
        if (turbo)
        {
            set(power_mode_turbo);
        }
        else
        if (config::lazy_suspend_tick < now -last_moving_at)
        {
            set(power_mode_lazy);
        }
        else
        {
            set(power_mode_brisk);
        }
    }
}
Preferences preferences;
struct mode_type
{
    typedef mode_type this_type;
    tick_type size;
    color_type color;
    boolean mute;
    boolean repeat;
    this_type & clear()
    {
        size = config::default_entry.size;
        color = config::default_entry.color;
        mute = false;
        repeat = false;
        return *this;
    }
    this_type & regulate()
    {
        size = clip<tick_type>((tick_type)config::preset[0].size, size, (tick_type)ARRAY_LAST(config::preset).size);
        color = config::preset[get_preset_index()].color;
        mute = true == mute;
        repeat = true == repeat;
        return *this;
    }
    this_type & save()
    {
        preferences.begin("sandglass-mode");
        preferences.putULong("size", size);
        preferences.putInt("color", color);
        preferences.putBool("mute", mute);
        preferences.putBool("repeat", repeat);
        preferences.end();
        return *this;
    }
    this_type & load()
    {
        preferences.begin("sandglass-mode", true);
        size = preferences.getULong("size", size);
        color = preferences.getInt("color", color);
        mute = preferences.getBool("mute", mute);
        repeat = preferences.getBool("repeat", repeat);
        preferences.end();
        return *this;
    }
    this_type & set_size(tick_type size)
    {
        this->size = size;
        return *this;
    }
    this_type & set_mute(bool mute)
    {
        this->mute = mute;
        return *this;
    }
    this_type & set_repeat(bool repeat)
    {
        this->repeat = repeat;
        return *this;
    }
    this_type & toggle_repeat()
    {
        repeat = ! repeat;
        return *this;
    }
    this_type & toggle_mute()
    {
        mute = ! mute;
        return *this;
    }
    this_type & change_to_next_mode()
    {
        if (repeat)
        {
            toggle_mute();
        }
        toggle_repeat();
        return *this;
    }
    int get_preset_index()
    {
        for(int i = 0; i < ARRAY_SIZE(config::preset); ++i)
        {
            if (size <= config::preset[i].size)
            {
                return i;
            }
        }
        return ARRAY_SIZE(config::preset) -1;
    }
    this_type & change_to_next_size()
    {
        const auto & entry = config::preset[(get_preset_index() +1) %ARRAY_SIZE(config::preset)];
        size = entry.size;
        color = entry.color;
        return *this;
    }
};
mode_type stored_mode;
namespace render
{
    enum zone
    {
        zone_zero = 0,
        zone_battery = 1,
        zone_glass = 2,
        zone_sand = 4,
        zone_rest = 8,
        zone_elapsed = 16,
        zone_mode = 32,
        zone_full = 63,
    };
    TFT_eSprite sprite(&M5.Lcd);
    void initialize()
    {
        sprite.createSprite(M5.Lcd.width(), M5.Lcd.height());
        sprite.setSwapBytes(false);
    }
    const int font_size = 2;
    enum class glass_direction_type
    {
        top,
        bottom,
    };
    enum class triangle_category_type
    {
        glass,
        hollow,
        container,
        sand
    };
    enum class sand_direction_type
    {
        center,
        bottom,
        left,
        right,
    };
    struct float_triangle_point_ref
    {
        float_triangle_point_ref(float & a_x, float & a_y) :x(a_x), y(a_y) { }
        float & x;
        float & y;
    };
    struct float_triangle
    {
        float x0;
        float y0;
        float x1;
        float y1;
        float x2;
        float y2;
        float_triangle_point_ref operator [] (int index)
        {
            switch(index)
            {
            case 0:
                return float_triangle_point_ref(x0, y0);
            case 1:
                return float_triangle_point_ref(x1, y1);
            case 2:
                return float_triangle_point_ref(x2, y2);
            default:
                throw new std::range_error("int_triangle_point_ref[index]");
            }
        }
    };
    float_triangle make_triangle
    (
        float x0,
        float y0,
        float x2,
        float y2,
        float x1,
        float y1
    )
    {
        float_triangle result;
        result.x0 = x0;
        result.y0 = y0;
        result.x1 = x1;
        result.y1 = y1;
        result.x2 = x2;
        result.y2 = y2;
        return result;
    }
    float get_line_x(float x0, float y0, float x1, float y1, float target_y)
    {
        if (y0 == y1)
        {
            return x1;
        }
        else
        {
            const float tilt = ((float)(x1 -x0)) / ((float)(y1 -y0));
            const float base_x = ((float)x0) -(((float)y0) *tilt);
            return clip((std::min)(x0, x1), base_x +(target_y *tilt), (std::max)(x0, x1));
        }
    }
    int get_line_x(float_triangle_point_ref p0, float_triangle_point_ref p1, int target_y)
    {
        return get_line_x(p0.x, p0.y, p1.x, p1.y, target_y);
    }
    void triangle
    (
        float_triangle coordinate,
        color_type color
    )
    {
        // fillTriangle の実装にバグがあるみたいで、これだとハングアップしてしまう、、、
        // sprite.fillTriangle
        // (
        //     coordinate.x0,
        //     coordinate.y0,
        //     coordinate.x1,
        //     coordinate.y1,
        //     coordinate.x2,
        //     coordinate.y2,
        //     color
        // );
        const float min_x = (std::min)((std::min)(coordinate.x0, coordinate.x1), coordinate.x2);
        const float max_x = (std::max)((std::max)(coordinate.x0, coordinate.x1), coordinate.x2);
        const float min_y = (std::min)((std::min)(coordinate.y0, coordinate.y1), coordinate.y2);
        const float max_y = (std::max)((std::max)(coordinate.y0, coordinate.y1), coordinate.y2);
        if (min_x == max_x)
        {
            sprite.drawFastVLine(min_x, min_y, (max_y -min_y) +1, color);
        }
        else
        if (min_y == max_y)
        {
            sprite.drawFastHLine(min_x, min_y, (max_x -min_x) +1, color);
        }
        else
        {
            const int top_i = coordinate.y0 <= coordinate.y1 ?
                coordinate.y0 <= coordinate.y2 ? 0: 2:
                coordinate.y1 <= coordinate.y2 ? 1: 2;
            const int bottom_i = coordinate.y0 <= coordinate.y1 ?
                coordinate.y1 <= coordinate.y2 ? 2: 1:
                coordinate.y0 <= coordinate.y2 ? 2: 0;
            const int middle_i = 3 -(top_i +bottom_i);
            for(int y = min_y; y <= max_y; ++y)
            {
                float x0 = y <= coordinate[middle_i].y ?
                    get_line_x(coordinate[top_i], coordinate[middle_i], y):
                    get_line_x(coordinate[middle_i], coordinate[bottom_i], y);
                float x1 = get_line_x(coordinate[top_i], coordinate[bottom_i], y);
                if (x0 <= x1)
                {
                    sprite.drawFastHLine(x0, y, (x1 -x0) +1, color);
                }
                else
                {
                    sprite.drawFastHLine(x1, y, (x0 -x1) +1, color);
                }
            }
        }
    }
    float_triangle calculate_triangle
    (
        bool isReverse,
        glass_direction_type glass_direction,
        triangle_category_type triangle_category,
        sand_direction_type sand_direction = sand_direction_type::bottom,
        float value = 0.0f
    )
    {
        const float screen_width = sprite.width() -1;
        const float screen_height = sprite.height() -1;
        const float center_x = screen_width / 2.0f;
        const float center_y = (screen_height / 2.0f) +(isReverse ? 0.0f: 1.0f);
        const float unit = sqrtf(screen_width *screen_width + screen_height * screen_height) *0.05f;
        const float font_height = 32.0f; // sprite.fontHeight(font_size);
        const float frame_width = screen_width - (unit * 2.0f);
        const float frame_height = screen_height - ((font_height * 2.0f) +1);
        const float angle = atanf(static_cast<float>(frame_height) /static_cast<float>(frame_width));
        const float unit_x = static_cast<float>(unit) *cosf(angle);
        const float unit_y = static_cast<float>(unit) *sinf(angle);
        const float sand_rate = 0.6;
        float_triangle result;
        //float size_rate = 1.0;
        switch (triangle_category)
        {
        case triangle_category_type::glass:
            result.x0 = center_x -(frame_width /2);
            result.y0 = center_y -(frame_height /2);
            result.x1 = center_x +(frame_width /2);
            result.y1 = center_y -(frame_height /2);
            result.x2 = center_x;
            result.y2 = center_y +(unit *2);
            break;
        case triangle_category_type::hollow:
            result.x0 = center_x -(frame_width /2 -unit_x);
            result.y0 = center_y -(frame_height /2 -(unit_y /2));
            result.x1 = center_x +(frame_width /2 -unit_x);
            result.y1 = center_y -(frame_height /2 -(unit_y /2));
            result.x2 = center_x;
            result.y2 = center_y +unit;
            break;
        case triangle_category_type::container:
            result.x0 = center_x -(frame_width /2 -(unit_x *2));
            result.y0 = center_y -(frame_height /2 -(unit_y *1));
            result.x1 = center_x +(frame_width /2 -(unit_x *2));
            result.y1 = center_y -(frame_height /2 -(unit_y *1));
            result.x2 = center_x;
            result.y2 = center_y;
            break;
        case triangle_category_type::sand:
            const auto container = calculate_triangle(isReverse, glass_direction_type::top, triangle_category_type::container);
            if (sand_direction_type::left == sand_direction || sand_direction_type::right == sand_direction)
            {
                const float height = static_cast<float>(container.y0 -container.y2) *sqrt(sand_rate *static_cast<float>(value));
                const float width = static_cast<float>(container.x1 -container.x2) *sqrt(sand_rate *static_cast<float>(value));
                switch (sand_direction)
                {
                case sand_direction_type::left:
                    result.x0 = container.x1 -width;
                    result.y0 = container.y0;
                    result.x1 = container.x1;
                    result.y1 = container.y1;
                    result.x2 = container.x1 -width;
                    result.y2 = container.y0 -height;
                    break;
                case sand_direction_type::right:
                    result.x0 = container.x0;
                    result.y0 = container.y0;
                    result.x1 = container.x0 +width;
                    result.y1 = container.y1;
                    result.x2 = container.x0 +width;
                    result.y2 = container.y0 -height;
                    break;
                }
            }
            else
            {
                const float height = static_cast<float>(container.y0 -container.y2) *sqrt(sand_rate *static_cast<float>(value) *0.5);
                const float width = static_cast<float>(container.x1 -container.x0) *sqrt(sand_rate *static_cast<float>(value) *0.5);
                switch (sand_direction)
                {
                case sand_direction_type::center:
                    result.x0 = container.x2 -(width /2.0f);
                    result.y0 = container.y2 +height;
                    result.x1 = container.x2 +(width /2.0f);
                    result.y1 = container.y2 +height;
                    result.x2 = container.x2;
                    result.y2 = container.y2;
                    break;
                case sand_direction_type::bottom:
                    result.x0 = container.x2 -(width /2.0f);
                    result.y0 = container.y0;
                    result.x1 = container.x2 +(width /2.0f);
                    result.y1 = container.y1;
                    result.x2 = container.x2;
                    result.y2 = container.y0 -height;
                    break;
                }
            }
            break;
        }
        switch (glass_direction)
        {
        case glass_direction_type::top:
            break;
        case glass_direction_type::bottom:
            result.y0 = center_y -(result.y0 -center_y);
            result.y1 = center_y -(result.y1 -center_y);
            result.y2 = center_y -(result.y2 -center_y);
            break;
        }
        return result;
    }
    void fall(float_triangle top_sand, float_triangle bottom_sand, color_type foreground_color, color_type background_color, const char stream[], int step)
    {
        auto stream_length = strlen(stream);
        for(int y = top_sand.y2; y <= bottom_sand.y2; ++y)
        {
            sprite.drawPixel
            (
                top_sand.x2,
                y,
                '.' == stream[(stream_length +y -(step %stream_length)) %stream_length] ?
                    foreground_color:
                    background_color
            );
        }
        // sprite.drawFastVLine(top_sand.x2, top_sand.y2, (bottom_sand.y2 -top_sand.y2) +1, foreground_color);
    }
    void printTime(int x, int y, tick_type value)
    {
        // auto total_seconds = (int)roundf(((float)value) /1000.0f);
        auto total_seconds = value /1000;
        auto seconds = static_cast<int>(total_seconds % 60);
        auto minutes = static_cast<int>(total_seconds / 60);
        // M5.Lcd.printf("%02d:%02d", minutes, seconds);
        char buffer[256];
        // sprintf(buffer, "%02d:%02d", minutes, seconds);
        sprintf(buffer, "%02d:%02d.%03d", minutes, seconds, value %1000);
        sprite.drawString(buffer, x, y, font_size);
    }
    void rest(bool isReverse, color_type foreground_color, tick_type value)
    {
        // M5.Lcd.setCursor((M5.Lcd.width() -(M5.Lcd.fontHeight(font_size) *5 /2)) /2, 0);
        sprite.setTextColor(foreground_color);
        sprite.setTextSize(font_size);
        sprite.setTextDatum(TC_DATUM);
        printTime(sprite.width() /2, isReverse ? 0: 1, value);
    }
    void elapsed(bool isReverse, color_type foreground_color, tick_type value)
    {
        // M5.Lcd.setCursor(M5.Lcd.width() /2, M5.Lcd.height() -M5.Lcd.fontHeight(font_size));
        sprite.setTextColor(foreground_color);
        sprite.setTextSize(font_size);
        sprite.setTextDatum(BC_DATUM);
        printTime(sprite.width() /2, sprite.height() -(isReverse ? 2: 1), value);
    }
    void sand(bool isReverse, float value, int step, color_type foreground_color, color_type background_color, sand_direction_type sand_direction)
    {
        const auto bottom_value = 1.0 - value;
        auto top_hollow = calculate_triangle(isReverse, glass_direction_type::top, triangle_category_type::hollow);
        auto bottom_hollow = calculate_triangle(isReverse, glass_direction_type::bottom, triangle_category_type::hollow);
        auto top_sand = calculate_triangle(isReverse, glass_direction_type::top, triangle_category_type::sand, sand_direction_type::bottom == sand_direction ? sand_direction_type::center: sand_direction, value);
        auto bottom_sand = calculate_triangle(isReverse, glass_direction_type::bottom, triangle_category_type::sand, sand_direction, bottom_value);
        triangle(top_hollow, background_color);
        triangle(bottom_hollow, background_color);
        const float delta = 0.0001;
        if (delta <= value)
        {
            triangle(top_sand, foreground_color);
        }
        if (delta <= bottom_value)
        {
            triangle(bottom_sand, foreground_color);
        }
        if (delta <= value && delta <= bottom_value && sand_direction_type::bottom == sand_direction)
        {
            fall
            (
                top_sand,
                bottom_sand,
                foreground_color,
                background_color,
                config::falling_sand_pattern,
                step
            );
        }
    }
    void hollow(bool isReverse, color_type foreground_color, color_type background_color)
    {
        triangle(calculate_triangle(isReverse, glass_direction_type::top, triangle_category_type::hollow), background_color);
        triangle(calculate_triangle(isReverse, glass_direction_type::bottom, triangle_category_type::hollow), background_color);
    }
    void glass(bool isReverse, color_type foreground_color, color_type background_color)
    {
        triangle(calculate_triangle(isReverse, glass_direction_type::top, triangle_category_type::glass), foreground_color);
        triangle(calculate_triangle(isReverse, glass_direction_type::bottom, triangle_category_type::glass), foreground_color);
        hollow(isReverse, foreground_color, background_color);
    }
    void mode_mark(bool isReverse, color_type foreground_color, const mode_type & mode)
    {
        sprite.setTextColor(foreground_color);
        sprite.setTextSize(font_size);
        if (mode.mute)
        {
            sprite.setTextDatum(ML_DATUM);
            sprite.drawString("M", 0, sprite.height() /2, font_size);
        }
        if (mode.repeat)
        {
            sprite.setTextDatum(MR_DATUM);
            sprite.drawString("R", sprite.width() -1, sprite.height() /2, font_size);
        }
    }
    void battery(bool isReverse, float battery_percentage, color_type foreground_color, color_type background_color)
    {
        int battery_bar_width = ((float)sprite.width()) *battery_percentage;
        sprite.drawFastHLine
        (
            0,
            isReverse ? sprite.height() -1: 0,
            battery_bar_width,
            foreground_color
        );
        sprite.drawFastHLine
        (
            battery_bar_width,
            isReverse ? sprite.height() -1: 0,
            sprite.width() -battery_bar_width,
            background_color
        );
    }
    void background(zone invalid_zone, bool isReverse, color_type foreground_color, color_type background_color)
    {
        if (invalid_zone & zone_glass)
        {
            sprite.fillRect(0, 0, M5.Lcd.width(), M5.Lcd.height(), background_color);
        }
        else
        {
            const float font_height = 32.0f; // sprite.fontHeight(font_size);
            const float font_width = font_height *0.8f;
            // if (invalid_zone & zone::battery)
            // {
            //     sprite.drawFastHLine
            //     (
            //         0,
            //         isReverse ? sprite.height() -1: 0,
            //         M5.Lcd.width(),
            //         background_color
            //     );
            // }
            if (invalid_zone & zone_sand)
            {
                hollow(isReverse, foreground_color, background_color);
            }
            if (invalid_zone & zone_rest)
            {
                sprite.fillRect(0, isReverse ? 0: 1, M5.Lcd.width(), font_height, background_color);
            }
            if (invalid_zone & zone_elapsed)
            {
                sprite.fillRect(0, sprite.height() -(font_height +(isReverse ? 1: 0)), M5.Lcd.width(), font_height, background_color);
            }
            if (invalid_zone & zone_mode)
            {
                sprite.fillRect(0, (sprite.height() -font_height) /2, font_width, font_height, background_color);
                sprite.fillRect(sprite.width() -font_width, (sprite.height() -font_height) /2, font_width, font_height, background_color);
            }
        }
    }
    namespace previous_screen_params
    {
        bool isReverse;
        tick_type rest_time;
        tick_type elapsed_time;
        int step;
        float battery_percentage;
        color_type foreground_color;
        color_type background_color;
        color_type battery_state_color;
        sand_direction_type direction;
        mode_type mode;
    };
    zone get_zone
    (
        bool isReverse,
        tick_type rest_time,
        tick_type elapsed_time,
        int step,
        float battery_percentage,
        color_type foreground_color,
        color_type background_color,
        color_type battery_state_color,
        sand_direction_type direction,
        const mode_type & mode
    )
    {
        int result = zone_zero;
        if
        (
            previous_screen_params::isReverse != isReverse ||
            previous_screen_params::foreground_color != foreground_color ||
            previous_screen_params::background_color != background_color
        )
        {
            result |= zone_battery;
            result |= zone_glass;
            result |= zone_sand;
            result |= zone_rest;
            result |= zone_elapsed;
            result |= zone_mode;
        }
        if
        (
            static_cast<int>(previous_screen_params::battery_percentage *1000) != static_cast<int>(battery_percentage *1000) ||
            previous_screen_params::battery_state_color != battery_state_color
        )
        {
            result |= zone_battery;
        }
        if (previous_screen_params::rest_time != rest_time)
        {
            result |= zone_rest;
        }
        if (previous_screen_params::elapsed_time != elapsed_time)
        {
            result |= zone_elapsed;
        }
        if
        (
            previous_screen_params::step != step ||
            previous_screen_params::direction != direction ||
            (previous_screen_params::rest_time != rest_time && 0 == rest_time) ||
            (previous_screen_params::elapsed_time != elapsed_time && 0 == elapsed_time)
        )
        {
            result |= zone_sand;
        }
        if
        (
            previous_screen_params::mode.repeat != mode.repeat ||
            previous_screen_params::mode.mute != mode.mute
        )
        {
            result |= zone_mode;
        }
        if (result & zone_battery)
        {
            previous_screen_params::battery_percentage = battery_percentage;
            previous_screen_params::battery_state_color = battery_state_color;
        }
        if (result & zone_glass)
        {
        }
        if (result & zone_sand)
        {
            previous_screen_params::step = step;
            previous_screen_params::direction = direction;
        }
        if (result & zone_rest)
        {
            previous_screen_params::rest_time = rest_time;
        }
        if (result & zone_elapsed)
        {
            previous_screen_params::elapsed_time = elapsed_time;
        }
        if (result & zone_mode)
        {
            previous_screen_params::mode = mode;
        }
        previous_screen_params::isReverse = isReverse;
        previous_screen_params::foreground_color = foreground_color;
        previous_screen_params::background_color = background_color;
        return static_cast<zone>(result);
    }
    void screen
    (
        bool isReverse,
        tick_type rest_time,
        tick_type elapsed_time,
        int step,
        float battery_percentage,
        color_type foreground_color,
        color_type background_color,
        color_type battery_state_color,
        sand_direction_type direction,
        const mode_type & mode
    )
    {
        zone invalid_zone = get_zone
        (
            isReverse,
            rest_time,
            elapsed_time,
            step,
            battery_percentage,
            foreground_color,
            background_color,
            battery_state_color,
            direction,
            mode
        );
        M5.Lcd.setRotation(isReverse ? 2: 0);
        M5.Lcd.startWrite();
        sprite.setRotation(0);
        background
        (
            invalid_zone,
            isReverse,
            foreground_color,
            background_color
        );
        if (invalid_zone & zone_glass)
        {
            glass(isReverse, foreground_color, background_color);
        }
        if (invalid_zone & zone_sand)
        {
            sand
            (
                isReverse,
                (0.0 +rest_time) / (0.0 +rest_time +elapsed_time),
                step,
                foreground_color,
                background_color,
                direction
            );
        }
        if (invalid_zone & zone_rest)
        {
            rest
            (
                isReverse,
                foreground_color,
                rest_time
            );
        }
        if (invalid_zone & zone_elapsed)
        {
            elapsed
            (
                isReverse,
                foreground_color,
                elapsed_time
            );
        }
        if (invalid_zone & zone_mode)
        {
            mode_mark
            (
                isReverse,
                foreground_color,
                mode
            );
        }
        if (invalid_zone & zone_battery)
        {
            battery
            (
                isReverse,
                battery_percentage,
                battery_state_color,
                background_color
            );
        }
        sprite.pushSprite(0, 0);
        M5.Lcd.endWrite();
    }
}
namespace state
{
    tick_type previous_at;
    rotate::accel_data last_accel_data;
    rotate_type last_rotate = 0;
    tick_type last_rotate_at;
    tick_type last_origin_top_volume;
    bool isStanding(rotate_type rotate)
    {
        return 0 == rotate || 2 == rotate;
    }
    void initialize()
    {
        render::initialize();
    }
    void reset_to_top()
    {
        last_origin_top_volume = 0;
    }
    void reset_to_bottom()
    {
        last_origin_top_volume = stored_mode.size;
    }
    bool update_tick(tick_type now, rotate_type rotate)
    {
        bool beep = false;
        if (isStanding(last_rotate))
        {
            const tick_type previous_origin_top_volume = last_origin_top_volume;
            if (0 == last_rotate)
            {
                last_origin_top_volume += now -previous_at;
                if (stored_mode.size <= last_origin_top_volume)
                {
                    if (stored_mode.repeat)
                    {
                        while(stored_mode.size <= last_origin_top_volume)
                        {
                            last_origin_top_volume -= stored_mode.size;
                        }
                        beep = true;
                    }
                    else
                    {
                        last_origin_top_volume = stored_mode.size;
                        beep = previous_origin_top_volume != stored_mode.size;
                    }
                }
            }
            else
            {
                last_origin_top_volume += previous_at -now;
                if (0 == last_origin_top_volume || stored_mode.size < last_origin_top_volume)
                {
                    if (stored_mode.repeat)
                    {
                        if ((signed long)last_origin_top_volume < 0)
                        {
                            last_origin_top_volume += stored_mode.size;
                        }
                        else
                        {
                            while(stored_mode.size <= last_origin_top_volume)
                            {
                                last_origin_top_volume -= stored_mode.size;
                            }
                        }
                        beep = true;
                    }
                    else
                    {
                        last_origin_top_volume = 0;
                        beep = previous_origin_top_volume != 0;
                    }
                }
            }
        }
        else
        {
            if (stored_mode.size <= last_origin_top_volume)
            {
                last_origin_top_volume = stored_mode.size;
            }
        }
        if (last_rotate != rotate)
        {
            last_rotate = rotate;
            last_rotate_at = now;
        }
        previous_at = now;
        return beep;
    }
    bool update()
    {
        tick_type now = millis();
        rotate::accel_data current_accel_data;
        bool is_moving = config::move_threshold < last_accel_data.abs(current_accel_data.get());
        if (is_moving)
        {
            last_accel_data = current_accel_data;
        }
        rotate_type rotate = last_accel_data.get_rotate(last_rotate);
        const tick_type previous_origin_top_volume = last_origin_top_volume;
        bool beep = update_tick(now, rotate);
        bool is_falling = previous_origin_top_volume != last_origin_top_volume; // 正確な判定ではないが、現在の実用上、これで問題ない。
        save_battery::update(is_moving || beep, is_falling, notification::is_active());
        int step = (now -last_rotate_at) /config::falling_sand_step_unit;
        bool isReverse = false;
        render::sand_direction_type direction;
        switch(rotate)
        {
        case 0:
            direction = render::sand_direction_type::bottom;
            break;
        case 1:
            direction = render::sand_direction_type::right;
            step = 0;
            break;
        case 2:
            isReverse = true;
            direction = render::sand_direction_type::bottom;
            break;
        case 3:
            direction = render::sand_direction_type::left;
            step = 0;
            break;
        }
        tick_type elapsed_time = last_origin_top_volume;
        tick_type rest_time = stored_mode.size -last_origin_top_volume;
        render::screen
        (
            isReverse,
            isReverse ? elapsed_time: rest_time,
            isReverse ? rest_time: elapsed_time,
            step,
            battery_state::get_percentage(),
            stored_mode.color,
            BLACK,
            battery_state::get_color(),
            direction,
            stored_mode
        );
        return beep;
    }
}
void setup()
{
    M5.begin();
    stored_mode.clear().load().regulate();
    M5.Imu.Init();
    //M5.Beep.begin();
    save_battery::initialize();
    state::initialize();
    notification::initialize();
    notification::first();
}
void loop()
{
    bool beep = false;
    if (M5.BtnA.wasReleased())
    {
        stored_mode.change_to_next_size().save();
        beep = true;
    }
    if (M5.BtnB.wasReleased())
    {
        stored_mode.change_to_next_mode().save();
        beep = true;
    }
    switch(M5.Axp.GetBtnPress())
    {
    case 1:
        state::reset_to_bottom();
        beep = true;
        break;
    case 2:
        state::reset_to_top();
        beep = true;
        break;
    }
    if (beep)
    {
        notification::single();
    }
    if (state::update() && ! beep)
    {
        notification::tri();
    }
    notification::update(stored_mode.mute);
    M5.update();
    M5.Beep.update();
    delay(1); // これだけで節電効果がある一方で、数値をデカくしてもあんまり効果は無いらしい。
}
