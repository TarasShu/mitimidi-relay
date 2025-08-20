# MidiMiti - MIDI Relay Controller for Pico W

A professional dual-interface MIDI relay controller that receives MIDI messages over USB cable or Bluetooth wireless and controls 4 relays connected to GPIO pins 16-19.

## Features

🎹 **Dual MIDI Input**: USB MIDI + Bluetooth MIDI  
⚡ **4 Relay Control**: GPIO pins 16-19 control external relays  
🎛️ **3 Control Methods**: Note messages, Control Change, Program Change  
📶 **Bluetooth LE**: Appears as "MidiMiti" in Bluetooth MIDI settings  
📱 **USB MIDI**: Appears as "MidiMiti" when connected via cable

## Hardware Setup

### Components Needed
- Raspberry Pi Pico W
- 4x Relay modules (3.3V or 5V compatible)
- Connecting wires

### Wiring
```
Pico W    →  Relay Module
GPIO 16   →  Relay 1 Control
GPIO 17   →  Relay 2 Control  
GPIO 18   →  Relay 3 Control
GPIO 19   →  Relay 4 Control
3.3V      →  VCC (if 3.3V relays)
GND       →  GND
```

## MIDI Control Methods

### 1. Note Messages
- **C4 (60)** → Relay 1
- **C#4 (61)** → Relay 2  
- **D4 (62)** → Relay 3
- **D#4 (63)** → Relay 4

Note On = Relay ON, Note Off = Relay OFF

### 2. Control Change (CC)
- **CC 1** → Relay 1
- **CC 2** → Relay 2
- **CC 3** → Relay 3
- **CC 4** → Relay 4

CC value ≥ 64 = ON, CC value < 64 = OFF

### 3. Program Change
- **Program 0** → Only Relay 1 ON
- **Program 1** → Only Relay 2 ON
- **Program 2** → Only Relay 3 ON
- **Program 3** → Only Relay 4 ON
- **Other Programs** → All Relays OFF

## Building

1. Ensure Pico SDK is installed at `/Users/mac/pico-sdk`
2. Navigate to project directory:
   ```bash
   cd /Users/mac/mitimidi-relay
   ```
3. Run build script:
   ```bash
   ./build.sh
   ```

## Flashing

1. Hold BOOTSEL button on Pico W while connecting USB
2. Copy `build/mitimidi-relay.uf2` to the RPI-RP2 drive
3. Device will reboot and start running

## Usage

### USB MIDI
1. Connect Pico W to computer via USB
2. Device appears as "MidiMiti" in DAW/MIDI software
3. Send MIDI messages to control relays

### Bluetooth MIDI
1. Look for "MidiMiti" in Bluetooth settings
2. Pair with your device (phone, tablet, computer)
3. Use in MIDI apps that support Bluetooth MIDI
4. Send MIDI messages wirelessly

### Console Monitoring
Connect UART adapter to GPIO 0 (TX) and 1 (RX) at 115200 baud to see:
- MIDI messages received
- Relay state changes
- Connection status

Example output:
```
[USB] Note On: Ch1 Note60 Vel127
Relay 1: ON
Relay States: [1:ON ] [2:OFF] [3:OFF] [4:OFF]

[BT] CC: Ch1 CC2 Val100
Relay 2: ON
Relay States: [1:ON ] [2:ON ] [3:OFF] [4:OFF]
```

## Applications

- **Stage Lighting**: Control lights with MIDI sequences
- **Effects Switching**: Guitar pedal switching via MIDI
- **Home Automation**: Control appliances with MIDI
- **Art Installations**: Interactive MIDI-controlled projects
- **Industrial Control**: MIDI-triggered machinery control

## Customization

Edit `main.c` to change:
- MIDI note mappings (`RELAY_X_NOTE` defines)
- GPIO pin assignments (`RELAY_X_PIN` defines)  
- MIDI channel filtering
- Control logic and timing

## Troubleshooting

- **No USB MIDI**: Check USB connection and drivers
- **No Bluetooth**: Ensure CYW43 initialized properly
- **Relays not switching**: Check GPIO connections and relay voltage requirements
- **No console output**: Verify UART connections (GPIO 0/1) and baud rate (115200)

<img width="1024" height="1536" alt="image" src="https://github.com/user-attachments/assets/771195cc-cbc4-4781-a747-ae34003009cf" />
