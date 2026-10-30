# Data-Driven Layer Indicators Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the hand-written per-layer indicator blocks in the `jongbinjung` Q4 keymap with one pass that derives key lighting from the keymap, and make Caps Lock / Caps Word glow all letter keys.

**Architecture:** All code lives in `rgb_matrix_indicators_advanced_user` and a few static helpers above it in `keymap.c`. A host-side Python checker applies the same rules to `qmk c2json` output and acts as the test oracle, since QMK keymaps have no unit tests. The readme documents the colours.

**Tech Stack:** QMK firmware (this repo), `qmk` CLI 1.2.0, Python 3 for the checker.

**Spec:** `docs/superpowers/specs/2026-10-30-indicator-rewrite-design.md`

Shorthand:

```bash
KM=keyboards/keychron/q4/ansi/keymaps/jongbinjung
BUILD='qmk compile -kb keychron/q4/ansi/v2 -km jongbinjung && qmk compile -kb keychron/q4/ansi/v1 -km jongbinjung'
CHECK="qmk c2json -kb keychron/q4/ansi/v2 -km jongbinjung $KM/keymap.c 2>/dev/null | python3 -I $KM/tools/check_indicators.py"
```

---

### Task 1: Host-side rule checker (the test oracle)

**Files:**
- Create: `keyboards/keychron/q4/ansi/keymaps/jongbinjung/tools/check_indicators.py`

The checker encodes the spec's per-LED rule and category table in Python, applies them to the compiled keymap, prints the lit set per mapped layer, and asserts the spec's named expectations. It is the reference the firmware must agree with. QMK's build only compiles `keymap.c` plus explicitly listed sources, so a `.py` file in a subfolder is inert.

- [ ] **Step 1: Write the checker**

```python
#!/usr/bin/env python3
"""Apply the indicator lighting rules from the design spec to a compiled keymap.

Usage (from the qmk_firmware root):
  qmk c2json -kb keychron/q4/ansi/v2 -km jongbinjung \
      keyboards/keychron/q4/ansi/keymaps/jongbinjung/keymap.c \
    | python3 -I keyboards/keychron/q4/ansi/keymaps/jongbinjung/tools/check_indicators.py

Prints which keys light up, and in what colour, on each mapped layer, then
checks the expectations listed in the spec. Exit code 1 on any failure.
The rules here must stay in step with color_for_keycode(), is_passthrough()
and mapped_layers in keymap.c.
"""
import json
import re
import sys

# Layer order as written in keymap.c (c2json preserves source order).
NAMES = ["MAC_BASE", "WIN_BASE", "MAC_FN", "WIN_FN", "SHR_FN",
         "SYS_FN", "NUMPAD", "FNPAD", "HHKB_FN", "VI"]
# Physical labels in LAYOUT_ansi_61 order.
LABELS = ("` 1 2 3 4 5 6 7 8 9 0 - = \\ "
          "Tab Q W E R T Y U I O P [ ] Bksp "
          "Esc A S D F G H J K L ; ' Ent "
          "LSft Z X C V B N M , . / RSft "
          "LCtl Opt Cmd Spc RCmd Fn1 Fn2 Fn3").split()
MAPPED = {"NUMPAD", "FNPAD", "HHKB_FN", "VI", "SYS_FN"}

# c2json keeps aliases as written; the firmware compares numeric keycodes.
ALIASES = {"KC_GRV": "KC_GRAVE", "KC_RIGHT": "KC_RGHT", "KC_LOPT": "KC_LALT",
           "KC_ROPT": "KC_RALT", "KC_LCMD": "KC_LGUI", "KC_RCMD": "KC_RGUI",
           "QK_BOOT": "QK_BOOTLOADER", "QK_LAYER_LOCK": "QK_LLCK"}

GREEN = {"KC_LEFT", "KC_DOWN", "KC_UP", "KC_RGHT", "KC_HOME", "KC_END"}
BLUE = {"KC_PGUP", "KC_PGDN", "VI_WORD", "VI_BACK"}
TEAL = {"KC_MPRV", "KC_MPLY", "KC_MNXT", "KC_MSTP", "KC_MUTE",
        "KC_VOLU", "KC_VOLD", "KC_BRIU", "KC_BRID"}
RED = {"KC_DEL", "QK_BOOTLOADER"}
AMBER = {"QK_LLCK", "VI_INS"}
TAP_RE = re.compile(r"^(?:LT\(\w+,|[LR]?(?:CTL|SFT|ALT|GUI|CMD|OPT)_T\()(\w+)\)$")


def norm(kc):
    kc = kc.replace(" ", "")
    return ALIASES.get(kc, kc)


