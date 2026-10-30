# Caps Word and Layer Lock Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add Layer Lock (Space locks any held layer; VI behaves like vi normal/insert modes), Caps Word via both shifts, matching RGB indicators, and a keymap readme to the `jongbinjung` Keychron Q4 keymap.

**Architecture:** All changes live in `keyboards/keychron/q4/ansi/keymaps/jongbinjung/`. Features are switched on in a new `rules.mk`, tuned in `config.h`, and wired in `keymap.c` (keymap table, `process_record_user`, `rgb_matrix_indicators_advanced_user`). The readme is hand-maintained Markdown.

**Tech Stack:** QMK firmware (this repo, `master` merged 2026-10-30), `qmk` CLI 1.2.0, STM32L432 target, Python 3 only for one-off verification one-liners.

**Spec:** `docs/superpowers/specs/2026-10-30-caps-word-layer-lock-design.md`

**No unit tests exist for QMK keymaps.** Each task is verified by compiling both board variants and, where the keymap table changes, by inspecting `qmk c2json` output. Every compile command below must end with `[OK]` lines and no `error:` output.

Shorthand used below:

```bash
KM=keyboards/keychron/q4/ansi/keymaps/jongbinjung
BUILD='qmk compile -kb keychron/q4/ansi/v1 -km jongbinjung && qmk compile -kb keychron/q4/ansi/v2 -km jongbinjung'
C2JSON="qmk c2json -kb keychron/q4/ansi/v1 -km jongbinjung $KM/keymap.c"
```

`c2json` lists layers in **source order**: `MAC_BASE, WIN_BASE, MAC_FN, WIN_FN, SHR_FN, SYS_FN, NUMPAD, FNPAD, HHKB_FN, VI` (note HHKB_FN before VI; the enum order differs). Within a layer, keys are in `LAYOUT_ansi_61` order: index 56 is the space bar, index 22 is the I key, index 14 is the Tab key, index 28 is the Esc/Ctrl key.

---

### Task 1: Enable Caps Word and Layer Lock

**Files:**
- Create: `keyboards/keychron/q4/ansi/keymaps/jongbinjung/rules.mk`
- Modify: `keyboards/keychron/q4/ansi/keymaps/jongbinjung/config.h` (append after line 30, the `HOLD_ON_OTHER_KEY_PRESS_PER_KEY` define)

- [ ] **Step 1: Create rules.mk**

```make
CAPS_WORD_ENABLE = yes
LAYER_LOCK_ENABLE = yes
```

- [ ] **Step 2: Append to config.h**

```c

// Layer Lock (QK_LLCK on the space position of every held layer). Auto-unlock
// after a minute idle so a forgotten lock does not surprise you later.
// https://docs.qmk.fm/features/layer_lock
#define LAYER_LOCK_IDLE_TIMEOUT 60000

// Caps Word is activated only by holding both (Space Cadet) shifts. No CW_TOGG
// key is placed, so Fn3+Tab stays a literal Caps Lock for host-side remaps.
// Hold both shifts past the tapping term before releasing, or the second
// shift's Space Cadet tap types a stray ")".
// https://docs.qmk.fm/features/caps_word
#define BOTH_SHIFTS_TURNS_ON_CAPS_WORD
```

- [ ] **Step 3: Build both variants**

Run: `eval $BUILD 2>&1 | grep -E 'error|\[ERRORS\]|Creating binary' `
Expected: two `Creating binary load file for flashing ... [OK]` lines, no `error` lines.

- [ ] **Step 4: Confirm the features compiled in**

Run: `grep -c "layer_lock\|caps_word" .build/obj_keychron_q4_ansi_v1_jongbinjung/cflags.txt`
Expected: a number greater than 0 (the `-DLAYER_LOCK_ENABLE` and `-DCAPS_WORD_ENABLE` flags are present).

- [ ] **Step 5: Commit**

```bash
git add $KM/rules.mk $KM/config.h
git commit -m "Enable Caps Word (both shifts) and Layer Lock on Q4 keymap

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>"
```

---

### Task 2: Space locks the held layer

**Files:**
- Modify: `keyboards/keychron/q4/ansi/keymaps/jongbinjung/keymap.c:120,127,134,148,155,162` (the bottom row of MAC_FN, WIN_FN, SHR_FN, NUMPAD, FNPAD, HHKB_FN)

- [ ] **Step 1: Replace the space-bar entry on six layers**

