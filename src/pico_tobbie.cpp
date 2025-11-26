// Example file - Public Domain
// Need help? https://tinyurl.com/bluepad32-help

#include <stddef.h>
#include <string.h>
#include <math.h>

#include "pico/stdlib.h"
#include "pico_motor_shim.hpp"
#include <pico/cyw43_arch.h>
#include <pico/time.h>
#include <uni.h>

#include "sdkconfig.h"

// Sanity check
#ifndef CONFIG_BLUEPAD32_PLATFORM_CUSTOM
#error "Pico W must use BLUEPAD32_PLATFORM_CUSTOM"
#endif

// Declarations
static void trigger_event_on_gamepad(uni_hid_device_t* d);

using namespace motor;

// Create an array of motor pointers
const pin_pair motor_pins[] = {pico_motor_shim::MOTOR_1, pico_motor_shim::MOTOR_2};
const uint NUM_MOTORS = count_of(motor_pins);
Motor *motors[NUM_MOTORS];
const uint WALK_DRIFT = 0.1;

const uint LED_EYE_R = 21;
const uint LED_EYE_L = 22;
//
// Platform Overrides
//
static void pico_tobbie_init(int argc, const char** argv) {
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    gpio_init(LED_EYE_R);
    gpio_set_dir(LED_EYE_R, GPIO_OUT);
    gpio_init(LED_EYE_L);
    gpio_set_dir(LED_EYE_L, GPIO_OUT);

    for(auto m = 0u; m < NUM_MOTORS; m++) {
        motors[m] = new Motor(motor_pins[m]);
        motors[m]->init();
        motors[m]->enable();
    }    

    logi("my_platform: init()\n");

#if 0
    uni_gamepad_mappings_t mappings = GAMEPAD_DEFAULT_MAPPINGS;

    // Inverted axis with inverted Y in RY.
    mappings.axis_x = UNI_GAMEPAD_MAPPINGS_AXIS_RX;
    mappings.axis_y = UNI_GAMEPAD_MAPPINGS_AXIS_RY;
    mappings.axis_ry_inverted = true;
    mappings.axis_rx = UNI_GAMEPAD_MAPPINGS_AXIS_X;
    mappings.axis_ry = UNI_GAMEPAD_MAPPINGS_AXIS_Y;

    // Invert A & B
    mappings.button_a = UNI_GAMEPAD_MAPPINGS_BUTTON_B;
    mappings.button_b = UNI_GAMEPAD_MAPPINGS_BUTTON_A;

    uni_gamepad_set_mappings(&mappings);
#endif
}

static void pico_tobbie_on_init_complete(void) {
    logi("my_platform: on_init_complete()\n");

    // Safe to call "unsafe" functions since they are called from BT thread

    // Start scanning and autoconnect to supported controllers.
    uni_bt_start_scanning_and_autoconnect_unsafe();

    // Based on runtime condition, you can delete or list the stored BT keys.
    if (1)
        uni_bt_del_keys_unsafe();
    else
        uni_bt_list_keys_unsafe();

    //    uni_bt_service_set_enabled(true);

    uni_property_dump_all();
}

static uni_error_t pico_tobbie_on_device_discovered(bd_addr_t addr, const char* name, uint16_t cod, uint8_t rssi) {
    // You can filter discovered devices here. Return any value different from UNI_ERROR_SUCCESS;
    // @param addr: the Bluetooth address
    // @param name: could be NULL, could be zero-length, or might contain the name.
    // @param cod: Class of Device. See "uni_bt_defines.h" for possible values.
    // @param rssi: Received Signal Strength Indicator (RSSI) measured in dBms. The higher (255) the better.

    // As an example, if you want to filter out keyboards, do:
    if (((cod & UNI_BT_COD_MINOR_MASK) & UNI_BT_COD_MINOR_KEYBOARD) == UNI_BT_COD_MINOR_KEYBOARD) {
        logi("Ignoring keyboard\n");
        return UNI_ERROR_IGNORE_DEVICE;
    }

    return UNI_ERROR_SUCCESS;
}

static void pico_tobbie_on_device_connected(uni_hid_device_t* d) {
    logi("my_platform: device connected: %p\n", d);
    // Turn off LED once init is done.
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);    
}

static void pico_tobbie_on_device_disconnected(uni_hid_device_t* d) {
    logi("my_platform: device disconnected: %p\n", d);
}

static uni_error_t pico_tobbie_on_device_ready(uni_hid_device_t* d) {
    logi("my_platform: device ready: %p\n", d);

    // You can reject the connection by returning an error.
    return UNI_ERROR_SUCCESS;
}

