# TypingTrainer — Practical Work #9

**Theme:** Keyboard events, eventFilter, rich-text highlighting, virtual keyboard.

## What is new in PR9

### Keyboard input
- eventFilter installed on pageTraining — intercepts QKeyEvent regardless of focus
- m_trainingActive flag — input only processed during active session
- handleKeyPress(): e->text() for chars, Qt::Key_Backspace, Qt::Key_Return/Enter handled
- All modifier/navigation keys ignored safely (no crash, no model change)

### TextModel additions
- goBack() — moves cursor back one char; stops at text beginning
- markError() / hasError(line, ch) — tracks incorrectly typed positions per cell
- clearErrors() — called on every resetPosition()

### Rich-text highlighting (QLabel HTML)
buildLineHtml() produces HTML spans:
  - #cfe8ff blue   = correctly typed chars
  - #ffb3b3 red    = error positions
  - #ffe08a yellow = current char (target)
  - plain          = upcoming text
  - previous line shown dimmed/blue as context

### Virtual keyboard highlight
findKeyButton(ch, key) searches by objectName first, then by enum, then by text scan.
highlightVirtualKey() / clearVirtualKeyHighlight() paint/restore one key per press.

### Removed
Step test button from PR7 is removed — real keyboard replaces it.

## Build

  cmake -S . -B build
  cmake --build build
  cp -r lessons/ build/
  ./build/TypingTrainer

Requires: Qt 6.x, CMake >= 3.16, C++17.
