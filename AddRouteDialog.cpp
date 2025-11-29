#include "AddRouteDialog.h"

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
        // Create transport
        TransportType transportType(transportTypeCombo->currentText());
        Transport transport(transportType, routeNumberSpin->value());

        // Create stops
        auto startStop = schedule->findOrCreateStop(startStopEdit->text().trimmed(), startCoordEdit->text().trimmed());
        auto endStop = schedule->findOrCreateStop(endStopEdit->text().trimmed(), endCoordEdit->text().trimmed());

        // Get intermediate stops
        QVector<QSharedPointer<Stop>> intermediateStops;
        for (auto i = 0; i < intermediateStopsList->count(); ++i) {
            auto itemText = intermediateStopsList->item(i)->text();
            auto name = itemText.left(itemText.indexOf(" ("));
            auto coord = itemText.mid(itemText.indexOf("(") + 1);
            coord = coord.left(coord.indexOf(")"));
            intermediateStops.push_back(schedule->findOrCreateStop(name, coord));
        }

        // Parse travel times
        QVector<int> travelTimes;
        auto timeStrings = travelTimesEdit->text().split(",");
        for (const auto& timeStr : timeStrings) {
            bool ok;
            auto time = timeStr.trimmed().toInt(&ok);
            if (ok && time > 0) {
                travelTimes.push_back(time);
            }
        }

        // Parse days - поддерживаем разные форматы ввода
        QStringList days;
        auto daysText = daysEdit->text().trimmed();
        if (!daysText.isEmpty()) {
            // Простой способ разбить строку без QRegExp
            // Заменяем все возможные разделители на запятые, затем разбиваем по запятым
            auto normalized = daysText;
            normalized = normalized.replace(';', ',');
            normalized = normalized.replace(' ', ',');
            days = normalized.split(',', Qt::SkipEmptyParts);

            // Приводим к стандартному формату (двухбуквенные сокращения)
            for (auto i = 0; i < days.size(); ++i) {
                auto day = days[i].toLower();
                if (day == "понедельник" || day == "понед" || day == "mon" || day == "monday") day = "пн";
                else if (day == "вторник" || day == "втор" || day == "tue" || day == "tuesday") day = "вт";
                else if (day == "среда" || day == "сред" || day == "wed" || day == "wednesday") day = "ср";
                else if (day == "четверг" || day == "четв" || day == "thu" || day == "thursday") day = "чт";
                else if (day == "пятница" || day == "пятн" || day == "fri" || day == "friday") day = "пт";
                else if (day == "суббота" || day == "субб" || day == "sat" || day == "saturday") day = "сб";
                else if (day == "воскресенье" || day == "воскр" || day == "sun" || day == "sunday") day = "вс";

                days[i] = day;
            }
        }

        // Проверяем обязательные поля
        if (startStopEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Введите начальную остановку");
            return;
        }

        if (endStopEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Введите конечную остановку");
            return;
        }

        if (travelTimes.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Введите время движения между остановками");
            return;
        }

        if (days.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Введите дни работы маршрута");
            return;
        }

        // Проверяем соответствие количества временных интервалов
        auto totalStops = 1 + intermediateStops.size() + 1; // начальная + промежуточные + конечная
        if (travelTimes.size() != totalStops - 1) {
            QMessageBox::warning(this, "Ошибка",
                                 QString("Количество временных интервалов (%1) должно соответствовать количеству перегонов (%2)\n\n"
                                         "Пример: для маршрута с 3 остановками нужно 2 временных интервала")
                                     .arg(travelTimes.size()).arg(totalStops - 1));
            return;
        }

        // Create start time
        TimeTransport startTime(startHourSpin->value(), startMinuteSpin->value());

        schedule->addRoute(transport, startStop, endStop, intermediateStops, travelTimes, days, startTime);

        QDialog::accept();

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", QString("Ошибка при добавлении маршрута: %1").arg(e.what()));
    }
}
