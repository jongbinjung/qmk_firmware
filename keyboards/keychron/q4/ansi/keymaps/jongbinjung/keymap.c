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
  // Custom keycodes for macros
  UNSAFE = SAFE_RANGE, // This is a custom keycode that is safe to use in QMK,
                       // starting from the SAFE_RANGE to avoid conflicts with
                       // existing keycodes.
  VI_WORD,  // vi navigation 'w'
  VI_BACK,  // vi navigation 'b'
  VI_INS    // vi 'i': leave a locked VI layer (back to "insert mode")
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case VI_WORD:
      if (record->event.pressed) {
        // Vi word forward: Ctrl + Right Arrow
        register_mods(MOD_LCTL);
        SEND_STRING(SS_DOWN(X_RIGHT));
      } else {
        // Release modifier on key release
        SEND_STRING(SS_UP(X_RIGHT));
        unregister_mods(MOD_LCTL);
      }
      break;

    case VI_BACK:
      if (record->event.pressed) {
        // Vi word backward: Ctrl + Left Arrow
        register_mods(MOD_LCTL);
        SEND_STRING(SS_DOWN(X_LEFT));
      } else {
        // Release modifier on key release
        SEND_STRING(SS_UP(X_LEFT));
        unregister_mods(MOD_LCTL);
      }
      break;

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
// to unlock. VI locks via Esc and unlocks via I instead; see
// process_record_user.
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
    KC_GRV,  KC_BRID,  KC_BRIU, KC_NO,   KC_NO,   RM_VALD, RM_VALU, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE,  KC_VOLD,  KC_VOLU,  RM_NEXT,
    RM_TOGG, RM_NEXT,  RM_VALU, RM_HUEU, RM_SATU, RM_SPDU, _______, _______, _______, _______, KC_INS,   KC_PGUP,  KC_HOME,  _______,
    _______, RM_PREV,  RM_VALD, RM_HUED, RM_SATD, RM_SPDD, _______, _______, KC_UP,   KC_NO,   KC_PGDN,  KC_END,   _______,
    _______,           _______, _______, _______, _______, _______, NK_TOGG, KC_LEFT, KC_DOWN, KC_RIGHT, KC_DEL,   _______,
    _______, _______,  _______,                    QK_LLCK,                           _______,  _______,  MO(SYS_FN),  _______),

  [WIN_FN] = LAYOUT_ansi_61(
    KC_GRV,  KC_BRID,  KC_BRIU, KC_NO,   KC_NO,   RM_VALD, RM_VALU, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD,  KC_VOLU,  RM_NEXT,
    RM_TOGG, RM_NEXT,  RM_VALU, RM_HUEU, RM_SATU, RM_SPDU, _______, _______, KC_APP,  KC_SCRL, KC_INS,  KC_PGUP,  KC_HOME,  _______,
    _______, RM_PREV,  RM_VALD, RM_HUED, RM_SATD, RM_SPDD, _______, _______, KC_UP,   KC_PSCR, KC_PGDN, KC_END,   _______,
    _______,           _______, _______, _______, _______, _______, NK_TOGG, KC_LEFT, KC_DOWN, KC_RGHT, KC_DEL,   _______,
    _______, _______,  _______,                            QK_LLCK,                            _______, _______,  MO(SYS_FN),  _______),

  [SHR_FN] = LAYOUT_ansi_61(
    KC_TILD, KC_F1,    KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,   KC_F11,   KC_F12,   _______,
    RM_TOGG, RM_NEXT,  RM_VALU, RM_HUEU, RM_SATU, RM_SPDU, _______, _______, _______, _______, _______,  _______,  _______,  _______,
    _______, RM_PREV,  RM_VALD, RM_HUED, RM_SATD, RM_SPDD, _______, _______, _______, _______, _______,  _______,            _______,
    _______,           _______, _______, _______, _______, _______, _______, _______, _______, _______,  _______,            _______,
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
     KC_GRV,    KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0, KC_MINS, KC_EQL,  KC_BSPC,
     KC_TAB, _______, VI_WORD, _______, _______, _______, _______, KC_PGUP,  VI_INS,  KC_END,   KC_NO, KC_PGDN,  KC_UP,   KC_DEL,
    _______, KC_MSTP, KC_MPLY, KC_PGDN, KC_MNXT, _______, KC_LEFT, KC_DOWN,   KC_UP, KC_RGHT, _______, _______,   RCTL_T(KC_ENT),
    _______,   KC_NO,  KC_DEL,   KC_NO, _______, VI_BACK, KC_PGDN,   KC_NO,   KC_NO,   KC_NO,   KC_NO, _______,
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

// clang-format on
/*  vim: set ts=2 sw=2 tw=80 et : */
