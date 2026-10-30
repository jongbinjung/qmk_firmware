# VI purity and Fn2 mouse layer — design

Keymap: `keyboards/keychron/q4/ansi/keymaps/jongbinjung/`. Board: Keychron Q4 ANSI v2.

## Goals

1. Make the VI layer a faithful vi normal-mode emulation: motions, operators,
   insert entries, visual mode, Ctrl chords, and repeat. Remove every binding
   that is not vi.
2. Give Fn2 (SHR_FN) a purpose. It becomes the keyboard-and-mouse layer: RGB,
   NKRO and bootloader access on the left hand, mouse keys on the right. Fn1
   (MAC_FN, WIN_FN) loses the RGB block and becomes a pure OS layer.

## Assumptions

- All shortcuts use Ctrl. The host remaps Ctrl to Cmd on macOS, so no
  OS-aware branching (this replaces the earlier "VI_WORD OS-awareness"
  follow-up, which is now closed as not needed).
- Mouse keys are already compiled in board-wide (`keyboards/keychron/q4/info.json`
  has `"mousekey": true`), so the Fn2 change adds no features.
- Repeat Key is a new feature (`REPEAT_KEY_ENABLE = yes`), expected under 1 KB.

## Part 1: VI layer

### Mental model

- **Normal mode**: VI layer active (held via Space, or locked via Space+Esc).
- **Insert mode**: VI layer off. Entered by i, a, I, A, o, O. Each unlocks VI
  (`layer_lock_off(VI)`), which is a no-op while VI is merely held.
- **Visual mode**: a firmware flag `vi_visual`, only meaningful while VI is
  active. Entered by v (charwise) or V (linewise). While on, cursor motions are
  sent with Shift so the host extends a selection. Left by an operator
  (y, d, x, p, u, U, Ctrl+r), by v again, by Esc tap, or automatically when the
  VI layer turns off.

### Key map

Blank cells are dead (`KC_NO`) in normal mode. "same" means the shifted
variant equals the plain one. Ctrl means the Esc key held (it is `LCTL_T`).

| Key | plain | Shift | Ctrl | kind |
|---|---|---|---|---|
| h j k l | arrows (plain `KC_*` in keymap) | host selection | host shortcut | motion |
| 0 | Home (`KC_HOME` in keymap) | | | motion |
| 4 | | `$` → End | | motion |
| 6 | | `^` → Home | | motion |
| w | Ctrl+Right | same | | motion |
| e | Ctrl+Right | same | | motion |
| b | Ctrl+Left | same | PgUp | motion |
| f | | | PgDn | motion |
| d | cut Ctrl+X | same | PgDn | operator (in visual, cuts the selection) |
| u | undo Ctrl+Z | redo Ctrl+Shift+Z | PgUp | operator |
| r | | | redo Ctrl+Shift+Z | operator |
| g | Ctrl+Home | Ctrl+End | | motion |
| x | Delete | Backspace | | operator |
| y | copy Ctrl+C | same | | operator |
| p | paste Ctrl+V | same | | operator |
| / | find Ctrl+F (`C(KC_F)` in keymap) | | | action |
| n | find next Ctrl+G | find previous Ctrl+Shift+G | | action |
| . | Repeat Key (`QK_REP` in keymap) | | | action |
| i | enter insert | Home, enter insert | | insert |
| a | Right, enter insert | End, enter insert | | insert |
| o | End, Enter, enter insert | Home, Enter, Up, enter insert | | insert |
| v | toggle visual | Home, visual on, Shift+End (select line) | | special |
| Esc tap | lock VI (existing); **in visual: leave visual instead** | | | special |
| Enter, Space, modifiers, Fn keys | unchanged | | | |

Kinds:

- **motion**: pressed with `register_code16`, released with `unregister_code16`,
  so the host auto-repeats held hjkl/w/b. In visual mode the registered code
  carries `S()`. The code actually registered is remembered per key so a
  visual toggle mid-hold cannot leave a key stuck.
