# VI purity and Fn2 mouse layer — implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the VI layer a faithful vi normal-mode emulation (motions, operators, insert entries, visual mode, Ctrl chords, repeat) and turn Fn2 into the keyboard-settings-plus-mouse layer, with Fn1 reduced to OS media and navigation.

**Architecture:** One `PROGMEM` table of vi keys drives a dispatcher in `process_record_user`; motions register/unregister (so the host auto-repeats), operators tap and end visual mode, insert entries tap and unlock VI. Visual mode is a flag that adds Shift to motions. Indicators get two colours (magenta edit family, cyan mouse) and a runtime recolour of motions while visual mode is on. The Python checker mirrors the colour rules.

**Tech Stack:** QMK (C), `qmk compile`, `qmk c2json` + Python 3 checker.

Spec: `docs/superpowers/specs/2026-10-30-vi-purity-fn2-mouse-design.md`.
Keymap dir: `KM=keyboards/keychron/q4/ansi/keymaps/jongbinjung`.

Deviation from spec, decided here: h j k l and 0 are custom keycodes
(`VI_H VI_J VI_K VI_L VI_0`) rather than plain `KC_*`, so visual-mode shift
and stuck-key safety use the same per-key `vi_down[]` path as every other
motion instead of a second interception mechanism. Motions whose Ctrl column
is empty pass through with the physical Ctrl kept (Esc+l is still the host's
Ctrl+Right); operators with an empty Ctrl column are dead under Ctrl.

---

### Task 1: vi engine and VI layer

**Files:**
- Modify: `$KM/rules.mk`
- Modify: `$KM/keymap.c` (custom keycodes, `process_record_user`, VI layer, new `layer_state_set_user`)

- [ ] **Step 1: Enable Repeat Key**

`$KM/rules.mk`:
```make
CAPS_WORD_ENABLE = yes
LAYER_LOCK_ENABLE = yes
REPEAT_KEY_ENABLE = yes
```

- [ ] **Step 2: Replace the custom keycode enum**

```c
enum custom_keycodes {
  UNSAFE = SAFE_RANGE, // first keycode that is safe for a keymap to define
  // vi normal-mode keys. Behaviour lives in vi_keys[] / process_vi_key();
  // VI_O and VI_V are multi-step and handled in process_record_user.
  VI_H, VI_J, VI_K, VI_L, VI_0, VI_W, VI_E, VI_B, VI_F, VI_G, VI_4, VI_6,
  VI_D, VI_U, VI_R, VI_X, VI_Y, VI_P, VI_N, VI_I, VI_A, VI_O, VI_V
};
```

- [ ] **Step 3: Add the vi engine above `process_record_user`**

