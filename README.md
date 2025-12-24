# ripple

A countdown timer app for Pebble smartwatches with multiple visual display modes.

## Display Modes

| Mode | Description | aplite | basalt | chalk | diorite | emery |
|------|-------------|--------|--------|-------|---------|-------|
| Text | Large numeric display showing time as h:mm:ss or m:ss | ![aplite](media/screenshots/aplite-text.png) | ![basalt](media/screenshots/basalt-text.png) | ![chalk](media/screenshots/chalk-text.png) | ![diorite](media/screenshots/diorite-text.png) | ![emery](media/screenshots/emery-text.png) |
| Blocks | 12x8 grid of blocks that empty as time passes | ![aplite](media/screenshots/aplite-blocks.png) | ![basalt](media/screenshots/basalt-blocks.png) | ![chalk](media/screenshots/chalk-blocks.png) | ![diorite](media/screenshots/diorite-blocks.png) | ![emery](media/screenshots/emery-blocks.png) |
| Vertical Blocks | 8x12 grid filling bottom to top, left to right | ![aplite](media/screenshots/aplite-vertical_blocks.png) | ![basalt](media/screenshots/basalt-vertical_blocks.png) | ![chalk](media/screenshots/chalk-vertical_blocks.png) | ![diorite](media/screenshots/diorite-vertical_blocks.png) | ![emery](media/screenshots/emery-vertical_blocks.png) |
| Clock | Analog clock face with filled arc and sweeping hand | ![aplite](media/screenshots/aplite-clock.png) | ![basalt](media/screenshots/basalt-clock.png) | ![chalk](media/screenshots/chalk-clock.png) | ![diorite](media/screenshots/diorite-clock.png) | ![emery](media/screenshots/emery-clock.png) |
| Ring | Thick circular arc that depletes clockwise | ![aplite](media/screenshots/aplite-ring.png) | ![basalt](media/screenshots/basalt-ring.png) | ![chalk](media/screenshots/chalk-ring.png) | ![diorite](media/screenshots/diorite-ring.png) | ![emery](media/screenshots/emery-ring.png) |
| Hourglass | Sand timer with particles moving between chambers | ![aplite](media/screenshots/aplite-hourglass.png) | ![basalt](media/screenshots/basalt-hourglass.png) | ![chalk](media/screenshots/chalk-hourglass.png) | ![diorite](media/screenshots/diorite-hourglass.png) | ![emery](media/screenshots/emery-hourglass.png) |
| Binary | Three rows of 6-bit binary dots (h/m/s) | ![aplite](media/screenshots/aplite-binary.png) | ![basalt](media/screenshots/basalt-binary.png) | ![chalk](media/screenshots/chalk-binary.png) | ![diorite](media/screenshots/diorite-binary.png) | ![emery](media/screenshots/emery-binary.png) |
| Radial | Concentric rings for hours, minutes, seconds | ![aplite](media/screenshots/aplite-radial.png) | ![basalt](media/screenshots/basalt-radial.png) | ![chalk](media/screenshots/chalk-radial.png) | ![diorite](media/screenshots/diorite-radial.png) | ![emery](media/screenshots/emery-radial.png) |
| Hex | Time in hexadecimal with decimal equivalent | ![aplite](media/screenshots/aplite-hex.png) | ![basalt](media/screenshots/basalt-hex.png) | ![chalk](media/screenshots/chalk-hex.png) | ![diorite](media/screenshots/diorite-hex.png) | ![emery](media/screenshots/emery-hex.png) |
| Matrix | Falling green digits with time in center | ![aplite](media/screenshots/aplite-matrix.png) | ![basalt](media/screenshots/basalt-matrix.png) | ![chalk](media/screenshots/chalk-matrix.png) | ![diorite](media/screenshots/diorite-matrix.png) | ![emery](media/screenshots/emery-matrix.png) |
| Water Level | Container with draining water level | ![aplite](media/screenshots/aplite-water_level.png) | ![basalt](media/screenshots/basalt-water_level.png) | ![chalk](media/screenshots/chalk-water_level.png) | ![diorite](media/screenshots/diorite-water_level.png) | ![emery](media/screenshots/emery-water_level.png) |
| Spiral Out | Spiral pattern expanding outward | ![aplite](media/screenshots/aplite-spiral_out.png) | ![basalt](media/screenshots/basalt-spiral_out.png) | ![chalk](media/screenshots/chalk-spiral_out.png) | ![diorite](media/screenshots/diorite-spiral_out.png) | ![emery](media/screenshots/emery-spiral_out.png) |
| Spiral In | Spiral pattern contracting inward | ![aplite](media/screenshots/aplite-spiral_in.png) | ![basalt](media/screenshots/basalt-spiral_in.png) | ![chalk](media/screenshots/chalk-spiral_in.png) | ![diorite](media/screenshots/diorite-spiral_in.png) | ![emery](media/screenshots/emery-spiral_in.png) |
| % Elapsed | Large percentage of time elapsed | ![aplite](media/screenshots/aplite-percent.png) | ![basalt](media/screenshots/basalt-percent.png) | ![chalk](media/screenshots/chalk-percent.png) | ![diorite](media/screenshots/diorite-percent.png) | ![emery](media/screenshots/emery-percent.png) |
| % Remaining | Large percentage of time remaining | ![aplite](media/screenshots/aplite-percent_remaining.png) | ![basalt](media/screenshots/basalt-percent_remaining.png) | ![chalk](media/screenshots/chalk-percent_remaining.png) | ![diorite](media/screenshots/diorite-percent_remaining.png) | ![emery](media/screenshots/emery-percent_remaining.png) |

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