def tap_of(kc):
    m = TAP_RE.match(kc)
    return norm(m.group(1)) if m else None


def is_passthrough(kc, base):
    if kc in ("KC_TRNS", "KC_NO") or kc == base:
        return True
    return tap_of(base) == kc


def color(kc):
    if kc in GREEN:
        return "green"
    if kc in BLUE:
        return "blue"
    if kc in TEAL:
        return "teal"
    if kc in RED:
        return "red"
    if kc in AMBER or re.match(r"^(MO|TG|LT)\(", kc):
        return "amber"
    if kc.startswith("RM_"):
        return "purple"
    return "white"


def lit_map(layers, layer_name, base_name):
    layer = [norm(k) for k in layers[NAMES.index(layer_name)]]
    base = [norm(k) for k in layers[NAMES.index(base_name)]]
    out = {}
    for i, (kc, bkc) in enumerate(zip(layer, base)):
        if not is_passthrough(kc, bkc):
            out[LABELS[i]] = color(kc)
    return out


def main():
    layers = json.load(sys.stdin)["layers"]
    failures = []
    for name in ["NUMPAD", "FNPAD", "HHKB_FN", "VI", "SYS_FN"]:
        lit = lit_map(layers, name, "MAC_BASE")
        by_color = {}
        for key, c in lit.items():
            by_color.setdefault(c, []).append(key)
        print(f"{name}:")
        for c in ["green", "blue", "teal", "red", "amber", "purple", "white"]:
            if c in by_color:
                print(f"  {c:6} {' '.join(by_color[c])}")
        # No key whose entry is transparent/none may ever be lit.
        raw = [norm(k) for k in layers[NAMES.index(name)]]
        for key, c in lit.items():
            if raw[LABELS.index(key)] in ("KC_TRNS", "KC_NO"):
                failures.append(f"{name}: {key} lit but entry is {raw[LABELS.index(key)]}")

    vi = lit_map(layers, "VI", "MAC_BASE")
    hhkb = lit_map(layers, "HHKB_FN", "MAC_BASE")
    fnpad = lit_map(layers, "FNPAD", "MAC_BASE")
    sysfn = lit_map(layers, "SYS_FN", "MAC_BASE")
    expect = [
        (vi.get("I") == "amber", "VI lights I amber (the unlock key)"),
        (not {"U", "W", "B"} & hhkb.keys(), "HHKB_FN leaves U W B dark"),
        ("N" not in fnpad, "FNPAD leaves N dark"),
        (sysfn == {"Fn3": "red"}, "SYS_FN lights only Fn3, red"),
        (vi.get("Esc") is None, "VI Esc/Ctrl is a pass-through (override paints it)"),
    ]
    for ok, msg in expect:
        print(("PASS " if ok else "FAIL ") + msg)
        if not ok:
            failures.append(msg)
    if failures:
        print("\nFAILURES:\n  " + "\n  ".join(failures))
        sys.exit(1)
    print("\nall checks passed")


if __name__ == "__main__":
    main()
