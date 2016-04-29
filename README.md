## noborder, the 'Borderless' mode switch

Switch current window to 'Borderless' mode with a hotkey.
The window will be borderless, centered and maximized, while maintaining the aspect ratio.
Press Alt+Backspace to toggle.

As of v1.2, options of 'Always on Top' and 'Exclude Taskbar' are available.

## What games are supported?

Most games that use DirectX/OpenGL, and support windowed mode, should work just fine.
However, you might have trouble using your mouse to select things in in-game menu.

For games that use GDI/GDI+, such as:
3D Pinball (WinXP), Hover! (Win95), Chip's Challenge, Rat Poker, Jewel Chase...
You need at least v1.3, and enable the 'DWM formula', in order to 'noborder' them.

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
If it doesn't, try to press Alt+Backspace.

## Why you made this?

To be honest, initially, this is created for the sake of my Touhou games. 640x480 windowed mode hurts.
Offcial Touhou games from 6 to 15, as well as CtC, MPP, RSS, all is well.
The only game that doesn't work is Touhou 7.5 aka Immaterial and Missing Power.

Update: Added 'DWM formula' in v1.3, now even Touhou 7.5 works wonder. :)

## WTF? noborder tiny? VC6.0?

Before you yell at me, please note that I was developing this in Visual Studio 2015.
I only convert the code to satisfy VC6.0, when I want to release.
Because VC6.0 can compile small EXE that links to msvcrt.dll.
The VC6.0 ver is meant for those who can't/didn't install vc2015 redist.

I know the disadvtanges of linking msvcrt.dll, that's why this is just an alternative.
Use 'noborder.exe' when you can, use 'noborder tiny.exe' only when you must.
