# jongbinjung Keychron Q4 keymap

Personal HHKB-flavoured keymap for the Keychron Q4 (ANSI). Diagrams below are
maintained by hand. To cross-check them against the source:

```
qmk c2json -kb keychron/q4/ansi/v1 -km jongbinjung keyboards/keychron/q4/ansi/keymaps/jongbinjung/keymap.c
```

This board is the **v2** revision (one SNLED27351 LED driver). Always build and
flash `keychron/q4/ansi/v2`. `v1` firmware runs fine but leaves every LED dark,
because it expects a second driver chip. The USB device version the OS shows
reflects whatever firmware is flashed, not the hardware, so do not use it to
pick the variant. Flash with `qmk flash -kb keychron/q4/ansi/v2 -km jongbinjung`
(the qmk user config defaults to this target, so a bare `qmk flash` also works).

## Layer overview

| Layer    | Reach it by                       | Lockable | Purpose |
|----------|-----------------------------------|----------|---------|
| MAC_BASE | DIP switch set to Mac             | no       | Base layer, Opt/Cmd modifiers |
| WIN_BASE | DIP switch set to Windows         | no       | Base layer, Win/Alt modifiers. No FPAD layer-tap on grave |
| MAC_FN   | hold Fn1 (Mac base)               | yes      | Media, brightness, RGB, arrows, nav |
| WIN_FN   | hold Fn1 (Windows base)           | yes      | Same as MAC_FN plus Menu, PrtSc, ScrLk |
| SHR_FN   | hold Fn2                          | yes      | F-keys on the number row, RGB |
| SYS_FN   | hold Fn1 then Fn2 (or Fn2 then Fn1) | no     | Bootloader on Fn3 |
| NUMPAD   | hold Tab                          | yes      | Numpad on U/I/O, J/K/L, N/M |
| FNPAD    | hold grave (Mac base only)        | yes      | F1 to F6 on the same keys as the numpad |
| HHKB_FN  | hold Fn3                          | yes      | HHKB Fn layer: F-keys, nav on [ ; ' /, media on A/S/D, Caps Lock on Tab |
| VI       | hold Space, or lock with Space+Esc | yes     | vi-style navigation: H/J/K/L arrows, w/b word jumps |

Key labels in the diagrams: top line is the tap action, bottom line is the hold
action. Blank keys fall through to the layer below. `x` means the key does
nothing on that layer.

## Layers

### MAC_BASE: DIP switch set to Mac
```
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----------+
|  `  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  0  |  -  |  =  |     \     |
| FPAD|     |     |     |     |     |     |     |     |     |     |     |     |           |
+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
|  Tab   |  Q  |  W  |  E  |  R  |  T  |  Y  |  U  |  I  |  O  |  P  |  [  |  ]  |  Bksp  |
|  NUM   |     |     |     |     |     |     |     |     |     |     |     |     |        |
+---------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-------------+
|   Esc   |  A  |  S  |  D  |  F  |  G  |  H  |  J  |  K  |  L  |  ;  |  '  |     Ent     |
|   Ctl   |     |     |     |     |     |     |     |     |     |     |     |     Ctl     |
+-------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+---------------+
|      (      |  Z  |  X  |  C  |  V  |  B  |  N  |  M  |  ,  |  .  |  /  |       )       |
|     Shft    |     |     |     |     |     |     |     |     |     |     |      Shft     |
+-------+------+------+-------------------------------------+-------+------+------+-------+
|  Ctrl | Opt  | Cmd  |                 Spc                 |  Cmd  | Fn1  | Fn2  |  Fn3  |
|       |      |      |                  VI                 |       |      |      |       |
+-------+------+------+-------------------------------------+-------+------+------+-------+
```

### WIN_BASE: DIP switch set to Windows
```
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----------+
|  `  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  0  |  -  |  =  |     \     |
|     |     |     |     |     |     |     |     |     |     |     |     |     |           |
+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
|  Tab   |  Q  |  W  |  E  |  R  |  T  |  Y  |  U  |  I  |  O  |  P  |  [  |  ]  |  Bksp  |
|  NUM   |     |     |     |     |     |     |     |     |     |     |     |     |        |
+---------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-------------+
|   Esc   |  A  |  S  |  D  |  F  |  G  |  H  |  J  |  K  |  L  |  ;  |  '  |     Ent     |
|   Ctl   |     |     |     |     |     |     |     |     |     |     |     |     Ctl     |
+-------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+---------------+
|      (      |  Z  |  X  |  C  |  V  |  B  |  N  |  M  |  ,  |  .  |  /  |       )       |
|     Shft    |     |     |     |     |     |     |     |     |     |     |      Shft     |
+-------+------+------+-------------------------------------+-------+------+------+-------+
|  Ctrl | Win  | Alt  |                 Spc                 |  Alt  | Fn1  | Fn2  |  Fn3  |
|       |      |      |                  VI                 |       |      |      |       |
+-------+------+------+-------------------------------------+-------+------+------+-------+
```

### MAC_FN: hold Fn1 on the Mac base (Space locks)
```
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----------+
|  `  | Bri-| Bri+|  x  |  x  | Val-| Val+| |<< | >|| | >>| | Mute| Vol-| Vol+|    Mod+   |
|     |     |     |     |     |     |     |     |     |     |     |     |     |           |
+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
|  RGB   | Mod+| Val+| Hue+| Sat+| Spd+|     |     |     |     | Ins | PgUp| Home|        |
|        |     |     |     |     |     |     |     |     |     |     |     |     |        |
+---------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-------------+
|         | Mod-| Val-| Hue-| Sat-| Spd-|     |     |  Up |  x  | PgDn| End |             |
|         |     |     |     |     |     |     |     |     |     |     |     |             |
+-------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+---------------+
|             |     |     |     |     |     | NKRO| Left| Down| Rght| Del |               |
|             |     |     |     |     |     |     |     |     |     |     |               |
+-------+------+------+-------------------------------------+-------+------+------+-------+
|       |      |      |                 LOCK                |       |      | SYS  |       |
|       |      |      |                                     |       |      |      |       |
+-------+------+------+-------------------------------------+-------+------+------+-------+
```

### WIN_FN: hold Fn1 on the Windows base (Space locks)
```
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----------+
|  `  | Bri-| Bri+|  x  |  x  | Val-| Val+| |<< | >|| | >>| | Mute| Vol-| Vol+|    Mod+   |
|     |     |     |     |     |     |     |     |     |     |     |     |     |           |
+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
|  RGB   | Mod+| Val+| Hue+| Sat+| Spd+|     |     | Menu| ScrL| Ins | PgUp| Home|        |
|        |     |     |     |     |     |     |     |     |     |     |     |     |        |
+---------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-------------+
|         | Mod-| Val-| Hue-| Sat-| Spd-|     |     |  Up | PrSc| PgDn| End |             |
|         |     |     |     |     |     |     |     |     |     |     |     |             |
+-------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+---------------+
|             |     |     |     |     |     | NKRO| Left| Down| Rght| Del |               |
|             |     |     |     |     |     |     |     |     |     |     |               |
+-------+------+------+-------------------------------------+-------+------+------+-------+
|       |      |      |                 LOCK                |       |      | SYS  |       |
|       |      |      |                                     |       |      |      |       |
+-------+------+------+-------------------------------------+-------+------+------+-------+
```

### SHR_FN: hold Fn2 (Space locks)
```
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----------+
|  ~  |  F1 |  F2 |  F3 |  F4 |  F5 |  F6 |  F7 |  F8 |  F9 | F10 | F11 | F12 |           |
|     |     |     |     |     |     |     |     |     |     |     |     |     |           |
+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
|  RGB   | Mod+| Val+| Hue+| Sat+| Spd+|     |     |     |     |     |     |     |        |
|        |     |     |     |     |     |     |     |     |     |     |     |     |        |
+---------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-------------+
|         | Mod-| Val-| Hue-| Sat-| Spd-|     |     |     |     |     |     |             |
|         |     |     |     |     |     |     |     |     |     |     |     |             |
+-------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+---------------+
|             |     |     |     |     |     |     |     |     |     |     |               |
|             |     |     |     |     |     |     |     |     |     |     |               |
+-------+------+------+-------------------------------------+-------+------+------+-------+
|       |      |      |                 LOCK                |       | SYS  |      |       |
|       |      |      |                                     |       |      |      |       |
+-------+------+------+-------------------------------------+-------+------+------+-------+
```

### SYS_FN: hold Fn1 + Fn2
```
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----------+
|     |     |     |     |     |     |     |     |     |     |     |     |     |           |
|     |     |     |     |     |     |     |     |     |     |     |     |     |           |
+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
|        |     |     |     |     |     |     |     |     |     |     |     |     |        |
|        |     |     |     |     |     |     |     |     |     |     |     |     |        |
+---------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-------------+
|         |     |     |     |     |     |     |     |     |     |     |     |             |
|         |     |     |     |     |     |     |     |     |     |     |     |             |
+-------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+---------------+
|             |     |     |     |     |     |     |     |     |     |     |               |
|             |     |     |     |     |     |     |     |     |     |     |               |
+-------+------+------+-------------------------------------+-------+------+------+-------+
|       |      |      |                                     |       |      |      |  BOOT |
|       |      |      |                                     |       |      |      |       |
+-------+------+------+-------------------------------------+-------+------+------+-------+
```

### NUMPAD: hold Tab (Space locks)
```
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----------+
| Esc |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  0  |     |     |           |
|     |     |     |     |     |     |     |     |     |     |     |     |     |           |
+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
|        |     |     |     |     |     |     |  4  |  5  |  6  | Bksp|  [  |  ]  |   \    |
|        |     |     |     |     |     |     |     |     |     |     |     |     |        |
+---------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-------------+
|         |     |     |     |     |     |     |  1  |  2  |  3  | Ent |     |             |
|         |     |     |     |     |     |     |     |     |     |     |     |             |
+-------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+---------------+
|             |     |     |     |     |     |  0  |  0  |  ,  |  .  |     |               |
|             |     |     |     |     |     |     |     |     |     |     |               |
+-------+------+------+-------------------------------------+-------+------+------+-------+
|       |      |      |                 LOCK                |       |      |      |       |
|       |      |      |                                     |       |      |      |       |
+-------+------+------+-------------------------------------+-------+------+------+-------+
```

### FNPAD: hold grave, Mac base only (Space locks)
```
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----------+
|     |  F1 |  F2 |  F3 |  F4 |  F5 |  F6 |  F7 |  F8 |  F9 | F10 | F11 | F12 |           |
|     |     |     |     |     |     |     |     |     |     |     |     |     |           |
+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
|        |     |     |     |     |     |     |  F4 |  F5 |  F6 |     |     |     |        |
|        |     |     |     |     |     |     |     |     |     |     |     |     |        |
+---------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-------------+
|         |     |     |     |     |     |     |  F1 |  F2 |  F3 |     |     |             |
|         |     |     |     |     |     |     |     |     |     |     |     |             |
+-------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+---------------+
|             |     |     |     |     |     |     |     |     |     |     |               |
|             |     |     |     |     |     |     |     |     |     |     |               |
+-------+------+------+-------------------------------------+-------+------+------+-------+
|       |      |      |                 LOCK                |       |      |      |       |
|       |      |      |                                     |       |      |      |       |
+-------+------+------+-------------------------------------+-------+------+------+-------+
```

### HHKB_FN: hold Fn3 (Space locks)
```
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----------+
| Esc |  F1 |  F2 |  F3 |  F4 |  F5 |  F6 |  F7 |  F8 |  F9 | F10 | F11 | F12 |    Del    |
|     |     |     |     |     |     |     |     |     |     |     |     |     |           |
+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
|  Caps  |  x  |  x  |  x  |  x  |  x  |  x  |  x  |  x  |  x  |  x  |  Up |  x  |  Del   |
|        |     |     |     |     |     |     |     |     |     |     |     |     |        |
+---------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-------------+
|         | Vol-| Vol+| Mute|  x  |  x  |  x  |  x  | Home| PgUp| Left| Rght|     Ent     |
|         |     |     |     |     |     |     |     |     |     |     |     |             |
+-------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+---------------+
|             |  x  | Del |  x  |  x  |  x  | PgDn|  x  | End | PgDn| Down|               |
|             |     |     |     |     |     |     |     |     |     |     |               |
+-------+------+------+-------------------------------------+-------+------+------+-------+
|  Ctrl | Opt  | Cmd  |                 LOCK                |  Cmd  |      |      |       |
|       |      |      |                                     |       |      |      |       |
+-------+------+------+-------------------------------------+-------+------+------+-------+
```

### VI: hold Space (Space+Esc locks, I unlocks)
```
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----------+
|  `  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  0  |  -  |  =  |    Bksp   |
|     |     |     |     |     |     |     |     |     |     |     |     |     |           |
+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
|  Tab   |     |  w> |     |     |     |     | PgUp| ins | End |  x  | PgDn|  Up |  Del   |
|        |     |     |     |     |     |     |     |     |     |     |     |     |        |
+---------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-------------+
|   lock  | Stop| >|| |PgDn | >>| |     | Left| Down|  Up | Rght|     |     |     Ent     |
|   Ctl   |     |     |     |     |     |     |     |     |     |     |     |     Ctl     |
+-------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+---------------+
|             |  x  | Del |  x  |     |  <b | PgDn|  x  |  x  |  x  |  x  |               |
|             |     |     |     |     |     |     |     |     |     |     |               |
+-------+------+------+-------------------------------------+-------+------+------+-------+
|  Ctrl | Opt  | Cmd  |                                     |  Cmd  |      |      |  Ctrl |
|       |      |      |                                     |       |      |      |       |
+-------+------+------+-------------------------------------+-------+------+------+-------+
```


## How to use it

### Layer Lock

- Hold a layer key for at least 200 ms, then tap **Space**. Release the layer
  key; the layer stays on and its activator key lights amber.
- Tap Space again to unlock. Fn1, Fn2 and Fn3 layers also unlock when you tap
  the Fn key again.
- NUMPAD (Tab) and FNPAD (grave) only unlock via Space or the idle timeout,
  because Tab and grave keep typing their normal characters while locked.
- Any locked layer unlocks by itself after 60 seconds without a keypress.
- Tapping Space too quickly after the layer key resolves the layer key as a
  tap (you get a Tab, a grave, or a space) instead of locking.
- On a locked Fn layer the alpha block is RGB controls. The RGB toggle (Tab) is
  ignored while locked, so a stray Tab cannot turn the lights off; hue,
  brightness, speed and mode still work. Unlock first if you really want to
  toggle.

### VI mode

- **Momentary:** hold Space, use H/J/K/L, release Space.
- **Normal mode (lock):** hold Space, tap **Esc**, release Space. VI stays on
  and the space bar lights amber. Esc in VI is idempotent: tapping it again does
  nothing and sends no Escape.
- **Insert mode (unlock):** tap **I**. Back to the base layer.
- While VI is locked, tapping Space types a space. Holding the Esc/Ctrl key is
  still Ctrl, so Ctrl+Enter and friends work inside VI.
- `w>` and `<b` are Ctrl+Right and Ctrl+Left (word jumps). On macOS these
  switch Spaces unless you remap them; Option+Arrow is the native word jump.

### Caps Word

- Hold **both shifts** together for at least 200 ms, then release. All letter
  keys glow yellow while it is on.
- Letters come out shifted and `-` becomes `_` until you type a space,
  punctuation, or go idle for 5 seconds.
- Caps Word never sends Caps Lock to the host, so a host-side Caps Lock remap
  is unaffected. The literal Caps Lock key is still Fn3+Tab; while host Caps
  Lock is on, all letter keys glow white.
- Releasing both shifts within 200 ms also fires the Space Cadet tap on the
  second shift and types a stray `)`. Hold a beat longer to avoid it.

### Tap-hold feel

- **Esc/Ctrl** and **Enter/Ctrl** become the modifier the instant another key
  is pressed while they are held (TMK-style). Fast Ctrl+A and Ctrl+Enter work
  without dwelling.
- **Space/VI**, **Tab/NUM** and **grave/FPAD** are lazy: a fast roll sends the
  tapped key, and the layer only engages once the key has been held for 200 ms.
- **Shifts** are Space Cadet: tap for `(` and `)`, hold for Shift.

### Numpad and F-pad

- Hold Tab: U/I/O are 4/5/6, J/K/L are 1/2/3, N and M are both 0, `,` and
  `.` stay as they are, the number row stays as digits, P is Backspace and `;`
  is Enter. Grave becomes a plain Esc.
- Hold grave (Mac base): F4/F5/F6 on U/I/O and F1/F2/F3 on J/K/L, with F1 to
  F12 on the number row.
- Lock either one with Space when entering a lot of numbers.

### Lighting

- On every layer except the two bases the animation is replaced by a map of
  the layer: keys that do something on the layer light up, everything else is
  dark. Keys that merely repeat the base layer stay dark too. The map is
  derived from the keymap, so remapping a key updates its light.
- Colours: green navigation (arrows, Home, End), blue page and word jumps,
  teal media, red Delete and bootloader, amber layer and lock keys, purple RGB
  controls, white anything else that is bound.
- A locked layer's activator key (Tab, grave, Space, Fn1, Fn2 or Fn3) is amber.
- Caps Lock glows all letters white; Caps Word glows them yellow. On a mapped
  layer, letters that are bound keep their layer colour and only the
  pass-through letters glow.
- `tools/check_indicators.py` prints the expected map from the compiled keymap:
  `qmk c2json -kb keychron/q4/ansi/v2 -km jongbinjung <keymap.c> | python3 -I tools/check_indicators.py`.

### RGB

- Fn1 or Fn2 then the Q/W/E/R/T row: mode next, brightness up, hue up,
  saturation up, speed up. The A/S/D/F/G row is the same, going down.
- Fn1+Tab or Fn2+Tab (held, not locked) toggles RGB. The toggle state is stored
  in EEPROM and survives reflashing. Layer and Caps indicators do not render
  while RGB is toggled off; lower the brightness instead if you want a dark
  board with indicators.

### Mac vs Windows

- The side DIP switch picks MAC_BASE or WIN_BASE. The bases differ in the
  bottom-row modifiers (Opt/Cmd vs Win/Alt) and in the grave key, which is a
  layer-tap to FNPAD only on Mac.
- Fn1 reaches MAC_FN on Mac and WIN_FN on Windows. The Windows one adds Menu,
  Print Screen and Scroll Lock.

### Recovery and flashing

- **Bootloader:** hold Fn1, then Fn2, then tap Fn3 (SYS_FN layer, `BOOT`).
  The board disconnects and shows up as a DFU device.
- Build: `qmk compile -kb keychron/q4/ansi/v2 -km jongbinjung`.
- Flash: `qmk flash -kb keychron/q4/ansi/v2 -km jongbinjung`, then enter the
  bootloader when prompted.
- **Lights dark, keys fine:** RGB is toggled off in EEPROM. Hold Fn1, tap Tab.
  If that does nothing, use Bootmagic: unplug, hold the top-left (grave) key
  while plugging in. That clears EEPROM and enters the bootloader; flash v2
  again afterwards and RGB comes back at defaults.
