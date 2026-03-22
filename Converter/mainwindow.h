#pragma once

#include <QMainWindow>
#include "converter.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Each tab: top field edited by user → update bottom
    void onTopEdited(const QString &text);
    // Each tab: bottom field edited by user → update top
    void onBottomEdited(const QString &text);
    // Unit combo changed → recalculate from whichever field was last edited
    void onUnitChanged();

private:
    Ui::MainWindow *ui;

    // Helper: resolve which Converter::Mode corresponds to current tab
    Converter::Mode currentMode() const;

    // Core recalculation: read fromEdit, write result to toEdit (signal-blocked).
    // Returns false if input is not a valid number (marks fromEdit red).
    bool recalc(QLineEdit *fromEdit, QComboBox *fromCombo,
                QLineEdit *toEdit,   QComboBox *toCombo,
                Converter::Mode mode);

    // Apply/clear error style on a line edit
    static void setError(QLineEdit *edit, bool error);

    // Track which field was last actively edited per tab (true = top, false = bottom)
    bool m_lastEditedTop[3] = { true, true, true };
};
