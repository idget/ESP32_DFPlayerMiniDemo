#pragma once

// ESP32 <-> DFPlayer Mini wiring
// - DFPlayer VCC -> 5V (or 3.3V per module spec), GND -> GND
// - DFPlayer TX -> ESP32 RX pin (UART2 RX)
// - DFPlayer RX -> ESP32 TX pin (UART2 TX) via level shifting/resistor divider recommended
// - SPK_1 / SPK_2 to small speaker OR use DAC OUT with amplifier
// - Insert microSD with files like /mp3/0001.mp3 or 0001.mp3 in root

// Use UART2 for DFPlayer
// Adjust these pins to your wiring; below matches common ESP32 dev kits
static const int DFPLAYER_TX_PIN = 17; // ESP32 TX2 -> DFPlayer RX
static const int DFPLAYER_RX_PIN = 16; // ESP32 RX2 <- DFPlayer TX

// Playback defaults
static const int DFPLAYER_DEFAULT_VOLUME = 20; // 0..30
