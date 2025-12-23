# Pebble Timer

A countdown timer app for Pebble smartwatches with multiple visual display modes.

## Features

- Preset timer options (5, 10, 15, 30 minutes)
- Custom timer with hours and minutes selection
- Pause, resume, and restart functionality
- Vibration alert on completion
- 9 different visual display modes
- Support for all Pebble platforms (Aplite, Basalt, Chalk, Diorite, Emery)

## Display Modes

| Mode | Description | Visual Style |
|------|-------------|--------------|
| Text | Standard digital countdown | Large numeric display showing time as h:mm:ss or m:ss |
| Blocks | Grid-based visualization | 12x8 grid of blocks that empty from bottom-right to top-left as time passes |
| Clock | Analog clock face | Circular clock with hour markers, filled arc for remaining time, and sweeping hand |
| Ring | Progress ring | Thick circular arc around screen edge that depletes clockwise |
| Hourglass | Sand timer simulation | Classic hourglass shape with particles moving from top chamber to bottom |
| Binary | Binary representation | Three rows of 6-bit binary dots showing hours, minutes, and seconds |
| Radial | Concentric progress rings | Three nested rings for hours (outer), minutes (middle), and seconds (inner) |
| Hex | Hexadecimal display | Time shown in base-16 format with 0x prefix and decimal equivalent |
| Matrix | Falling characters | Matrix-style falling green digits with time displayed in center |

## Controls

### Preset Selection Screen

| Button | Action |
|--------|--------|
| UP | Previous preset option |
| DOWN | Next preset option |
| SELECT | Start timer with selected preset |
| SELECT (hold) | Cycle through display modes |
| BACK | Exit app |

### Custom Time Entry

| Button | Action |
|--------|--------|
| UP | Increase value |
| DOWN | Decrease value |
| SELECT | Confirm and proceed |
| BACK | Return to preset selection |

### Timer Running

| Button | Action |
|--------|--------|
| SELECT | Pause timer |
| SELECT (hold) | Cycle through display modes |
| BACK | Pause and show exit confirmation |

### Timer Paused

| Button | Action |
|--------|--------|
| SELECT | Resume timer |
| SELECT (hold) | Cycle through display modes |
| UP | Restart timer from beginning |
| DOWN | Cancel timer and return to presets |
| BACK | Show exit confirmation |

### Timer Complete

| Button | Action |
|--------|--------|
| Any button | Dismiss and return to preset selection |

## Color Themes

On color Pebble models (Basalt, Chalk, Emery), the app uses distinct colors:

- Running timer: Green text
- Paused timer: Yellow text
- Low time warning (under 10 seconds): Red text
- Completed: Bright green text

On black and white models (Aplite, Diorite), all elements display in white on black.

## Building

Requires the Pebble SDK.

```bash
pebble build
```

## Installing

```bash
pebble install --phone <IP_ADDRESS>
```

```bach
pebble install --cloudpebble
```

Or install the generated `.pbw` file from the `build` directory.

