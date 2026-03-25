#!/usr/bin/env bash
# Simple clipboard monitor that calls the C++ compare_tool

PROTECTED_DIR="$HOME/Desktop/protected"
THRESHOLD=0.70
POLL_INTERVAL=0.7
COMPARE_BIN="$(pwd)/plugins/content_fingerprinting/cpp/compare_tool"

if [[ ! -x "$COMPARE_BIN" ]]; then
  echo "compare_tool not found or not executable: $COMPARE_BIN"
  echo "Build it first with: cmake -S plugins/content_fingerprinting -B build/plugin && cmake --build build/plugin -j"
  exit 2
fi

last_clipboard=""
while true; do
  clip=$(xclip -selection clipboard -o 2>/dev/null || true)
  if [[ -n "$clip" && "$clip" != "$last_clipboard" ]]; then
    # call compare_tool
    # use printf to pipe safely
    result=$(printf "%s" "$clip" | "$COMPARE_BIN" --protected-dir "$PROTECTED_DIR" --threshold "$THRESHOLD")
    # result format: MATCH 90 /path/to/file  OR NO_MATCH 12 /path
    if [[ $result =~ ^MATCH[[:space:]]([0-9]+) ]]; then
      pct=${BASH_REMATCH[1]}
      # notify and clear clipboard
      notify-send "DLP: paste blocked" "Content matches protected file ($pct% similar). Paste blocked."
      printf "" | xclip -selection clipboard
    elif [[ $result =~ ^NO_MATCH[[:space:]]([0-9]+) ]]; then
      pct=${BASH_REMATCH[1]}
      # optional: show warning if above some lower threshold
      if (( pct >= 70 )); then
        notify-send "DLP: similar content detected" "Content is ${pct}% similar to a protected file. Paste blocked." 
        printf "" | xclip -selection clipboard
      fi
    fi
    last_clipboard="$clip"
  fi
  sleep $POLL_INTERVAL
done
