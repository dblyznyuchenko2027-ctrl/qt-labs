# TypingTrainer — Practical Work #10

Theme: QTimer, QElapsedTimer, CPM/WPM metrics, accuracy, QSettings.

## What is new in PR10

### Timer & elapsed time
- QTimer (1 s interval) starts on Start/Restart, stops on Return/finish/exit
- QElapsedTimer measures exact wall-clock duration (monotonic, no drift)
- onTimerTick() fires every second and calls updateMetricsDisplay()

### Session statistics (SessionStats struct)
- totalKeystrokes: every printable key press
- correctChars:    accepted correct chars
- errors:          wrong key presses
- accuracy():      100 * correctChars / totalKeystrokes  (100 % when nothing typed)
- cpm(ms):         correctChars * 60000 / elapsedMs      (0 for first 500 ms)
- wpm(ms):         cpm / 5  (classic 5-chars-per-word rule)

### Speed mode toggle
- Settings menu: "Switch CPM / WPM"
- Updates lblSpeedValue label and Results page in real time

### Session lifecycle
- Start:   resetPosition + resetStats + startTimer + go to Training page
- Restart: same, but stay on Training page
- Return:  stopTimer + go to Start page (timer never ticks in background)
- Finish:  stopTimer + copy results to Results page + go to Results page

### QSettings (persist across restarts)
Keys stored:
  session/lastLessonPath  — full path of last selected lesson file
  session/speedMode       — "CPM" or "WPM"

On launch: loadSettings() restores speed mode; scanLessons() restores lesson selection.
On exit/start: saveSettings() writes current values.
Edge case: saved lesson file deleted -> falls back to first available lesson silently.

## Build

  cmake -S . -B build
  cmake --build build
  cp -r lessons/ build/
  ./build/TypingTrainer

Requires: Qt 6.x, CMake >= 3.16, C++17.
