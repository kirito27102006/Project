#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolButton>
#include "TransportSchedule.h"
#include "AddRouteDialog.h"
#include "FindTransportDialog.h"
#include "RouteDetailsDialog.h"  // ДОБАВЛЯЕМ ЭТУ СТРОКУ

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void addRoute();
    void removeRoute(int routeNumber);
    void showRouteDetails(int routeNumber);
    void openFindTransportDialog();
    void refreshTable();
    void showStatistics();

private:
    void setupUI();
    void setupTable();
    void populateTable();

    TransportSchedule* schedule;
    QTableWidget* routesTable;
    QPushButton* addButton;
    QPushButton* findTransportButton;
    QPushButton* refreshButton;
    QPushButton* statisticsButton;
};

#endif
