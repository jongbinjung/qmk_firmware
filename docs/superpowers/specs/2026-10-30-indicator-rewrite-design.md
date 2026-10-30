# Data-driven layer indicators for the `jongbinjung` Keychron Q4 keymap

Date: 2026-10-30
Branch: `jb/q4`
File: `keyboards/keychron/q4/ansi/keymaps/jongbinjung/keymap.c`,
function `rgb_matrix_indicators_advanced_user`

## Problem

1. Each per-layer indicator case walks the key matrix and looks keys up on
   `SYS_FN` instead of the layer being drawn. SYS_FN is transparent everywhere
   except Fn3 (LED 60), so the loop blanks every LED and runs a hard-coded paint
   block when it reaches LED 60. It works only because LED 60 is in the last of
   QMK's five render chunks. The SYS_FN case hard-codes `60`.
2. The hand-written key lists have drifted from the keymap. Measured against
   the compiled keymap: HHKB_FN lights U, W, B which are `KC_NO` and leaves
   Tab, K, L, comma, period, Backspace dark; FNPAD lights N which is
   transparent and leaves the F1 to F12 row dark; NUMPAD leaves M, comma,
   period, P, semicolon and the brackets dark; VI leaves I (the unlock key),
   O, the brackets and Backspace dark.

Out of scope: indicators not rendering while RGB is toggled off (QMK skips the
callbacks when the effect is disabled). Brightness zero remains the "dark board
with indicators" mode.

## Design

Replace the five per-layer `case` blocks with one generic pass that derives
lighting from the keymap. The Layer Lock and Caps indicators added earlier stay
exactly as they are, drawn after the generic pass.

### Which layers

The same set the old code handled: `NUMPAD`, `FNPAD`, `HHKB_FN`, `VI`,
`SYS_FN`. On these the background animation is replaced by the key map (every
LED in the chunk is set). `MAC_FN`, `WIN_FN`, `SHR_FN` and the base layers are
untouched: the animation shows through, as today. The set is a bitmask constant
so adding a layer is a one-token change.

### Per-LED rule

For each matrix position whose LED index falls in `[led_min, led_max)`:

- `kc` = keycode at that position on the active layer
  (`get_highest_layer(layer_state | default_layer_state)`).
- `base` = keycode at that position on the base layer
  (`get_highest_layer(default_layer_state)`).
- **Dark** (`RGB_OFF`) when the key is a pass-through: `kc` is `KC_TRNS` or
  `KC_NO`; or `kc == base`; or `base` is a layer-tap or mod-tap and `kc` is
  its tap keycode. This keeps VI's explicit number row, Tab, Enter/Ctrl and
  bottom-row modifiers dark, since they repeat the base layer.
- Otherwise **colour by keycode category**:

  | Category | Keycodes | Colour |
  |---|---|---|
  | Navigation | `KC_LEFT KC_DOWN KC_UP KC_RGHT KC_HOME KC_END` | `RGB_GREEN` |
  | Page / word jump | `KC_PGUP KC_PGDN VI_WORD VI_BACK` | `RGB_BLUE` |
  | Media | `KC_MPRV KC_MPLY KC_MNXT KC_MSTP KC_MUTE KC_VOLU KC_VOLD KC_BRIU KC_BRID` | `RGB_TEAL` |
  | Destructive | `KC_DEL QK_BOOT` | `RGB_RED` |
  | Layer / lock | `QK_LLCK VI_INS`, any `MO()`, `TG()`, `LT()` | `RGB_ORANGE` |
  | RGB controls | `RM_ON` through `RM_SPDD` | `RGB_PURPLE` |
  | Everything else bound | digits, F-keys, Esc, Tab, Caps, brackets, Enter, punctuation, NKRO, modifiers | `RGB_WHITE` |

### Caps glow (Caps Lock and Caps Word)

Replaces the single-LED Caps indicator on the Esc/Ctrl key. When host Caps
Lock is on, every letter key (`KC_A` to `KC_Z` on the base layer) glows
`RGB_WHITE`; when Caps Word is on, `RGB_YELLOW`. Caps Lock wins if both are on.

