#pragma once

#include <QDialog>
#include "ticket.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TicketDialog; }
QT_END_NAMESPACE

class TicketDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Mode { View, Edit, Create };

    explicit TicketDialog(QWidget *parent = nullptr);
    ~TicketDialog();

    void setMode(Mode mode);
    Mode mode() const;

    void loadTicket(const Ticket &ticket);
    Ticket collectTicket() const;

signals:
    void createRequested(const Ticket &ticket);
    void updateRequested(const Ticket &ticket);

private slots:
    void onFormChanged();
    void onSaveClicked();
    void onEditClicked();
    void onCancelClicked();

private:
    bool isTitleValid() const;
    bool isFormValid() const;
    void updateUiForMode();
    void updateValidationUi();
    void updateButtonsState();

    Ui::TicketDialog *ui;
    Mode m_mode = Mode::Create;
    int m_currentId = 0;
};
