# Pebble Timer

A countdown timer app for Pebble smartwatches with multiple visual display modes.

## Features

- Preset timer options (5, 10, 15, 30 minutes)
- Custom timer with hours and minutes selection
- Pause, resume, and restart functionality
- Vibration alert on completion
- 15 different visual display modes
- Support for all Pebble platforms (Aplite, Basalt, Chalk, Diorite, Emery)

## Display Modes

| Mode | Description | Screenshot |
|------|-------------|------------|
| Text | Large numeric display showing time as h:mm:ss or m:ss | ![Text](screenshots/basalt-text.png) |
| Blocks | 12x8 grid of blocks that empty as time passes | ![Blocks](screenshots/basalt-blocks.png) |
| Vertical Blocks | 8x12 grid filling bottom to top, left to right | ![Vertical Blocks](screenshots/basalt-vertical_blocks.png) |
| Clock | Analog clock face with filled arc and sweeping hand | ![Clock](screenshots/basalt-clock.png) |
| Ring | Thick circular arc that depletes clockwise | ![Ring](screenshots/basalt-ring.png) |
| Hourglass | Sand timer with particles moving between chambers | ![Hourglass](screenshots/basalt-hourglass.png) |
| Binary | Three rows of 6-bit binary dots (h/m/s) | ![Binary](screenshots/basalt-binary.png) |
| Radial | Concentric rings for hours, minutes, seconds | ![Radial](screenshots/basalt-radial.png) |
| Hex | Time in hexadecimal with decimal equivalent | ![Hex](screenshots/basalt-hex.png) |
| Matrix | Falling green digits with time in center | ![Matrix](screenshots/basalt-matrix.png) |
| Water Level | Container with draining water level | ![Water Level](screenshots/basalt-water_level.png) |
| Spiral Out | Spiral pattern expanding outward | ![Spiral Out](screenshots/basalt-spiral_out.png) |
| Spiral In | Spiral pattern contracting inward | ![Spiral In](screenshots/basalt-spiral_in.png) |
| % Elapsed | Large percentage of time elapsed | ![% Elapsed](screenshots/basalt-percent.png) |
| % Remaining | Large percentage of time remaining | ![% Remaining](screenshots/basalt-percent_remaining.png) |

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
| DOWN | Pause timer |
| SELECT (hold) | Cycle through display modes |
| UP (hold) | Toggle time text visibility |
| BACK | Pause and show exit confirmation |

### Timer Paused

| Button | Action |
|--------|--------|
| DOWN | Resume timer |
| SELECT (hold) | Cycle through display modes |
| UP | Restart timer from beginning |
| BACK | Show exit confirmation |

## Color Themes

On color Pebble models (Basalt, Chalk, Emery), the app uses distinct colors:

- Running timer: Green text
- Paused timer: Yellow text
- Low time warning (under 10 seconds): Red text
- Completed: Bright green text

## development

Requires the [Pebble SDK](https://developer.rebble.io/sdk/).

```bash
make build
```


```bash
make install-ip IP=<IP_ADDRESS>

```

```bash
make install-cloudpebble
```

