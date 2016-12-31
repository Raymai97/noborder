@echo off
REM Compilable with g++.exe (tdm-1) 5.1.0
REM It will produce EXE that supports Win2K and XP
REM The EXE file size will be big though

setlocal enableDelayedExpansion
set linkFlag=-mwindows -Wl,--major-os-version,5

g++ -c MAIN.cpp -std=gnu++11 -U __STRICT_ANSI__ 

windres res.rc -O coff -o res.res

g++ MAIN.o res.res -s -o "nbd_gcc.exe" !linkFlag!
