#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLineEdit>
#include <QComboBox>
#include <QDoubleValidator>
#include <QSignalBlocker>

// -----------------------------------------------------------------------
// Helpers — group widgets per tab in arrays for DRY access
// -----------------------------------------------------------------------
struct TabWidgets {
    QLineEdit *editTop;
    QComboBox *comboTop;
    QLineEdit *editBottom;
    QComboBox *comboBottom;
    Converter::Mode mode;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ---- Validators ----
    auto *dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::StandardNotation);

    for (QLineEdit *e : {
             ui->lengthEditTop, ui->lengthEditBottom,
             ui->massEditTop,   ui->massEditBottom,
             ui->tempEditTop,   ui->tempEditBottom })
    {
        e->setValidator(new QDoubleValidator(this));
    }

    // ---- Connect top fields (user edit) ----
    connect(ui->lengthEditTop,  &QLineEdit::textEdited, this, &MainWindow::onTopEdited);
    connect(ui->massEditTop,    &QLineEdit::textEdited, this, &MainWindow::onTopEdited);
    connect(ui->tempEditTop,    &QLineEdit::textEdited, this, &MainWindow::onTopEdited);

    // ---- Connect bottom fields (user edit) ----
    connect(ui->lengthEditBottom, &QLineEdit::textEdited, this, &MainWindow::onBottomEdited);
    connect(ui->massEditBottom,   &QLineEdit::textEdited, this, &MainWindow::onBottomEdited);
    connect(ui->tempEditBottom,   &QLineEdit::textEdited, this, &MainWindow::onBottomEdited);

    // ---- Connect combos ----
    for (QComboBox *cb : {
             ui->lengthComboTopUnit, ui->lengthComboBottomUnit,
             ui->massComboTopUnit,   ui->massComboBottomUnit,
             ui->tempComboTopUnit,   ui->tempComboBottomUnit })
    {
        connect(cb, &QComboBox::currentIndexChanged, this, &MainWindow::onUnitChanged);
    }

    // ---- Connect tab switch ----
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onUnitChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// -----------------------------------------------------------------------
// Resolve current tab → Mode and widgets
// -----------------------------------------------------------------------
Converter::Mode MainWindow::currentMode() const
{
    switch (ui->tabWidget->currentIndex()) {
    case 0: return Converter::Mode::Length;
    case 1: return Converter::Mode::Mass;
    default: return Converter::Mode::Temperature;
    }
}

// Returns the tab widgets for the given tab index
static void tabWidgetsFor(int idx, Ui::MainWindow *ui,
                          QLineEdit *&eTop, QComboBox *&cTop,
                          QLineEdit *&eBot, QComboBox *&cBot,
                          Converter::Mode &mode)
{
    switch (idx) {
    case 0:
        eTop = ui->lengthEditTop;  cTop = ui->lengthComboTopUnit;
        eBot = ui->lengthEditBottom; cBot = ui->lengthComboBottomUnit;
        mode = Converter::Mode::Length; break;
    case 1:
        eTop = ui->massEditTop;    cTop = ui->massComboTopUnit;
        eBot = ui->massEditBottom; cBot = ui->massComboBottomUnit;
        mode = Converter::Mode::Mass; break;
    default:
        eTop = ui->tempEditTop;    cTop = ui->tempComboTopUnit;
        eBot = ui->tempEditBottom; cBot = ui->tempComboBottomUnit;
        mode = Converter::Mode::Temperature; break;
    }
}

// -----------------------------------------------------------------------
// Core recalculation
// -----------------------------------------------------------------------
bool MainWindow::recalc(QLineEdit *fromEdit, QComboBox *fromCombo,
                        QLineEdit *toEdit,   QComboBox *toCombo,
                        Converter::Mode mode)
{
    const QString text = fromEdit->text().trimmed();
    if (text.isEmpty()) {
        QSignalBlocker b(toEdit);
        toEdit->clear();
        setError(fromEdit, false);
        setError(toEdit, false);
        return true;
    }

    bool ok = false;
    double value = text.toDouble(&ok);
    if (!ok) {
        setError(fromEdit, true);
        return false;
    }

    setError(fromEdit, false);

    double result = Converter::convert(mode, value, fromCombo->currentIndex(), toCombo->currentIndex());

    QSignalBlocker b(toEdit);
    toEdit->setText(Converter::format(result));
    setError(toEdit, false);
    return true;
}

void MainWindow::setError(QLineEdit *edit, bool error)
{
    edit->setStyleSheet(error
        ? "QLineEdit { border: 1px solid #e74c3c; background: #fdf0ef; }"
        : "");
}

// -----------------------------------------------------------------------
// Slots
// -----------------------------------------------------------------------
void MainWindow::onTopEdited(const QString &)
{
    int idx = ui->tabWidget->currentIndex();
    m_lastEditedTop[idx] = true;

    QLineEdit *eTop, *eBot; QComboBox *cTop, *cBot; Converter::Mode mode;
    tabWidgetsFor(idx, ui, eTop, cTop, eBot, cBot, mode);
    recalc(eTop, cTop, eBot, cBot, mode);
}

void MainWindow::onBottomEdited(const QString &)
{
    int idx = ui->tabWidget->currentIndex();
    m_lastEditedTop[idx] = false;

    QLineEdit *eTop, *eBot; QComboBox *cTop, *cBot; Converter::Mode mode;
    tabWidgetsFor(idx, ui, eTop, cTop, eBot, cBot, mode);
    recalc(eBot, cBot, eTop, cTop, mode);
}

void MainWindow::onUnitChanged()
{
    int idx = ui->tabWidget->currentIndex();

    QLineEdit *eTop, *eBot; QComboBox *cTop, *cBot; Converter::Mode mode;
    tabWidgetsFor(idx, ui, eTop, cTop, eBot, cBot, mode);

    // Recalculate from whichever field the user last edited on this tab
    if (m_lastEditedTop[idx]) {
        recalc(eTop, cTop, eBot, cBot, mode);
    } else {
        recalc(eBot, cBot, eTop, cTop, mode);
    }
}
