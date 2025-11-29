#include "MainWindow.h"
#include <QIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), schedule(new TransportSchedule("transport_schedule.dat", this)) {
    setupUI();
}

void MainWindow::setupUI() {
    setWindowTitle("Расписание городского транспорта");
    setMinimumSize(900, 600);

    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* mainLayout = new QVBoxLayout(centralWidget);

    // Кнопки управления
    auto* buttonLayout = new QHBoxLayout();

    addButton = new QPushButton("Добавить маршрут");
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addRoute);
    buttonLayout->addWidget(addButton);

    findTransportButton = new QPushButton("Поиск транспорта");
    connect(findTransportButton, &QPushButton::clicked, this, &MainWindow::openFindTransportDialog);
    buttonLayout->addWidget(findTransportButton);

    refreshButton = new QPushButton("Обновить");
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshTable);
    buttonLayout->addWidget(refreshButton);

    // Добавляем кнопку статистики
    statisticsButton = new QPushButton("Статистика");
    connect(statisticsButton, &QPushButton::clicked, this, &MainWindow::showStatistics);
    buttonLayout->addWidget(statisticsButton);

    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    // Таблица маршрутов
    setupTable();
    mainLayout->addWidget(routesTable);
}

void MainWindow::showRouteDetails(int routeNumber) {
    auto allSchedules = schedule->getAllSchedules();

    for (const auto& scheduleItem : allSchedules) {
        if (scheduleItem.getRoute().getRouteNumber() == routeNumber) {
            auto* dialog = new RouteDetailsDialog(schedule, scheduleItem, this);
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            connect(dialog, &RouteDetailsDialog::finished, this, &MainWindow::refreshTable);
            dialog->exec();
            return;
        }
    }

    QMessageBox::information(this, "Маршрут не найден",
                             QString("Маршрут №%1 не найден.").arg(routeNumber));
}

void MainWindow::setupTable() {
    routesTable = new QTableWidget(this);
    routesTable->setColumnCount(7);

    // Устанавливаем заголовки
    QStringList headers;
    headers << "Номер" << "Тип" << "Начальная остановка" << "Конечная остановка" << "Время начала" << "Дни работы" << "Действие";
    routesTable->setHorizontalHeaderLabels(headers);

    // Настраиваем отображение заголовков
    routesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    routesTable->horizontalHeader()->setStretchLastSection(false);
    routesTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);

    // ОТКЛЮЧАЕМ сортировку по клику на заголовки
    routesTable->setSortingEnabled(false);

    routesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    routesTable->setSelectionMode(QAbstractItemView::SingleSelection);

    // Устанавливаем минимальную высоту строк заголовка
    routesTable->horizontalHeader()->setMinimumHeight(40);

    populateTable();
}

