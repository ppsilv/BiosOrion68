# BiosOrion68
  From 2026-08-28 this project became to be a bios to boot orion68DOS.


**Orion68** is a modular 16-bit hobby computer built around the Motorola 68000 CPU.
The project was created as a personal learning experience to explore classic computer architecture using discrete logic and modular hardware design.

The system is built using a **backplane architecture**, allowing multiple expansion boards connected through a 96-pin bus. This modular approach makes it easy to experiment with new hardware, peripherals, and system configurations.

## Architecture

The Orion68 is based on the Motorola 68000 microprocessor and uses a traditional memory-mapped architecture. Address decoding is implemented using GAL devices and standard 74HC logic components.

## Current Hardware

* **CPU:** Motorola 68000
* **ROM:** 16 KB (planned expansion to 64 KB)
* **SRAM:** 1 MB (with plans to expand to 2–3 MB)
* **Serial I/O:** Quad UART (4 serial ports)
* **Address decoding:** GAL devices and 74HC138
* **Bus:** 96-pin system bus
* **Backplane:** supports up to 5 boards (3 cm spacing) or 9 boards (1.5 cm spacing)

## Expansion Boards

The system is designed to be modular, with each board implementing a specific subsystem. Current and planned boards include:

* CPU board
* Memory board (ROM/SRAM)
* Serial communication board
* Experimental video interface using a Raspberry Pi Pico
* Additional expansion boards for future peripherals

## Goals of the Project

The main goals of Orion68 are:

* Learn and experiment with classic microcomputer architecture
* Build a modular 68000-based computer system from discrete components
* Explore hardware/software interaction at a low level
* Create a flexible platform for retro-computing experiments

## Status

The first prototype has already been built and successfully tested.
The current version introduces a redesigned modular architecture with a dedicated backplane and expansion boards.

---

Orion68 is an ongoing hobby project focused on learning, experimentation, and the enjoyment of building computers from the ground up.
