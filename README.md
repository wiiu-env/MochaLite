# Mocha lite - a simple custom firmware
This a lite version of the [original mocha](https://github.com/dimok789/mocha).

## Usage
Place the `payload.elf` in the `sd:/wiiu` folder of your sd card and run the [browser exploit](https://github.com/wiiu-env/JsTypeHax), this will reboot the OS with modikation.

You can also place a RPX as `root.rpx` in the `sd:/wiiu` folder which will be run before the system menu. For example you can use this [payload](https://github.com/wiiu-env/SystemMenuHook) to create a main-hook.

Opening the "Health and Safety" application will try to run the [Homebrew Launcher](https://github.com/dimok789/homebrew_launcher/) from `sd:/wiiu/apps/homebrew_launcher/homebrew_launcher.rpx` from sd card.

## IOSU patches
- disable sig-checks
- RPX redirection
- overall sd access
- wupserver and own IPC which can be used with [libiosuhax](https://github.com/wiiu-env/libiosuhax).

## Building

For building you just need [wut](https://github.com/devkitPro/wut/) installed, then use the `make` command.

## Credits
dimok
Maschell
orboditilt
QuarkTheAwesome