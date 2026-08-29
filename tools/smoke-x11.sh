#!/usr/bin/env bash
#
# tools/smoke-x11.sh - headless X11 smoke test for the portable Linux bundle.
#
# Boots a private Xvfb server + openbox, launches the bundle's VoiceTyper
# launcher on the fake display and verifies the X11 runtime stack end to end:
#   - the bundled ld-linux launcher + $ORIGIN .so closure load the app
#   - SDL2 creates the app window on the fake display
#   - EWMH _NET_ACTIVE_WINDOW resolves (foreground-window detection path)
#   - XTest fake keys reach a focused window (text-injection path)
#
# Usage: tools/smoke-x11.sh [bundle-dir]
#   bundle-dir  portable bundle directory (default: nix build .#portable-x11).

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR" || exit 1

usage() {
	cat <<EOF
Usage: tools/smoke-x11.sh [bundle-dir]

Headless X11 smoke test for a portable Linux bundle. Starts a private Xvfb
display + openbox, launches the bundle's VoiceTyper and verifies the SDL
window, the EWMH active-window and the XTest injection paths. Exits non-zero
on failure.

  bundle-dir  portable bundle directory (default: nix build .#portable-x11).
  -h|--help   Show this help.
EOF
}

WORK="$(mktemp -d)"
FAILED=1
XVFB_PID=""
OPENBOX_PID=""
APP_PID=""
XEV_PID=""

cleanup() {
	for pid in "$XEV_PID" "$APP_PID" "$OPENBOX_PID" "$XVFB_PID"; do
		if [ -n "$pid" ]; then
			kill "$pid" 2>/dev/null || true
		fi
	done
	if [ "$FAILED" -eq 0 ]; then
		rm -rf "$WORK"
	else
		echo "Logs kept at: $WORK" >&2
	fi
}
trap cleanup EXIT

die() {
	echo "Error: $*" >&2
	exit 1
}

BUNDLE=""
while [ "$#" -gt 0 ]; do
	case "$1" in
		-h|--help)
			usage
			exit 0
			;;
		-*)
			die "Unknown option '$1'."
			;;
		*)
			[ -z "$BUNDLE" ] || die "Unexpected extra argument '$1'."
			BUNDLE="$1"
			;;
	esac
	shift
done

mapfile -t TOOLS < <(nix build --no-link --print-out-paths \
	nixpkgs#xorg.xorgserver nixpkgs#openbox nixpkgs#xdotool nixpkgs#xorg.xev)
[ "${#TOOLS[@]}" -eq 4 ] || die "expected 4 tool paths from nix build."
XVFB="${TOOLS[0]}/bin/Xvfb"
OPENBOX="${TOOLS[1]}/bin/openbox"
XDOTOOL="${TOOLS[2]}/bin/xdotool"
XEV="${TOOLS[3]}/bin/xev"

if [ -z "$BUNDLE" ]; then
	echo "[smoke] building .#portable-x11 ..."
	BUNDLE="$(nix build --no-link --print-out-paths .#portable-x11)"
fi
echo "[smoke] bundle: $BUNDLE"

for f in VoiceTyper VoiceTyper.elf ld-linux-x86-64.so.2 libX11.so.6 libXtst.so.6; do
	[ -f "$BUNDLE/$f" ] || die "bundle is missing '$f'."
done

DISPLAY_NUM=""
for n in $(seq 99 999); do
	if [ ! -e "/tmp/.X11-unix/X$n" ]; then
		DISPLAY_NUM="$n"
		break
	fi
done
[ -n "$DISPLAY_NUM" ] || die "no free display number in 99-999."
DISPLAY=":$DISPLAY_NUM"
export DISPLAY

"$XVFB" ":$DISPLAY_NUM" -screen 0 1280x800x24 -nolisten tcp >"$WORK/xvfb.log" 2>&1 &
XVFB_PID=$!
XVFB_UP=0
for _ in $(seq 1 50); do
	if "$XDOTOOL" getdisplaygeometry >/dev/null 2>&1; then
		XVFB_UP=1
		break
	fi
	sleep 0.2
done
[ "$XVFB_UP" -eq 1 ] || die "Xvfb did not come up on $DISPLAY."
echo "[smoke] Xvfb up on $DISPLAY"

"$OPENBOX" >"$WORK/openbox.log" 2>&1 &
OPENBOX_PID=$!

SDL_AUDIODRIVER=dummy "$BUNDLE/VoiceTyper" >"$WORK/app.log" 2>&1 &
APP_PID=$!
APP_WIN=""
for _ in $(seq 1 75); do
	kill -0 "$APP_PID" 2>/dev/null || die "app exited before opening a window (see $WORK/app.log)."
	APP_WIN="$("$XDOTOOL" search --name VoiceTyper 2>/dev/null | head -n1 || true)"
	[ -n "$APP_WIN" ] && break
	sleep 0.2
done
[ -n "$APP_WIN" ] || die "app window never appeared on $DISPLAY."
echo "[smoke] app window up (id $APP_WIN)"

ACTIVE=""
for _ in $(seq 1 50); do
	if timeout 5 "$XDOTOOL" windowactivate --sync "$APP_WIN" >/dev/null 2>&1; then
		ACTIVE="$("$XDOTOOL" getactivewindow 2>/dev/null || true)"
		if [ "$ACTIVE" = "$APP_WIN" ]; then
			break
		fi
		ACTIVE=""
	fi
	sleep 0.2
done
[ -n "$ACTIVE" ] || die "EWMH activation never landed on the app window (see $WORK/openbox.log)."
echo "[smoke] EWMH active window is the app window (id $APP_WIN)"

"$XEV" >"$WORK/xev.log" 2>&1 &
XEV_PID=$!
XEV_WIN=""
for _ in $(seq 1 50); do
	kill -0 "$XEV_PID" 2>/dev/null || die "xev exited before mapping its window."
	XEV_WIN="$("$XDOTOOL" search --name "Event tester" 2>/dev/null | head -n1 || true)"
	[ -n "$XEV_WIN" ] && break
	sleep 0.2
done
[ -n "$XEV_WIN" ] || die "xev window never appeared on $DISPLAY."
"$XDOTOOL" windowactivate --sync "$XEV_WIN"
"$XDOTOOL" key h i
sleep 1
grep -a -q "keysym 0x68, h" "$WORK/xev.log" || die "xev did not receive the fake 'h' keypress."
grep -a -q "keysym 0x69, i" "$WORK/xev.log" || die "xev did not receive the fake 'i' keypress."
echo "[smoke] XTest fake keys delivered to the focused window"

kill -0 "$APP_PID" 2>/dev/null || die "app crashed during the smoke run (see $WORK/app.log)."
kill "$APP_PID" 2>/dev/null || true
APP_DOWN=0
for _ in $(seq 1 25); do
	if ! kill -0 "$APP_PID" 2>/dev/null; then
		APP_DOWN=1
		break
	fi
	sleep 0.2
done
if [ "$APP_DOWN" -ne 1 ]; then
	kill -9 "$APP_PID" 2>/dev/null || true
	echo "[smoke] note: app ignored SIGTERM, killed with SIGKILL"
fi

FAILED=0
echo "[smoke] PASS"
