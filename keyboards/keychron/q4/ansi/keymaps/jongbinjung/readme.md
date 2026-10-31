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
| MAC_FN   | hold Fn1 (Mac base)               | yes      | OS: media on the left hand (Q/W/E/R transport, A/S/D volume) and number row, brightness, arrows, nav, Del |
| WIN_FN   | hold Fn1 (Windows base)           | yes      | Same as MAC_FN plus Menu, PrtSc, ScrLk |
| SHR_FN   | hold Fn2                          | yes      | Keyboard: RGB, NKRO; mouse keys on the right hand; F-keys on the number row |
| SYS_FN   | hold Fn1 then Fn2 (or Fn2 then Fn1) | no     | Bootloader on Fn3 |
| NUMPAD   | hold Tab                          | yes      | Numpad on U/I/O, J/K/L, N/M |
| FNPAD    | hold grave (Mac base only)        | yes      | F1 to F6 on the same keys as the numpad |
| HHKB_FN  | hold Fn3                          | yes      | HHKB Fn layer: F-keys, nav on [ ; ' /, media on A/S/D, Caps Lock on Tab |
| VI       | hold Space, or lock with Space+Esc | yes     | vi normal mode: hjkl, w/b/e, 0 ^ $, g/G, x, u, y/p, visual mode on v |

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
|  `  | Bri-| Bri+|  x  |  x  |  x  |  x  | |<< | >|| | >>| | Mute| Vol-| Vol+|           |
|     |     |     |     |     |     |     |     |     |     |     |     |     |           |
+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
|        | |<< | >|| | >>| | Stop|     |     |     |     |     | Ins | PgUp| Home|        |
|        |     |     |     |     |     |     |     |     |     |     |     |     |        |
+---------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-------------+
|         | Vol-| Vol+| Mute|     |     |     |     |  Up |  x  | PgDn| End |             |
|         |     |     |     |     |     |     |     |     |     |     |     |             |
+-------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+---------------+
|             |     |     |     |     |     |     | Left| Down| Rght| Del |               |
|             |     |     |     |     |     |     |     |     |     |     |               |
+-------+------+------+-------------------------------------+-------+------+------+-------+
|       |      |      |                 LOCK                |       |      | SYS  |       |
|       |      |      |                                     |       |      |      |       |
+-------+------+------+-------------------------------------+-------+------+------+-------+
```

### WIN_FN: hold Fn1 on the Windows base (Space locks)
```
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----------+
|  `  | Bri-| Bri+|  x  |  x  |  x  |  x  | |<< | >|| | >>| | Mute| Vol-| Vol+|           |
|     |     |     |     |     |     |     |     |     |     |     |     |     |           |
+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
|        | |<< | >|| | >>| | Stop|     |     |     | Menu| ScrL| Ins | PgUp| Home|        |
|        |     |     |     |     |     |     |     |     |     |     |     |     |        |
+---------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-------------+
|         | Vol-| Vol+| Mute|     |     |     |     |  Up | PrSc| PgDn| End |             |
|         |     |     |     |     |     |     |     |     |     |     |     |             |
+-------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+---------------+
|             |     |     |     |     |     |     | Left| Down| Rght| Del |               |
|             |     |     |     |     |     |     |     |     |     |     |               |
+-------+------+------+-------------------------------------+-------+------+------+-------+
|       |      |      |                 LOCK                |       |      | SYS  |       |
|       |      |      |                                     |       |      |      |       |
+-------+------+------+-------------------------------------+-------+------+------+-------+
```

### SHR_FN: hold Fn2 (Space locks) — keyboard settings + mouse
```
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----------+
|  ~  |  F1 |  F2 |  F3 |  F4 |  F5 |  F6 |  F7 |  F8 |  F9 | F10 | F11 | F12 |           |
|     |     |     |     |     |     |     |     |     |     |     |     |     |           |
+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
|  RGB   | Mod+| Val+| Hue+| Sat+| Spd+|     | WhUp| LClk| MClk| RClk|     |     |        |
|        |     |     |     |     |     |     |     |     |     |     |     |     |        |
+---------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-------------+
|         | Mod-| Val-| Hue-| Sat-| Spd-| MsL | MsDn| MsUp| MsR |     |     |             |
|         |     |     |     |     |     |     |     |     |     |     |     |             |
+-------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+---------------+
|             |     |     |     |     | NKRO| WhDn| Slow| Med | Fast|     |               |
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
|        | PgUp| PgDn|     |     |     |     |  4  |  5  |  6  | Bksp|  [  |  ]  |   \    |
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

### VI: hold Space (Space+Esc locks; i, a, o unlock)
```
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----------+
|  `  |  1  |  2  |  3  |  x  |  5  |  x  |  7  |  8  |  9  | Home|  -  |  =  |           |
|     |     |     |     | $End|     |^Home|     |     |     |     |     |     |           |
+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
|  Tab   |  x  |  w> |  e> |  x  |  x  | yank| undo| ins | open| pste|  x  |  x  |        |
|        |     |     |     |     |     |     | redo| ^ins|above|     |     |     |        |
+---------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-------------+
|   lock  | ins>|  x  | cut |  x  | Top | Left| Down|  Up | Rght|  x  |  x  |     Ent     |
|   Ctl   | ins$|     |     |     | Bot |     |     |     |     |     |     |     Ctl     |
+-------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+---------------+
|             |  x  | del |  x  | vis |  <b | next|  x  |  x  | rep | find|               |
|             |     | bksp|     | line|     | prev|     |     |     |     |               |
+-------+------+------+-------------------------------------+-------+------+------+-------+
|  Ctrl | Opt  | Cmd  |                                     |  Cmd  |      |      |  Ctrl |
|       |      |      |                                     |       |      |      |       |
+-------+------+------+-------------------------------------+-------+------+------+-------+
```
Second line = with Shift (`^ins` inserts at line start, `ins$` appends at line
end, `above` opens a line above, `line` selects the whole line). With Esc held
(Ctrl): b and u page up, f and d page down, r redoes. Keys marked `x` are dead
in normal mode; Esc tap locks VI, or leaves visual mode if it is on.


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
- On a locked Fn2 the Q–T and A–G keys are RGB controls. The RGB toggle (Tab)
  is ignored while locked, so a stray Tab cannot turn the lights off; hue,
  brightness, speed and mode still work. Unlock first if you really want to
  toggle.

### VI mode

Hold Space for a momentary normal mode, or hold Space and tap **Esc** to lock
it (the space bar lights amber). Everything below assumes VI is active.

- **Motions:** h j k l arrows; w and e word forward, b word back (Ctrl+Arrow
  on the host); 0 and ^ line start, $ line end; g top of document, G bottom.
  Hold Esc for the vi Ctrl chords: Ctrl+b / Ctrl+u page up, Ctrl+f / Ctrl+d
  page down. Shift+motion is the host's own selection. Esc+l and friends are
  the host's Ctrl+Arrow, so Ctrl shortcuts still work inside VI.
- **Operators:** x Delete, X Backspace, d cut, y copy, p paste, u undo, U or
  Ctrl+r redo. `/` opens find, n and N jump to the next and previous match,
  `.` repeats the last key (QMK Repeat Key, not a full vi dot-repeat).
- **Insert (leave VI):** i, a (after cursor), I (line start), A (line end),
  o (open line below), O (open line above). Each unlocks VI; while VI is only
  held, they just send their motion.
- **Visual mode:** v starts it, V selects the current line first. Motions now
  extend the selection and their lights turn magenta. y, d, x, p or u act on
  the selection and drop back to normal; v or Esc cancel. Releasing Space, or
  leaving VI any other way, also cancels.
- Letters with no vi meaning are dead in normal mode, so VI never types text.
  The number row still types digits (counts are not supported). While VI is
  locked, tapping Space types a space.
- All shortcuts use Ctrl; macOS is expected to remap Ctrl to Cmd.

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
  is Enter. Grave becomes a plain Esc, and Q and W are Page Up and Page Down
  for scrolling with the left hand while entering numbers.
- Hold grave (Mac base): F4/F5/F6 on U/I/O and F1/F2/F3 on J/K/L, with F1 to
  F12 on the number row.
- Lock either one with Space when entering a lot of numbers.

### Mouse keys

- Hold Fn2: H/J/K/L move the pointer, U and N scroll up and down, I is left
  click, O middle, P right. Hold M, `,` or `.` while moving for slow, medium
  or fast speed. Lock with Space for longer mouse work.

### Lighting

- On every layer except the two bases the animation is replaced by a map of
  the layer: keys that do something on the layer light up, everything else is
  dark. Keys that merely repeat the base layer stay dark too. The map is
  derived from the keymap, so remapping a key updates its light.
- Colours: green navigation (arrows, Home, End, hjkl), blue page, word and
  line jumps, teal media, red Delete, x and bootloader, magenta edit keys
  (cut, copy, paste, undo, visual, repeat), amber layer, lock and insert keys,
  purple RGB controls, cyan mouse keys, white anything else that is bound.
  In VI visual mode the motion keys turn magenta.
- A locked layer's activator key (Tab, grave, Space, Fn1, Fn2 or Fn3) is amber.
- Caps Lock glows all letters white; Caps Word glows them yellow. On a mapped
  layer, letters that are bound keep their layer colour and only the
  pass-through letters glow.
- `tools/check_indicators.py` prints the expected map from the compiled keymap:
  `qmk c2json -kb keychron/q4/ansi/v2 -km jongbinjung <keymap.c> | python3 -I tools/check_indicators.py`.

### RGB and keyboard settings

- Fn2 then the Q/W/E/R/T row: mode next, brightness up, hue up, saturation
  up, speed up. The A/S/D/F/G row is the same, going down.
- Fn2+Tab (held, not locked) toggles RGB. The toggle state is stored in EEPROM
  and survives reflashing. Layer and Caps indicators do not render while RGB
  is toggled off; lower the brightness instead if you want a dark board with
  indicators.
- Fn2+B toggles NKRO (n-key rollover: more than six keys at once). Also stored
  in EEPROM. Off is the safer setting for BIOS screens and KVM switches.

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
- **Lights dark, keys fine:** RGB is toggled off in EEPROM. Hold Fn2, tap Tab.
  If that does nothing, use Bootmagic: unplug, hold the top-left (grave) key
  while plugging in. That clears EEPROM and enters the bootloader; flash v2
  again afterwards and RGB comes back at defaults.
