#include "ticketdialog.h"
#include "ui_ticketdialog.h"

TicketDialog::TicketDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TicketDialog)
{
    ui->setupUi(this);

    connect(ui->editTitle, &QLineEdit::textChanged,
            this, &TicketDialog::onFormChanged);
    connect(ui->editDescription, &QPlainTextEdit::textChanged,
            this, &TicketDialog::onFormChanged);
    connect(ui->comboPriority, &QComboBox::currentTextChanged,
            this, &TicketDialog::onFormChanged);
    connect(ui->comboStatus, &QComboBox::currentTextChanged,
            this, &TicketDialog::onFormChanged);

    connect(ui->btnSave, &QPushButton::clicked,
            this, &TicketDialog::onSaveClicked);
    connect(ui->btnEdit, &QPushButton::clicked,
            this, &TicketDialog::onEditClicked);
    connect(ui->btnCancel, &QPushButton::clicked,
            this, &TicketDialog::onCancelClicked);
    connect(ui->btnClose, &QPushButton::clicked,
            this, &QDialog::close);

    updateUiForMode();
    updateValidationUi();
    updateButtonsState();
}

TicketDialog::~TicketDialog()
{
    delete ui;
}

void TicketDialog::setMode(Mode mode)
{
    m_mode = mode;
    updateUiForMode();
    updateButtonsState();
}

TicketDialog::Mode TicketDialog::mode() const
{
    return m_mode;
}

void TicketDialog::loadTicket(const Ticket &ticket)
{
    m_currentId = ticket.id;
    ui->labelIdValue->setText(QString::number(ticket.id));
    ui->editTitle->setText(ticket.title);
    ui->editDescription->setPlainText(ticket.description);
    ui->comboPriority->setCurrentText(ticket.priority);
    ui->comboStatus->setCurrentText(ticket.status);
    ui->labelCreatedValue->setText(ticket.createdAt.toString("yyyy-MM-dd hh:mm"));
}

Ticket TicketDialog::collectTicket() const
{
    Ticket t;
    t.id = m_currentId;
    t.title = ui->editTitle->text().trimmed();
    t.description = ui->editDescription->toPlainText().trimmed();
    t.priority = ui->comboPriority->currentText();
    t.status = ui->comboStatus->currentText();
    return t;
}

bool TicketDialog::isTitleValid() const
{
    return !ui->editTitle->text().trimmed().isEmpty();
}

bool TicketDialog::isFormValid() const
{
    return isTitleValid();
}

void TicketDialog::updateUiForMode()
{
    const bool editable = (m_mode == Mode::Edit || m_mode == Mode::Create);

    ui->editTitle->setReadOnly(!editable);
    ui->editDescription->setReadOnly(!editable);
    ui->comboPriority->setEnabled(editable);
    ui->comboStatus->setEnabled(editable);

    ui->btnSave->setVisible(editable);
    ui->btnCancel->setVisible(editable);
    ui->btnEdit->setVisible(m_mode == Mode::View);
    ui->btnClose->setVisible(m_mode == Mode::View);

    if (m_mode == Mode::Create)
        setWindowTitle("New Ticket");
    else if (m_mode == Mode::Edit)
        setWindowTitle("Edit Ticket");
    else
        setWindowTitle("View Ticket");
}

void TicketDialog::updateValidationUi()
{
    if (isTitleValid())
    {
        ui->labelTitleError->clear();
        ui->editTitle->setStyleSheet("");
    }
    else
    {
        ui->labelTitleError->setText("Field Title is required.");
        ui->editTitle->setStyleSheet("border: 1px solid red;");
    }
}

void TicketDialog::updateButtonsState()
{
    const bool editable = (m_mode == Mode::Edit || m_mode == Mode::Create);
    ui->btnSave->setEnabled(editable && isFormValid());
}

void TicketDialog::onFormChanged()
{
    updateValidationUi();
    updateButtonsState();
}

void TicketDialog::onSaveClicked()
{
    if (!isFormValid())
        return;

    Ticket ticket = collectTicket();

    if (m_mode == Mode::Create)
        emit createRequested(ticket);
    else if (m_mode == Mode::Edit)
        emit updateRequested(ticket);

    close();
}

void TicketDialog::onEditClicked()
{
    setMode(Mode::Edit);
}

void TicketDialog::onCancelClicked()
{
    close();
}