void MainWindow::populateTable() {
    routesTable->setRowCount(0);
    auto allSchedules = schedule->getAllSchedules();

    // Сортируем маршруты по номеру в порядке возрастания
    auto sortedSchedules = allSchedules;
    std::sort(sortedSchedules.begin(), sortedSchedules.end(),
              [](const Schedule& a, const Schedule& b) {
                  return a.getRoute().getRouteNumber() < b.getRoute().getRouteNumber();
              });

    for (auto i = 0; i < sortedSchedules.size(); ++i) {
        const auto& sched = sortedSchedules[i];
        const auto& route = sched.getRoute();
        auto routeNumber = route.getRouteNumber();

        routesTable->insertRow(i);
        routesTable->setItem(i, 0, new QTableWidgetItem(QString::number(routeNumber)));
        routesTable->setItem(i, 1, new QTableWidgetItem(route.getTransport().getType().getName()));
        routesTable->setItem(i, 2, new QTableWidgetItem(route.getStartStop()->getName()));
        routesTable->setItem(i, 3, new QTableWidgetItem(route.getEndStop()->getName()));
        routesTable->setItem(i, 4, new QTableWidgetItem(sched.getStartTime().toString()));
        routesTable->setItem(i, 5, new QTableWidgetItem(route.getDays().join(", ")));

        // Центрируем текст в ячейках
        for (auto col = 0; col < 6; ++col) {
            auto* item = routesTable->item(i, col);
            if (item) {
                item->setTextAlignment(Qt::AlignCenter);
            }
        }

        // Создаем контейнер для кнопок
        auto* buttonsWidget = new QWidget();
        auto* buttonsLayout = new QHBoxLayout(buttonsWidget);
        buttonsLayout->setContentsMargins(2, 2, 2, 2);
        buttonsLayout->setSpacing(2);

        // Кнопка "Показать маршрут"
        auto* showRouteButton = new QToolButton();
        showRouteButton->setIcon(QIcon::fromTheme("edit-find", QIcon(":/icons/route.png")));
        showRouteButton->setText("Маршрут");
        showRouteButton->setToolTip("Показать полный маршрут с остановками");
        showRouteButton->setIconSize(QSize(16, 16));
        showRouteButton->setStyleSheet("QToolButton { border: 1px solid #c0c0c0; border-radius: 3px; padding: 3px; background-color: #e8f4ff; }");

        // Подключаем кнопку к слоту показа маршрута
        connect(showRouteButton, &QToolButton::clicked, [this, routeNumber]() {
            showRouteDetails(routeNumber);
        });

        // Кнопка удаления
        auto* deleteButton = new QToolButton();
        deleteButton->setIcon(QIcon::fromTheme("edit-delete", QIcon(":/icons/delete.png")));
        deleteButton->setText("Удалить");
        deleteButton->setToolTip("Удалить маршрут");
        deleteButton->setIconSize(QSize(16, 16));
        deleteButton->setStyleSheet("QToolButton { border: 1px solid #c0c0c0; border-radius: 3px; padding: 3px; background-color: #ffe8e8; }");

        connect(deleteButton, &QToolButton::clicked, [this, routeNumber]() {
            removeRoute(routeNumber);
        });

        // Добавляем кнопки в layout
        buttonsLayout->addWidget(showRouteButton);
        buttonsLayout->addWidget(deleteButton);
        buttonsLayout->setAlignment(Qt::AlignCenter);

        routesTable->setCellWidget(i, 6, buttonsWidget);
    }

    // Настраиваем ширину колонок
    routesTable->resizeColumnsToContents();
    routesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    routesTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    routesTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);

    // Увеличиваем ширину последней колонки для двух кнопок
    routesTable->setColumnWidth(6, 150);
}

void MainWindow::addRoute() {
    AddRouteDialog dialog(schedule, this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshTable();
    }
}

void MainWindow::removeRoute(int routeNumber) {
    auto reply = QMessageBox::question(this, "Подтверждение",
                                       "Вы уверены, что хотите удалить маршрут №" + QString::number(routeNumber) + "?",
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        schedule->removeRoute(routeNumber);
        refreshTable();
    }
}

void MainWindow::openFindTransportDialog() {
    FindTransportDialog dialog(schedule, this);
    dialog.exec();
}

void MainWindow::refreshTable() {
    populateTable();
}

void MainWindow::showStatistics() {
    auto routeStats = schedule->getRouteStatistics();
    auto stopStats = schedule->getStopStatistics();

    auto statsText = QString();
    statsText += QString("=== СТАТИСТИКА СИСТЕМЫ ===\n\n");
    statsText += QString("Маршруты:\n");
    statsText += QString("  Всего маршрутов: %1\n").arg(routeStats.totalRoutes);
    statsText += QString("  Автобусы: %1\n").arg(routeStats.busCount);
    statsText += QString("  Троллейбусы: %1\n").arg(routeStats.trolleybusCount);
    statsText += QString("  Трамваи: %1\n").arg(routeStats.tramCount);
    statsText += QString("  Среднее количество остановок на маршрут: %1\n").arg(routeStats.averageStopsPerRoute);

    statsText += QString("\nОстановки:\n");
    statsText += QString("  Всего остановок: %1\n").arg(stopStats.totalStops);
    statsText += QString("  Активных остановок: %1\n").arg(stopStats.activeStops);

    if (!stopStats.mostPopularStops.isEmpty()) {
        statsText += QString("\nСамые популярные остановки:\n");
        auto count = std::min(3, static_cast<int>(stopStats.mostPopularStops.size()));
        for (auto i = 0; i < count; ++i) {
            statsText += QString("  %1. %2\n").arg(i + 1).arg(stopStats.mostPopularStops[i]->getName());
        }
    }

    QMessageBox::information(this, "Статистика системы", statsText);
}
