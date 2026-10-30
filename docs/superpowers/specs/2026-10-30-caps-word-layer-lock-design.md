# Caps Word and Layer Lock for the `jongbinjung` Keychron Q4 keymap

Date: 2026-10-30
Branch: `jb/q4`
Keymap: `keyboards/keychron/q4/ansi/keymaps/jongbinjung/`

## Goal

Add two core QMK features to the keymap:

1. **Caps Word**, as a replacement for Caps Lock.
2. **Layer Lock**, so any held layer can be pinned without holding its key, with
   the VI layer behaving like vi's normal/insert modes.

Out of scope: the indicator-loop rewrite identified in the earlier review, the
`VI_WORD`/`VI_BACK` OS-awareness change, and any RGB animation or timeout
additions. Those remain separate follow-ups.

## Current state (relevant parts)

Held layers and their activators:

| Layer     | Activator                | Kind |
|-----------|--------------------------|------|
| NUMPAD    | Tab (`LT(NUMPAD,KC_TAB)`) | layer-tap |
| FNPAD     | Grave (`LT(FNPAD,KC_GRAVE)`, MAC_BASE only) | layer-tap |
| VI        | Space (`LT(VI,KC_SPC)`)  | layer-tap |
| HHKB_FN   | Fn3 (`MO(HHKB_FN)`)      | momentary |
| MAC_FN / WIN_FN | Fn1 (`MO(...)`)    | momentary |
| SHR_FN    | Fn2 (`MO(SHR_FN)`)       | momentary |
| SYS_FN    | Fn1+Fn2 (`MO(SYS_FN)` on the Fn layers) | momentary |

VI is currently toggled on with `TG(VI)` on the space position of MAC_FN and
SHR_FN, and toggled off with `TG(VI)` on the I key of the VI layer.

Shifts are Space Cadet (`SC_LSPO`, `SC_RSPC`) on all defaults. The Command
feature is not enabled on the Q4.

Tapping term is the QMK default, 200 ms. Layer-taps use the lazy default
decision; the two mod-taps use per-key hold-on-other-key-press.

## Design

### 1. Layer Lock placement

The space position becomes `QK_LLCK` on every held layer except VI:

| Layer   | Space position before | after     |
|---------|-----------------------|-----------|
| MAC_FN  | `TG(VI)`              | `QK_LLCK` |
| WIN_FN  | `_______`             | `QK_LLCK` |
| SHR_FN  | `TG(VI)`              | `QK_LLCK` |
| NUMPAD  | `_______`             | `QK_LLCK` |
| FNPAD   | `_______`             | `QK_LLCK` |
| HHKB_FN | `_______`             | `QK_LLCK` |
| SYS_FN  | `_______`             | unchanged |

Behavior:

- **Lock:** hold the layer key past the 200 ms tapping term, tap space.
- **Unlock:** tap space again. For the `MO` layers (Fn1, Fn2, Fn3) pressing the
  Fn key again also unlocks; this is built into `process_layer_lock.c`.
- NUMPAD and FNPAD are layer-taps, so while locked, Tab and Grave keep typing
  their tap keys. Their only unlock paths are space or the idle timeout.
- `LAYER_LOCK_IDLE_TIMEOUT` is set to 60000 ms. Any keypress resets the timer.
- Fn1+Space and Fn2+Space no longer toggle VI. VI entry moves to section 2.

### 2. VI as a vi modal

- **Enter normal mode (lock VI):** the Esc/Ctrl key stays `_______` on the VI
  layer, so it resolves to `LCTL_T(KC_ESC)`. In `process_record_user`, when
  the keycode is `LCTL_T(KC_ESC)`, `record->tap.count` is non-zero, and
  `layer_state_is(VI)` is true, call `layer_lock_on(VI)` on press and return
  `false` on both press and release so no Escape is sent. Holding the key still
  gives Ctrl inside VI. Tapping Esc in an already locked VI is a no-op, like vi.
- **Enter insert mode (unlock VI):** a new custom keycode `VI_INS` replaces
  `TG(VI)` on the I key of the VI layer. On press it calls `layer_lock_off(VI)`
  and returns `false`. If VI is merely held and not locked, it does nothing.
