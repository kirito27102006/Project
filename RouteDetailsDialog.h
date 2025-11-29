#ifndef ROUTEDETAILSDIALOG_H
#define ROUTEDETAILSDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QLabel>
#include <QHeaderView>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include "Route.h"
#include "Schedule.h"
#include "TransportSchedule.h"

class RouteDetailsDialog : public QDialog {
    Q_OBJECT

public:
    RouteDetailsDialog(TransportSchedule* transportSchedule, const Schedule& schedule, QWidget *parent = nullptr);

private slots:
    void editRoute();
    void saveRoute();
    void cancelEdit();
    void addStop();
    void removeStop();
    void updateArrivalTimes();

private:
    void setupUI(const Route& route, const TimeTransport& startTime);
    void populateStopsTable();
    void calculateArrivalTimes();
    TimeTransport calculateArrivalTime(int stopIndex);

    TransportSchedule* transportSchedule;
    Schedule currentSchedule;
    Route originalRoute;

    QTableWidget* stopsTable;
    QLabel* infoLabel;
    QPushButton* editButton;
    QPushButton* saveButton;
    QPushButton* cancelButton;
    QPushButton* addStopButton;
    QPushButton* removeStopButton;

    QWidget* editPanel;
    QSpinBox* startHourSpin;
    QSpinBox* startMinuteSpin;
    QComboBox* daysCombo;

    bool isEditing;
};

#endif
