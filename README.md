# sandglass for M5StickC Plus

## requirements

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
|main|Change timer|
|left side(power off)|Power on|
|left side( < 1s)|Reset to top|
|left side( < 4s)|Reset to bottom|
|left side( >= 4s)|Power off|
|right siede|Change mode|

### battery bar

|color|description|
|---|---|
|blue|changing|
|green|normal|
|yellow|Low battery|
|red|Almost empty battery|

### mode

|mode|description|
|---|---|
|R|Repeat|
|M|Mute|

## License

[Boost Software License](./LICENSE_1_0.txt)