```

- [ ] **Step 2: Run it against the current keymap**

Run: `eval $CHECK`
Expected: five layer blocks printed, then five `PASS` lines and `all checks passed`. The checker verifies the *rules* against the keymap; it is expected to pass before the firmware change, because the firmware is what is being brought into line with it. Key things to eyeball in the output: VI lists `I` under amber and `H J K L` under green; HHKB_FN lists `[ ; ' /` under green and does not list `U W B`; NUMPAD lists `M , .` under white.

- [ ] **Step 3: Commit**

```bash
git add $KM/tools/check_indicators.py
git commit -m "Add host-side checker for the data-driven indicator rules

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>"
```

---

### Task 2: Rewrite the indicator function

**Files:**
- Modify: `keyboards/keychron/q4/ansi/keymaps/jongbinjung/keymap.c:211-403` (from the `set_indicator` comment through the end of `rgb_matrix_indicators_advanced_user`)

- [ ] **Step 1: Replace the whole indicator region**

Delete everything from the line `// Set one indicator LED only if it falls in this render chunk. Unlike the` down to and including the closing `}` of `rgb_matrix_indicators_advanced_user` (the line after `return false;`). Put this in its place:

```c
// Set one indicator LED only if it falls in this render chunk. Unlike the
// RGB_MATRIX_INDICATOR_SET_COLOR macro, this accepts the RGB_* colour macros.
static void set_indicator(uint8_t led_min, uint8_t led_max, uint8_t led,
                          uint8_t r, uint8_t g, uint8_t b) {
  if (led >= led_min && led < led_max) {
    rgb_matrix_set_color(led, r, g, b);
  }
}

// Layers whose lighting is derived from the keymap: every LED in the chunk is
// set, replacing the animation. The Fn layers and the bases are left to the
// animation. Add a layer here to give it the same treatment.
static const layer_state_t mapped_layers =
    (1UL << NUMPAD) | (1UL << FNPAD) | (1UL << HHKB_FN) | (1UL << VI) | (1UL << SYS_FN);

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

// Keys whose meaning lives in code rather than in the keymap entry, so the
// generic pass cannot see them. Painted after it, on mapped layers only.
static const struct { uint8_t layer; uint8_t led; rgb_t color; } overrides[] = {
  {VI, I_CAPS, {RGB_ORANGE}},  // Esc/Ctrl tap locks VI (see process_record_user)
};

// A key is a pass-through when it does nothing new on this layer: transparent,
// none, identical to the base layer, or the base key's tap action (KC_TAB over
// LT(NUMPAD,KC_TAB)). Pass-throughs stay dark so only real bindings light up.
static bool is_passthrough(uint16_t kc, uint16_t base) {
  if (kc == KC_TRNS || kc == KC_NO || kc == base) {
    return true;
  }
  if (IS_QK_LAYER_TAP(base) && kc == QK_LAYER_TAP_GET_TAP_KEYCODE(base)) {
    return true;
  }
  if (IS_QK_MOD_TAP(base) && kc == QK_MOD_TAP_GET_TAP_KEYCODE(base)) {
    return true;
  }
  return false;
}

// Colour by what a bound key does. Anything bound but uncategorised is white.
// Mirrored by tools/check_indicators.py; keep the two in step.
static rgb_t color_for_keycode(uint16_t kc) {
  switch (kc) {
    case KC_LEFT: case KC_DOWN: case KC_UP: case KC_RGHT:
    case KC_HOME: case KC_END:
      return (rgb_t){RGB_GREEN};                      // navigation
    case KC_PGUP: case KC_PGDN: case VI_WORD: case VI_BACK:
      return (rgb_t){RGB_BLUE};                       // page / word jumps
    case KC_MPRV: case KC_MPLY: case KC_MNXT: case KC_MSTP: case KC_MUTE:
    case KC_VOLU: case KC_VOLD: case KC_BRIU: case KC_BRID:
      return (rgb_t){RGB_TEAL};                       // media
    case KC_DEL: case QK_BOOT:
      return (rgb_t){RGB_RED};                        // destructive
    case QK_LLCK: case VI_INS:
    case QK_MOMENTARY ... QK_MOMENTARY_MAX:
    case QK_TOGGLE_LAYER ... QK_TOGGLE_LAYER_MAX:
    case QK_LAYER_TAP ... QK_LAYER_TAP_MAX:
      return (rgb_t){RGB_ORANGE};                     // layer / lock
    case RM_ON ... RM_SPDD:
      return (rgb_t){RGB_PURPLE};                     // RGB controls
    default:
      return (rgb_t){RGB_WHITE};
  }
}

// Caps glow: host Caps Lock paints the letter keys white (and wins); Caps Word
// paints them yellow. Returns false when neither is active.
static bool caps_glow(rgb_t *out) {
  if (host_keyboard_led_state().caps_lock) {
    *out = (rgb_t){RGB_WHITE};
    return true;
  }
  if (is_caps_word_on()) {
    *out = (rgb_t){RGB_YELLOW};
    return true;
  }
  return false;
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
  const uint8_t layer  = get_highest_layer(layer_state | default_layer_state);
  const uint8_t base   = get_highest_layer(default_layer_state);
  const bool    mapped = (mapped_layers >> layer) & 1;
  rgb_t         glow;
  const bool    caps   = caps_glow(&glow);

  // One pass over the matrix. Priority per LED: layer binding (mapped layers)
  // > caps glow on letter keys > dark on mapped layers / animation elsewhere.
  if (mapped || caps) {
    for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
      for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
        const uint8_t led = g_led_config.matrix_co[row][col];
        if (led == NO_LED || led < led_min || led >= led_max) {
          continue;
        }
        const keypos_t pos     = {col, row};
        const uint16_t base_kc = keymap_key_to_keycode(base, pos);
        if (mapped) {
          const uint16_t kc = keymap_key_to_keycode(layer, pos);
          if (!is_passthrough(kc, base_kc)) {
            const rgb_t c = color_for_keycode(kc);
            rgb_matrix_set_color(led, c.r, c.g, c.b);
            continue;
          }
        }
        if (caps && base_kc >= KC_A && base_kc <= KC_Z) {
          rgb_matrix_set_color(led, glow.r, glow.g, glow.b);
        } else if (mapped) {
          rgb_matrix_set_color(led, RGB_OFF);
        }
      }
    }
  }

  if (mapped) {
    for (uint8_t i = 0; i < ARRAY_SIZE(overrides); i++) {
      if (overrides[i].layer == layer) {
        set_indicator(led_min, led_max, overrides[i].led,
                      overrides[i].color.r, overrides[i].color.g, overrides[i].color.b);
      }
    }
  }

  // Layer Lock state: light the locked layer's activator key amber. Drawn last
  // so it wins; the space bar is a pass-through on every layer anyway.
  for (uint8_t i = 0; i < ARRAY_SIZE(lock_leds); i++) {
    if (is_layer_locked(lock_leds[i].layer)) {
      set_indicator(led_min, led_max, lock_leds[i].led, RGB_ORANGE);
    }
  }
  return false;
}
```

- [ ] **Step 2: Build both variants**

Run: `eval $BUILD 2>&1 | grep -E 'error|warning: |\[ERRORS\]|Creating binary'`
Expected: two `Creating binary load file ... [OK]` lines, no errors, no warnings mentioning `keymap.c`. If the compiler complains about `case` ranges, it is not using GNU C; QMK always does, so that would indicate a toolchain problem rather than a code one.

- [ ] **Step 3: Confirm the old blocks are gone**

Run: `grep -c "keymap_key_to_keycode(SYS_FN" $KM/keymap.c; grep -c "rgb_matrix_set_color(60" $KM/keymap.c; grep -c "CAPS_LOCK_LED_INDEX" $KM/keymap.c`
Expected: `0`, `0`, `0`.

- [ ] **Step 4: Re-run the checker**

Run: `eval $CHECK | tail -7`
Expected: five `PASS` lines and `all checks passed` (unchanged; the firmware now implements the same rules).

- [ ] **Step 5: Commit**

```bash
git add $KM/keymap.c
git commit -m "Derive layer indicators from the keymap; caps glows all letters

Replaces the per-layer paint blocks (which looked keys up on SYS_FN and
had drifted from the keymap) with one pass: bound keys coloured by
category, pass-throughs dark, Caps Lock / Caps Word glowing the letter
keys white / yellow.

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>"
```

---

### Task 3: Readme

**Files:**
- Modify: `keyboards/keychron/q4/ansi/keymaps/jongbinjung/readme.md` (Caps Word section; new Lighting section before RGB)

- [ ] **Step 1: Update the Caps Word bullets**

Replace:
```markdown
- Hold **both shifts** together for at least 200 ms, then release. The Esc/Ctrl
  key lights yellow.
```
with:
```markdown
- Hold **both shifts** together for at least 200 ms, then release. All letter
  keys glow yellow while it is on.
```
and replace:
```markdown
- Caps Word never sends Caps Lock to the host, so a host-side Caps Lock remap
  is unaffected. The literal Caps Lock key is still Fn3+Tab (white LED).
```
with:
```markdown
- Caps Word never sends Caps Lock to the host, so a host-side Caps Lock remap
  is unaffected. The literal Caps Lock key is still Fn3+Tab; while host Caps
  Lock is on, all letter keys glow white.
```

- [ ] **Step 2: Add a Lighting section**

Insert directly before the line `### RGB`:

```markdown
### Lighting

- On NUMPAD, FNPAD, HHKB_FN, VI and SYS_FN the animation is replaced by a map
  of the layer: keys that do something on the layer light up, everything else
  is dark. Keys that merely repeat the base layer stay dark too. The map is
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

```

- [ ] **Step 3: Commit**

```bash
git add $KM/readme.md
git commit -m "Readme: document derived layer lighting and caps glow

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>"
```

---

### Task 4: Final verification

- [ ] **Step 1: Clean build, checker, status**

Run:
```bash
qmk clean -a >/dev/null 2>&1; eval $BUILD 2>&1 | grep -E 'error|warning: |\[ERRORS\]|Creating binary'
eval $CHECK | tail -2
git status --short && git log --oneline -4
```
Expected: two `[OK]` binary lines, `all checks passed`, clean tree, the three commits from Tasks 1 to 3 on top.

- [ ] **Step 2: Push and hand over for on-device checks**

```bash
git push origin jb/q4
```
Then report: flash v2; lock VI and confirm I is amber and H/J/K/L green; hold Fn3 and confirm the bracket, semicolon, quote and slash keys are green while U, W and B are dark; Fn3+Tab turns all letters white; both shifts held turns them yellow until the next space.
