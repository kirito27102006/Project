#ifndef FINDTRANSPORTDIALOG_H
#define FINDTRANSPORTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QHeaderView>
#include <QShowEvent>
#include <QTabWidget>
#include "TransportSchedule.h"

class FindTransportDialog : public QDialog {
    Q_OBJECT

public:
    FindTransportDialog(TransportSchedule* schedule, QWidget *parent = nullptr);

private slots:
    void findNextTransport();
    void showAllRoutesForStop();

private:
    void setupUI();
    void updateStopsCombo();
    void populateAllRoutesTable(const QString& stopName);

    TransportSchedule* schedule;
    QComboBox* findStopCombo;
    QTableWidget* nextTransportTable;
    QTableWidget* allRoutesTable;
    QPushButton* findButton;
    QPushButton* showAllRoutesButton;
    QTabWidget* tabWidget;

protected:
    void showEvent(QShowEvent* event) override;
};

#endif
