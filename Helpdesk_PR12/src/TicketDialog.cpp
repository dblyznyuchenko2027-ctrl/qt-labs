#include "TicketDialog.h"
#include "ui_TicketDialog.h"

TicketDialog::TicketDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::TicketDialog)
{
    ui->setupUi(this);
    // Non-modal: don't set Qt::Dialog blocking flags
    setWindowFlag(Qt::Window, true);
    setAttribute(Qt::WA_DeleteOnClose, false); // reuse the same dialog instance

    connect(ui->btnEdit,   &QPushButton::clicked, this, &TicketDialog::onBtnEdit);
    connect(ui->btnSave,   &QPushButton::clicked, this, &TicketDialog::onBtnSave);
    connect(ui->btnCancel, &QPushButton::clicked, this, &TicketDialog::onBtnCancel);
    connect(ui->btnClose,  &QPushButton::clicked, this, &TicketDialog::onBtnClose);
}

TicketDialog::~TicketDialog() { delete ui; }

// ── Public API ────────────────────────────────────────────────────────────────

void TicketDialog::setModeNew(int nextId)
{
    Ticket t = Ticket::makeNew(nextId);
    m_original = t;
    populateFields(t);
    applyMode(DialogMode::New);
    setWindowTitle(tr("New Ticket"));
}

void TicketDialog::setModeView(const Ticket &t)
{
    m_original = t;
    populateFields(t);
    applyMode(DialogMode::View);
    setWindowTitle(tr("View Ticket"));
}

void TicketDialog::setModeEdit(const Ticket &t)
{
    m_original = t;
    populateFields(t);
    applyMode(DialogMode::Edit);
    setWindowTitle(tr("Edit Ticket"));
}

Ticket TicketDialog::currentTicket() const
{
    Ticket t   = m_original;           // keep id, createdAt
    t.title       = ui->editTitle->text().trimmed();
    t.description = ui->editDescription->toPlainText();
    t.priority    = ui->comboPriority->currentText();
    t.status      = ui->comboStatus->currentText();
    return t;
}

// ── Private mode logic ────────────────────────────────────────────────────────

void TicketDialog::applyMode(DialogMode mode)
{
    m_mode = mode;
    const bool isView = (mode == DialogMode::View);
    const bool isEditOrNew = !isView;

    setFieldsReadOnly(isView);

    // View mode: show Edit + Close; hide Save + Cancel
    ui->btnEdit->setVisible(isView);
    ui->btnClose->setVisible(isView);
    ui->btnSave->setVisible(isEditOrNew);
    ui->btnCancel->setVisible(isEditOrNew);

    if (!isView)
        ui->editTitle->setFocus();
}

void TicketDialog::populateFields(const Ticket &t)
{
    ui->lblIdValue->setText(t.id > 0 ? QString::number(t.id) : tr("(new)"));
    ui->editTitle->setText(t.title);
    ui->editDescription->setPlainText(t.description);

    // Set combo selections
    auto setCombo = [](QComboBox *cb, const QString &value) {
        int idx = cb->findText(value);
        if (idx >= 0) cb->setCurrentIndex(idx);
    };
    setCombo(ui->comboPriority, t.priority);
    setCombo(ui->comboStatus,   t.status);

    ui->lblCreatedValue->setText(
        t.createdAt.isValid()
        ? t.createdAt.toString(QStringLiteral("yyyy-MM-dd hh:mm"))
        : tr("—"));
}

void TicketDialog::setFieldsReadOnly(bool ro)
{
    ui->editTitle->setReadOnly(ro);
    ui->editDescription->setReadOnly(ro);
    ui->comboPriority->setEnabled(!ro);
    ui->comboStatus->setEnabled(!ro);
}

// ── Button slots ──────────────────────────────────────────────────────────────

void TicketDialog::onBtnEdit()
{
    applyMode(DialogMode::Edit);
    setWindowTitle(tr("Edit Ticket"));
}

void TicketDialog::onBtnSave()
{
    if (ui->editTitle->text().trimmed().isEmpty()) {
        ui->editTitle->setPlaceholderText(tr("Title is required!"));
        ui->editTitle->setFocus();
        return;
    }
    emit ticketSaveRequested(currentTicket());
    applyMode(DialogMode::View);
    setWindowTitle(tr("View Ticket"));
}

void TicketDialog::onBtnCancel()
{
    if (m_mode == DialogMode::New) {
        hide();  // close dialog entirely for New
    } else {
        // Restore original values and go back to View
        populateFields(m_original);
        applyMode(DialogMode::View);
        setWindowTitle(tr("View Ticket"));
    }
}

void TicketDialog::onBtnClose()
{
    hide();
}