```c
// ---- vi emulation ---------------------------------------------------------
// Normal mode is the VI layer. Insert mode is leaving it. Visual mode is a
// flag: while set, motions are sent with Shift so the host extends a
// selection. All shortcuts use Ctrl (the host remaps Ctrl to Cmd on macOS).
enum vi_kind {
  VK_MOTION,    // register on press / unregister on release; shifted in visual
  VK_OPERATOR,  // tap, then leave visual mode
  VK_ACTION,    // tap; visual mode untouched, never shifted
  VK_INSERT,    // tap, leave visual mode, unlock VI
};

// plain: sent normally (KC_NO = nothing). shift: with a shift held (0 = same
// as plain). ctrl: with Ctrl held, always a bare tap (0 = dead for
// operators/actions/inserts, "pass through with Ctrl kept" for motions).
typedef struct {
  uint16_t key, plain, shift, ctrl;
  uint8_t  kind;
} vi_key_t;

static const vi_key_t vi_keys[] PROGMEM = {
  {VI_H, KC_LEFT,    0,          0,          VK_MOTION},
  {VI_J, KC_DOWN,    0,          0,          VK_MOTION},
  {VI_K, KC_UP,      0,          0,          VK_MOTION},
  {VI_L, KC_RGHT,    0,          0,          VK_MOTION},
  {VI_0, KC_HOME,    0,          0,          VK_MOTION},
  {VI_4, KC_NO,      KC_END,     0,          VK_MOTION},   // $
  {VI_6, KC_NO,      KC_HOME,    0,          VK_MOTION},   // ^
  {VI_W, C(KC_RGHT), 0,          0,          VK_MOTION},
  {VI_E, C(KC_RGHT), 0,          0,          VK_MOTION},
  {VI_B, C(KC_LEFT), 0,          KC_PGUP,    VK_MOTION},   // Ctrl+b page up
  {VI_F, KC_NO,      0,          KC_PGDN,    VK_MOTION},   // Ctrl+f page down
  {VI_G, C(KC_HOME), C(KC_END),  0,          VK_MOTION},   // g / G
  {VI_D, C(KC_X),    0,          KC_PGDN,    VK_OPERATOR}, // cut; Ctrl+d half page
  {VI_U, C(KC_Z),    S(C(KC_Z)), KC_PGUP,    VK_OPERATOR}, // undo / redo; Ctrl+u
  {VI_R, KC_NO,      0,          S(C(KC_Z)), VK_OPERATOR}, // Ctrl+r redo
  {VI_X, KC_DEL,     KC_BSPC,    0,          VK_OPERATOR},
  {VI_Y, C(KC_C),    0,          0,          VK_OPERATOR},
  {VI_P, C(KC_V),    0,          0,          VK_OPERATOR},
  {VI_N, C(KC_G),    S(C(KC_G)), 0,          VK_ACTION},   // find next / previous
  {VI_I, KC_NO,      KC_HOME,    0,          VK_INSERT},
  {VI_A, KC_RGHT,    KC_END,     0,          VK_INSERT},
};

static bool     vi_visual;                      // visual mode flag
static uint16_t vi_down[ARRAY_SIZE(vi_keys)];   // code a held motion registered

// Tap kc with the physical modifiers suppressed, so Esc+b yields a bare PgUp
// and X a bare Backspace. kc may carry its own C()/S() wrappers.
static void vi_tap(uint16_t kc) {
  if (kc == KC_NO) {
    return;
  }
  const uint8_t mods = get_mods();
  clear_mods();
  tap_code16(kc);
  set_mods(mods);
}

static void vi_leave_visual(void) {
  vi_visual = false;
}

static void vi_insert(void) {
  vi_leave_visual();
  layer_lock_off(VI);  // no-op while VI is only held via Space
}

static bool is_vi_motion(uint16_t kc) {
  for (uint8_t i = 0; i < ARRAY_SIZE(vi_keys); i++) {
    if (pgm_read_word(&vi_keys[i].key) == kc) {
      return pgm_read_byte(&vi_keys[i].kind) == VK_MOTION;
    }
  }
  return false;
}

// Returns false when the keycode was a vi key (handled), true otherwise.
static bool process_vi_key(uint16_t keycode, keyrecord_t *record) {
  for (uint8_t i = 0; i < ARRAY_SIZE(vi_keys); i++) {
    vi_key_t k;
    memcpy_P(&k, &vi_keys[i], sizeof(k));
    if (k.key != keycode) {
      continue;
    }
    if (!record->event.pressed) {
      // Release: let go of whatever this key registered, whatever the
      // modifiers or visual state are now.
      if (vi_down[i]) {
        unregister_code16(vi_down[i]);
        vi_down[i] = 0;
      }
      return false;
    }
    const uint8_t mods = get_mods();
    if ((mods & MOD_MASK_CTRL) && !(k.kind == VK_MOTION && k.ctrl == 0)) {
      vi_tap(k.ctrl);  // vi Ctrl chord (Esc held): always a single tap
      return false;
    }
    uint16_t kc = ((mods & MOD_MASK_SHIFT) && k.shift) ? k.shift : k.plain;
    if (k.kind == VK_MOTION) {
      if (kc != KC_NO) {
        if (vi_visual) {
          kc |= QK_LSFT;
        }
        vi_down[i] = kc;
        register_code16(kc);  // physical mods stay: Shift+l still selects
      }
      return false;
    }
    vi_tap(kc);
    if (k.kind == VK_OPERATOR) {
      vi_leave_visual();
    } else if (k.kind == VK_INSERT) {
      vi_insert();
    }
    return false;
  }
  return true;
}

layer_state_t layer_state_set_user(layer_state_t state) {
  if (!layer_state_cmp(state, VI)) {
    vi_leave_visual();  // Space released, i/a/o unlocked, or idle timeout
  }
  return state;
}
```

