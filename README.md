# sandglass for M5StickC Plus

![image](https://user-images.githubusercontent.com/645907/173222306-3b8348a4-ee0d-4407-831d-d796709369be.png)

## features

- software sanglass
- timer preset: 30s, 1m, 1m30s, 2m, 3m, 5m, 10m, 15m, 30m, 60m
- battery gauge
- auto power off
- repeat mode
- mute mode

## requirements

- [M5StickC Plus ( K016-P )](https://docs.m5stack.com/en/core/m5stickc_plus)
- [Arduino IDE](https://www.arduino.cc/en/software)
- [VCP Drivers](https://ftdichip.com/drivers/vcp-drivers/)
- [Visual Studio Code](https://code.visualstudio.com/)
- [Arduino for Visual Studio Code](https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino)

## preparation for build

0. Start Arduino IDE
1. `Preferences` -> Set `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json` to `Additional Board Manager URLs`
2. `Board Manager`( `Arduino: Board Manager` command in VS Code ) -> filter `esp32` -> install `esp32`
3. `Library Manager`( `Arduino: Library Manager` command in VS Code ) -> filter `M5StickCPlus` -> install `M5StickCPlus`

## how to build & upload

0. Start VS Code
1. Run `Arduino: Board Config` Command from Command Palette. Select `M5Stick-C (esp32)` on `Select Board`.
2. Connect a M5StickC Plus to your PC with USB cable.
3. Run `Arduino: Select Serial Port` Command from Command Palette. Select a M5StickC Plus's Port.
4. Run `Arduino: Upload` Command from Command Palette

## user interface

### button

|button|description|
|---|---|
|main|change timer|
|left side(power off)|power on|
|left side( < 1s)|reset to top|
|left side( < 4s)|reset to bottom|
|left side( >= 4s)|power off|
|right siede|change mode|

### battery gauge

|color|description|
|---|---|
|blue|charging ( gyro off )|
|green|normal|
|yellow|low battery|
|red|almost empty battery|

### mode

|mode|description|
|---|---|
|R|repeat|
|M|mute|

### timer preset

|timer|color|
|---|---|
|30s|blue|
|1m|red|
|1m30s|green|
|2m|cyan|
|3m|magenta|
|5m|yellow|
|10m|orange|
|15m|green yellow|
|30m|pink|
|60m|white|

You can edit timer preset in [`config.h`](./config.h).

## License

[Boost Software License](./LICENSE_1_0.txt)
