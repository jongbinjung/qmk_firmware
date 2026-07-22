/* Copyright 2022 @ Keychron (https://www.keychron.com)
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

#pragma once

// https://docs.qmk.fm/tap_hold#permissive-hold
// Layer-taps (space/tab/grave) stay on QMK's lazy, timeout-based default so
// fast rolls send the tapped key (matches TMK/HHKB "bean" behavior).
/* #define PERMISSIVE_HOLD */

// TMK resolves mod-taps to the modifier the instant another key interrupts
// them. QMK gates that behind HOLD_ON_OTHER_KEY_PRESS, so enable it per-key
// (see get_hold_on_other_key_press in keymap.c) only for the mod-taps
// (Esc/Ctrl, Enter/RCtrl) while leaving the layer-taps lazy.
// https://docs.qmk.fm/tap_hold#hold-on-other-key-press
#define HOLD_ON_OTHER_KEY_PRESS_PER_KEY