- **operator**: tapped once, then visual mode ends.
- **action**: tapped once, visual mode untouched, never shifted by visual.
- **insert**: tapped once (if anything to send), visual ends, `layer_lock_off(VI)`.
- Ctrl variants are always single taps.

All taps and the Ctrl/Shift variants are sent with the current modifiers
cleared and then restored, so Ctrl+b produces a bare PgUp, not Ctrl+PgUp, and
X produces a bare Backspace. Plain motions keep physical modifiers, so
Shift+l is still a host selection and Ctrl+l (Esc held) is still the host's
Ctrl+Right. Physical Shift is read from `get_mods()`; Space Cadet registers the
shift modifier on press and the user hook runs before Space Cadet
(`quantum/quantum.c`, `process_record_kb` precedes `process_space_cadet`), so
this is reliable.

Removed from the VI layer, all non-vi:

- media on A, S, F;
- the duplicate nav cluster: U = PgUp, O = End, `[` = PgDn, `]` = Up, N = PgDn;
- Backspace key = Delete and backslash key = Backspace (both revert to
  transparent, i.e. their base meaning);
- transparent letters Q, E, R, T, Y, G, V, `;`, `'` that typed text in normal
  mode. Every letter without a vi meaning is now dead: q, s, t, z, c, m, `;`,
  `'`, `,`, `[`, `]`.

Kept although not strictly vi: the number row 1 2 3 5 7 8 9 - = still type
(counts are unsupported; typing them is harmless). 4 and 6 are dead when
unshifted so that `$` and `^` have a home.

### Implementation shape (keymap.c)

- New custom keycodes replace `VI_WORD`, `VI_BACK`, `VI_INS`:
  `VI_W, VI_E, VI_B, VI_F, VI_D, VI_U, VI_R, VI_G, VI_X, VI_Y, VI_P, VI_N,
  VI_I, VI_A, VI_O, VI_V, VI_4, VI_6`.
- A `PROGMEM` table `vi_keys[]` of `{keycode, plain, shift, ctrl, kind}`
  (16-bit keycodes with `C()`/`S()` wrappers; 0 = send nothing) drives a single
  `process_vi_key()` dispatcher. `VI_O` and `VI_V` (multi-step) are handled in
  a short switch before the table.
- Visual-mode shift for the plain arrow and Home keys on the VI layer is
  applied by intercepting `KC_LEFT..KC_RGHT` and `KC_HOME` in
  `process_record_user` when `layer_state_is(VI) && vi_visual`.
- `layer_state_set_user` clears `vi_visual` whenever VI leaves the state
  (space released, I/a/o unlock, idle timeout).
- The existing `LCTL_T(KC_ESC)` tap case gains: if `vi_visual`, clear it and
  return (no lock change).
- `rules.mk`: add `REPEAT_KEY_ENABLE = yes`.

### Indicators

- Colour categories gain **magenta** for the edit family: `VI_X` stays red
  (destructive) but `VI_U, VI_R, VI_Y, VI_P, VI_D, VI_V, QK_REP` are magenta.
  `VI_W, VI_E, VI_B, VI_F, VI_G, VI_4, VI_6` are blue (jumps), `VI_D` is magenta,
  `VI_N` and `C(KC_F)` are white, `VI_I, VI_A, VI_O` are orange (they leave
  the layer, like `VI_INS` today).
- **Visual mode cue**: while `vi_visual` is on, every VI key the generic pass
  painted green or blue (the motions) is repainted magenta. Runtime only; the
  checker cannot see it.
- `tools/check_indicators.py` mirrors the new keycode colours and gains
  assertions: VI `4` and `6` blue, VI `.` magenta, VI `u` magenta, VI `a`
  orange, VI `s` dark, VI Backspace pass-through.

## Part 2: Fn2 becomes keyboard + mouse; Fn1 becomes OS-only

