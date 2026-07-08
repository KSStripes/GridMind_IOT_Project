# Arduino and ESP8266 MCU Setup on NVIDIA DGX Spark

**Project:** CM3040 GridMind  
**Planned host:** NVIDIA DGX Spark  
**Target boards:** AZDelivery/Lolin NodeMCU V3 ESP8266 ESP-12F  
**Prepared:** 6 July 2026  

## Day 1 verified progress — 6 July 2026

### Development host

- Hostname: `spark-e09a`
- Architecture: `aarch64`
- Operating system: Ubuntu 24.04.4 LTS on NVIDIA DGX Spark
- Kernel: `6.17.0-1021-nvidia`
- Python: 3.12.3
- User: `kristins` (`uid=1000`)
- Initial recorded groups: `kristins adm sudo audio plugdev users lpadmin`.
- The user was subsequently added to `dialout`; active membership was verified with `groups`.
- Home storage was approximately 2% used, so storage is sufficient.
- `lsusb` ran successfully and listed the DGX USB buses.

### Arduino and ESP8266 setup

- Downloaded file: `arduino-1.8.19-linuxaarch64.tar.xz`
- The SHA-512 checksum was compared with the official Arduino value and verified.
- Arduino was extracted to `/home/kristins/Applications/arduino-1.8.19/`.
- Arduino IDE launched successfully and displayed `Arduino 1.8.19` in the window title.
- Terminal warning observed: `Failed to load module "canberra-gtk-module"`.
  This is currently treated as a harmless optional GTK sound-module warning because the IDE opened and operated normally. No package was installed in response.
- ESP8266 Boards Manager URL was accepted:
  `https://arduino.esp8266.com/stable/package_esp8266com_index.json`
- Installed board package: **esp8266 by ESP8266 Community 3.1.2**.
- Selected board: **NodeMCU 1.0 (ESP-12E Module)**.
- The built-in Blink example compiled successfully with `Done compiling`.
- Reported IROM/code-in-flash size: 232,148 bytes.
- USB identification for the first board: QinHeng CH340, USB ID `1a86:7523`.
- Ubuntu's `brltty` service claimed the CH340 interface after restart and prevented `/dev/ttyUSB0` from being created. Kernel evidence was:
  `usbfs: interface 0 claimed by ch341 while 'brltty' sets config #1`.
- The student confirmed that no USB Braille display is used. `brltty.service` and `brltty-udev.service` were therefore permanently masked with `systemctl mask --now`. This retained the installed package while preventing the confirmed CH340 conflict on future boots.
- After reconnecting the board, `/dev/ttyUSB0` was created successfully and selected in Arduino IDE.
- Blink uploaded successfully to the first physical NodeMCU, and its blue built-in LED was visually verified blinking regularly.
- The unchanged Blink sketch also uploaded successfully to the second physical NodeMCU on `/dev/ttyUSB0`, and its blue built-in LED was visually verified blinking regularly.
- Evidence files:
  - `screenshots/board1-upload.png`
  - `screenshots/board1-upload.jpeg`
  - `screenshots/board1-upload.MOV`
- A DGX logout/shutdown caused the local HDMI output to disappear after the NVIDIA boot logo even though Ubuntu and GDM had booted successfully. Network ping and SSH confirmed the host was healthy. HDMI hot-plug restored text-console output, and `Ctrl+Alt+F2` returned to the GUI. Avoid logout/reboot unless it is strictly required and agreed in advance.

### Exact stopping point

- Board 1 and Board 2 have both passed USB detection, Blink compilation/upload, and physical LED verification.
- No external circuit or component has been connected.
- The DGX Spark is confirmed as the development host.

### Next safe action

Work in one small step at a time because this is the student's first time handling Arduino hardware.

1. Retain Board 1 as the Node A candidate and Board 2 as the Node B candidate in separate labelled storage.
2. On Day 2, test one external LED with a current-limiting resistor, then one button using `INPUT_PULLUP`.
3. Do not assemble a complete node until the pin map and circuit design have been reviewed.

## Purpose and decision rule

This guide tests whether the DGX Spark can serve as the GridMind development computer while satisfying the coursework requirement that all source code be developed and compiled using Arduino IDE.

The DGX Spark has an ARM64 processor and runs Ubuntu 24.04-based DGX OS. The official Arduino IDE 2 Linux desktop downloads target x86-64, so the planned native DGX route uses **Arduino IDE 1.8.19 Legacy for Linux ARM 64-bit**.

Apply a strict **90-minute troubleshooting limit** to the complete DGX route. Retain the DGX Spark only if Arduino IDE launches, the ESP8266 package installs, Blink compiles, USB detection works, and Blink uploads successfully to both intended boards. Otherwise, move to the M3 Mac and the official Arduino IDE 2 Apple Silicon build. Do not add x86 emulation or build Arduino IDE 2 from source merely to retain the DGX as host.

