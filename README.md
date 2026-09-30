# DirectKeycast

Notice: Under construction. Lightweight usage is now available. If you encounter any issues, feel free to report them in the Issues section.

Rev of [Keycastow](https://github.com/brookhong/KeyCastOW).

![](https://i.postimg.cc/VLGnmh7W/image.png)

## How to build and run

Requirements: Visual Studio 2022 or 2026 with the "Desktop development with C++" workload, CMake 3.25+, Git.

```powershell
git clone --recursive https://github.com/fanlusky/DirectKeycast.git
cd DirectKeycast
.\scripts\llaunch.ps1
```

The scripts pick the newest installed Visual Studio, initialize the `msimeui` submodule if needed, and output to `build\bin\<Config>\directkeycast.exe`.

| Script | What it does |
| --- | --- |
| `scripts\lcompile.ps1` | Build Debug |
| `scripts\lcompile-release.ps1` | Build Release |
| `scripts\llaunch.ps1` | Build Debug, then run |
| `scripts\llaunch-release.ps1` | Build Release, then run |
| `scripts\lrun.ps1 [-Configuration Release]` | Run an existing build |
| `scripts\build.ps1` | The underlying script: `-Configuration Debug\|Release`, `-Preset auto\|vs2026\|vs2022`, `-Fresh`, `-Run` |

The wrappers forward extra arguments, e.g. `.\scripts\lcompile.ps1 -Preset vs2022 -Fresh`. You can also use the presets directly (`cmake --preset vs2026` then `cmake --build --preset vs2026-release`) or open the folder in Visual Studio.

For clangd, generate `.clangd` with the paths of your checkout:

```powershell
python .\scripts\prepare_env.py
```

## Settings

Left-click the tray icon (or run `directkeycast.exe` again) to open the settings window. Right-click the tray icon for the quick menu.

Settings are saved to `%LOCALAPPDATA%\DirectKeycast\config.toml`; `assets\config\config.toml` documents every key.

## Hotkeys

- `Ctrl + Alt + Shift + F11`: show / hide the key overlay.
- `Ctrl + Alt + Shift + F12`: exit.

## How to exit

Use the tray menu, press `Ctrl + Alt + Shift + F12`, or kill it from task manager.
