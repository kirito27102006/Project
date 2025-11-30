#include "AddRouteDialog.h"
#include "DayOfWeekService.h"
#include <QInputDialog>

AddRouteDialog::AddRouteDialog(TransportSchedule* schedule, QWidget *parent)
    : QDialog(parent), schedule(schedule) {
    setupUI();
    setWindowTitle("Добавить маршрут");
    setMinimumSize(600, 700);
}

void AddRouteDialog::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    // Transport info
    auto* transportGroup = new QGroupBox("Информация о транспорте");
    auto* transportLayout = new QFormLayout(transportGroup);

    transportTypeCombo = new QComboBox;
    transportTypeCombo->addItems(TransportType::getAllTypeNames());
    transportLayout->addRow("Тип транспорта:", transportTypeCombo);

    routeNumberSpin = new QSpinBox;
    routeNumberSpin->setRange(1, 999);
    transportLayout->addRow("Номер маршрута:", routeNumberSpin);

    mainLayout->addWidget(transportGroup);

    // Stops info
    auto* stopsGroup = new QGroupBox("Остановки");
    auto* stopsLayout = new QGridLayout(stopsGroup);

    stopsLayout->addWidget(new QLabel("Начальная остановка:"), 0, 0);
    startStopEdit = new QLineEdit;
    stopsLayout->addWidget(startStopEdit, 0, 1);
    stopsLayout->addWidget(new QLabel("Координаты:"), 0, 2);
    startCoordEdit = new QLineEdit;
    stopsLayout->addWidget(startCoordEdit, 0, 3);

    stopsLayout->addWidget(new QLabel("Конечная остановка:"), 1, 0);
    endStopEdit = new QLineEdit;
    stopsLayout->addWidget(endStopEdit, 1, 1);
    stopsLayout->addWidget(new QLabel("Координаты:"), 1, 2);
    endCoordEdit = new QLineEdit;
    stopsLayout->addWidget(endCoordEdit, 1, 3);

    stopsLayout->addWidget(new QLabel("Промежуточные остановки:"), 2, 0);
    intermediateStopsList = new QListWidget;
    stopsLayout->addWidget(intermediateStopsList, 3, 0, 1, 3);

    auto* intermediateButtonsLayout = new QVBoxLayout;
    intermediateStopEdit = new QLineEdit;
    intermediateStopEdit->setPlaceholderText("Название остановки");
    intermediateButtonsLayout->addWidget(intermediateStopEdit);

    intermediateCoordEdit = new QLineEdit;
    intermediateCoordEdit->setPlaceholderText("Координаты");
    intermediateButtonsLayout->addWidget(intermediateCoordEdit);

    auto* addIntermediateBtn = new QPushButton("Добавить остановку");
    connect(addIntermediateBtn, &QPushButton::clicked, this, &AddRouteDialog::addIntermediateStop);
    intermediateButtonsLayout->addWidget(addIntermediateBtn);

    auto* removeIntermediateBtn = new QPushButton("Удалить остановку");
    connect(removeIntermediateBtn, &QPushButton::clicked, this, &AddRouteDialog::removeIntermediateStop);
    intermediateButtonsLayout->addWidget(removeIntermediateBtn);

    stopsLayout->addLayout(intermediateButtonsLayout, 3, 3);

    mainLayout->addWidget(stopsGroup);

    // Schedule info
    auto* scheduleGroup = new QGroupBox("Расписание");
    auto* scheduleLayout = new QFormLayout(scheduleGroup);

    travelTimesEdit = new QLineEdit;
    travelTimesEdit->setPlaceholderText("Время в минутах через запятую (например: 5,10,15)");
    scheduleLayout->addRow("Время движения между остановками:", travelTimesEdit);

    daysEdit = new QLineEdit;
    daysEdit->setPlaceholderText("пн,вт,ср,чт,пт,сб,вс (через запятую)");
    scheduleLayout->addRow("Дни работы:", daysEdit);

    auto* timeLayout = new QHBoxLayout;
    startHourSpin = new QSpinBox;
    startHourSpin->setRange(0, 23);
    startHourSpin->setValue(6);
    startMinuteSpin = new QSpinBox;
    startMinuteSpin->setRange(0, 59);
    startMinuteSpin->setValue(0);
    timeLayout->addWidget(startHourSpin);
    timeLayout->addWidget(new QLabel(":"));
    timeLayout->addWidget(startMinuteSpin);
    timeLayout->addStretch();
    scheduleLayout->addRow("Время начала работы:", timeLayout);

    mainLayout->addWidget(scheduleGroup);

    // Buttons
    auto* buttonLayout = new QHBoxLayout;
    auto* okButton = new QPushButton("Добавить");
    connect(okButton, &QPushButton::clicked, this, &AddRouteDialog::accept);
    auto* cancelButton = new QPushButton("Отмена");
    connect(cancelButton, &QPushButton::clicked, this, &AddRouteDialog::reject);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
}

void AddRouteDialog::addIntermediateStop() {
    auto name = intermediateStopEdit->text().trimmed();
    auto coord = intermediateCoordEdit->text().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите название остановки");
        return;
    }

    intermediateStopsList->addItem(name + " (" + coord + ")");
    intermediateStopEdit->clear();
    intermediateCoordEdit->clear();
}

