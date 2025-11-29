#ifndef ADDROUTEDIALOG_H
#define ADDROUTEDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QListWidget>
#include <QPushButton>
#include <QMessageBox>
#include "TransportSchedule.h"

class AddRouteDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddRouteDialog(TransportSchedule* schedule, QWidget *parent = nullptr);

private slots:
    void addIntermediateStop();
    void removeIntermediateStop();
    void accept() override;

private:
    void setupUI();
    bool validateInput();
    void createAndAddRoute();
    QVector<QSharedPointer<Stop>> getIntermediateStops() const;
    QVector<int> parseTravelTimes() const;
    QStringList parseDays() const;

    TransportSchedule* schedule;

    QComboBox* transportTypeCombo;
    QSpinBox* routeNumberSpin;
    QLineEdit* startStopEdit;
    QLineEdit* startCoordEdit;
    QLineEdit* endStopEdit;
    QLineEdit* endCoordEdit;
    QListWidget* intermediateStopsList;
    QLineEdit* intermediateStopEdit;
    QLineEdit* intermediateCoordEdit;
    QLineEdit* travelTimesEdit;
    QLineEdit* daysEdit;
    QSpinBox* startHourSpin;
    QSpinBox* startMinuteSpin;
};

#endif
