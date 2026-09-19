# Crkbd-ZMK

Custom ZMK firmware configuration for the Corne keyboard.

The configuration contains separate Mac and Windows base/Mouse layers and
supports automatic Mouse layer activation from Ploopy Nano 2 movement.

## Auto Mouse Layer

The keyboard-side Auto Mouse Layer controller listens to two independent
Ploopy notification transports.

### macOS — AutoMouseLayer-HID

The Ploopy sends a 32-byte Raw HID packet beginning with:

    A 01

`Ploopy-Bridge-HID` forwards that packet from the Ploopy to the keyboard.

The keyboard receives the Raw HID notification and activates the configured
Mac Mouse layer.

### Windows — AutoMouseLayer-LED

Windows does not use the Raw HID bridge for Auto Mouse Layer.

The Ploopy generates a Caps Lock keyboard event. Windows then updates the
Caps Lock LED state and sends that state to the keyboard. The ZMK firmware
uses the Caps Lock transition as the Auto Mouse Layer signal.

The LED signals are separate:

- **Caps Lock** — AutoMouseLayer
- **ScrollLock** — DragScroll

### Layer selection

The controller explicitly maps:

| OS | Base layer | Mouse layer |
|---|---:|---:|
| macOS | 0 | 3 |
| Windows | 4 | 7 |

The keyboard firmware owns the layer activation and timeout logic. It does
not infer the operating-system layer from the highest active layer.

If the Mouse layer was already active manually, the automatic controller
does not claim ownership.

### Timeout

The current timeout is:

    CONFIG_ZMK_AUTO_MOUSE_LAYER_TIMEOUT_MS=500

After the timeout expires without another Auto Mouse Layer notification, the
controller releases a Mouse layer that it activated itself.

## Other pointing features

The keyboard also provides the existing Raw HID DragScroll behavior.

`DragScroll-HID` uses:

    S  → DragScroll ON
    s  → DragScroll OFF

DragScroll remains independent of Auto Mouse Layer.

## Build

Firmware builds are handled by GitHub Actions using the ZMK v0.3.0 user
configuration workflow.

The repository workflow is:

    .github/workflows/build.yml

The custom firmware configuration is under:

    config/

The Auto Mouse Layer implementation is:

    config/src/auto_mouse_layer.c

The device-tree binding is:

    config/dts/bindings/zmk,auto-mouse-layer.yaml