- [ ] **Step 4: Rewrite the vi cases in `process_record_user`**

Delete the `VI_WORD`, `VI_BACK` and `VI_INS` cases. At the top of the function, before the `switch`:

```c
  if (!process_vi_key(keycode, record)) {
    return false;
  }
```

New cases:

```c
    case VI_O:
      // o: open a line below and enter insert. O: open above.
      if (record->event.pressed) {
        const bool above = get_mods() & MOD_MASK_SHIFT;
        vi_tap(above ? KC_HOME : KC_END);
        vi_tap(KC_ENT);
        if (above) {
          vi_tap(KC_UP);
        }
        vi_insert();
      }
      return false;

    case VI_V:
      // v toggles visual mode. V starts it linewise: Home, then select to End.
      if (record->event.pressed) {
        if (vi_visual) {
          vi_leave_visual();
        } else if (get_mods() & MOD_MASK_SHIFT) {
          vi_tap(KC_HOME);
          vi_visual = true;
          vi_tap(S(KC_END));
        } else {
          vi_visual = true;
        }
      }
      return false;
```

Replace the `LCTL_T(KC_ESC)` case body:

```c
    case LCTL_T(KC_ESC):
      // vi "Esc": a *tap* of Esc/Ctrl while VI is active leaves visual mode if
      // it is on, otherwise locks VI (normal mode), instead of sending Escape.
      // Holding it is still Ctrl inside VI; tapping it in an already-locked VI
      // is a no-op, like vi. The key is transparent on the VI layer, so the
      // mod-tap keycode arrives here.
      if (record->tap.count && layer_state_is(VI)) {
        if (record->event.pressed) {
          if (vi_visual) {
            vi_leave_visual();
          } else {
            layer_lock_on(VI);
          }
        }
        return false;
      }
      break;
```

Update the Layer Lock comment above `keymaps[]`: "VI locks via Esc and unlocks via i, a, o (insert) instead".

- [ ] **Step 5: Rewrite the VI layer**

```c
  [VI] = LAYOUT_ansi_61(
     KC_GRV,    KC_1,    KC_2,    KC_3,    VI_4,    KC_5,    VI_6,    KC_7,    KC_8,    KC_9,    VI_0, KC_MINS,  KC_EQL, _______,
     KC_TAB,   KC_NO,    VI_W,    VI_E,    VI_R,   KC_NO,    VI_Y,    VI_U,    VI_I,    VI_O,    VI_P,   KC_NO,   KC_NO, _______,
    _______,    VI_A,   KC_NO,    VI_D,    VI_F,    VI_G,    VI_H,    VI_J,    VI_K,    VI_L,   KC_NO,   KC_NO,   RCTL_T(KC_ENT),
    _______,   KC_NO,    VI_X,   KC_NO,    VI_V,    VI_B,    VI_N,   KC_NO,   KC_NO,  QK_REP, C(KC_F), _______,
    KC_LCTL, KC_LOPT, KC_LCMD,                   _______,                   KC_RCMD, _______, _______, KC_RCTL)
```

- [ ] **Step 6: Compile both variants**

Run: `qmk compile -kb keychron/q4/ansi/v2 -km jongbinjung 2>&1 | grep -E "warning|error|\[OK\]|\[ERRORS\]|\[WARNINGS\]"` and the same for `v1`.
Expected: `[OK]` lines, no warnings or errors.

- [ ] **Step 7: Commit**

```bash
git add $KM/rules.mk $KM/keymap.c
git commit -m "VI layer: table-driven vi emulation with visual mode, Ctrl chords, repeat"
```

---

### Task 2: Fn1 to OS-only, Fn2 to keyboard + mouse

**Files:**
- Modify: `$KM/keymap.c` (MAC_FN, WIN_FN, SHR_FN)

