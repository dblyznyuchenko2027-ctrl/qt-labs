#include "MainWindow.h"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QFileDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QTextStream>
#include <QToolBar>
#include <QFile>
#include <QFileInfo>

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupEditor();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupConnections();

    updateWindowTitle();
    updateActionStates();
    resize(800, 600);

    // Keyboard-first: initial focus goes to the editor
    m_editor->setFocus();
}

// ─────────────────────────────────────────────
//  Setup helpers
// ─────────────────────────────────────────────
void MainWindow::setupEditor()
{
    m_editor = new QPlainTextEdit(this);
    m_editor->setObjectName("editor");

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(11);
    m_editor->setFont(font);

    setCentralWidget(m_editor);
}

void MainWindow::setupMenuBar()
{
    // ── File ──────────────────────────────────
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    m_actNew = new QAction(QIcon::fromTheme("document-new",
                           QIcon(":/icons/new.png")), tr("&New"), this);
    m_actNew->setShortcut(QKeySequence::New);
    m_actNew->setStatusTip(tr("Create a new document"));
    fileMenu->addAction(m_actNew);

    m_actOpen = new QAction(QIcon::fromTheme("document-open",
                            QIcon(":/icons/open.png")), tr("&Open…"), this);
    m_actOpen->setShortcut(QKeySequence::Open);
    m_actOpen->setStatusTip(tr("Open an existing file"));
    fileMenu->addAction(m_actOpen);

    fileMenu->addSeparator();

    m_actSave = new QAction(QIcon::fromTheme("document-save",
                            QIcon(":/icons/save.png")), tr("&Save"), this);
    m_actSave->setShortcut(QKeySequence::Save);
    m_actSave->setStatusTip(tr("Save the current file"));
    fileMenu->addAction(m_actSave);

    m_actSaveAs = new QAction(QIcon::fromTheme("document-save-as",
                              QIcon(":/icons/saveas.png")), tr("Save &As…"), this);
    m_actSaveAs->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
    m_actSaveAs->setStatusTip(tr("Save the file under a new name"));
    fileMenu->addAction(m_actSaveAs);

    fileMenu->addSeparator();

    m_actExit = new QAction(QIcon::fromTheme("application-exit",
                            QIcon(":/icons/exit.png")), tr("E&xit"), this);
    m_actExit->setShortcut(QKeySequence::Quit);   // Ctrl+Q on most platforms
    m_actExit->setStatusTip(tr("Exit the application"));
    fileMenu->addAction(m_actExit);

    // ── Edit ──────────────────────────────────
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    m_actUndo = new QAction(QIcon::fromTheme("edit-undo",
                            QIcon(":/icons/undo.png")), tr("&Undo"), this);
    m_actUndo->setShortcut(QKeySequence::Undo);
    m_actUndo->setStatusTip(tr("Undo the last action"));
    editMenu->addAction(m_actUndo);

    m_actRedo = new QAction(QIcon::fromTheme("edit-redo",
                            QIcon(":/icons/redo.png")), tr("&Redo"), this);
    m_actRedo->setShortcut(QKeySequence::Redo);
    m_actRedo->setStatusTip(tr("Redo the last undone action"));
    editMenu->addAction(m_actRedo);

    editMenu->addSeparator();

    m_actCut = new QAction(QIcon::fromTheme("edit-cut",
                           QIcon(":/icons/cut.png")), tr("Cu&t"), this);
    m_actCut->setShortcut(QKeySequence::Cut);
    m_actCut->setStatusTip(tr("Cut selected text to clipboard"));
    editMenu->addAction(m_actCut);

    m_actCopy = new QAction(QIcon::fromTheme("edit-copy",
                            QIcon(":/icons/copy.png")), tr("&Copy"), this);
    m_actCopy->setShortcut(QKeySequence::Copy);
    m_actCopy->setStatusTip(tr("Copy selected text to clipboard"));
    editMenu->addAction(m_actCopy);

    m_actPaste = new QAction(QIcon::fromTheme("edit-paste",
                             QIcon(":/icons/paste.png")), tr("&Paste"), this);
    m_actPaste->setShortcut(QKeySequence::Paste);
    m_actPaste->setStatusTip(tr("Paste text from clipboard"));
    editMenu->addAction(m_actPaste);

    editMenu->addSeparator();

    m_actSelectAll = new QAction(QIcon::fromTheme("edit-select-all"),
                                 tr("Select &All"), this);
    m_actSelectAll->setShortcut(QKeySequence::SelectAll);
    m_actSelectAll->setStatusTip(tr("Select all text"));
    editMenu->addAction(m_actSelectAll);

    // ── Help ──────────────────────────────────
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    m_actAbout = new QAction(QIcon::fromTheme("help-about"),
                             tr("&About QuickPad"), this);
    m_actAbout->setStatusTip(tr("Show information about QuickPad"));
    helpMenu->addAction(m_actAbout);
}