> **Important:** Perform one checkpoint at a time. At the first failure, preserve the exact output and diagnose it before applying a fix.

## Stage 1 — Inspect the DGX Spark

These commands inspect the computer without installing or changing the development environment:

```bash
date -Is
uname -m
uname -r
cat /etc/os-release
hostnamectl
python3 --version
id
groups
```

Expected key results:

- `uname -m` reports `aarch64`.
- The OS information identifies DGX OS based on Ubuntu 24.04.
- Python is version 3.7 or newer.
- The account will ideally belong to `sudo`; it may not yet belong to `dialout`.

Inspect the graphical session:

```bash
printf 'Session: %s\nDesktop: %s\nDisplay: %s\nWayland: %s\n' \
  "$XDG_SESSION_TYPE" \
  "$XDG_CURRENT_DESKTOP" \
  "$DISPLAY" \
  "$WAYLAND_DISPLAY"
```

Check available storage:

```bash
df -h "$HOME"
```

Confirm that USB inspection is available:

```bash
command -v lsusb
lsusb
```

Record the architecture, OS release, kernel, Python version, desktop status, internet status, and relevant account groups.

Do not update the complete operating system merely for this Arduino test. NVIDIA recommends the DGX Dashboard for DGX OS updates, and an OS update would add unnecessary variables during this timeboxed compatibility check.

## Stage 2 — Download the correct Arduino IDE