- [ ] **Step 1: MAC_FN and WIN_FN**

```c
  [MAC_FN] = LAYOUT_ansi_61(
    KC_GRV,  KC_BRID,  KC_BRIU, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE,  KC_VOLD,  KC_VOLU,  _______,
    _______, _______,  _______, _______, _______, _______, _______, _______, _______, _______, KC_INS,   KC_PGUP,  KC_HOME,  _______,
    _______, _______,  _______, _______, _______, _______, _______, _______, KC_UP,   KC_NO,   KC_PGDN,  KC_END,   _______,
    _______,           _______, _______, _______, _______, _______, _______, KC_LEFT, KC_DOWN, KC_RIGHT, KC_DEL,   _______,
    _______, _______,  _______,                    QK_LLCK,                           _______,  _______,  MO(SYS_FN),  _______),

  [WIN_FN] = LAYOUT_ansi_61(
    KC_GRV,  KC_BRID,  KC_BRIU, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD,  KC_VOLU,  _______,
    _______, _______,  _______, _______, _______, _______, _______, _______, KC_APP,  KC_SCRL, KC_INS,  KC_PGUP,  KC_HOME,  _______,
    _______, _______,  _______, _______, _______, _______, _______, _______, KC_UP,   KC_PSCR, KC_PGDN, KC_END,   _______,
    _______,           _______, _______, _______, _______, _______, _______, KC_LEFT, KC_DOWN, KC_RGHT, KC_DEL,   _______,
    _______, _______,  _______,                            QK_LLCK,                            _______, _______,  MO(SYS_FN),  _______),
```

- [ ] **Step 2: SHR_FN**

```c
  [SHR_FN] = LAYOUT_ansi_61(
    KC_TILD, KC_F1,    KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,   KC_F11,   KC_F12,   _______,
    RM_TOGG, RM_NEXT,  RM_VALU, RM_HUEU, RM_SATU, RM_SPDU, _______, MS_WHLU, MS_BTN1, MS_BTN3, MS_BTN2,  _______,  _______,  _______,
    _______, RM_PREV,  RM_VALD, RM_HUED, RM_SATD, RM_SPDD, MS_LEFT, MS_DOWN, MS_UP,   MS_RGHT, _______,  _______,            _______,
    _______,           _______, _______, _______, _______, NK_TOGG, MS_WHLD, MS_ACL0, MS_ACL1, MS_ACL2,  _______,            _______,
    _______, _______,  _______,                             QK_LLCK,                           _______,  MO(SYS_FN),  _______,  _______),
```

- [ ] **Step 3: Compile both variants** (as Task 1 step 6). Expected clean.

- [ ] **Step 4: Commit**

```bash
git add $KM/keymap.c
git commit -m "Fn1 is OS media/nav only; Fn2 holds RGB, NKRO and mouse keys"
```

---

### Task 3: Indicators and checker

**Files:**
- Modify: `$KM/keymap.c` (`color_for_keycode`, indicator loop)
- Modify: `$KM/tools/check_indicators.py`

- [ ] **Step 1: Colour rules in `color_for_keycode`**

```c
static rgb_t color_for_keycode(uint16_t kc) {
  switch (kc) {
    case KC_LEFT: case KC_DOWN: case KC_UP: case KC_RGHT:
    case KC_HOME: case KC_END:
    case VI_H: case VI_J: case VI_K: case VI_L: case VI_0:
      return (rgb_t){RGB_GREEN};                      // navigation
    case KC_PGUP: case KC_PGDN:
    case VI_W: case VI_E: case VI_B: case VI_F: case VI_G: case VI_4: case VI_6:
      return (rgb_t){RGB_BLUE};                       // page / word / line jumps
    case KC_MPRV: case KC_MPLY: case KC_MNXT: case KC_MSTP: case KC_MUTE:
    case KC_VOLU: case KC_VOLD: case KC_BRIU: case KC_BRID:
      return (rgb_t){RGB_TEAL};                       // media
    case KC_DEL: case QK_BOOT: case VI_X:
      return (rgb_t){RGB_RED};                        // destructive
    case VI_D: case VI_U: case VI_R: case VI_Y: case VI_P: case VI_V: case QK_REP:
      return (rgb_t){RGB_MAGENTA};                    // edit / clipboard / visual
    case QK_LLCK: case VI_I: case VI_A: case VI_O:
    case QK_MOMENTARY ... QK_MOMENTARY_MAX:
    case QK_TOGGLE_LAYER ... QK_TOGGLE_LAYER_MAX:
    case QK_LAYER_TAP ... QK_LAYER_TAP_MAX:
      return (rgb_t){RGB_ORANGE};                     // layer / lock / insert
    case RM_ON ... RM_SPDD:
      return (rgb_t){RGB_PURPLE};                     // RGB controls
    case QK_MOUSE_CURSOR_UP ... QK_MOUSE_ACCELERATION_2:
      return (rgb_t){RGB_CYAN};                       // mouse keys
    default:
      return (rgb_t){RGB_WHITE};
  }
}
```

