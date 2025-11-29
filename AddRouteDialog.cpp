#include "AddRouteDialog.h"

AddRouteDialog::AddRouteDialog(TransportSchedule* schedule, QWidget *parent)
    : QDialog(parent), schedule(schedule) {
    setupUI();
    setWindowTitle("Добавить маршрут");
    setMinimumSize(600, 700);
}

void AddRouteDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Transport info
    QGroupBox* transportGroup = new QGroupBox("Информация о транспорте");
    QFormLayout* transportLayout = new QFormLayout(transportGroup);

    transportTypeCombo = new QComboBox;
    transportTypeCombo->addItems(TransportType::getAllTypeNames());
    transportLayout->addRow("Тип транспорта:", transportTypeCombo);

    routeNumberSpin = new QSpinBox;
    routeNumberSpin->setRange(1, 999);
    transportLayout->addRow("Номер маршрута:", routeNumberSpin);

    mainLayout->addWidget(transportGroup);

    // Stops info
    QGroupBox* stopsGroup = new QGroupBox("Остановки");
    QGridLayout* stopsLayout = new QGridLayout(stopsGroup);

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

    QVBoxLayout* intermediateButtonsLayout = new QVBoxLayout;
    intermediateStopEdit = new QLineEdit;
    intermediateStopEdit->setPlaceholderText("Название остановки");
    intermediateButtonsLayout->addWidget(intermediateStopEdit);

    intermediateCoordEdit = new QLineEdit;
    intermediateCoordEdit->setPlaceholderText("Координаты");
    intermediateButtonsLayout->addWidget(intermediateCoordEdit);

    QPushButton* addIntermediateBtn = new QPushButton("Добавить остановку");
    connect(addIntermediateBtn, &QPushButton::clicked, this, &AddRouteDialog::addIntermediateStop);
    intermediateButtonsLayout->addWidget(addIntermediateBtn);

    QPushButton* removeIntermediateBtn = new QPushButton("Удалить остановку");
    connect(removeIntermediateBtn, &QPushButton::clicked, this, &AddRouteDialog::removeIntermediateStop);
    intermediateButtonsLayout->addWidget(removeIntermediateBtn);

    stopsLayout->addLayout(intermediateButtonsLayout, 3, 3);

    mainLayout->addWidget(stopsGroup);

    // Schedule info
    QGroupBox* scheduleGroup = new QGroupBox("Расписание");
    QFormLayout* scheduleLayout = new QFormLayout(scheduleGroup);

    travelTimesEdit = new QLineEdit;
    travelTimesEdit->setPlaceholderText("Время в минутах через запятую (например: 5,10,15)");
    scheduleLayout->addRow("Время движения между остановками:", travelTimesEdit);

    daysEdit = new QLineEdit;
    daysEdit->setPlaceholderText("пн,вт,ср,чт,пт,сб,вс (через запятую)");
    scheduleLayout->addRow("Дни работы:", daysEdit);

    QHBoxLayout* timeLayout = new QHBoxLayout;
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
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    QPushButton* okButton = new QPushButton("Добавить");
    connect(okButton, &QPushButton::clicked, this, &AddRouteDialog::accept);
    QPushButton* cancelButton = new QPushButton("Отмена");
    connect(cancelButton, &QPushButton::clicked, this, &AddRouteDialog::reject);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
}

void AddRouteDialog::addIntermediateStop() {
    QString name = intermediateStopEdit->text().trimmed();
    QString coord = intermediateCoordEdit->text().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите название остановки");
        return;
    }

    intermediateStopsList->addItem(name + " (" + coord + ")");
    intermediateStopEdit->clear();
    intermediateCoordEdit->clear();
}

void AddRouteDialog::removeIntermediateStop() {
    int row = intermediateStopsList->currentRow();
    if (row >= 0) {
        delete intermediateStopsList->takeItem(row);
    }
}

void AddRouteDialog::accept() {
    try {
        // Валидация типа транспорта
        auto transportValidation = ValidationService::validateTransportType(transportTypeCombo->currentText());
        if (!transportValidation.isValid) {
            QMessageBox::warning(this, "Ошибка", transportValidation.errorMessage);
            return;
        }

        // Валидация данных остановок
        auto startStopValidation = ValidationService::validateStopData(startStopEdit->text().trimmed(), startCoordEdit->text().trimmed());
        if (!startStopValidation.isValid) {
            QMessageBox::warning(this, "Ошибка", "Начальная остановка: " + startStopValidation.errorMessage);
            return;
        }

        auto endStopValidation = ValidationService::validateStopData(endStopEdit->text().trimmed(), endCoordEdit->text().trimmed());
        if (!endStopValidation.isValid) {
            QMessageBox::warning(this, "Ошибка", "Конечная остановка: " + endStopValidation.errorMessage);
            return;
        }

        // Валидация времени
        auto timeValidation = ValidationService::validateTimeData(startHourSpin->value(), startMinuteSpin->value());
        if (!timeValidation.isValid) {
            QMessageBox::warning(this, "Ошибка", timeValidation.errorMessage);
            return;
        }

        // Create transport
        TransportType transportType(transportTypeCombo->currentText());
        Transport transport(transportType, routeNumberSpin->value());

        // Create stops
        auto startStop = schedule->findOrCreateStop(startStopEdit->text().trimmed(), startCoordEdit->text().trimmed());
        auto endStop = schedule->findOrCreateStop(endStopEdit->text().trimmed(), endCoordEdit->text().trimmed());

        // Get intermediate stops
        QVector<QSharedPointer<Stop>> intermediateStops;
        for (int i = 0; i < intermediateStopsList->count(); ++i) {
            QString itemText = intermediateStopsList->item(i)->text();
            QString name = itemText.left(itemText.indexOf(" ("));
            QString coord = itemText.mid(itemText.indexOf("(") + 1);
            coord = coord.left(coord.indexOf(")"));

            auto stopValidation = ValidationService::validateStopData(name, coord);
            if (!stopValidation.isValid) {
                QMessageBox::warning(this, "Ошибка",
                                     QString("Промежуточная остановка %1: %2").arg(name).arg(stopValidation.errorMessage));
                return;
            }

            intermediateStops.push_back(schedule->findOrCreateStop(name, coord));
        }

        // Parse travel times
        QVector<int> travelTimes;
        QStringList timeStrings = travelTimesEdit->text().split(",");
        for (const QString& timeStr : timeStrings) {
            bool ok;
            int time = timeStr.trimmed().toInt(&ok);
            if (ok && time > 0) {
                travelTimes.push_back(time);
            }
        }

        // Parse days using DayOfWeekService
        QStringList days = DayOfWeekService::parseDaysString(daysEdit->text());

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

        // Проверяем соответствие количества временных интервалов
        int totalStops = 1 + intermediateStops.size() + 1; // начальная + промежуточные + конечная
        auto travelTimeValidation = ValidationService::validateTravelTimes(travelTimes, totalStops - 1);
        if (!travelTimeValidation.isValid) {
            QMessageBox::warning(this, "Ошибка", travelTimeValidation.errorMessage);
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
