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
    explicit RouteDetailsDialog(TransportSchedule* transportSchedule, const Schedule& schedule, QWidget *parent = nullptr);

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
    void populateStopRow(int row, const RouteStop& stop, int totalStops, const QVector<int>& travelTimes);
    QString formatStopName(const QString& name, int index, int totalStops) const;
    void setEditingFlagsForRow(int row, int totalStops)const;
    bool shouldBeEditable(int col, int row, int totalStops) const;
    void removeFormattingSymbol(QTableWidgetItem* item) const;
    QString extractStopNameFromTable(int row) const;
    bool collectStopsAndTravelTimes(QVector<QSharedPointer<Stop>>& collectedStops, QVector<int>& collectedTravelTimes);
    bool validateRouteData(const QVector<QSharedPointer<Stop>>& collectedStops, const QVector<int>& collectedTravelTimes);
    QStringList parseDays() const;
    void createAndSaveRoute(const QVector<QSharedPointer<Stop>>& collectedStops, const QVector<int>& collectedTravelTimes, const QStringList& days);
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

    bool isEditing = false;
};

#endif
