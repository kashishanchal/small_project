# DLP Clipboard Monitor (C++ comparator + shell monitor)

This variant uses a native C++ comparator (`compare_tool`) to compute normalized Levenshtein similarity against protected files and a small shell monitor (`monitor.sh`) that polls the clipboard and blocks pastes by clearing the clipboard and showing a notification.

Build

From the repository root:

```bash
cmake -S plugins/content_fingerpinting -B build/plugin -DCMAKE_BUILD_TYPE=Release
cmake --build build/plugin -j
```

Note: replace the source path if your CMakeLists is in a different location — the plugin CMake is at `plugins/content_fingerprinting/cpp`.

Run monitor

Install dependencies:

```bash
sudo apt update
sudo apt install xclip libnotify-bin
```

Then run the monitor (from repo root):

```bash
bash projects/dlp_clipboard_monitor/monitor.sh
```

Configuration
- Protected files directory: `~/Desktop/protected` by default. Put plain text files there.
- Thresholds: configured inside `monitor.sh` (default 0.70). `compare_tool` supports `--threshold`.

Notes
- This is a prototype. For production integrate with system clipboard hooks and a daemon/service.