- While VI is locked, a plain tap of space types a space. Holding and releasing
  space leaves VI on. Both are existing Layer Lock behavior for layer-taps.
- No `TG(VI)` remains anywhere in the keymap after this change.

Gesture summary: hold Space, tap Esc, release Space: VI stays on. Tap I: back
to the base layer.

### 3. Caps Word

- `CW_TOGG` replaces `KC_CAPS` on the Tab position of HHKB_FN (Fn3+Tab).
- `BOTH_SHIFTS_TURNS_ON_CAPS_WORD` is defined. Pressing both Space Cadet
  shifts turns Caps Word on immediately. Releasing both within 200 ms also
  fires the second shift's tap and types a stray `)`; holding past 200 ms
  before releasing avoids that. This is documented, accepted behavior.
- Caps Word idle timeout stays at the 5 s default. No other Caps Word options.
- Caps Lock itself is no longer bound anywhere on the keymap.

### 4. RGB indicators

All additions go in `rgb_matrix_indicators_advanced_user`, after the existing
per-layer `switch`, so they paint over the layer colors (the existing cases
first turn every LED in range off, then paint).

- **Locked layer:** light the layer's activator LED in amber (`RGB_ORANGE`)
  when `is_layer_locked(layer)` is true:

  | Layer           | LED     |
  |-----------------|---------|
  | NUMPAD          | `I_TAB` |
  | FNPAD           | `I_ESC` (the grave key) |
  | VI              | `I_SPC` |
  | HHKB_FN         | `I_FN3` |
  | MAC_FN, WIN_FN  | `I_FN1` |
  | SHR_FN          | `I_FN2` |

- **Caps Word:** light `CAPS_LOCK_LED_INDEX` (index 28, the Esc/Ctrl key) in
  yellow (`RGB_YELLOW`) when `is_caps_word_on()` is true.
- **Caps Lock:** the existing white Caps Lock indicator moves from before the
  `switch` to after it, next to the Caps Word check, so it is no longer wiped by
  the per-layer "all off" loops. The dead `!rgb_matrix_get_flags()` branch is
  dropped in the move. Caps Lock (host LED state) wins over Caps Word if both
  are somehow on.
- Use `RGB_MATRIX_INDICATOR_SET_COLOR` for these so the `led_min`/`led_max`
  chunking is respected.

### 5. Files

- `rules.mk` (new):
  ```make
  CAPS_WORD_ENABLE = yes
  LAYER_LOCK_ENABLE = yes
  ```
- `config.h` (add):
  ```c
  #define LAYER_LOCK_IDLE_TIMEOUT 60000
  #define BOTH_SHIFTS_TURNS_ON_CAPS_WORD
  ```
- `keymap.c`: keymap edits from sections 1 to 3, `VI_INS` in
  `custom_keycodes`, the two `process_record_user` cases, and the indicator
  additions from section 4. Keep the existing comment style explaining why
  (TMK parity notes, vi mental model).

## Verification

Build both targets; both must succeed with no new warnings:

```
qmk compile -kb keychron/q4/ansi/v1 -km jongbinjung
qmk compile -kb keychron/q4/ansi/v2 -km jongbinjung
```

Manual checklist after flashing (user):

1. Hold Tab 300 ms, tap Space, release Tab: numbers on U/I/O etc. persist; Tab
   LED amber. Tap Space: back to base.
2. Hold Fn3, tap Space, release: HHKB_FN persists; Fn3 LED amber. Tap Fn3:
   back to base.
3. Hold Fn1, tap Space, release: Fn layer persists; tap hue up repeatedly
   without holding Fn1. Tap Fn1: back to base.
4. Hold Space, tap Esc, release Space: VI persists; Space LED amber. H/J/K/L
   move the cursor. Tap Space: types a space, VI stays. Tap I: back to base.
5. In locked VI, hold Esc/Ctrl and press Enter: Ctrl+Enter is sent.
6. Fn3+Tab: Caps Word on, Esc/Ctrl LED yellow; type `foo_bar` then space:
   `FOO_BAR` and Caps Word off.
7. Hold both shifts 300 ms, release: Caps Word on, nothing typed.
8. Leave any layer locked for 60 s idle: it unlocks.