static void pico_tobbie_on_controller_data(uni_hid_device_t* d, uni_controller_t* ctl) {
    static uint8_t leds = 0;
    uni_gamepad_t* gp;
    bool forward=true;

    // Print device Id before dumping gamepad.
    logi("(%p) id=%d ", d, uni_hid_device_get_idx_for_instance(d));
    uni_controller_dump(ctl);

    switch (ctl->klass) {
        case UNI_CONTROLLER_CLASS_GAMEPAD:
            gp = &ctl->gamepad;

            if (gp->buttons & BUTTON_SHOULDER_L) {
                motors[1]->speed(1);
                gpio_put(LED_EYE_L, 1);
                gpio_put(LED_EYE_R, 0);
            }

            if (gp->buttons & BUTTON_SHOULDER_R) {
                motors[1]->speed(-1);
                gpio_put(LED_EYE_L, 0);
                gpio_put(LED_EYE_R, 1);

            }

            if (! (gp->buttons & (BUTTON_SHOULDER_L | BUTTON_SHOULDER_R))) {
                motors[1]->speed(0);
                if (forward)
                {
                    motors[1]->speed(WALK_DRIFT);
                }
                else
                {
                    motors[1]->speed(-WALK_DRIFT);
                }
            }

            if (gp->buttons & BUTTON_A) {
                motors[0]->speed(1);
                motors[1]->speed(WALK_DRIFT);
                forward=true;
                gpio_put(LED_EYE_L, 1);
                gpio_put(LED_EYE_R, 1);
            }
            if (gp->buttons & BUTTON_B) {
                motors[0]->speed(-1);
                motors[1]->speed(-WALK_DRIFT);
                forward=false;
            }      
            if (gp->buttons & BUTTON_X) {                
            }                            
            if (gp->buttons & BUTTON_Y) {
                motors[0]->stop();
                motors[1]->stop();
                gpio_put(LED_EYE_L, 0);
                gpio_put(LED_EYE_R, 0);
            }    

            /*
            if (gp->axis_y<-GAMEPAD_DEAD_ZONE)
            {
                motors[0]->speed(-gp->axis_y/512); 
            }
            if (gp->axis_y>GAMEPAD_DEAD_ZONE)
            {
                motors[0]->speed(-gp->axis_y/512); 
            } */           

            // Axis: control RGB color
            if ((gp->buttons & BUTTON_Y) && d->report_parser.set_lightbar_color != NULL) {
                uint8_t r = (gp->axis_x * 256) / 512;
                uint8_t g = (gp->axis_y * 256) / 512;
                uint8_t b = (gp->axis_rx * 256) / 512;
                d->report_parser.set_lightbar_color(d, r, g, b);
            }

            break;
        case UNI_CONTROLLER_CLASS_BALANCE_BOARD:
            // Do something
            uni_balance_board_dump(&ctl->balance_board);
            break;
        case UNI_CONTROLLER_CLASS_MOUSE:
            // Do something
            uni_mouse_dump(&ctl->mouse);
            break;
        case UNI_CONTROLLER_CLASS_KEYBOARD:
            // Do something
            uni_keyboard_dump(&ctl->keyboard);
            break;
        default:
            loge("Unsupported controller class: %d\n", ctl->klass);
            break;
    }
}

static const uni_property_t* pico_tobbie_get_property(uni_property_idx_t idx) {
    ARG_UNUSED(idx);
    return NULL;
}

static void pico_tobbie_on_oob_event(uni_platform_oob_event_t event, void* data) {
    switch (event) {
        case UNI_PLATFORM_OOB_GAMEPAD_SYSTEM_BUTTON:
            // Optional: do something when "system" button gets pressed.
            trigger_event_on_gamepad((uni_hid_device_t*)data);
            break;

        case UNI_PLATFORM_OOB_BLUETOOTH_ENABLED:
            // When the "bt scanning" is on / off. Could be triggered by different events
            // Useful to notify the user
            logi("pico_tobbie_on_oob_event: Bluetooth enabled: %d\n", (bool)(data));
            break;

        default:
            logi("pico_tobbie_on_oob_event: unsupported event: 0x%04x\n", event);
    }
}

static void pico_tobbie_on_gamepad_data(uni_hid_device_t* d, uni_gamepad_t* gp) {
}

static void pico_tobbie_device_dump(uni_hid_device_t* d) {
}
static void pico_tobbie_register_console_cmds(void) {    
}

//
// Helpers
//
static void trigger_event_on_gamepad(uni_hid_device_t* d) {
    if (d->report_parser.play_dual_rumble != NULL) {
        d->report_parser.play_dual_rumble(d, 0 /* delayed start ms */, 50 /* duration ms */, 128 /* weak magnitude */,
                                          40 /* strong magnitude */);
    }

    if (d->report_parser.set_player_leds != NULL) {
        static uint8_t led = 0;
        led += 1;
        led &= 0xf;
        d->report_parser.set_player_leds(d, led);
    }

    if (d->report_parser.set_lightbar_color != NULL) {
        static uint8_t red = 0x10;
        static uint8_t green = 0x20;
        static uint8_t blue = 0x40;

        red += 0x10;
        green -= 0x20;
        blue += 0x40;
        d->report_parser.set_lightbar_color(d, red, green, blue);
    }
}

//
// Entry Point
//

struct uni_platform* get_my_platform(void) {
    static struct uni_platform plat = {
        .name = "My Platform",
        .init = pico_tobbie_init,
        .on_init_complete = pico_tobbie_on_init_complete,
        .on_device_discovered = pico_tobbie_on_device_discovered,
        .on_device_connected = pico_tobbie_on_device_connected,
        .on_device_disconnected = pico_tobbie_on_device_disconnected,
        .on_device_ready = pico_tobbie_on_device_ready,
        .on_gamepad_data = pico_tobbie_on_gamepad_data,
        .on_controller_data = pico_tobbie_on_controller_data,
        .get_property = pico_tobbie_get_property,
        .on_oob_event = pico_tobbie_on_oob_event,
        .device_dump = pico_tobbie_device_dump,
        .register_console_cmds = pico_tobbie_register_console_cmds
    };

    return &plat;
}