Each edit changes only the 4th entry (the 6.25u space bar) of the layer's last row. Full old and new lines:

MAC_FN (line 120):
```c
    _______, _______,  _______,                    TG(VI),                            _______,  _______,  MO(SYS_FN),  _______),
```
becomes
```c
    _______, _______,  _______,                    QK_LLCK,                           _______,  _______,  MO(SYS_FN),  _______),
```

WIN_FN (line 127):
```c
    _______, _______,  _______,                            _______,                            _______, _______,  MO(SYS_FN),  _______),
```
becomes
```c
    _______, _______,  _______,                            QK_LLCK,                            _______, _______,  MO(SYS_FN),  _______),
```

SHR_FN (line 134):
```c
    _______, _______,  _______,                             TG(VI),                            _______,  MO(SYS_FN),  _______,  _______),
```
becomes
```c
    _______, _______,  _______,                             QK_LLCK,                           _______,  MO(SYS_FN),  _______,  _______),
```

NUMPAD (line 148):
```c
    _______, _______, _______,                            _______,                        _______, _______, _______, _______),
```
becomes
```c
    _______, _______, _______,                            QK_LLCK,                        _______, _______, _______, _______),
```

FNPAD (line 155): identical old line to NUMPAD's, identical new line.

HHKB_FN (line 162):
```c
    KC_LCTL, KC_LOPT, KC_LCMD,                        _______,             KC_RCMD, _______, _______, _______),
```
becomes
```c
    KC_LCTL, KC_LOPT, KC_LCMD,                        QK_LLCK,             KC_RCMD, _______, _______, _______),
```

Do **not** touch SYS_FN (line 141) or VI (line 171): SYS_FN stays transparent, and VI's space is the key being held.

- [ ] **Step 2: Add a comment above the keymap table explaining the convention**

Insert directly above line 100 (`const uint16_t PROGMEM keymaps[]...`):

```c
// Layer Lock convention: the space bar is QK_LLCK on every held layer except
// VI (whose activator *is* space). Hold the layer key past TAPPING_TERM, tap
// space, release: the layer stays on. Tap space again (or the layer's Fn key)
// to unlock. VI locks via Esc and unlocks via I instead; see
// process_record_user.
```

- [ ] **Step 3: Verify the table with c2json**

