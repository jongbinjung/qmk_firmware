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
  VI_BACK  // vi navigation 'b'
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
  }
  return true;
};

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
    _______, _______,  _______,                   _______,                            _______,  _______,  MO(SYS_FN),  _______),

  [WIN_FN] = LAYOUT_ansi_61(
    KC_GRV,  KC_BRID,  KC_BRIU, KC_NO,   KC_NO,   RM_VALD, RM_VALU, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD,  KC_VOLU,  RM_NEXT,
    RM_TOGG, RM_NEXT,  RM_VALU, RM_HUEU, RM_SATU, RM_SPDU, _______, _______, KC_APP,  KC_SCRL, KC_INS,  KC_PGUP,  KC_HOME,  _______,
    _______, RM_PREV,  RM_VALD, RM_HUED, RM_SATD, RM_SPDD, _______, _______, KC_UP,   KC_PSCR, KC_PGDN, KC_END,   _______,
    _______,           _______, _______, _______, _______, _______, NK_TOGG, KC_LEFT, KC_DOWN, KC_RGHT, KC_DEL,   _______,
    _______, _______,  _______,                            _______,                            _______, _______,  MO(SYS_FN),  _______),

  [SHR_FN] = LAYOUT_ansi_61(
    KC_TILD, KC_F1,    KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,   KC_F11,   KC_F12,   _______,
    RM_TOGG, RM_NEXT,  RM_VALU, RM_HUEU, RM_SATU, RM_SPDU, _______, _______, _______, _______, _______,  _______,  _______,  _______,
    _______, RM_PREV,  RM_VALD, RM_HUED, RM_SATD, RM_SPDD, _______, _______, _______, _______, _______,  _______,            _______,
    _______,           _______, _______, _______, _______, _______, _______, _______, _______, _______,  _______,            _______,
    _______, _______,  _______,                            _______,                            _______,  MO(SYS_FN),  _______,  _______),

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
    _______, _______, _______,                            _______,                        _______, _______, _______, _______),

  [FNPAD] = LAYOUT_ansi_61(
    _______,   KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,  KC_F10,  KC_F11,  KC_F12, _______,
    _______, _______, _______, _______, _______, _______, _______,   KC_F4,   KC_F5,   KC_F6, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______, _______,   KC_F1,   KC_F2,   KC_F3, _______, _______, _______,
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______,                            _______,                        _______, _______, _______, _______),

  [HHKB_FN] = LAYOUT_ansi_61(
     KC_ESC,   KC_F1,   KC_F2,   KC_F3, KC_F4, KC_F5,   KC_F6, KC_F7,   KC_F8,   KC_F9,  KC_F10,   KC_F11,  KC_F12, KC_DEL,
    KC_CAPS,   KC_NO,   KC_NO,   KC_NO, KC_NO, KC_NO,   KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,    KC_UP,  KC_NO,  KC_DEL,
    _______, KC_VOLD, KC_VOLU, KC_MUTE, KC_NO, KC_NO,   KC_NO, KC_NO, KC_HOME, KC_PGUP, KC_LEFT, KC_RIGHT,  KC_ENT,
    _______,   KC_NO,  KC_DEL,   KC_NO, KC_NO, KC_NO, KC_PGDN, KC_NO,  KC_END, KC_PGDN, KC_DOWN,  _______,
    KC_LCTL, KC_LOPT, KC_LCMD,                        _______,             KC_RCMD, _______, _______, _______),

  [VI] = LAYOUT_ansi_61(
     KC_GRV,    KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0, KC_MINS, KC_EQL,  KC_BSPC,
     KC_TAB, _______, VI_WORD, _______, _______, _______, _______, KC_PGUP, KC_HOME,  KC_END,   KC_NO, KC_PGDN,  KC_UP,   KC_DEL,
    _______, KC_MSTP, KC_MPLY, KC_MPRV, KC_MNXT, _______, KC_LEFT, KC_DOWN,   KC_UP, KC_RGHT, _______, _______,   RCTL_T(KC_ENT),
    _______,   KC_NO,  KC_DEL,   KC_NO, _______, VI_BACK, KC_PGDN,   KC_NO,   KC_NO,   KC_NO,   KC_NO, _______,
    KC_LCTL, KC_LOPT, KC_LCMD,                   _______,                   KC_RCMD, _______, _______, KC_RCTL)

};

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {

  // Indicators
  if (host_keyboard_led_state().caps_lock) {
    RGB_MATRIX_INDICATOR_SET_COLOR(CAPS_LOCK_LED_INDEX, 255, 255, 255);
  } else {
    if (!rgb_matrix_get_flags()) {
      RGB_MATRIX_INDICATOR_SET_COLOR(CAPS_LOCK_LED_INDEX, 0, 0, 0);
    }
  }

  switch(get_highest_layer(layer_state|default_layer_state)) {

    case NUMPAD:
      for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
          uint8_t index = g_led_config.matrix_co[row][col];

          if (index >= led_min && index < led_max && index != NO_LED) {

            if(keymap_key_to_keycode(SYS_FN, (keypos_t){col,row}) == KC_TRNS) {
              rgb_matrix_set_color(index, RGB_OFF);
            } else {
              rgb_matrix_set_color(I_7, RGB_WHITE);
              rgb_matrix_set_color(I_8, RGB_WHITE);
              rgb_matrix_set_color(I_9, RGB_WHITE);

              rgb_matrix_set_color(I_U, RGB_WHITE);
              rgb_matrix_set_color(I_I, RGB_WHITE);
              rgb_matrix_set_color(I_O, RGB_WHITE);

              rgb_matrix_set_color(I_J, RGB_WHITE);
              rgb_matrix_set_color(I_K, RGB_WHITE);
              rgb_matrix_set_color(I_L, RGB_WHITE);

              rgb_matrix_set_color(I_N, RGB_WHITE);
            }

          }
        }
      }
      break;

    case FNPAD:
      for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
          uint8_t index = g_led_config.matrix_co[row][col];

          if (index >= led_min && index < led_max && index != NO_LED) {
            uint16_t keycode = keymap_key_to_keycode(SYS_FN, (keypos_t){col,row});

            if(keycode == KC_TRNS || keycode == KC_NO) {
              rgb_matrix_set_color(index, RGB_OFF);
            } else {
              rgb_matrix_set_color(I_7, RGB_RED);
              rgb_matrix_set_color(I_8, RGB_RED);
              rgb_matrix_set_color(I_9, RGB_RED);
              rgb_matrix_set_color(I_U, RGB_RED);
              rgb_matrix_set_color(I_I, RGB_RED);
              rgb_matrix_set_color(I_O, RGB_RED);
              rgb_matrix_set_color(I_J, RGB_RED);
              rgb_matrix_set_color(I_K, RGB_RED);
              rgb_matrix_set_color(I_L, RGB_RED);
              rgb_matrix_set_color(I_N, RGB_RED);
            }

          }
        }
      }
      break;

    case HHKB_FN:
      for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
          uint8_t index = g_led_config.matrix_co[row][col];

          if (index >= led_min && index < led_max && index != NO_LED) {

            uint16_t keycode = keymap_key_to_keycode(SYS_FN, (keypos_t){col,row});

            if(keycode == KC_TRNS || keycode == KC_NO) {
              rgb_matrix_set_color(index, RGB_OFF);
            } else {
              rgb_matrix_set_color(I_A, RGB_PURPLE);
              rgb_matrix_set_color(I_S, RGB_TEAL);
              rgb_matrix_set_color(I_D, RGB_RED);

              // Arrows
              rgb_matrix_set_color(I_LBRC, RGB_GREEN);
              rgb_matrix_set_color(I_SCLN, RGB_GREEN);
              rgb_matrix_set_color(I_QUOT, RGB_GREEN);
              rgb_matrix_set_color(I_SLSH, RGB_GREEN);

              rgb_matrix_set_color(I_X, RGB_RED);

              rgb_matrix_set_color(I_U, RGB_BLUE);
              rgb_matrix_set_color(I_N, RGB_BLUE);
              rgb_matrix_set_color(I_W, RGB_BLUE);
              rgb_matrix_set_color(I_B, RGB_BLUE);
            }

          }
        }
      }
      break;

    case VI:
      for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
          uint8_t index = g_led_config.matrix_co[row][col];

          if (index >= led_min && index < led_max && index != NO_LED) {

            uint16_t keycode = keymap_key_to_keycode(SYS_FN, (keypos_t){col,row});

            if(keycode == KC_TRNS || keycode == KC_NO) {
              rgb_matrix_set_color(index, RGB_OFF);
            } else {
              rgb_matrix_set_color(I_A, RGB_RED);
              rgb_matrix_set_color(I_S, RGB_GREEN);
              rgb_matrix_set_color(I_D, RGB_TEAL);
              rgb_matrix_set_color(I_F, RGB_TEAL);

              rgb_matrix_set_color(I_H, RGB_GREEN);
              rgb_matrix_set_color(I_J, RGB_GREEN);
              rgb_matrix_set_color(I_K, RGB_GREEN);
              rgb_matrix_set_color(I_L, RGB_GREEN);

              rgb_matrix_set_color(I_X, RGB_RED);

              rgb_matrix_set_color(I_U, RGB_BLUE);
              rgb_matrix_set_color(I_N, RGB_BLUE);
              rgb_matrix_set_color(I_W, RGB_BLUE);
              rgb_matrix_set_color(I_B, RGB_BLUE);
            }

          }
        }
      }
      break;

    case SYS_FN:
      for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
          uint8_t index = g_led_config.matrix_co[row][col];

          if (index >= led_min && index < led_max && index != NO_LED) {
            uint16_t keycode = keymap_key_to_keycode(SYS_FN, (keypos_t){col,row});

            if(keycode == KC_TRNS || keycode == KC_NO) {
              rgb_matrix_set_color(index, RGB_OFF);
            } else {
              rgb_matrix_set_color(60, RGB_RED);
            }

          }
        }
      }
      break;

    default:
      break;
  }
  return false;
}

// clang-format on
/*  vim: set ts=2 sw=2 tw=80 et : */
