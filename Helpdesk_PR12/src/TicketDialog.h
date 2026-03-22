#pragma once
#include <QDialog>
#include "TicketTableModel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TicketDialog; }
QT_END_NAMESPACE

enum class DialogMode { View, Edit, New };

class TicketDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TicketDialog(QWidget *parent = nullptr);
    ~TicketDialog() override;

    // Set mode and populate fields
    void setModeNew(int nextId);
    void setModeView(const Ticket &t);
    void setModeEdit(const Ticket &t);

    // Returns ticket built from current fields (call before accepting)
    Ticket currentTicket() const;

signals:
    void ticketSaveRequested(const Ticket &t);  // emitted on Save
    void ticketDeleteRequested(int ticketId);   // (optional, for future use)

private slots:
    void onBtnEdit();
    void onBtnSave();
    void onBtnCancel();
    void onBtnClose();

private:
    void applyMode(DialogMode mode);
    void populateFields(const Ticket &t);
    void setFieldsReadOnly(bool ro);

    Ui::TicketDialog *ui;
    DialogMode  m_mode    = DialogMode::View;
    Ticket      m_original;   // snapshot before edit (for Cancel)
};
