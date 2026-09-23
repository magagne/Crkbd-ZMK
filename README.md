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

The Ploopy generates a Caps Lock keyboard event when physical ball movement
starts. Windows updates the Caps Lock state and sends the resulting LED state
to the keyboard. The ZMK firmware uses that Caps Lock LED state as the
Auto Mouse Layer signal:

- Caps Lock ON → Windows Mouse layer ON
- Caps Lock OFF → Windows Mouse layer OFF

The Windows path is state-based and does not use the normal Auto Mouse Layer
timeout. The 450 ms timeout applies to the Raw HID/macOS path.

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

The current Raw HID/macOS timeout is:

    CONFIG_ZMK_BEHAVIOR_AUTO_MOUSE_LAYER_TIMEOUT_MS=450

After the timeout expires without another Raw HID Auto Mouse Layer
notification, the controller releases a Mouse layer that it activated itself.

The Windows Caps Lock LED path is state-based and does not use this timeout.

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

    config/src/behavior_auto_mouse_layer.c

The device-tree binding is:

    config/dts/bindings/behaviors/zmk,behavior-auto-mouse-layer.yaml
