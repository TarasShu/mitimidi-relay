#!/bin/bash

# Build script for mitimidi-relay

set -e

echo "Building MIDI Relay Controller..."

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Set environment variables for Pico SDK
export PICO_SDK_PATH=/Users/mac/pico-sdk
export PICO_EXTRAS_PATH=/Users/mac/pico-extras

# Configure with CMake
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Build the project
make -j$(sysctl -n hw.logicalcpu)

echo ""
echo "Build complete!"
echo "Output files:"
ls -la mitimidi-relay.*

echo ""
echo "=== MidiMiti MIDI Relay Controller ==="
echo "Controls 4 relays on GPIO pins 16-19"
echo ""
echo "MIDI Control Methods:"
echo "1. NOTES: C4(60)=Relay1, C#4(61)=Relay2, D4(62)=Relay3, D#4(63)=Relay4"
echo "   - Note On = Relay ON"
echo "   - Note Off = Relay OFF"
echo ""
echo "2. CONTROL CHANGE: CC1-4 control Relay1-4"
echo "   - CC value >= 64 = Relay ON"
echo "   - CC value < 64 = Relay OFF"
echo ""
echo "3. PROGRAM CHANGE: 0-3 select single relay"
echo "   - Program 0 = Only Relay1 ON"
echo "   - Program 1 = Only Relay2 ON"
echo "   - Program 2 = Only Relay3 ON"
echo "   - Program 3 = Only Relay4 ON"
echo "   - Other programs = All relays OFF"
echo ""
echo "To flash to Pico W:"
echo "1. Hold BOOTSEL button and connect USB"
echo "2. Copy mitimidi-relay.uf2 to the RPI-RP2 drive"
echo "3. The device will reboot and be ready to control relays!"
echo ""
echo "🎯 DEVICE NAMES:"
echo "📱 USB MIDI: 'MidiMiti' (cable connection)"
echo "📶 Bluetooth: 'MidiMiti' (wireless connection)"
echo ""
echo "Use your Happy Birthday MIDI files:"
echo "• happy_birthday.mid - traditional melody"
echo "• happy_birthday_relay.mid - optimized relay light show"