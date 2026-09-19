#define DT_DRV_COMPAT zmk_auto_mouse_layer

#include <zephyr/kernel.h>

#include <zmk/event_manager.h>
#include <zmk/keymap.h>
#include <raw_hid/events.h>
#include <zmk/events/hid_indicators_changed.h>
#include <zephyr/sys/util.h>

#define PLOOPY_AUTO_MOUSE_LAYER         0x41
#define PLOOPY_AUTO_MOUSE_LAYER_VERSION 0x01
#define PLOOPY_CAPS_LOCK_INDICATOR    BIT(1)

struct auto_mouse_layer_config {
    zmk_keymap_layer_id_t mac_base_layer;
    zmk_keymap_layer_id_t mac_mouse_layer;
    zmk_keymap_layer_id_t windows_base_layer;
    zmk_keymap_layer_id_t windows_mouse_layer;
};

static const struct auto_mouse_layer_config auto_mouse_layer_config = {
    .mac_base_layer = DT_INST_PROP(0, mac_base_layer),
    .mac_mouse_layer = DT_INST_PROP(0, mac_mouse_layer),
    .windows_base_layer = DT_INST_PROP(0, windows_base_layer),
    .windows_mouse_layer = DT_INST_PROP(0, windows_mouse_layer),
};


static bool auto_mouse_layer_active;
static zmk_keymap_layer_id_t auto_mouse_layer;

static bool caps_lock_indicator_initialized;
static bool last_caps_lock_indicator;

static void auto_mouse_layer_timeout(struct k_work *work);

K_WORK_DELAYABLE_DEFINE(
    auto_mouse_layer_timeout_work,
    auto_mouse_layer_timeout
);
static void auto_mouse_layer_timeout(struct k_work *work) {
    ARG_UNUSED(work);

    if (!auto_mouse_layer_active) {
        return;
    }

    if (zmk_keymap_layer_active(auto_mouse_layer)) {
        zmk_keymap_layer_deactivate(auto_mouse_layer);
    }

    auto_mouse_layer_active = false;
}

static zmk_keymap_layer_id_t get_auto_mouse_layer(void) {
    if (zmk_keymap_layer_active(auto_mouse_layer_config.windows_base_layer)) {
        return auto_mouse_layer_config.windows_mouse_layer;
    }

    if (zmk_keymap_layer_active(auto_mouse_layer_config.mac_base_layer)) {
        return auto_mouse_layer_config.mac_mouse_layer;
    }

    return ZMK_KEYMAP_LAYER_ID_INVAL;
}

static void activate_auto_mouse_layer(void) {
    zmk_keymap_layer_id_t target = get_auto_mouse_layer();

    if (target == ZMK_KEYMAP_LAYER_ID_INVAL) {
        return;
    }

    if (auto_mouse_layer_active && auto_mouse_layer != target) {
        if (zmk_keymap_layer_active(auto_mouse_layer)) {
            zmk_keymap_layer_deactivate(auto_mouse_layer);
        }

        auto_mouse_layer_active = false;
    }

    /*
     * If the Mouse layer is already active, it was not activated
     * by this controller. Leave it alone and do not claim ownership.
     */
    if (!zmk_keymap_layer_active(target)) {
        if (zmk_keymap_layer_activate(target) == 0) {
            auto_mouse_layer = target;
            auto_mouse_layer_active = true;
        }
    }

    k_work_reschedule(
        &auto_mouse_layer_timeout_work,
        K_MSEC(CONFIG_ZMK_AUTO_MOUSE_LAYER_TIMEOUT_MS)
    );
}

static int hid_auto_mouse_layer_listener(const zmk_event_t *eh) {
    struct raw_hid_received_event *event =
        as_raw_hid_received_event(eh);

    if (!event || event->length < 2) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    if (event->data[0] != PLOOPY_AUTO_MOUSE_LAYER ||
        event->data[1] != PLOOPY_AUTO_MOUSE_LAYER_VERSION) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    activate_auto_mouse_layer();

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(hid_auto_mouse_layer, hid_auto_mouse_layer_listener);
ZMK_SUBSCRIPTION(hid_auto_mouse_layer, raw_hid_received_event);

static int led_auto_mouse_layer_listener(const zmk_event_t *eh) {
    const struct zmk_hid_indicators_changed *event =
        as_zmk_hid_indicators_changed(eh);

    if (!event) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    const bool caps_lock =
        (event->indicators & PLOOPY_CAPS_LOCK_INDICATOR) != 0;

    if (!caps_lock_indicator_initialized) {
        last_caps_lock_indicator = caps_lock;
        caps_lock_indicator_initialized = true;
        return ZMK_EV_EVENT_BUBBLE;
    }

    if (caps_lock != last_caps_lock_indicator) {
        last_caps_lock_indicator = caps_lock;

        if (zmk_keymap_layer_active(auto_mouse_layer_config.windows_base_layer) &&
            !auto_mouse_layer_active &&
            !zmk_keymap_layer_active(auto_mouse_layer_config.windows_mouse_layer)) {
            activate_auto_mouse_layer();
        }
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(led_auto_mouse_layer, led_auto_mouse_layer_listener);
ZMK_SUBSCRIPTION(led_auto_mouse_layer, zmk_hid_indicators_changed);
