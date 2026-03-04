# EasyCharge - Electric Vehicle Charging Station Management System

Final project for Advanced C Programming 

---

## Overview

EasyCharge is a console-based management system for electric vehicle charging stations. Each station contains multiple charging ports (Fast / Mid / Slow), and vehicles can start charging, join a waiting queue, and stop charging with automatic payment calculation. All data is persisted to CSV files and updated after every operation.

---

## Data Structures

| Structure | Usage |
|-----------|-------|
| BST (Binary Search Tree) | Station lookup by ID, vehicle lookup by license plate |
| Linked List | Port list per station |
| Queue | Waiting queue per station |

---

## Project Structure

```
EasyCharge/
├── src/
│   ├── types.h          – Shared type definitions
│   ├── queue.c/h        – Waiting queue ADT
│   ├── port_list.c/h    – Charging port linked list ADT
│   ├── car_bst.c/h      – Cars BST (keyed by license plate)
│   ├── station_bst.c/h  – Stations BST (keyed by ID)
│   ├── file_io.c/h      – File load/save (CSV)
│   ├── menu_ops.c/h     – Menu operations
│   └── main.c           – Entry point
├── Stations.txt          – Station data
├── Ports.txt             – Port data
├── Cars.txt              – Vehicle data
├── LineOfCars.txt        – Queue data
├── Makefile
├── compile.bat           – Windows build script
└── run.bat               – Windows run script
```

---

## Build & Run

### Windows (easiest)

```
Double-click compile.bat    ← builds the executable
Double-click run.bat        ← runs the program
```

### Manual (GCC / MSYS2)

```bash
gcc -Wall -Wextra -g -std=c99 -Isrc \
    src/main.c src/queue.c src/port_list.c \
    src/car_bst.c src/station_bst.c \
    src/file_io.c src/menu_ops.c \
    -o build/easycharge.exe -lm
```

### Linux / Mac

```bash
make
./build/easycharge
```

> **Note:** Always run from the project root directory, not from `build/`, so the program can locate the `.txt` data files.

---

## Menu Operations

| # | Operation |
|---|-----------|
| 1 | Find nearest charging station by coordinates |
| 2 | Start charging a vehicle (or join queue if full) |
| 3 | Check vehicle charging status |
| 4 | Stop charging and calculate payment |
| 5 | Display all stations |
| 6 | Display vehicles at a station |
| 7 | Station statistics report |
| 8 | Display top 5 customers by total payment |
| 9 | Add a new charging port |
| 10 | Release ports that exceeded 10 hours |
| 11 | Remove an out-of-order port |
| 12 | Remove a customer |
| 13 | Close a station |
| 0 | Exit (saves all data) |

---

## Requirements

- GCC with C99 support (MSYS2 / MinGW64 recommended on Windows)
- No external libraries — standard C library only

---

*Submitted as a final project for the Advanced C Programming course*
