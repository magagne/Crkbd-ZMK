#define DT_DRV_COMPAT zmk_behavior_raw_drag_scroll


#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <raw_hid/events.h>

/* MATTHIEU */
#include <zephyr/logging/log.h>
#include <zmk/keycode_state_changed.h>
LOG_MODULE_REGISTER(raw_drag_scroll, LOG_LEVEL_DBG);



static uint8_t drag_scroll_on[32] = {
    0x53
};

static uint8_t drag_scroll_off[32] = {
    0x73
};

static int on_drag_scroll_pressed(struct zmk_behavior_binding *binding,
                                  struct zmk_behavior_binding_event event) {
    LOG_DBG("DRAG SCROLL PRESSED"); /* MATTHIEU*/
    raise_zmk_keycode_state_changed_from_encoded(HID_USAGE_KEYBOARD_F24, true, event.timestamp);
    
    raise_raw_hid_sent_event(
        (struct raw_hid_sent_event){
            .data = drag_scroll_on,
            .length = sizeof(drag_scroll_on),
        }
    );

    return ZMK_EV_EVENT_BUBBLE;
}

static int on_drag_scroll_released(struct zmk_behavior_binding *binding,
                                   struct zmk_behavior_binding_event event) {
    LOG_DBG("DRAG SCROLL RELEASED"); /* MATTHIEU*/
    raise_zmk_keycode_state_changed_from_encoded(HID_USAGE_KEYBOARD_F24, false, event.timestamp);

    raise_raw_hid_sent_event(
        (struct raw_hid_sent_event){
            .data = drag_scroll_off,
            .length = sizeof(drag_scroll_off),
        }
    );

    return ZMK_EV_EVENT_BUBBLE;
}

static const struct behavior_driver_api drag_scroll_driver_api = {
    .binding_pressed = on_drag_scroll_pressed,
    .binding_released = on_drag_scroll_released,
};

#define DRAG_SCROLL_INST(n) \
    BEHAVIOR_DT_INST_DEFINE( \
        n, NULL, NULL, NULL, NULL, \
        POST_KERNEL, \
        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, \
        &drag_scroll_driver_api \
    );


/* MATTHIEU */
#if DT_HAS_COMPAT_STATUS_OKAY(zmk_behavior_raw_drag_scroll)
#warning "RAW_DRAG_SCROLL COMPAT FOUND"
#endif
/* END OF MATTHIEU */

DT_INST_FOREACH_STATUS_OKAY(DRAG_SCROLL_INST)
