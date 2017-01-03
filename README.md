## noborder, the 'Borderless' mode switch

Switch current window to 'Borderless' mode while maintaining the aspect ratio.
Press hotkey (Alt+Backspace by default) to toggle.
Works on Windows 10/8.1/8/7/Vista/XP/2000.

![img](http://raymai97.github.io/noborder/pinball3D_small.jpg)

Starting by v1.2, 'Always on Top' and 'Exclude Taskbar' options are provided.
Starting by v1.3, you may opt to use 'DWM formula', which works better for some games.
Starting by v1.4, 'Win+Backspace' can be enabled as optional hotkey for noborder.

You may compile yourself (with MSVC2015/TDM-GCC), or [download the build here](http://raymai97.github.io/noborder/).
This project is released under [MIT](./LICENSE) license.

## What games are supported?

Most games that use DirectX/OpenGL and support window mode should work just fine.
However, you might have trouble using your mouse to select things in in-game menu.
If the game doesn't resize when you 'noborder' it, you should use 'DWM formula'.

## About 'DWM formula', the alternative mode

This mode requires DWM (Desktop Window Manager).
For Win2K/XP users, sorry this is unsupported by your OS.
For Vista/Win7 users, you must use the Aero theme to enable DWM.
For Win8/8.1/10 users, it just works. :)

This mode is ONLY for KEYBOARD-ONLY games, at least for now.
You CAN'T USE your mouse to interact with 'nobordered' window.
To use 'menu-bar', press ALT and use your arrow keys.
What about game's popup? How do you interact? Don't worry about this.
Your window will be restored automatically when such popup is detected.
If it doesn't, just press the hotkey (Alt+Backspace by default).

## Why you made this?

To be honest, initially, this is created for the sake of my Touhou games. 640x480 window mode hurts.
Offcial Touhou games from 6 to 15, as well as CtC, MPP, RSS, all is well.
The only game that doesn't work is Touhou 7.5 aka Immaterial and Missing Power.

Update: Added 'DWM formula' in v1.3, now even Touhou 7.5 works wonder. :)

## What happened to noborder tiny (MSVC6.0 build)?

No longer available as I've switched to [modern C++](http://raymai97.github.io/noborder/faq.html) and MSVC6.0 doesn't play well with modern C++.
