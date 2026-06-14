# SpindleRPM (Arduino Nano + Hall Sensor + 1" OLED)

This project reads a hall-effect sensor mounted near a spindle magnet and shows spindle RPM on a 1" I2C OLED display.

On power-up, the OLED runs a startup count animation from `0` to `10000` over `5` seconds, then switches to live RPM display.

## Project Structure

- `SpindleRPM.ino` - Main Arduino sketch
- `arduino-libraries.txt` - Required Arduino libraries
- `.gitignore` - Common Arduino build artifact ignores
- `README.md` - Project documentation and wiring

## Hardware

- Arduino Nano (ATmega328P)
- Hall sensor module (digital output)
- 1" OLED display (SSD1306, I2C, typically `128x64`, address `0x3C`)
- 1 magnet on spindle
- Jumper wires

## Connectivity (Wiring)

### Hall Sensor Module -> Arduino Nano

- `VCC` -> `5V`
- `GND` -> `GND`
- `DO`/`D0` (digital output) -> `D2` (external interrupt pin)
- `A0` (analog output) -> not used in this firmware (leave unconnected unless you want analog field-strength sensing)

### OLED I2C Display -> Arduino Nano

- `VCC` -> `5V`
- `GND` -> `GND`
- `SDA` -> `A4`
- `SCK` -> `A5`

## How RPM is Calculated

- Each magnet pass generates one pulse from the hall sensor.
- Time between pulses is measured in microseconds.
- RPM uses:

  $$RPM = \frac{60,000,000}{\text{pulseIntervalUs} \times \text{pulsesPerRev}}$$

- In this project, `pulsesPerRev = 1` (single magnet).

## Features in Current Firmware

- Interrupt-based pulse capture on `D2`
- Input debounce/noise rejection (`MIN_PULSE_INTERVAL_US`)
- Signal timeout to force `0 RPM` when spindle stops
- Startup animation: `0 -> 10000` in `5s`
- Smoothed RPM display for stable readout

## Arduino IDE Setup

1. Install **Arduino IDE**.
2. Open `SpindleRPM.ino`.
3. In Library Manager, install:
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`
4. Select board: **Arduino Nano**.
5. Select processor matching your Nano bootloader.
6. Select COM port and upload.

## PlatformIO Setup (VS Code)

1. Install the **PlatformIO IDE** extension in VS Code.
2. Open this folder (`SpindleRPM`) in VS Code.
3. PlatformIO will use `platformio.ini` and fetch required libraries automatically.

Build:

```powershell
Set-Location "C:\Users\mike\source\Arduino-ESP\SpindleRPM"
& "$env:USERPROFILE\.platformio\python3\python.exe" -m platformio run --project-dir ".\src"
```

Upload (replace COM port if needed):

```powershell
Set-Location "C:\Users\mike\source\Arduino-ESP\SpindleRPM"
& "$env:USERPROFILE\.platformio\python3\python.exe" -m platformio run -t upload --project-dir ".\src" --upload-port COM3
```

Serial monitor:

```powershell
Set-Location "C:\Users\mike\source\Arduino-ESP\SpindleRPM"
& "$env:USERPROFILE\.platformio\python3\python.exe" -m platformio device monitor --port COM3 --baud 115200
```

## VS Code Tasks

This workspace includes `.vscode/tasks.json` with ready-to-run tasks:

- `PIO: Build`
- `PIO: Upload`
- `PIO: Upload (COM3)`
- `PIO: Monitor (COM3, 115200)`
- `PIO: Clean`

Run from **Terminal -> Run Task...** and select the task.
If your Nano is not on `COM3`, either edit the task or use `PIO: Upload` (auto port detection).

## Notes / Tuning

- If your hall module outputs active-high pulses, change interrupt mode in `setup()` from `FALLING` to `RISING`.
- If you add multiple magnets, set `PULSES_PER_REV` accordingly.
- If your OLED is at `0x3D`, update `OLED_ADDRESS`.

## Dependency Files

- `arduino-libraries.txt` is for Arduino IDE users.
- `platformio.ini` is for PlatformIO users.
