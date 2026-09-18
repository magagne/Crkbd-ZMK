#include <zephyr/kernel.h>
#include <zmk/event_manager.h>
#include <zmk/keymap.h>
#include <raw_hid/events.h>

#define PLOOPY_MOUSE_ACTIVITY         0x41
#define PLOOPY_MOUSE_ACTIVITY_VERSION 0x01

#define MAC_BASE_LAYER                0
#define MAC_MOUSE_LAYER               3
#define WINDOWS_BASE_LAYER            4
#define WINDOWS_MOUSE_LAYER           7

static struct k_work_delayable mouse_activity_timeout_work;

static bool auto_mouse_layer_active;
static zmk_keymap_layer_id_t auto_mouse_layer;

static zmk_keymap_layer_id_t get_mouse_layer(void) {
    if (zmk_keymap_layer_active(WINDOWS_BASE_LAYER)) {
        return WINDOWS_MOUSE_LAYER;
    }

    if (zmk_keymap_layer_active(MAC_BASE_LAYER)) {
        return MAC_MOUSE_LAYER;
    }

    return UINT8_MAX;
}

static void mouse_activity_timeout(struct k_work *work) {
    ARG_UNUSED(work);

    if (!auto_mouse_layer_active) {
        return;
    }

    if (zmk_keymap_layer_active(auto_mouse_layer)) {
        zmk_keymap_layer_deactivate(auto_mouse_layer);
    }

    auto_mouse_layer_active = false;
}

static void activate_mouse_layer(void) {
    zmk_keymap_layer_id_t target = get_mouse_layer();

    if (target == UINT8_MAX) {
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
        &mouse_activity_timeout_work,
        K_MSEC(CONFIG_ZMK_MOUSE_ACTIVITY_TIMEOUT_MS)
    );
}

static int mouse_activity_listener(const zmk_event_t *eh) {
    struct raw_hid_received_event *event =
        as_raw_hid_received_event(eh);

    if (!event || event->length < 2) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    if (event->data[0] != PLOOPY_MOUSE_ACTIVITY ||
        event->data[1] != PLOOPY_MOUSE_ACTIVITY_VERSION) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    activate_mouse_layer();

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(mouse_activity, mouse_activity_listener);
ZMK_SUBSCRIPTION(mouse_activity, raw_hid_received_event);

static int mouse_activity_init(void) {
    k_work_init_delayable(
        &mouse_activity_timeout_work,
        mouse_activity_timeout
    );

    return 0;
}

SYS_INIT(
    mouse_activity_init,
    APPLICATION,
    CONFIG_APPLICATION_INIT_PRIORITY
);
