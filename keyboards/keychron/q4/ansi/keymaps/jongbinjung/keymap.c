/* Copyright 2022 @ Keychron (https://www.keychron.com)
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>

#include QMK_KEYBOARD_H

// clang-format off

enum layers {
  MAC_BASE,
  WIN_BASE,
  MAC_FN,
  WIN_FN,
  SHR_FN,
  SYS_FN,
  NUMPAD,
  FNPAD,
  VI,
  HHKB_FN,
};

enum q4_index {
  // Index maps, used for RGB matrix controls
  I_ESC, I_1, I_2, I_3, I_4, I_5, I_6, I_7, I_8, I_9, I_0, I_MINS, I_EQL, I_BSLS,
  I_TAB, I_Q, I_W, I_E, I_R, I_T, I_Y, I_U, I_I, I_O, I_P, I_LBRC, I_RBRC, I_BSPC,
  I_CAPS, I_A, I_S, I_D, I_F, I_G, I_H, I_J, I_K, I_L, I_SCLN, I_QUOT, I_ENT,
  I_LSFT, I_Z, I_X, I_C, I_V, I_B, I_N, I_M, I_COMM, I_DOT, I_SLSH, I_RSFT,
  I_LCTL, I_LOPT, I_LCMD, I_SPC, I_RCMD, I_FN1, I_FN2, I_FN3
};

enum custom_keycodes {
  UNSAFE = SAFE_RANGE, // first keycode that is safe for a keymap to define
  // vi normal-mode keys. Behaviour lives in vi_keys[] / process_vi_key();
  // VI_O and VI_V are multi-step and handled in process_record_user.
  VI_H, VI_J, VI_K, VI_L, VI_0, VI_W, VI_E, VI_B, VI_F, VI_G, VI_4, VI_6,
  VI_D, VI_U, VI_R, VI_X, VI_Y, VI_P, VI_N, VI_I, VI_A, VI_O, VI_V
};

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

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  if (!process_vi_key(keycode, record)) {
    return false;
  }
  switch (keycode) {
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

    case RM_TOGG:
      // Guard: on a *locked* Fn layer, Tab is RM_TOGG. A stray press turns RGB
      // off, QMK persists that to EEPROM, and every indicator (including the
      // amber lock light) disappears with it, surviving reflashes. Only honour
      // the toggle while the layer is physically held; the other RGB keys
      // (hue, brightness, speed, mode) stay usable when locked.
      if (is_layer_locked(get_highest_layer(layer_state))) {
        return false;
      }
      break;
  }
  return true;
};

// TMK's default mod-tap behavior converts the key to its modifier the moment
// another key interrupts it (interrupted => hold), which lets fast rolls like
// "Esc + J" register as "Ctrl + J" without dwelling on Esc. QMK gates that
// behind HOLD_ON_OTHER_KEY_PRESS, so enable it only for the mod-taps here.
// Layer-taps (space/tab/grave) intentionally fall through to the lazy default
// so rolls send the tapped key instead of switching layers.
bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case LCTL_T(KC_ESC):
    case RCTL_T(KC_ENT):
      return true;  // mod-taps: interrupt => hold (TMK-like, eager)
    default:
      return false; // layer-taps: lazy, timeout-based (roll-friendly)
  }
}

// Layer Lock convention: the space bar is QK_LLCK on every held layer except
// VI (whose activator *is* space). Hold the layer key past TAPPING_TERM, tap
// space, release: the layer stays on. Tap space again (or the layer's Fn key)
// to unlock. VI locks via Esc and unlocks via the insert keys (i, a, o)
// instead; see the vi emulation section above.
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [MAC_BASE] = LAYOUT_ansi_61(
    LT(FNPAD,KC_GRAVE),    KC_1,    KC_2, KC_3, KC_4, KC_5, KC_6, KC_7,    KC_8,   KC_9,       KC_0,    KC_MINS, KC_EQL,  KC_BSLS,
     LT(NUMPAD,KC_TAB),    KC_Q,    KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U,    KC_I,   KC_O,       KC_P,    KC_LBRC, KC_RBRC, KC_BSPC,
        LCTL_T(KC_ESC),    KC_A,    KC_S, KC_D, KC_F, KC_G, KC_H, KC_J,    KC_K,   KC_L,    KC_SCLN,    KC_QUOT, RCTL_T(KC_ENT),
               SC_LSPO,    KC_Z,    KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT,    KC_SLSH,             SC_RSPC,
               KC_LCTL, KC_LOPT, KC_LCMD,           LT(VI,KC_SPC),              KC_RCMD, MO(MAC_FN), MO(SHR_FN), MO(HHKB_FN)),

  [WIN_BASE] = LAYOUT_ansi_61(
             KC_GRAVE,    KC_1,    KC_2, KC_3, KC_4, KC_5, KC_6, KC_7,    KC_8,    KC_9,       KC_0,    KC_MINS, KC_EQL,  KC_BSLS,
    LT(NUMPAD,KC_TAB),    KC_Q,    KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U,    KC_I,    KC_O,       KC_P,    KC_LBRC, KC_RBRC, KC_BSPC,
       LCTL_T(KC_ESC),    KC_A,    KC_S, KC_D, KC_F, KC_G, KC_H, KC_J,    KC_K,    KC_L,    KC_SCLN,    KC_QUOT, RCTL_T(KC_ENT),
              SC_LSPO,    KC_Z,    KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM,  KC_DOT,    KC_SLSH,             SC_RSPC,
              KC_LCTL, KC_LGUI, KC_LALT,            LT(VI, KC_SPC),             KC_RALT, MO(WIN_FN), MO(SHR_FN), MO(HHKB_FN)),

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

  // Keyboard settings on the left hand, mouse keys on the right.
  [SHR_FN] = LAYOUT_ansi_61(
    KC_TILD, KC_F1,    KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,   KC_F11,   KC_F12,   _______,
    RM_TOGG, RM_NEXT,  RM_VALU, RM_HUEU, RM_SATU, RM_SPDU, _______, MS_WHLU, MS_BTN1, MS_BTN3, MS_BTN2,  _______,  _______,  _______,
    _______, RM_PREV,  RM_VALD, RM_HUED, RM_SATD, RM_SPDD, MS_LEFT, MS_DOWN, MS_UP,   MS_RGHT, _______,  _______,            _______,
    _______,           _______, _______, _______, _______, NK_TOGG, MS_WHLD, MS_ACL0, MS_ACL1, MS_ACL2,  _______,            _______,
    _______, _______,  _______,                             QK_LLCK,                           _______,  MO(SYS_FN),  _______,  _______),

  [SYS_FN] = LAYOUT_ansi_61(
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
    _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
    _______, _______, _______,                            _______,                            _______, _______, _______, QK_BOOTLOADER),

  [NUMPAD] = LAYOUT_ansi_61(
    KC_ESC,     KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6, KC_7, KC_8,      KC_9, KC_0,        _______, _______, _______,
    _______, _______, _______, _______, _______, _______, _______, KC_4, KC_5,      KC_6, KC_BSPC,     KC_LBRC, KC_RBRC, KC_BSLS,
    _______, _______, _______, _______, _______, _______, _______, KC_1, KC_2,      KC_3, KC_KP_ENTER, _______, _______,
    _______, _______, _______, _______, _______, _______,    KC_0, KC_0, KC_COMM, KC_DOT, _______, _______,
    _______, _______, _______,                            QK_LLCK,                        _______, _______, _______, _______),

  [FNPAD] = LAYOUT_ansi_61(
    _______,   KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,  KC_F10,  KC_F11,  KC_F12, _______,
    _______, _______, _______, _______, _______, _______, _______,   KC_F4,   KC_F5,   KC_F6, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______, _______,   KC_F1,   KC_F2,   KC_F3, _______, _______, _______,
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______,                            QK_LLCK,                        _______, _______, _______, _______),

  [HHKB_FN] = LAYOUT_ansi_61(
     KC_ESC,   KC_F1,   KC_F2,   KC_F3, KC_F4, KC_F5,   KC_F6, KC_F7,   KC_F8,   KC_F9,  KC_F10,   KC_F11,  KC_F12, KC_DEL,
    KC_CAPS,   KC_NO,   KC_NO,   KC_NO, KC_NO, KC_NO,   KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,    KC_UP,  KC_NO,  KC_DEL,
    _______, KC_VOLD, KC_VOLU, KC_MUTE, KC_NO, KC_NO,   KC_NO, KC_NO, KC_HOME, KC_PGUP, KC_LEFT, KC_RIGHT,  KC_ENT,
    _______,   KC_NO,  KC_DEL,   KC_NO, KC_NO, KC_NO, KC_PGDN, KC_NO,  KC_END, KC_PGDN, KC_DOWN,  _______,
    KC_LCTL, KC_LOPT, KC_LCMD,                        QK_LLCK,             KC_RCMD, _______, _______, _______),

  [VI] = LAYOUT_ansi_61(
     KC_GRV,    KC_1,    KC_2,    KC_3,    VI_4,    KC_5,    VI_6,    KC_7,    KC_8,    KC_9,    VI_0, KC_MINS,  KC_EQL, _______,
     KC_TAB,   KC_NO,    VI_W,    VI_E,    VI_R,   KC_NO,    VI_Y,    VI_U,    VI_I,    VI_O,    VI_P,   KC_NO,   KC_NO, _______,
    _______,    VI_A,   KC_NO,    VI_D,    VI_F,    VI_G,    VI_H,    VI_J,    VI_K,    VI_L,   KC_NO,   KC_NO,   RCTL_T(KC_ENT),
    _______,   KC_NO,    VI_X,   KC_NO,    VI_V,    VI_B,    VI_N,   KC_NO,   KC_NO,  QK_REP, C(KC_F), _______,
    KC_LCTL, KC_LOPT, KC_LCMD,                   _______,                   KC_RCMD, _______, _______, KC_RCTL)

};

// Set one indicator LED only if it falls in this render chunk. Unlike the
// RGB_MATRIX_INDICATOR_SET_COLOR macro, this accepts the RGB_* colour macros.
static void set_indicator(uint8_t led_min, uint8_t led_max, uint8_t led,
                          uint8_t r, uint8_t g, uint8_t b) {
  if (led >= led_min && led < led_max) {
    rgb_matrix_set_color(led, r, g, b);
  }
}

// Layers whose lighting is derived from the keymap: every LED in the chunk is
// set, replacing the animation. Only the base layers are left to the
// animation. Add a layer here to give it the same treatment.
static const layer_state_t mapped_layers =
    (1UL << MAC_FN) | (1UL << WIN_FN) | (1UL << SHR_FN) | (1UL << SYS_FN) |
    (1UL << NUMPAD) | (1UL << FNPAD) | (1UL << HHKB_FN) | (1UL << VI);

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
            // Visual mode: motions go magenta so the selection state is visible.
            const rgb_t c = (vi_visual && layer == VI && is_vi_motion(kc))
                                ? (rgb_t){RGB_MAGENTA}
                                : color_for_keycode(kc);
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

// clang-format on
/*  vim: set ts=2 sw=2 tw=80 et : */
