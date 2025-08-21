# DirectKeycast

Notice: Under construction. Lightweight usage is now available. If you encounter any issues, feel free to report them in the Issues section.

The latest release is at [latest](https://github.com/lsq/DirectKeycast/releases/tag/latest).

Rev of [Keycastow](https://github.com/brookhong/KeyCastOW).

![](https://i.postimg.cc/VLGnmh7W/image.png)

## How to build and run

### For Windows MSVC
Notice: Please make sure you open this settings before you start to compile,

![](https://github.com/user-attachments/assets/6fd14c4e-b09c-421e-8263-f35b8251a732)

Then, generate the config files,

```powershell
python .\scripts\prepare_env.py
```

Then, run the following command in your powershell,

```powershell
.\scripts\llaunch.ps1
```

### For Windows GCC

```bash
bash ./scripts/build.sh
```

## How to exit

Kill it at the system tray pop-up menu.

Or, use hotkey: `Ctrl + Alt + Shift + F12`.

Or, kill it from task manager.