void AddRouteDialog::removeIntermediateStop() {
    auto row = intermediateStopsList->currentRow();
    if (row >= 0) {
        delete intermediateStopsList->takeItem(row);
    }
}

void AddRouteDialog::accept() {
    try {
        if (!validateInput()) {
            return;
        }

        createAndAddRoute();
        QDialog::accept();

    } catch (const std::invalid_argument& e) {
        QMessageBox::critical(this, "Ошибка ввода данных",
                              QString("Ошибка при добавлении маршрута: %1").arg(e.what()));
    } catch (const std::out_of_range& e) {
        QMessageBox::critical(this, "Ошибка диапазона",
                              QString("Ошибка при добавлении маршрута: %1").arg(e.what()));
    } catch (const std::logic_error& e) {
        QMessageBox::critical(this, "Логическая ошибка",
                              QString("Ошибка при добавлении маршрута: %1").arg(e.what()));
    } catch (const std::runtime_error& e) {
        QMessageBox::critical(this, "Ошибка выполнения",
                              QString("Ошибка при добавлении маршрута: %1").arg(e.what()));
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Неизвестная ошибка",
                              QString("Ошибка при добавлении маршрута: %1").arg(e.what()));
    }
}

bool AddRouteDialog::validateInput() {
    if (startStopEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите начальную остановку");
        return false;
    }

    if (endStopEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите конечную остановку");
        return false;
    }

    auto travelTimes = parseTravelTimes();
    if (travelTimes.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите время движения между остановками");
        return false;
    }

    if (auto days = parseDays(); days.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите дни работы маршрута");
        return false;
    }

    auto intermediateStops = getIntermediateStops();
    if (auto totalStops = 1 + intermediateStops.size() + 1; travelTimes.size() != totalStops - 1) {
        QMessageBox::warning(this, "Ошибка",
                             QString("Количество временных интервалов (%1) должно соответствовать количеству перегонов (%2)\n\n"
                                     "Пример: для маршрута с 3 остановками нужно 2 временных интервала")
                                 .arg(travelTimes.size()).arg(totalStops - 1));
        return false;
    }

    return true;
}

void AddRouteDialog::createAndAddRoute() {
    // Create transport
    TransportType transportType(transportTypeCombo->currentText());
    Transport transport(transportType, routeNumberSpin->value());

    // Create stops
    auto startStop = schedule->findOrCreateStop(
        startStopEdit->text().trimmed(), startCoordEdit->text().trimmed());
    auto endStop = schedule->findOrCreateStop(
        endStopEdit->text().trimmed(), endCoordEdit->text().trimmed());

    // Get intermediate stops and travel times
    auto intermediateStops = getIntermediateStops();
    auto travelTimes = parseTravelTimes();
    auto days = parseDays();

    // Create start time
    TimeTransport startTime(startHourSpin->value(), startMinuteSpin->value());

    schedule->addRoute(transport, startStop, endStop, intermediateStops,
                       travelTimes, days, startTime);
}

QVector<QSharedPointer<Stop>> AddRouteDialog::getIntermediateStops() const {
    QVector<QSharedPointer<Stop>> intermediateStops;
    for (auto i = 0; i < intermediateStopsList->count(); ++i) {
        auto itemText = intermediateStopsList->item(i)->text();
        auto name = itemText.left(itemText.indexOf(" ("));
        auto coord = itemText.mid(itemText.indexOf("(") + 1);
        coord = coord.left(coord.indexOf(")"));
        intermediateStops.push_back(schedule->findOrCreateStop(name, coord));
    }
    return intermediateStops;
}

QVector<int> AddRouteDialog::parseTravelTimes() const {
    QVector<int> travelTimes;
    auto timeStrings = travelTimesEdit->text().split(",");
    for (const auto& timeStr : timeStrings) {
        bool ok;
        auto time = timeStr.trimmed().toInt(&ok);
        if (ok && time > 0) {
            travelTimes.push_back(time);
        }
    }
    return travelTimes;
}

QStringList AddRouteDialog::parseDays() const {
    if (auto daysText = daysEdit->text().trimmed(); !daysText.isEmpty()) {
        auto normalized = daysText;
        normalized = normalized.replace(';', ',');
        normalized = normalized.replace(' ', ',');
        auto days = normalized.split(',', Qt::SkipEmptyParts);

        for (auto &day : days) {
            day = day.trimmed().toLower();

            if (day == "понедельник" || day == "понед" || day == "mon" || day == "monday")
                day = "пн";
            else if (day == "вторник" || day == "втор" || day == "tue" || day == "tuesday")
                day = "вт";
            else if (day == "среда" || day == "сред" || day == "wed" || day == "wednesday")
                day = "ср";
            else if (day == "четверг" || day == "четв" || day == "thu" || day == "thursday")
                day = "чт";
            else if (day == "пятница" || day == "пятн" || day == "fri" || day == "friday")
                day = "пт";
            else if (day == "суббота" || day == "субб" || day == "sat" || day == "saturday")
                day = "сб";
            else if (day == "воскресенье" || day == "воскр" || day == "sun" || day == "sunday")
                day = "вс";
        }
        return days;
    }
    return {};
}