- [ ] **Step 2: Visual-mode recolour in the indicator loop**

Replace the `if (!is_passthrough(kc, base_kc)) { ... }` block body:

```c
          if (!is_passthrough(kc, base_kc)) {
            // Visual mode: motions go magenta so the selection state is visible.
            const rgb_t c = (vi_visual && layer == VI && is_vi_motion(kc))
                                ? (rgb_t){RGB_MAGENTA}
                                : color_for_keycode(kc);
            rgb_matrix_set_color(led, c.r, c.g, c.b);
            continue;
          }
```

- [ ] **Step 3: Checker colour sets and assertions**

In `check_indicators.py` replace the colour sets:

```python
GREEN = {"KC_LEFT", "KC_DOWN", "KC_UP", "KC_RGHT", "KC_HOME", "KC_END",
         "VI_H", "VI_J", "VI_K", "VI_L", "VI_0"}
BLUE = {"KC_PGUP", "KC_PGDN", "VI_W", "VI_E", "VI_B", "VI_F", "VI_G", "VI_4", "VI_6"}
TEAL = {"KC_MPRV", "KC_MPLY", "KC_MNXT", "KC_MSTP", "KC_MUTE",
        "KC_VOLU", "KC_VOLD", "KC_BRIU", "KC_BRID"}
RED = {"KC_DEL", "QK_BOOTLOADER", "VI_X"}
MAGENTA = {"VI_D", "VI_U", "VI_R", "VI_Y", "VI_P", "VI_V", "QK_REP"}
AMBER = {"QK_LLCK", "VI_I", "VI_A", "VI_O"}
```

`color()` gains, after the red check:

```python
    if kc in MAGENTA:
        return "magenta"
```

and after the purple check:

```python
    if kc.startswith("MS_"):
        return "cyan"
```

The colour print order becomes `["green", "blue", "teal", "red", "magenta", "amber", "purple", "cyan", "white"]`.

Add `ALIASES` entry `"QK_REPEAT_KEY": "QK_REP"` (harmless if c2json keeps `QK_REP`).

Expectations: drop the MAC_FN Tab line and add:

```python
        (vi.get("4") == "blue" and vi.get("6") == "blue", "VI $ and ^ (4, 6) are blue"),
        (vi.get(".") == "magenta" and vi.get("U") == "magenta", "VI . and u are magenta"),
        (vi.get("A") == "amber", "VI a (append) is amber"),
        ("S" not in vi and "Bksp" not in vi, "VI leaves S and Backspace dark"),
        (shr.get("Tab") == "purple", "SHR_FN Tab (RGB toggle) is purple"),
        (shr.get("J") == "cyan" and shr.get("B") == "white", "SHR_FN J is cyan (mouse), B white (NKRO)"),
        ("Q" not in macfn and "Tab" not in macfn, "MAC_FN leaves Q and Tab dark (RGB moved off)"),
```

with `shr = lit_map(layers, "SHR_FN", "MAC_BASE")` and `macfn = lit_map(layers, "MAC_FN", "MAC_BASE")` defined next to `vi`.

- [ ] **Step 4: Compile both variants, run the checker**

