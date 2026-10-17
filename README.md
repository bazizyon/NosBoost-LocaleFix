# Locale Fix

A NosBoost mod that fixes special letters (like ş, ı, ğ, ł, č) turning into boxes or question marks.

The game sometimes forgets which letters it should draw, mostly after you resize the window. This mod tells it again and keeps it that way.

## How to use

Install it, then resize the game window once. The letters come back.

If your letters are still wrong, set your language's code page in `mods\LocaleFix.ini`:

```
[LocaleFix]
CodePage=1254
```

1254 is Turkish, 1250 is Central European (Polish, Czech), 1251 is Russian, 1252 is Western European.

## Install

Download the DLL from this repo's Releases and put it in the game's `mods` folder. NosBoost loads it by itself. Open it from the NosBoost mods menu (F9).