void MainWindow::setupToolBar()
{
    QToolBar *tb = addToolBar(tr("Main"));
    tb->setObjectName("mainToolBar");
    tb->setToolButtonStyle(Qt::ToolButtonIconOnly);

    tb->addAction(m_actNew);
    tb->addAction(m_actOpen);
    tb->addAction(m_actSave);
    tb->addAction(m_actSaveAs);
    tb->addSeparator();
    tb->addAction(m_actCut);
    tb->addAction(m_actCopy);
    tb->addAction(m_actPaste);
    tb->addAction(m_actUndo);
    tb->addAction(m_actRedo);
    tb->addSeparator();
    tb->addAction(m_actAbout);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel(tr("Ready"), this);
    m_statusLabel->setMinimumWidth(140);

    m_positionLabel = new QLabel(tr("Line 1, Col 1"), this);
    m_positionLabel->setMinimumWidth(120);
    m_positionLabel->setAlignment(Qt::AlignCenter);

    m_modifiedLabel = new QLabel(this);
    m_modifiedLabel->setMinimumWidth(70);
    m_modifiedLabel->setAlignment(Qt::AlignCenter);

    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_positionLabel);
    statusBar()->addPermanentWidget(m_modifiedLabel);
}

void MainWindow::setupConnections()
{
    // File actions
    connect(m_actNew,    &QAction::triggered, this, &MainWindow::onNew);
    connect(m_actOpen,   &QAction::triggered, this, &MainWindow::onOpen);
    connect(m_actSave,   &QAction::triggered, this, &MainWindow::onSave);
    connect(m_actSaveAs, &QAction::triggered, this, &MainWindow::onSaveAs);
    connect(m_actExit,   &QAction::triggered, this, &MainWindow::onExit);

    // Edit actions
    connect(m_actUndo,      &QAction::triggered, this, &MainWindow::onUndo);
    connect(m_actRedo,      &QAction::triggered, this, &MainWindow::onRedo);
    connect(m_actCut,       &QAction::triggered, this, &MainWindow::onCut);
    connect(m_actCopy,      &QAction::triggered, this, &MainWindow::onCopy);
    connect(m_actPaste,     &QAction::triggered, this, &MainWindow::onPaste);
    connect(m_actSelectAll, &QAction::triggered, this, &MainWindow::onSelectAll);

    // Help
    connect(m_actAbout, &QAction::triggered, this, &MainWindow::onAbout);

    // Editor state
    connect(m_editor, &QPlainTextEdit::textChanged,
            this, &MainWindow::onTextChanged);
    connect(m_editor, &QPlainTextEdit::copyAvailable,
            this, &MainWindow::onCopyAvailable);

    // Cursor position → status bar
    connect(m_editor, &QPlainTextEdit::cursorPositionChanged, this, [this]() {
        QTextCursor c = m_editor->textCursor();
        int line = c.blockNumber() + 1;
        int col  = c.columnNumber() + 1;
        m_positionLabel->setText(tr("Line %1, Col %2").arg(line).arg(col));
    });

    // Undo / Redo availability
    connect(m_editor, &QPlainTextEdit::undoAvailable,
            m_actUndo, &QAction::setEnabled);
    connect(m_editor, &QPlainTextEdit::redoAvailable,
            m_actRedo, &QAction::setEnabled);

    // Clipboard
    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            this, &MainWindow::onClipboardDataChanged);
}

// ─────────────────────────────────────────────
//  closeEvent — handles the window's X button
// ─────────────────────────────────────────────
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

// ─────────────────────────────────────────────
//  File slots
// ─────────────────────────────────────────────
void MainWindow::onNew()
{
    if (!maybeSave())
        return;

    {
        QSignalBlocker blocker(m_editor);   // don't fire textChanged during clear
        m_editor->clear();
    }

    m_currentFilePath.clear();
    setModified(false);
    updateWindowTitle();
    m_statusLabel->setText(tr("New document"));
    m_editor->setFocus();
}

void MainWindow::onOpen()
{
    if (!maybeSave())
        return;

    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        QString(),
        tr("Text Files (*.txt *.md *.cpp *.h *.py);;All Files (*)"));

    if (path.isEmpty()) {
        m_statusLabel->setText(tr("Open cancelled"));
        m_editor->setFocus();
        return;
    }

    if (loadFile(path)) {
        m_statusLabel->setText(tr("Opened: %1").arg(QFileInfo(path).fileName()));
    }
    m_editor->setFocus();
}

void MainWindow::onSave()
{
    if (m_currentFilePath.isEmpty()) {
        onSaveAs();
        return;
    }
    if (saveFile(m_currentFilePath)) {
        m_statusLabel->setText(tr("Saved: %1").arg(QFileInfo(m_currentFilePath).fileName()));
    }
    m_editor->setFocus();
}

void MainWindow::onSaveAs()
{
    const QString path = QFileDialog::getSaveFileName(
        this,
        tr("Save File As"),
        m_currentFilePath.isEmpty()
            ? QStringLiteral("Untitled.txt")
            : m_currentFilePath,
        tr("Text Files (*.txt *.md *.cpp *.h *.py);;All Files (*)"));

    if (path.isEmpty()) {
        m_statusLabel->setText(tr("Save cancelled"));
        m_editor->setFocus();
        return;
    }

    if (saveFile(path)) {
        m_currentFilePath = path;
        updateWindowTitle();
        m_statusLabel->setText(tr("Saved as: %1").arg(QFileInfo(path).fileName()));
    }
    m_editor->setFocus();
}

