## What's noborder?

Simply put, it is a tool, that makes a window to be borderless.

## How to use?

When noborder is running, just press Alt+Backspace, the current window will be maximized in such a way that:
* the aspect ratio will be maintained
* the window will be centered and Always On Top

Press Alt+Backspace again, everything will be reverted in no time.

## Why do this?

* 'Alt+Tab' can be quite time-consuming sometimes.
* Some games have windowed mode designed for ants, such as Touhou Imperishable Night.
* Let's say you have a game, in windowed mode, 20FPS if 1600x900, 30FPS if 1280x720. If the game doesn't change resolution as the window size changed, you can set it to 1280x720 first, and then noborder it. Voila, you can get similar experience as if using 1600x900, yet still having 30FPS.

## What games are supported?

Most games that use hardware acceleration aka graphic card, and support windowed mode, should work just fine.
However, you might have trouble using your mouse to select things in in-game menu.

## Why you made this?

To be honest, initially, this is created for the sake of my Touhou games. 640x480 windowed mode hurts.
Offcial Touhou games from 6 to 15, as well as CtC, MPP, RSS, all is well.
The only game that doesn't work is Touhou 7.5 aka Immaterial and Missing Power.

## WTF? noborder tiny? VC6.0?

As you may noticed, I still use Visual C++ 6.0.
Before you yell at me, please note that I was developing this in Visual Studio 2015.
I only convert the code to satisfy VC6.0, when I want to release.

Why? Because VC6.0 can compile small EXE that relys on msvcrt.dll, which doesn't need vc redist to run.
You may stick with the "noborder.exe" if you want.
"noborder tiny.exe" is for those who doesn't have vc2015 redist installed.
