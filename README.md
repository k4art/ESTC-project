*Final Project of Embedded Systems Laboratory 2022-2023*

# LED Color Picker

Controlling the color of an LED via a button, CLI over USB, and BLE.
The lastest set color presists in flash memory.

For nRF52840 Dongle (pca10059).

## Build

Requirements:
- RF52 Nordic SDK, set the path to SDK to `SDK_ROOT` environment variable;
- ARM EABI (bare-metal) GNU toolchain (arm-none-eabi-gcc),
- `make`.

```bash
make -C pca10059/s140/armgcc dfu
```

or with CLI over USB enabled,
```bash
make -C pca10059/s140/armgcc dfu ESTC_USB_CLI_ENABLED=1
```

## Usage

When the dongle is plugged in the first time, there is no stored color in the flash.
In this case the default color is calculated in HSV color model:
H = (`DEVICE_ID mod 100`)%, S = 100%, V = 100%, where `DEVICE_ID` is 7202.

The color is stored in flash in HSV color model. Because of rounding during
conversions between HSV and RGB, setting some of RGB values results in slight
different RGB value after conversions to HSV and then back to RGB. For example,
after setting `0x00FF00` RGB color via BLE, the color read back is `0x03FF00`.

### Via Button

Changing the color of LED process consists of 3 stages,
each one modifies one component of the color according to HSV color model.
*Double click* the button to change the mode of the color picker in the following cyclic order:
1. Viewer (initially)
2. Changing Hue
3. Changing Saturation
4. Changing Value

Once the Viewer mode is reached, the set color overwrites the previous one stored in flash.
To change a color component according to the current mode **long press** the button.

### Via CLI

Change the color via `rgb` and `hsv` commands.

```
> rgb <red> <green> <blue>
> hsv <hue> <saturation> <value>
```

These commands accept decimal values, for example, for red
```
> rgb 255 0 0
```
and does not affect the operation mode of via-button controller,
the set color overwrites the previous color stored in flash.


### Via BLE

The system advertizes its itself as *ESTC Color Picker*. Using
a custom service with *LED color* and *Change LED color* attributes a user can
read and enable notification of current color with the first and change
the color with the second one. Similar to CLI behavior, changing color via BLE
does not affect the operation mode of via-button controller,
and the written color overwrites the previous color stored in flash.

Both attributes employes RGB color model, which can be interpreted as 3-byte value.
Write and interprent the value in MSB ordering, so that to modify the LED color
to red, use `0xFF0000`, for blue - `0x0000FF`.

Write the *Change LED color* attribute requires bonding, "Just Works" method is employed
with no MITM protection. After some device bonded with the system, the system will still
allow other devices to connect and bond with it.

## Debug

The system opens a COM port for logging purposes. If the CLI is enabled,
the logger COM port opens before the CLI, so `/dev/ttyACM0` is expect to be the logs,
while `/dev/ttyACM1` is CLI.

The flash pages in the custom `fvarstorage` module are not fully utilized before switching
to the next one. So that for testing purposes the corner-case with page translation
happens often. See `/flash/fvarstorage/internals/fvarstorage_page_format_defs.h` file and
`PAGE_RECORDS_SIZE` specifically to change this behavior.
