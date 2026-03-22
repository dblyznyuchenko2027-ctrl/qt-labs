# Helpdesk — Practical Work #11

Theme: QMainWindow, QTableView, non-modal dialogs, layouts, QSortFilterProxyModel.

## Features

- QMainWindow with MenuBar (File / Ticket / View / Help) and ToolBar
- QTableView backed by custom QAbstractTableModel (TicketModel)
  - Columns: ID | Title | Priority | Status | Created At
  - Colour-coded Status and Priority cells
  - Sortable columns, alternating row colours
- QSortFilterProxyModel for live text search
- Filter bar: Status combo + Priority combo + Search field + Clear button
- Status bar: "Total: N | Filtered: M"

## Non-modal TicketDialog (one instance, three modes)

| Mode | Fields | Buttons |
|------|--------|---------|
| View | read-only | Edit + Close |
| Edit | editable | Save + Cancel |
| New  | editable (empty) | Save + Cancel |

- `show()` — non-blocking, main window stays interactive
- Cancel in Edit → restores original values, returns to View mode
- Cancel in New → hides dialog
- Dialog is reused (not recreated each time)

## Keyboard shortcuts
- Ctrl+N   New ticket
- Ctrl+E   Edit selected
- Ctrl+Return  View selected
- Delete   Delete selected
- F5       Refresh
- Ctrl+Q   Exit

## Build

  cmake -S . -B build
  cmake --build build
  ./build/Helpdesk

Requires: Qt 6.x, CMake >= 3.16, C++17.

## File structure

  Helpdesk/
  ├── CMakeLists.txt
  ├── .gitignore
  ├── README.md
  ├── resources/resources.qrc
  └── src/
      ├── main.cpp
      ├── TicketModel.h / TicketModel.cpp   (data model + Ticket struct)
      ├── TicketDialog.h / TicketDialog.cpp (non-modal dialog, 3 modes)
      ├── TicketDialog.ui
      ├── MainWindow.h / MainWindow.cpp
      └── MainWindow.ui
