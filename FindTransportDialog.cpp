#include "FindTransportDialog.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include <QTabWidget>

FindTransportDialog::FindTransportDialog(TransportSchedule* schedule, QWidget *parent)
    : QDialog(parent), schedule(schedule) {
    setupUI();
    setWindowTitle("Поиск транспорта");
    setMinimumSize(800, 600);
}

void FindTransportDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* findLayout = new QHBoxLayout;
    findLayout->addWidget(new QLabel("Остановка:"));

    findStopCombo = new QComboBox;
    updateStopsCombo();
    findLayout->addWidget(findStopCombo);

    findButton = new QPushButton("Найти ближайший транспорт");
    connect(findButton, &QPushButton::clicked, this, &FindTransportDialog::findNextTransport);
    findLayout->addWidget(findButton);

    showAllRoutesButton = new QPushButton("Показать все маршруты");
    connect(showAllRoutesButton, &QPushButton::clicked, this, &FindTransportDialog::showAllRoutesForStop);
    findLayout->addWidget(showAllRoutesButton);

    mainLayout->addLayout(findLayout);

    tabWidget = new QTabWidget;

    QWidget* nearestTab = new QWidget;
    QVBoxLayout* nearestLayout = new QVBoxLayout(nearestTab);

    nextTransportTable = new QTableWidget;
    nextTransportTable->setColumnCount(5);
    QStringList headers;
    headers << "Маршрут" << "Тип" << "Время прибытия" << "Время ожидания" << "Конечная остановка";
    nextTransportTable->setHorizontalHeaderLabels(headers);
    nextTransportTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    nextTransportTable->horizontalHeader()->setStretchLastSection(true);
    nextTransportTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    nextTransportTable->horizontalHeader()->setMinimumHeight(40);
    nextTransportTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    nearestLayout->addWidget(nextTransportTable);
    tabWidget->addTab(nearestTab, "Ближайший транспорт");

    QWidget* allRoutesTab = new QWidget;
    QVBoxLayout* allRoutesLayout = new QVBoxLayout(allRoutesTab);

    allRoutesTable = new QTableWidget;
    allRoutesTable->setColumnCount(6);
    QStringList allRoutesHeaders;
    allRoutesHeaders << "Маршрут" << "Тип" << "Начальная остановка" << "Конечная остановка" << "Время начала" << "Дни работы";
    allRoutesTable->setHorizontalHeaderLabels(allRoutesHeaders);
    allRoutesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    allRoutesTable->horizontalHeader()->setStretchLastSection(true);
    allRoutesTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    allRoutesTable->horizontalHeader()->setMinimumHeight(40);
    allRoutesTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    allRoutesLayout->addWidget(allRoutesTable);
    tabWidget->addTab(allRoutesTab, "Все маршруты через остановку");

    mainLayout->addWidget(tabWidget);
}

void FindTransportDialog::updateStopsCombo() {
    findStopCombo->clear();

    auto activeStops = schedule->getActiveStops();

    std::sort(activeStops.begin(), activeStops.end(),
              [](const QSharedPointer<Stop>& a, const QSharedPointer<Stop>& b) {
                  return a->getName() < b->getName();
              });

    for (const auto& stop : activeStops) {
        findStopCombo->addItem(stop->getName());
    }

    if (findStopCombo->count() > 0) {
        findStopCombo->setCurrentIndex(0);
    }
}

void FindTransportDialog::findNextTransport() {
    QString stopName = findStopCombo->currentText();
    if (stopName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите остановку");
        return;
    }

    auto nextSchedules = schedule->findNextTransport(stopName);
    nextTransportTable->setRowCount(nextSchedules.size());

    TimeTransport currentTime = schedule->getCurrentTime();
    QString currentDay = schedule->getCurrentDayOfWeek();

    if (nextSchedules.isEmpty()) {
        QMessageBox::information(this, "Результат поиска",
                                 QString("На остановке \"%1\" нет транспорта в ближайшее время.\n\n"
                                         "Текущее время: %2\n"
                                         "Текущий день: %3")
                                     .arg(stopName)
                                     .arg(currentTime.toString())
                                     .arg(currentDay));
        return;
    }

    for (int i = 0; i < nextSchedules.size(); ++i) {
        const auto& sched = nextSchedules[i];
        const auto& route = sched.getRoute();

        TimeTransport arrivalTime;
        try {
            arrivalTime = route.getArrivalTimeAtStop(stopName);
        } catch (...) {
            continue;
        }

        // Используем ArrivalTimeService для форматирования времени ожидания
        int waitMinutes = ArrivalTimeService::calculateWaitTime(currentTime, arrivalTime);
        QString waitTime = ArrivalTimeService::formatWaitTime(waitMinutes);

        nextTransportTable->setItem(i, 0, new QTableWidgetItem(QString::number(route.getRouteNumber())));
        nextTransportTable->setItem(i, 1, new QTableWidgetItem(route.getTransport().getType().getName()));
        nextTransportTable->setItem(i, 2, new QTableWidgetItem(arrivalTime.toString()));
        nextTransportTable->setItem(i, 3, new QTableWidgetItem(waitTime));
        nextTransportTable->setItem(i, 4, new QTableWidgetItem(route.getEndStop()->getName()));

        for (int col = 0; col < 5; ++col) {
            QTableWidgetItem* item = nextTransportTable->item(i, col);
            if (item) {
                item->setTextAlignment(Qt::AlignCenter);
            }
        }
    }

    nextTransportTable->resizeColumnsToContents();
    nextTransportTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);

    tabWidget->setCurrentIndex(0);
}

void FindTransportDialog::showAllRoutesForStop() {
    QString stopName = findStopCombo->currentText();
    if (stopName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите остановку");
        return;
    }

    populateAllRoutesTable(stopName);
    tabWidget->setCurrentIndex(1);
}

void FindTransportDialog::populateAllRoutesTable(const QString& stopName) {
    // Используем SearchService для поиска всех маршрутов через остановку
    auto routesThroughStop = SearchService::findSchedulesByStop(schedule->getAllSchedules(), stopName);

    allRoutesTable->setRowCount(routesThroughStop.size());

    if (routesThroughStop.isEmpty()) {
        QMessageBox::information(this, "Все маршруты",
                                 QString("Через остановку \"%1\" не проходит ни один маршрут.")
                                     .arg(stopName));
        return;
    }

    for (int i = 0; i < routesThroughStop.size(); ++i) {
        const auto& sched = routesThroughStop[i];
        const auto& route = sched.getRoute();

        allRoutesTable->setItem(i, 0, new QTableWidgetItem(QString::number(route.getRouteNumber())));
        allRoutesTable->setItem(i, 1, new QTableWidgetItem(route.getTransport().getType().getName()));
        allRoutesTable->setItem(i, 2, new QTableWidgetItem(route.getStartStop()->getName()));
        allRoutesTable->setItem(i, 3, new QTableWidgetItem(route.getEndStop()->getName()));
        allRoutesTable->setItem(i, 4, new QTableWidgetItem(sched.getStartTime().toString()));
        allRoutesTable->setItem(i, 5, new QTableWidgetItem(route.getDays().join(", ")));

        for (int col = 0; col < 6; ++col) {
            QTableWidgetItem* item = allRoutesTable->item(i, col);
            if (item) {
                item->setTextAlignment(Qt::AlignCenter);
            }
        }
    }

    allRoutesTable->resizeColumnsToContents();
    allRoutesTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
}

void FindTransportDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    updateStopsCombo();
}
