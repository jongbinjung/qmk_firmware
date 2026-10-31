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
MAPPED = {"MAC_FN", "WIN_FN", "SHR_FN", "SYS_FN", "NUMPAD", "FNPAD", "HHKB_FN", "VI"}

# c2json keeps aliases as written; the firmware compares numeric keycodes.
ALIASES = {"KC_GRV": "KC_GRAVE", "KC_RIGHT": "KC_RGHT", "KC_LOPT": "KC_LALT",
           "KC_ROPT": "KC_RALT", "KC_LCMD": "KC_LGUI", "KC_RCMD": "KC_RGUI",
           "QK_BOOT": "QK_BOOTLOADER", "QK_LAYER_LOCK": "QK_LLCK",
           "QK_REPEAT_KEY": "QK_REP"}

GREEN = {"KC_LEFT", "KC_DOWN", "KC_UP", "KC_RGHT", "KC_HOME", "KC_END",
         "VI_H", "VI_J", "VI_K", "VI_L", "VI_0"}
BLUE = {"KC_PGUP", "KC_PGDN", "VI_W", "VI_E", "VI_B", "VI_F", "VI_G", "VI_4", "VI_6"}
TEAL = {"KC_MPRV", "KC_MPLY", "KC_MNXT", "KC_MSTP", "KC_MUTE",
        "KC_VOLU", "KC_VOLD", "KC_BRIU", "KC_BRID"}
RED = {"KC_DEL", "QK_BOOTLOADER", "VI_X"}
MAGENTA = {"VI_D", "VI_U", "VI_R", "VI_Y", "VI_P", "VI_V", "QK_REP"}
AMBER = {"QK_LLCK", "VI_I", "VI_A", "VI_O"}
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
    if kc in MAGENTA:
        return "magenta"
    if kc in AMBER or re.match(r"^(MO|TG|LT)\(", kc):
        return "amber"
    if kc.startswith("RM_"):
        return "purple"
    if kc.startswith("MS_"):
        return "cyan"
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
    for name in ["MAC_FN", "WIN_FN", "SHR_FN", "SYS_FN", "NUMPAD", "FNPAD", "HHKB_FN", "VI"]:
        lit = lit_map(layers, name, "MAC_BASE")
        by_color = {}
        for key, c in lit.items():
            by_color.setdefault(c, []).append(key)
        print(f"{name}:")
        for c in ["green", "blue", "teal", "red", "magenta", "amber", "purple", "cyan", "white"]:
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
    shr = lit_map(layers, "SHR_FN", "MAC_BASE")
    macfn = lit_map(layers, "MAC_FN", "MAC_BASE")
    expect = [
        (vi.get("I") == "amber", "VI lights I amber (the unlock key)"),
        (not {"U", "W", "B"} & hhkb.keys(), "HHKB_FN leaves U W B dark"),
        ("N" not in fnpad, "FNPAD leaves N dark"),
        (lit_map(layers, "NUMPAD", "MAC_BASE").get("Q") == "blue"
         and lit_map(layers, "NUMPAD", "MAC_BASE").get("W") == "blue", "NUMPAD Q and W (page keys) are blue"),
        (sysfn == {"Fn3": "red"}, "SYS_FN lights only Fn3, red"),
        (vi.get("Esc") is None, "VI Esc/Ctrl is a pass-through (override paints it)"),
        (vi.get("4") == "blue" and vi.get("6") == "blue", "VI $ and ^ (4, 6) are blue"),
        (vi.get(".") == "magenta" and vi.get("U") == "magenta", "VI . and u are magenta"),
        (vi.get("A") == "amber", "VI a (append) is amber"),
        ("S" not in vi and "Bksp" not in vi, "VI leaves S and Backspace dark"),
        (shr.get("Tab") == "purple", "SHR_FN Tab (RGB toggle) is purple"),
        (shr.get("J") == "cyan" and shr.get("B") == "white", "SHR_FN J is cyan (mouse), B white (NKRO)"),
        ("Tab" not in macfn and macfn.get("Q") == "teal" and macfn.get("A") == "teal",
         "MAC_FN Tab dark (RGB moved off); Q and A teal (left-hand media)"),
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
