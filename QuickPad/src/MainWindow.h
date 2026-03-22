#pragma once

#include <QMainWindow>
#include <QString>

class QPlainTextEdit;
class QAction;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // File menu
    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();
    void onExit();

    // Edit menu
    void onCut();
    void onCopy();
    void onPaste();
    void onSelectAll();
    void onUndo();
    void onRedo();

    // Help menu
    void onAbout();

    // State updates
    void onTextChanged();
    void onCopyAvailable(bool available);
    void onClipboardDataChanged();

private:
    // Setup
    void setupEditor();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();

    // File operations
    bool saveFile(const QString &filePath);
    bool loadFile(const QString &filePath);

    // Dirty state management
    bool maybeSave();           // returns false if user cancelled
    void setModified(bool modified);
    void updateWindowTitle();
    void updateActionStates();

    // Widgets
    QPlainTextEdit  *m_editor   = nullptr;

    // Status bar labels
    QLabel *m_statusLabel       = nullptr;
    QLabel *m_positionLabel     = nullptr;
    QLabel *m_modifiedLabel     = nullptr;

    // Actions — File
    QAction *m_actNew           = nullptr;
    QAction *m_actOpen          = nullptr;
    QAction *m_actSave          = nullptr;
    QAction *m_actSaveAs        = nullptr;
    QAction *m_actExit          = nullptr;

    // Actions — Edit
    QAction *m_actUndo          = nullptr;
    QAction *m_actRedo          = nullptr;
    QAction *m_actCut           = nullptr;
    QAction *m_actCopy          = nullptr;
    QAction *m_actPaste         = nullptr;
    QAction *m_actSelectAll     = nullptr;

    // Actions — Help
    QAction *m_actAbout         = nullptr;

    // State
    QString m_currentFilePath;
    bool    m_modified          = false;
};
