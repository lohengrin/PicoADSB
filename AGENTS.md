# Pico ADSB

## Description

Software grabing ADS-B data from a free site like `https://adsb.lol/`
Display planes over me in a range of 50km (can be changed)
The left half of the screen display small triangle by plane with orientation  "like a radar".
The right half of the screen show airplane type, flight number and destination of the closest plane (displayed bigger on the map).

- C++20
- CMake

## Hardware

This project is based on following hardware:
- Raspberry Pico W (RP2040 + Wireless)
- ePaper ink screen : Waveshare Pico-ePaper 2.13 (V4)

## Documentation

**adsb.lol** `https://api.adsb.lol/api/openapi.json`
**Waveshare Pico-ePaper 2.13 (V4)** 
 - Doc: `https://www.waveshare.com/wiki/Pico-ePaper-2.13?srsltid=AfmBOooa5fpbz7vxZWxV2gdXz5d5i0YEGMS8QjKrqKzSdAcFszFMwIBn` 
 - Sample code: `/home/lohengrin/PICO/Pico_ePaper_Code`

## Integration and Tests
- Flashing can be done using picotool: `picotool load -f -x <uf2 file>`
- Connection to running Pico: `minicom -b 115200 -o -D /dev/ttyACM0`