```bash
qmk c2json -kb keychron/q4/ansi/v2 -km jongbinjung $KM/keymap.c | python3 -I $KM/tools/check_indicators.py
```
Expected: per-layer colour listing, twelve `PASS` lines, `all checks passed`.

- [ ] **Step 5: Commit**

```bash
git add $KM/keymap.c $KM/tools/check_indicators.py
git commit -m "Indicators: magenta edit keys, cyan mouse keys, visual-mode recolour"
```

---

### Task 4: Readme

**Files:**
- Modify: `$KM/readme.md`

- [ ] **Step 1: Layer overview rows**

```
| MAC_FN   | hold Fn1 (Mac base)               | yes      | OS: media, brightness, arrows, nav, Del |
| WIN_FN   | hold Fn1 (Windows base)           | yes      | Same as MAC_FN plus Menu, PrtSc, ScrLk |
| SHR_FN   | hold Fn2                          | yes      | Keyboard: RGB, NKRO; mouse keys on the right hand; F-keys on the number row |
| VI       | hold Space, or lock with Space+Esc | yes     | vi normal mode: hjkl, w/b/e, 0 ^ $, g/G, x, u, y/p, visual mode on v |
```

- [ ] **Step 2: Redraw MAC_FN, WIN_FN, SHR_FN, VI diagrams** per the spec. VI diagram second line shows the Shift variant; add below it:

```
Second line = with Shift. With Esc held (Ctrl): b and u page up, f and d
page down, r redoes. Keys marked x are dead in normal mode.
```

- [ ] **Step 3: Rewrite "VI mode" guidelines**

```
### VI mode

Hold Space for a momentary normal mode, or hold Space and tap **Esc** to lock
it (the space bar lights amber). Everything below assumes VI is active.

- **Motions:** h j k l arrows; w and e word forward, b word back (Ctrl+Arrow
  on the host); 0 and ^ line start, $ line end; g top of document, G bottom.
  Hold Esc for the vi Ctrl chords: Ctrl+b / Ctrl+u page up, Ctrl+f / Ctrl+d
  page down. Shift+motion is the host's own selection.
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
  The number row still types digits (counts are not supported).
- All shortcuts use Ctrl; macOS is expected to remap Ctrl to Cmd.
```

- [ ] **Step 4: Add "Mouse keys" section after "Numpad and F-pad"**

```
### Mouse keys

- Hold Fn2: H/J/K/L move the pointer, U and N scroll up and down, I is left
  click, O middle, P right. Hold M, `,` or `.` while moving for slow, medium
  or fast speed. Lock with Space for longer mouse work.
```

- [ ] **Step 5: Update RGB, Lighting, Layer Lock, Recovery text**

- Layer Lock bullet: "On a locked Fn2 the Q–T and A–G keys are RGB controls. The RGB toggle (Tab)…"
- Lighting colours: "green navigation (arrows, Home, End, hjkl), blue page, word and line jumps, teal media, red Delete, x and bootloader, magenta edit keys (cut, copy, paste, undo, visual, repeat), amber layer, lock and insert keys, purple RGB controls, cyan mouse keys, white anything else that is bound." Add: "In VI visual mode the motion keys turn magenta."
- RGB: "Fn2 then the Q/W/E/R/T row…", "Fn2+Tab (held, not locked) toggles RGB…", add "Fn2+B toggles NKRO (n-key rollover), stored in EEPROM."
- Recovery "Lights dark, keys fine": "Hold Fn2, tap Tab."

- [ ] **Step 6: Commit**

```bash
git add $KM/readme.md
git commit -m "Readme: vi normal/insert/visual guide, Fn2 keyboard+mouse layer"
```

---

### Task 5: Spec touch-up, memory, push

- [ ] Update the spec's key table rows for hjkl and 0 to say `VI_H…VI_L`, `VI_0` (custom, same mechanism), and the Indicators bullet to list them green.
- [ ] Update `~/.claude/projects/-Users-jongbin-qmk-firmware/memory/q4-keymap-followups.md`: remove VI_WORD OS-awareness and Repeat Key; note the Fn1 Q–T / A–G keys are free.
- [ ] `git push origin jb/q4`.