Run:
```bash
eval $C2JSON | python3 -I -c 'import json,sys; L=json.load(sys.stdin)["layers"]; print([l[56] for l in L]); print(sum(k=="TG(VI)" for l in L for k in l), "TG(VI) left")'
```
Expected: a 10-item list whose entries, in order, are: an `LT(VI...)` for MAC_BASE, an `LT(VI...)` for WIN_BASE, `QK_LLCK`, `QK_LLCK`, `QK_LLCK`, `KC_TRNS` (SYS_FN), `QK_LLCK`, `QK_LLCK`, `QK_LLCK` (HHKB_FN), `KC_TRNS` (VI). Second line: `1 TG(VI) left` (the one on VI's I key, replaced in Task 3).

- [ ] **Step 4: Build both variants**

Run: `eval $BUILD 2>&1 | grep -E 'error|\[ERRORS\]|Creating binary'`
Expected: two `[OK]` binary lines, no errors.

- [ ] **Step 5: Commit**

```bash
git add $KM/keymap.c
git commit -m "Space bar locks the held layer (QK_LLCK) on all held layers but VI

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>"
```

---

### Task 3: VI as a vi modal (Esc locks, I unlocks)

**Files:**
- Modify: `keyboards/keychron/q4/ansi/keymaps/jongbinjung/keymap.c:46-53` (custom_keycodes enum), `:55-82` (process_record_user), `:169` (VI layer, I key)

- [ ] **Step 1: Add the VI_INS keycode**

Replace the enum (lines 46 to 53):
```c
enum custom_keycodes {
  // Custom keycodes for macros
  UNSAFE = SAFE_RANGE, // This is a custom keycode that is safe to use in QMK,
                       // starting from the SAFE_RANGE to avoid conflicts with
                       // existing keycodes.
  VI_WORD,  // vi navigation 'w'
  VI_BACK  // vi navigation 'b'
};
```
with
```c
enum custom_keycodes {
  // Custom keycodes for macros
  UNSAFE = SAFE_RANGE, // This is a custom keycode that is safe to use in QMK,
                       // starting from the SAFE_RANGE to avoid conflicts with
                       // existing keycodes.
  VI_WORD,  // vi navigation 'w'
  VI_BACK,  // vi navigation 'b'
  VI_INS    // vi 'i': leave a locked VI layer (back to "insert mode")
};
```

- [ ] **Step 2: Put VI_INS on the I key of the VI layer**

Line 169 (second row of `[VI]`):
```c
     KC_TAB, _______, VI_WORD, _______, _______, _______, _______, KC_PGUP,  TG(VI),  KC_END,   KC_NO, KC_PGDN,  KC_UP,   KC_DEL,
```
becomes
```c
     KC_TAB, _______, VI_WORD, _______, _______, _______, _______, KC_PGUP,  VI_INS,  KC_END,   KC_NO, KC_PGDN,  KC_UP,   KC_DEL,
```

- [ ] **Step 3: Handle VI_INS and the Esc tap in process_record_user**

Inside `process_record_user`, after the `case VI_BACK:` block's `break;` (line 79) and before the closing `}` of the `switch`, add:

```c

    case VI_INS:
      // vi "i": leave normal mode. layer_lock_off is a no-op when VI is only
      // being held via space, so this never fights the momentary layer.
      if (record->event.pressed) {
        layer_lock_off(VI);
      }
      return false;

    case LCTL_T(KC_ESC):
      // vi "Esc": a *tap* of Esc/Ctrl while VI is active locks VI (normal
      // mode) instead of sending Escape. Holding it is still Ctrl inside VI,
      // and tapping it in an already-locked VI is a no-op, like vi. The key is
      // transparent on the VI layer, so the mod-tap keycode arrives here.
      if (record->tap.count && layer_state_is(VI)) {
        if (record->event.pressed) {
          layer_lock_on(VI);
        }
        return false;
      }
      break;
```

The existing `return true;` after the switch stays.

- [ ] **Step 4: Verify the table with c2json**

Run:
```bash
eval $C2JSON | python3 -I -c 'import json,sys; L=json.load(sys.stdin)["layers"]; print("I key on VI:", L[9][22]); print(sum(k=="TG(VI)" for l in L for k in l), "TG(VI) left")'
```
Expected:
```
I key on VI: VI_INS
0 TG(VI) left
```

- [ ] **Step 5: Build both variants**

Run: `eval $BUILD 2>&1 | grep -E 'error|warning: |\[ERRORS\]|Creating binary'`
Expected: two `[OK]` binary lines, no errors, no new warnings mentioning `keymap.c`.

- [ ] **Step 6: Commit**

```bash
git add $KM/keymap.c
git commit -m "VI layer: Esc tap locks (normal mode), I unlocks (insert mode)

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>"
```

---

### Task 4: RGB indicators for lock state, Caps Word and Caps Lock

**Files:**
- Modify: `keyboards/keychron/q4/ansi/keymaps/jongbinjung/keymap.c` (the `rgb_matrix_indicators_advanced_user` function; after Task 3 it starts a few lines below 173)

Background: `RGB_MATRIX_INDICATOR_SET_COLOR(i, r, g, b)` is a 4-argument macro, and `RGB_ORANGE` expands to three comma-separated values *after* argument counting, so it cannot be passed to that macro. A small helper function takes the colour macros fine.

- [ ] **Step 1: Add a range-checked colour helper above the indicator function**

Insert directly above `bool rgb_matrix_indicators_advanced_user(...)`:

```c
// Set one indicator LED only if it falls in this render chunk. Unlike the
// RGB_MATRIX_INDICATOR_SET_COLOR macro, this accepts the RGB_* colour macros.
static void set_indicator(uint8_t led_min, uint8_t led_max, uint8_t led,
                          uint8_t r, uint8_t g, uint8_t b) {
  if (led >= led_min && led < led_max) {
    rgb_matrix_set_color(led, r, g, b);
  }
}

// Which LED to light (amber) when a layer is locked: its activator key.
static const struct { uint8_t layer; uint8_t led; } lock_leds[] = {
  {NUMPAD,  I_TAB},
  {FNPAD,   I_ESC},   // I_ESC is the top-left (grave) key in this enum
  {VI,      I_SPC},
  {HHKB_FN, I_FN3},
  {MAC_FN,  I_FN1},
  {WIN_FN,  I_FN1},
  {SHR_FN,  I_FN2},
};
```

- [ ] **Step 2: Remove the old Caps Lock block at the top of the function**

Delete these lines (currently the first thing inside the function):
```c

  // Indicators
  if (host_keyboard_led_state().caps_lock) {
    RGB_MATRIX_INDICATOR_SET_COLOR(CAPS_LOCK_LED_INDEX, 255, 255, 255);
  } else {
    if (!rgb_matrix_get_flags()) {
      RGB_MATRIX_INDICATOR_SET_COLOR(CAPS_LOCK_LED_INDEX, 0, 0, 0);
    }
  }
```
so the function body begins with `switch(get_highest_layer(...)) {`. (The `!rgb_matrix_get_flags()` branch was dead code: nothing in this keymap ever changes the flags.)

- [ ] **Step 3: Add the new indicators after the switch**

Between the `switch` block's closing `}` and `return false;` at the end of the function, insert:

```c

  // Drawn after the per-layer blocks above, which first blank every LED in
  // range, so these always win.

  // Layer Lock state: light the locked layer's activator key amber.
  for (uint8_t i = 0; i < ARRAY_SIZE(lock_leds); i++) {
    if (is_layer_locked(lock_leds[i].layer)) {
      set_indicator(led_min, led_max, lock_leds[i].led, RGB_ORANGE);
    }
  }

  // Caps: host Caps Lock (literal key, Fn3+Tab) is white and wins; Caps Word
  // (both shifts) is yellow. Caps Word never changes the host LED state.
  if (host_keyboard_led_state().caps_lock) {
    set_indicator(led_min, led_max, CAPS_LOCK_LED_INDEX, RGB_WHITE);
  } else if (is_caps_word_on()) {
    set_indicator(led_min, led_max, CAPS_LOCK_LED_INDEX, RGB_YELLOW);
  }
```

- [ ] **Step 4: Build both variants**

Run: `eval $BUILD 2>&1 | grep -E 'error|warning: |\[ERRORS\]|Creating binary'`
Expected: two `[OK]` binary lines, no errors, no warnings mentioning `keymap.c`. If the compiler reports `implicit declaration of function 'is_layer_locked'` or `'is_caps_word_on'`, Task 1's `rules.mk` was not picked up: confirm the file name and that it sits next to `keymap.c`.

- [ ] **Step 5: Commit**

```bash
git add $KM/keymap.c
git commit -m "Indicators: amber for locked layers, yellow for Caps Word, fix Caps Lock

Caps Lock moved after the per-layer blocks so it is no longer wiped by
them; the dead rgb_matrix_get_flags() branch is gone.

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>"
```

---

### Task 5: Keymap readme

**Files:**
- Create: `keyboards/keychron/q4/ansi/keymaps/jongbinjung/readme.md`

- [ ] **Step 1: Write the readme**

Write the following file verbatim. The diagrams were generated from `qmk c2json` with the Task 2 and Task 3 edits applied, with one manual override: the Esc/Ctrl key on the VI diagram reads `lock`/`Ctl` because its tap is intercepted in code.

````markdown
# jongbinjung Keychron Q4 keymap

Personal HHKB-flavoured keymap for the Keychron Q4 (ANSI). Diagrams below are
maintained by hand. To cross-check them against the source:

```
qmk c2json -kb keychron/q4/ansi/v1 -km jongbinjung keyboards/keychron/q4/ansi/keymaps/jongbinjung/keymap.c
```

Flash with `qmk flash -kb keychron/q4/ansi/v1 -km jongbinjung` (or `v2`; the
USB device version in System Information tells you which board you have:
1.0.x is v1, 2.0.x is v2).

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

### MAC_BASE
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

### WIN_BASE
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

### MAC_FN
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

### WIN_FN
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

### SHR_FN
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

### SYS_FN
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

### NUMPAD
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

### FNPAD
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

### HHKB_FN
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

### VI
```
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----------+
|  `  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  0  |  -  |  =  |    Bksp   |
|     |     |     |     |     |     |     |     |     |     |     |     |     |           |
+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
|  Tab   |     |  w> |     |     |     |     | PgUp| ins | End |  x  | PgDn|  Up |  Del   |
|        |     |     |     |     |     |     |     |     |     |     |     |     |        |
+---------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-------------+
|   lock  | Stop| >|| | |<< | >>| |     | Left| Down|  Up | Rght|     |     |     Ent     |
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

- Hold **both shifts** together for at least 200 ms, then release. The Esc/Ctrl
  key lights yellow.
- Letters come out shifted and `-` becomes `_` until you type a space,
  punctuation, or go idle for 5 seconds.
- Caps Word never sends Caps Lock to the host, so a host-side Caps Lock remap
  is unaffected. The literal Caps Lock key is still Fn3+Tab (white LED).
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

### RGB

- Fn1 or Fn2 then the Q/W/E/R/T row: mode next, brightness up, hue up,
  saturation up, speed up. The A/S/D/F/G row is the same, going down.
- Fn1+Tab or Fn2+Tab toggles RGB. Layer and Caps indicators do not render
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
- Build: `qmk compile -kb keychron/q4/ansi/v1 -km jongbinjung` (or `v2`).
- Flash: `qmk flash -kb keychron/q4/ansi/v1 -km jongbinjung`, then enter the
  bootloader when prompted.
````

- [ ] **Step 2: Cross-check the diagrams against the compiled keymap**

Run:
```bash
eval $C2JSON | python3 -I -c '
import json,sys
L=json.load(sys.stdin)["layers"]
names=["MAC_BASE","WIN_BASE","MAC_FN","WIN_FN","SHR_FN","SYS_FN","NUMPAD","FNPAD","HHKB_FN","VI"]
for n,l in zip(names,L):
    print(n, "space:", l[56], "| I:", l[22], "| Tab:", l[14], "| Esc/Ctrl:", l[28])'
```
Expected:
```
MAC_BASE space: LT(VI,KC_SPC) | I: KC_I | Tab: LT(NUMPAD,KC_TAB) | Esc/Ctrl: LCTL_T(KC_ESC)
WIN_BASE space: LT(VI,KC_SPC) | I: KC_I | Tab: LT(NUMPAD,KC_TAB) | Esc/Ctrl: LCTL_T(KC_ESC)
MAC_FN   space: QK_LLCK | I: KC_TRNS | Tab: RM_TOGG | Esc/Ctrl: KC_TRNS
WIN_FN   space: QK_LLCK | I: KC_APP | Tab: RM_TOGG | Esc/Ctrl: KC_TRNS
SHR_FN   space: QK_LLCK | I: KC_TRNS | Tab: RM_TOGG | Esc/Ctrl: KC_TRNS
SYS_FN   space: KC_TRNS | I: KC_TRNS | Tab: KC_TRNS | Esc/Ctrl: KC_TRNS
NUMPAD   space: QK_LLCK | I: KC_5 | Tab: KC_TRNS | Esc/Ctrl: KC_TRNS
FNPAD    space: QK_LLCK | I: KC_F5 | Tab: KC_TRNS | Esc/Ctrl: KC_TRNS
HHKB_FN  space: QK_LLCK | I: KC_NO | Tab: KC_CAPS | Esc/Ctrl: KC_TRNS
VI       space: KC_TRNS | I: VI_INS | Tab: KC_TAB | Esc/Ctrl: KC_TRNS
```
(`LT(VI,KC_SPC)` may print with a space after the comma for WIN_BASE; that is fine.) Then open the readme and the c2json output side by side and walk each of the ten diagrams row by row, confirming every non-blank label corresponds to the keycode in that position. Fix any mismatch in the readme, not the keymap.

- [ ] **Step 3: Commit**

```bash
git add $KM/readme.md
git commit -m "Add keymap readme with layer diagrams and usage notes

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>"
```

---

### Task 6: Final verification

**Files:** none modified.

- [ ] **Step 1: Clean build of both variants**

Run:
```bash
qmk clean -a >/dev/null 2>&1; eval $BUILD 2>&1 | grep -E 'error|warning: |\[ERRORS\]|Creating binary'
ls -la keychron_q4_ansi_v1_jongbinjung.bin keychron_q4_ansi_v2_jongbinjung.bin
```
Expected: two `[OK]` binary lines, no errors, no warnings mentioning `keymap.c`. Both `.bin` files exist and are under 60 KB (the pre-change size was 46 KB; the two features add a few KB).

- [ ] **Step 2: Confirm the working tree is clean and the history reads well**

Run: `git status --short && git log --oneline master..HEAD`
Expected: no uncommitted files; the log shows the spec commits followed by the five implementation commits from Tasks 1 to 5.

- [ ] **Step 3: Hand over for on-device testing**

The remaining checks need the physical board. Report to the user that the branch is ready to flash, and point them to the manual checklist in the spec's Verification section (lock NUMPAD, HHKB_FN and an Fn layer with Space; Space+Esc then I for VI; Ctrl+Enter inside locked VI; both shifts for Caps Word with the yellow LED; Fn3+Tab still toggles host Caps Lock; 60 s idle unlock). Do not push unless asked.
