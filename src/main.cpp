#include <Arduino.h>
#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include "hardware_config.h"

// We'll use HardwareSerial(2) on ESP32 for reliable UART
HardwareSerial dfSerial(2); // UART2
DFRobotDFPlayerMini dfplayer;

bool dfReady = false;
int currentVolume = DFPLAYER_DEFAULT_VOLUME;

// Wait for SD card/device to be ready by polling file count
static bool waitForSdReady(unsigned long timeoutMs, int &outFiles) {
  unsigned long start = millis();
  outFiles = -1;
  while (millis() - start < timeoutMs) {
    int files = dfplayer.readFileCounts();
    if (files >= 0) { // got a response
      outFiles = files;
      return true;
    }
    delay(150);
  }
  return false;
}

void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut: Serial.println("DFPlayer: Time Out!"); break;
    case WrongStack: Serial.println("DFPlayer: Wrong Stack!"); break;
    case DFPlayerCardInserted: Serial.println("DFPlayer: Card Inserted"); break;
    case DFPlayerCardRemoved: Serial.println("DFPlayer: Card Removed"); break;
    case DFPlayerCardOnline: Serial.println("DFPlayer: Card Online"); break;
    case DFPlayerUSBInserted: Serial.println("DFPlayer: USB Inserted"); break;
    case DFPlayerUSBRemoved: Serial.println("DFPlayer: USB Removed"); break;
    case DFPlayerUSBOnline: Serial.println("DFPlayer: USB Online"); break;
    case DFPlayerPlayFinished: Serial.printf("DFPlayer: Play Finished track %d\n", value); break;
    case DFPlayerError:
      Serial.print("DFPlayer: Error - ");
      switch (value) {
        case Busy: Serial.println("Card not found"); break;
        case Sleeping: Serial.println("Sleeping"); break;
        case SerialWrongStack: Serial.println("Serial Wrong Stack"); break;
        case CheckSumNotMatch: Serial.println("Check Sum Not Match"); break;
        case FileIndexOut: Serial.println("File Index Out"); break;
        case FileMismatch: Serial.println("File Mismatch"); break;
        case Advertise: Serial.println("In Advertise"); break;
        default: Serial.println(value); break;
      }
      break;
    default: break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nESP32 DFPlayer Mini Demo");

  // Start UART2 for DFPlayer
  dfSerial.begin(9600, SERIAL_8N1, DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
  // Allow DFPlayer to fully power up before init attempts (some clones need >1s)
  delay(1500);

  bool began = false;
  for (int i = 0; i < 3 && !began; ++i) {
    began = dfplayer.begin(dfSerial);
    if (!began) {
      Serial.printf("DFPlayer begin failed (try %d). Retrying...\n", i + 1);
      delay(600);
    }
  }

  if (!began) {
    Serial.println("DFPlayer not responding. Check wiring and SD.");
    Serial.println("Wiring: DF TX->ESP RX16, DF RX<-ESP TX17 (via level shifter/divider). 5V on VCC.");
  } else {
    dfReady = true;
    Serial.println("DFPlayer initialized.");
    dfplayer.setTimeOut(1000);
    // Explicitly select SD before setting volume/EQ (avoid extra reset after begin for better stability)
    dfplayer.outputDevice(DFPLAYER_DEVICE_SD);
    delay(1200);

    currentVolume = DFPLAYER_DEFAULT_VOLUME;
    dfplayer.volume(currentVolume);
    dfplayer.EQ(DFPLAYER_EQ_NORMAL);

    // Ensure SD is ready before attempting playback
    int totalFiles = -1;
    bool sdReady = waitForSdReady(4000, totalFiles);
    Serial.printf("SD ready: %s, files=%d\n", sdReady ? "yes" : "no", totalFiles);

    // Always try /mp3/0001.mp3 first
    Serial.println("Attempting: /mp3/0001.mp3");
    delay(500);
    dfplayer.playMp3Folder(1);
    delay(1500);
    int state = dfplayer.readState();
    int curr = dfplayer.readCurrentFileNumber();
    Serial.printf("Initial play state: %d (1=playing), current=%d\n", state, curr);
    if (state != 1) {
      Serial.println("Playback didn’t start; falling back to root index 1 (0001.mp3 in root)");
      dfplayer.play(1);
    }
  }

  // Simple demo: no buttons or commands
}

unsigned long lastReport = 0;
bool retriedOnce = false;

void loop() {
  if (dfReady && dfplayer.available()) {
    uint8_t type = dfplayer.readType();
    int value = dfplayer.read();
    printDetail(type, value);
  }

  // Periodic status
  if (millis() - lastReport > 3000) {
    lastReport = millis();
    if (dfReady) {
      int state = dfplayer.readState();
      int curr = dfplayer.readCurrentFileNumber();
      Serial.printf("Status: state=%d current=%d\n", state, curr);
    }
  }

  // One-time self-retry if not playing a few seconds after boot
  if (dfReady && !retriedOnce && millis() > 5000) {
    int st = dfplayer.readState();
    if (st != 1) {
      Serial.println("Not playing after 5s; reselecting SD and retrying 0001...");
      dfplayer.outputDevice(DFPLAYER_DEVICE_SD);
      delay(200);
      dfplayer.playMp3Folder(1);
      retriedOnce = true;
    }
  }

  // No button or serial control; just keep reporting
}