Interaction with the layer map, decided per LED inside the same loop:

- On a mapped layer, a letter that is **bound** on that layer (not a
  pass-through) keeps its layer colour; the glow does not cover it. A letter
  that is a pass-through glows instead of going dark.
- On an unmapped layer (base, MAC_FN, WIN_FN, SHR_FN), letters glow over the
  animation and all other LEDs are left to the animation.
- Letter-ness is judged on the base layer entry, so it is the same set of
  keys on every layer.

### Overrides

A small table of `{layer, led, colour}` painted after the generic pass, for
keys whose meaning is not visible in the keymap entry:

- `VI`, `I_CAPS` (the Esc/Ctrl key), `RGB_ORANGE`: the key is transparent on
  VI but its tap locks the layer via `process_record_user`.

No other overrides. Any future "I want this one key a different colour" goes
in this table rather than back into per-layer blocks.

### Code shape

- `static rgb_t color_for_keycode(uint16_t kc)`: the category switch above,
  returning a `rgb_t` (QMK's `{r, g, b}` struct).
- `static bool is_passthrough(uint16_t kc, uint16_t base)`: the dark rule.
- `static bool caps_glow(rgb_t *out)`: true with white if host Caps Lock is
  on, else true with yellow if `is_caps_word_on()`, else false.
- `static const uint16_t mapped_layers`: bitmask of the five layers.
- `static const struct { uint8_t layer; uint8_t led; rgb_t color; } overrides[]`.
- In `rgb_matrix_indicators_advanced_user`: one matrix loop over LEDs in the
  chunk. For each: if the layer is mapped and the key is bound, paint the
  category colour. Else if caps glow is active and the base entry is a letter,
  paint the glow. Else if the layer is mapped, paint `RGB_OFF`; otherwise leave
  the LED to the animation. Then the overrides (only when the layer is
  mapped), then the existing Layer Lock indicator. The old single-LED Caps
  block is removed. Return `false` as today.
- The `switch` and its five cases are deleted. The `q4_index` enum stays (used
  by `lock_leds` and `overrides`).

### Expected result per layer (output of tools/check_indicators.py)

Keys not listed are dark. Dark includes keys that repeat the base layer, such
as NUMPAD's number row, brackets, comma and period.

- NUMPAD: white grave (Esc), U I O, P (Backspace), Backspace position
  (backslash), J K L, semicolon (Enter), N M; amber Space.
- FNPAD: white number row (F1 to F12), U I O, J K L; amber Space.
- HHKB_FN: green bracket, K, semicolon, quote, comma, slash; blue L, N,
  period; teal A S D; red backslash position (Delete), Backspace, X; amber
  Space; white number row (F-keys) and Tab (Caps Lock). U W B dark. Bottom-row
  modifiers dark on the Mac base, white on the Windows base.
- VI: green O, right bracket, H J K L; blue W U left bracket D B N; teal
  A S F; red Backspace, X; amber I, plus Esc/Ctrl via the override; white
  backslash position (Backspace) and Fn3 (`KC_RCTL`). Number row, Tab, Enter,
  bottom-row modifiers dark.
- SYS_FN: red Fn3. Everything else dark.

Caps glow expected result: on the base layer with Caps Lock on, the 26 letter
keys are white and everything else animates. With Caps Word on, yellow. In
locked VI with Caps Word on, H J K L stay green, W B D blue, A S F teal, X red,
I amber, and the remaining letters (Q E R T Y P G C V M) glow yellow.

## Verification

- Both targets compile with no warnings.
- A host-side Python check applies the same rules to `qmk c2json` output and
  prints the lit set per layer; it must match the list above, and in
  particular: VI lights I; HHKB_FN does not light U, W, B; FNPAD does not light
  N; no layer lights a key whose entry is `KC_TRNS` or `KC_NO`.
- On the board: lock VI, confirm I is lit; hold Fn3, confirm the arrow cluster
  on the brackets, semicolon, quote and slash is green and U/W/B are dark.
  Fn3+Tab: all letters white. Both shifts held: all letters yellow, then back
  to the animation after a space.
