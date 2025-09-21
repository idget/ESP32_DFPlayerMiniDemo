# ESP32_DFPlayerMiniDemo

Simple PlatformIO project to control a DFPlayer Mini MP3 module from an ESP32 and play a file from microSD.

## Hardware

- `ESP32 DevKit` (e.g., esp32dev)
- `DFPlayer Mini`
- MicroSD card (FAT32)
- Speaker 3W 4–8Ω (or external amplifier)
- Optional: level shifting for DFPlayer RX (ESP32 TX -> DF RX) using resistor divider (1k/2k typical)

### Wiring (default pins in `src/hardware_config.h`)

- Power: DFPlayer `VCC` -> 5V, `GND` -> GND (common ground with ESP32)
  - If your ESP32 does not expose a `5V`/`VIN` pin, use an external 5V supply to power DFPlayer `VCC` and the level shifter `HV`. Tie all grounds together (ESP32 GND, DFPlayer GND, 5V GND).
- Serial: DFPlayer `TX` -> ESP32 `GPIO16` (RX2)
- Serial: DFPlayer `RX` <- ESP32 `GPIO17` (TX2) via divider recommended
- Audio: Use `SPK_1` and `SPK_2` to speaker, or `DAC_R`/`DAC_L` to amplifier

Change pins in `hardware_config.h` if you wire differently.

#### Wiring with a Level Shifter (recommended)

Use a 4‑channel bidirectional logic level shifter (BSS138 type). See `examples/wiring_esp32_dfplayer_level_shifter.svg`:

- Power the shifter: `LV` -> ESP32 `3V3`, `HV` -> 5V, `GND` -> common ground.
- Channel A (ESP32 -> DF RX):
  - `LV1` <- ESP32 `GPIO17` (TX2)
  - `HV1` -> DFPlayer `RX`
- Channel B (DF TX -> ESP32):
  - `HV2` <- DFPlayer `TX`
  - `LV2` -> ESP32 `GPIO16` (RX2)
- Keep all grounds common: ESP32 GND, DFPlayer GND, shifter GND

Notes:

- DFPlayer typically understands 3.3V UART, but many clones behave better with level shifting on DF `RX`.
- DF `TX` into ESP32 `RX` is usually safe directly; using the shifter both ways is also fine.
- DF `BUSY` (optional): goes LOW while playing; you can route via shifter to any ESP32 GPIO if needed.

ESP‑WROOM‑32 (Aitrip) board labels:

- `GPIO16` is often labeled `RX2` or `D16` on the silkscreen.
- `GPIO17` is often labeled `TX2` or `D17`.
- Use the board’s `5V` (or `VIN`) to power DFPlayer `VCC`, and `3V3` to power the shifter `LV`.
  - If your board has no `5V` pin, power DFPlayer `VCC` and shifter `HV` from an external 5V supply.
- Avoid using `GPIO1`/`GPIO3` (UART0) for DFPlayer; those are used by USB serial.


## SD Card Layout and Names

- Format card as FAT32.
- Recommended: place files in `/mp3` folder with 4-digit names: `/mp3/0001.mp3`, `/mp3/0002.mp3`, ...
- Alternatively root: `0001.mp3`, `0002.mp3`, ...

This demo tries to play `/mp3/0001.mp3` first, otherwise plays track index `1`.

## Audio Output

Two simple options:

- 2‑wire speaker (built‑in amp):
  - Connect the speaker directly to DFPlayer `SPK_1` and `SPK_2`. Do not connect either speaker wire to GND — the output is bridge‑tied (BTL).
  - Use 4–8 Ω small speaker. Start with moderate volume (e.g., 15–22) to avoid distortion.
  - Keep speaker leads short; twist the pair to reduce noise. A 100 µF electrolytic + 0.1 µF ceramic across DF `VCC/GND` helps stability.

- 3.5 mm jack (line‑out) for later:
  - Use DFPlayer `DAC_L` and `DAC_R` pins for left/right line‑level outputs; sleeve to GND.
  - Add DC‑blocking capacitors in series with each channel (e.g., 4.7–10 µF, positive toward DFPlayer), and optionally a 100 kΩ resistor from jack tip/ring to GND to reduce pop.
  - Feed powered speakers, an amplifier board (e.g., PAM8403), or headphones via an amp. The DFPlayer line out is not a headphone driver.
  - For mono from both L/R, do not tie `DAC_L` and `DAC_R` directly together — mix through resistors (e.g., 220 Ω–1 kΩ each) into the jack tip.

## Build and Upload (PlatformIO)

Ensure you have PlatformIO installed in VS Code.

1. Open this folder in VS Code.
2. Check `platformio.ini` has `board = esp32dev` (or adjust to your board).
3. Connect your ESP32 and select the correct COM port if needed.
4. Build and upload:

```powershell
pio run
pio run -t upload
pio device monitor -b 115200
```

## Troubleshooting

- If you see `DFPlayer not responding`:
  - Verify 5V power and common ground.
  - Cross TX/RX: DF `TX` -> ESP `RX16`, DF `RX` <- ESP `TX17`.
  - Add resistor divider on DF `RX` if using 5V DFPlayer.
  - Ensure SD card is FAT32 and contains valid MP3/WAV files.
  - Try lowering volume or different speaker.
- If playback is noisy or distorted:
  - Use a proper speaker and power supply.
  - Avoid powering DFPlayer from USB 5V of PC if unstable; use separate 5V supply and common GND.
- No sound but tracks advance:
  - Check speaker wiring (SPK_1/SPK_2) or use DAC to amplifier.

This demo starts playback of `/mp3/0001.mp3` on boot. No buttons or controls are required.

## Credits

Uses the library `DFRobotDFPlayerMini` from DFRobot.