void MainWindow::onExit()
{
    close();   // triggers closeEvent → maybeSave
}

// ─────────────────────────────────────────────
//  Edit slots
// ─────────────────────────────────────────────
void MainWindow::onCut()        { m_editor->cut();       m_editor->setFocus(); }
void MainWindow::onCopy()       { m_editor->copy();      m_editor->setFocus(); }
void MainWindow::onPaste()      { m_editor->paste();     m_editor->setFocus(); }
void MainWindow::onSelectAll()  { m_editor->selectAll(); m_editor->setFocus(); }
void MainWindow::onUndo()       { m_editor->undo();      m_editor->setFocus(); }
void MainWindow::onRedo()       { m_editor->redo();      m_editor->setFocus(); }

// ─────────────────────────────────────────────
//  Help slot
// ─────────────────────────────────────────────
void MainWindow::onAbout()
{
    QMessageBox::about(this,
        tr("About QuickPad"),
        tr("<b>QuickPad</b> v1.0<br><br>"
           "A simple text editor built with Qt Widgets.<br>"
           "Practical work #5 — Qt Widgets / QAction / File I/O / Keyboard-first UX"));
    m_editor->setFocus();
}

// ─────────────────────────────────────────────
//  State update slots
// ─────────────────────────────────────────────
void MainWindow::onTextChanged()
{
    if (!m_modified) {
        setModified(true);
    }
    updateActionStates();
}

void MainWindow::onCopyAvailable(bool available)
{
    m_actCut->setEnabled(available);
    m_actCopy->setEnabled(available);
}

void MainWindow::onClipboardDataChanged()
{
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    m_actPaste->setEnabled(mime && mime->hasText());
}

// ─────────────────────────────────────────────
//  File helpers
// ─────────────────────────────────────────────
bool MainWindow::loadFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Open Error"),
            tr("Cannot open file:\n%1\n\n%2").arg(filePath, file.errorString()));
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString content = in.readAll();

    {
        QSignalBlocker blocker(m_editor);
        m_editor->setPlainText(content);
    }

    m_currentFilePath = filePath;
    setModified(false);
    updateWindowTitle();
    return true;
}

bool MainWindow::saveFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Save Error"),
            tr("Cannot save file:\n%1\n\n%2").arg(filePath, file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << m_editor->toPlainText();

    setModified(false);
    return true;
}

// ─────────────────────────────────────────────
//  Dirty-state / confirmation
// ─────────────────────────────────────────────
bool MainWindow::maybeSave()
{
    if (!m_modified)
        return true;

    const QString docName = m_currentFilePath.isEmpty()
        ? tr("Untitled")
        : QFileInfo(m_currentFilePath).fileName();

    QMessageBox dlg(this);
    dlg.setWindowTitle(tr("Unsaved Changes"));
    dlg.setText(tr("The document <b>%1</b> has unsaved changes.").arg(docName));
    dlg.setInformativeText(tr("Do you want to save before continuing?"));
    dlg.setIcon(QMessageBox::Warning);

    QPushButton *saveBtn   = dlg.addButton(tr("Save"),        QMessageBox::AcceptRole);
    QPushButton *discardBtn= dlg.addButton(tr("Don't Save"),  QMessageBox::DestructiveRole);
    QPushButton *cancelBtn = dlg.addButton(QMessageBox::Cancel);
    Q_UNUSED(discardBtn)

    dlg.setDefaultButton(saveBtn);   // keyboard-first: Save is focused by default
    dlg.exec();

    if (dlg.clickedButton() == cancelBtn)
        return false;

    if (dlg.clickedButton() == saveBtn) {
        onSave();
        // If save was cancelled (e.g. Save As dialog cancelled), abort
        if (m_modified)
            return false;
    }

    return true;
}

void MainWindow::setModified(bool modified)
{
    m_modified = modified;
    m_modifiedLabel->setText(modified ? tr("Modified") : QString());
    updateWindowTitle();
    m_actSave->setEnabled(modified || m_currentFilePath.isEmpty() == false);
}

void MainWindow::updateWindowTitle()
{
    const QString docName = m_currentFilePath.isEmpty()
        ? tr("Untitled")
        : QFileInfo(m_currentFilePath).fileName();
    setWindowTitle(tr("%1%2 — QuickPad")
                   .arg(docName)
                   .arg(m_modified ? QStringLiteral("*") : QString()));
}

void MainWindow::updateActionStates()
{
    // Undo/Redo: driven by undoAvailable / redoAvailable signals — no manual update needed.

    // Cut / Copy: driven by copyAvailable — initial state when document is empty
    const bool hasSelection = m_editor->textCursor().hasSelection();
    m_actCut->setEnabled(hasSelection);
    m_actCopy->setEnabled(hasSelection);

    // Paste: driven by clipboard — set initial state here
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    m_actPaste->setEnabled(mime && mime->hasText());

    // Select All: meaningful only when there is text
    m_actSelectAll->setEnabled(!m_editor->document()->isEmpty());
}
