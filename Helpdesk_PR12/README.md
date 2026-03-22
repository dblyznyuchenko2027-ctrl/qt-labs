# Helpdesk — Practical Work #12

Theme: Model/View, QAbstractTableModel, begin/end signals, dataChanged, sync with dialog.

## What is new in PR12

### TicketTableModel (renamed and fully documented)
Replaces TicketModel with proper PR12 class name and expanded implementation.

Required QAbstractTableModel overrides:
  rowCount()    - returns 0 when parent.isValid() (no tree nesting)
  columnCount() - returns COL_COUNT (5 columns)
  data()        - handles DisplayRole, ForegroundRole, TextAlignmentRole, UserRole
  headerData()  - horizontal column names + vertical row numbers

CRUD methods with correct Model/View signals:
  addTicket(t)         - beginInsertRows / append / endInsertRows
  updateTicket(row, t) - mutate data / emit dataChanged(topLeft, bottomRight)
  removeTicket(row)    - beginRemoveRows / removeAt / endRemoveRows

### Columns
  COL_ID       ResizeToContents  centred
  COL_TITLE    Stretch           fills remaining space
  COL_PRIORITY ResizeToContents  colour: Critical=red, High=orange, Low=gray
  COL_STATUS   ResizeToContents  colour: Open=darkRed, InProgress=blue, Resolved=green
  COL_CREATED  ResizeToContents  format: yyyy-MM-dd hh:mm

### MainWindow changes
  - Uses TicketTableModel instead of TicketModel
  - selectionModel() connected after setModel() (correct order)
  - selectedSourceRow() maps proxy index -> source row
  - onTicketSaved(): addTicket or updateTicket depending on m_editingRow
  - onActionDelete(): removeTicket with confirmation dialog

### Responsibility boundary (PR12 key concept)
  Model: stores data, emits signals, knows nothing about UI
  View:  QTableView displays data, manages selection
  MainWindow: connects model to view, opens dialogs, maps selection to CRUD

## Build

  cmake -S . -B build
  cmake --build build
  ./build/Helpdesk

Requires: Qt 6.x, CMake >= 3.16, C++17.