### SHR_FN (Fn2) after

```
 ~    F1  F2  F3  F4  F5  F6  F7  F8  F9  F10 F11 F12  ___
RGBt  Nxt V+  H+  S+  Sp+ ___ WhU B1  B3  B2  ___ ___  ___
___   Prv V-  H-  S-  Sp- MsL MsD MsU MsR ___ ___      ___
___       ___ ___ ___ ___ NKRO WhD A0  A1  A2 ___      ___
___  ___  ___             LOCK            ___ SYS ___  ___
```

- Left hand unchanged: `RM_TOGG` on Tab, `RM_NEXT RM_VALU RM_HUEU RM_SATU RM_SPDU`
  on Q–T, `RM_PREV RM_VALD RM_HUED RM_SATD RM_SPDD` on A–G.
- `NK_TOGG` moves here, onto B.
- Right hand mouse: `MS_LEFT MS_DOWN MS_UP MS_RGHT` on H J K L; `MS_WHLU` on U,
  `MS_WHLD` on N; `MS_BTN1` (left) on I, `MS_BTN3` (middle) on O, `MS_BTN2`
  (right) on P; `MS_ACL0 MS_ACL1 MS_ACL2` on M , . (hold for slow / medium /
  fast). Default accelerated mouse-key mode, no tuning in this change.
- F1–F12 and `~` stay. They are duplicates of FNPAD and HHKB_FN, but the row
  is otherwise unused and removing them changes nothing else. (Veto here if
  you want the row dark instead.)
- Space `QK_LLCK` and Fn1 → `MO(SYS_FN)` unchanged.

### MAC_FN / WIN_FN (Fn1) after

Remove `RM_TOGG` (Tab), `RM_NEXT`/`RM_VALU`/… on Q–T and A–G, `RM_VALD`/`RM_VALU`
on 5/6, `RM_NEXT` on backslash, and `NK_TOGG` on N. All become transparent
(5, 6 become `KC_NO` like 3 and 4). Everything else stays: brightness, media,
mute/volume on the number row, Ins/PgUp/Home/PgDn/End, arrows, Del,
Windows extras (App, ScrLk, PrtSc), Space lock, Fn2 → `MO(SYS_FN)`.

The `RM_TOGG`-while-locked guard in `process_record_user` still applies (Fn2
locked + Tab).

### Indicators

- New colour **cyan** for mouse keys (`MS_UP … MS_ACL2`, i.e. the
  `QK_MOUSE_CURSOR_UP … QK_MOUSE_ACCELERATION_2` range).
- Checker: MAC_FN Tab becomes pass-through (dark) instead of purple; replace
  that assertion with "SHR_FN Tab purple" and add "SHR_FN J cyan", "SHR_FN B
  white", "MAC_FN Q dark".

## Readme

- VI diagram redrawn; VI guideline section rewritten around normal / insert /
  visual, with the Ctrl chords and `.`.
- MAC_FN, WIN_FN, SHR_FN diagrams redrawn; SHR_FN title becomes
  "SHR_FN: hold Fn2 (Space locks) — keyboard settings + mouse".
- Lighting legend adds magenta (edit / visual) and cyan (mouse), and notes the
  visual-mode recolour.
- Layer overview table: SHR_FN description updated; MAC_FN/WIN_FN say "OS
  media and navigation".

## Verification

- `qmk compile` for `keychron/q4/ansi/v1` and `v2`, clean, no warnings.
- Checker passes with the new assertions.
- On device (user): v + jjj + y copies lines; d in visual cuts; Esc leaves
  visual; o opens a line and drops to insert; Fn2 + hjkl moves the pointer.

## Out of scope

- Counts, text objects, dot-repeat of full edits, registers, marks, `;` `,`
  `f`/`t` motions, `%`, `*`.
- Mouse-key speed tuning (kinetic mode) — try defaults first.
- Anything on the freed Fn1 Q–T / A–G keys.