From the [official Arduino software page](https://www.arduino.cc/en/software), select:

- **Version:** Arduino IDE 1.8.19 Legacy
- **Platform:** Linux ARM 64 bits
- **Filename:** `arduino-1.8.19-linuxaarch64.tar.xz`

Do **not** download or use:

- the Arduino IDE 2 Linux AppImage or ZIP, because the official Linux desktop package targets x86-64;
- `arduino-1.8.19-linux64.tar.xz`, which is also x86-64;
- `arduino-1.8.19-linuxarm.tar.xz`, which is the 32-bit ARM build;
- an `apt install arduino` distribution package;
- Arduino CLI or PlatformIO as the primary build environment, because the coursework specifies Arduino IDE.

The terminal download commands are:

```bash
cd "$HOME/Downloads"
curl -fLO https://downloads.arduino.cc/arduino-1.8.19-linuxaarch64.tar.xz
curl -fLO https://downloads.arduino.cc/arduino-1.8.19.sha512sum.txt
```

Verify the downloaded archive:

```bash
sha512sum arduino-1.8.19-linuxaarch64.tar.xz
```

The expected SHA-512 value is:

```text
6760507edc510fcb45a46be7b2130d1d519562580a0717fade3c5f124eb7128cb31e75d373082f941a6c543a4925076eb16ab3701f20d1df6186b9f6a2756b9e
```

An automatic comparison can be made with:

```bash
grep 'arduino-1.8.19-linuxaarch64.tar.xz$' \
  arduino-1.8.19.sha512sum.txt | sha512sum --check
```

Expected result:

```text
arduino-1.8.19-linuxaarch64.tar.xz: OK
```

Stop if the result is not `OK`.

## Stage 3 — Extract and test-launch Arduino IDE

Test the application from the user's home directory before adding system-wide links or shortcuts:

```bash
mkdir -p "$HOME/Applications"
tar -xJf "$HOME/Downloads/arduino-1.8.19-linuxaarch64.tar.xz" \
  -C "$HOME/Applications"
cd "$HOME/Applications/arduino-1.8.19"
./arduino
```

Launching Arduino IDE will create user configuration and cache data, normally under `~/.arduino15`.

This checkpoint passes when:

- the Arduino IDE window opens;
- its menus respond normally; and
- the terminal shows no architecture, Java, or missing-library error.

If it fails, save the complete terminal output and stop. Do not immediately install another Java version, introduce x86 emulation, or attempt an IDE 2 source build.

Arduino's official Linux guide also offers this installation command:

```bash
cd "$HOME/Applications/arduino-1.8.19"
sudo sh install.sh
```

This is an optional system-level operation that may add an application launcher and links. It is unnecessary until direct launching has succeeded.

## Stage 4 — Add ESP8266 board support

In Arduino IDE:

1. Open **File → Preferences**.
2. Find **Additional Boards Manager URLs**.
3. Enter:

   ```text
   https://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```

4. Select **OK**.
5. Open **Tools → Board → Boards Manager**.
6. Search for `esp8266`.
7. Select **esp8266 by ESP8266 Community**.
8. Install the stable version offered and record its exact version.

The upstream stable release listed at the time this guide was prepared was 3.1.2. Use and record the version actually offered by Boards Manager rather than assuming a version.

After installation, select:

**Tools → Board → ESP8266 Boards → NodeMCU 1.0 (ESP-12E Module)**

Keep the default board settings for the first test. Record:

- ESP8266 core version;
- board selection;
- CPU frequency;
- flash size; and
- upload speed.

## Stage 5 — Compile Blink before connecting hardware

Open:

**File → Examples → 01.Basics → Blink**

Click **Verify**, represented by the checkmark button.

This proves that Arduino IDE 1.8.19 can invoke the ESP8266 compilation toolchain on the DGX Spark. It does not yet prove that USB uploading works.

Preserve:

- a screenshot of the IDE and selected board;
- the complete successful compilation output; and
- the displayed program-storage and memory usage.

If compilation fails, stop and save the first complete error. This is a DGX go/no-go checkpoint.

## Stage 6 — Detect one bare NodeMCU

Do not attach sensors, LEDs, jumper wires, or a breadboard. Use only:

- one NodeMCU board;
- one known data-capable Micro-USB cable; and
- the USB-A-to-USB-C adapter.

Before connecting the board, run:

```bash
lsusb
find /dev -maxdepth 1 -type c \
  \( -name 'ttyUSB*' -o -name 'ttyACM*' \) -print
```

Connect the bare board and repeat:

```bash
lsusb
find /dev -maxdepth 1 -type c \
  \( -name 'ttyUSB*' -o -name 'ttyACM*' \) -print
```

Likely observations are:

- a QinHeng/WCH CH340 USB-serial device appears in `lsusb`;
- a port such as `/dev/ttyUSB0` appears; and
- a small power LED illuminates on the board.

Inspect recent kernel messages if necessary:

```bash
journalctl --dmesg --since '-2 minutes' --no-pager
```

Do not install a third-party CH340 driver pre-emptively. Linux normally includes the required kernel driver.

## Stage 7 — Check serial-port permissions

If the port is `/dev/ttyUSB0`, inspect it with:

```bash
ls -l /dev/ttyUSB0
groups
```

The device will commonly belong to the `dialout` group. If the current user is already in `dialout`, do not change the group configuration.

Only if Arduino reports a serial-port permission error, the normal Ubuntu correction is:

```bash
sudo usermod -aG dialout "$USER"
```

This changes system account membership. Log out of the desktop completely and log back in before checking:

```bash
groups
```

Do not launch Arduino IDE with `sudo`, and do not use `chmod 777` on the serial device.

## Stage 8 — Upload Blink to the first board

In Arduino IDE:

1. Confirm **NodeMCU 1.0 (ESP-12E Module)** is selected.
2. Select **Tools → Port → `/dev/ttyUSB0`**, adjusting the name if detection produced another port.
3. Keep the built-in Blink example open.
4. Click **Upload**, represented by the right-arrow button.

A successful upload commonly ends with output similar to:

```text
Leaving...
Hard resetting via RTS pin...
```

The board's built-in LED should flash. Many ESP8266 NodeMCU boards use inverted logic for the built-in LED; that is normal.

Preserve:

- the complete successful upload output;
- a screenshot showing the board and port selections;
- a clear photograph or short video of the blinking board; and
- a physical-board identifier such as **Node A candidate**.

Do not connect both project boards simultaneously during the first proof.

## Stage 9 — Repeat with the second board

Disconnect the first board, connect the second board with the same cable, and repeat detection and Blink upload.

The serial port may remain `/dev/ttyUSB0` when only one board is connected. This is normal. Preserve separate evidence proving that the second physical board works.

Record the final results:

```text
Host:
Architecture:
OS:
Kernel:
Arduino IDE version:
ESP8266 core version:
Board menu selection:
CPU frequency:
Flash size:
Upload speed:
Board 1 port and result:
Board 2 port and result:
Development-host decision:
Evidence file locations:
```

## DGX Spark completion criteria

The native DGX route succeeds only if all of the following are verified within the timebox:

1. Arduino IDE 1.8.19 launches normally.
2. ESP8266 board support installs through Boards Manager.
3. The Blink example compiles for the selected NodeMCU board.
4. Linux detects the USB-serial interface.
5. Blink uploads to the first physical board.
6. Blink uploads to the second physical board.

If any essential checkpoint remains unreliable, use the M3 Mac with the current official Arduino IDE 2 Apple Silicon package.

## Recommended primary sources

- [Arduino software downloads](https://www.arduino.cc/en/software)
- [Arduino IDE 1 installation on Linux](https://docs.arduino.cc/software/ide-v1/tutorials/Linux)
- [Arduino explanation of supported IDE versions](https://support.arduino.cc/hc/en-us/articles/22301294333084-Supported-versions-of-Arduino-IDE)
- [ESP8266 Arduino Core installation guide](https://arduino-esp8266.readthedocs.io/en/latest/installing.html)
- [ESP8266 Arduino Core releases](https://github.com/esp8266/Arduino/releases)
- [NVIDIA DGX Spark User Guide](https://docs.nvidia.com/dgx/dgx-spark/index.html)
- [NVIDIA DGX Spark architecture and software overview](https://docs.nvidia.com/dgx/dgx-spark-porting-guide/overview.html)
