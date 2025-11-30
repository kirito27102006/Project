#include "RouteDetailsDialog.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QInputDialog>

RouteDetailsDialog::RouteDetailsDialog(TransportSchedule* transportSchedule, const Schedule& schedule, QWidget *parent)
    : QDialog(parent), transportSchedule(transportSchedule), currentSchedule(schedule),
    originalRoute(schedule.getRoute()), isEditing(false) {
    setupUI(schedule.getRoute(), schedule.getStartTime());
    setWindowTitle(QString("Маршрут %1 №%2").arg(schedule.getRoute().getTransport().getType().getName()).arg(schedule.getRoute().getRouteNumber()));
    setMinimumSize(800, 600);
}

void RouteDetailsDialog::setupUI(const Route& route, const TimeTransport& startTime) {
    auto* mainLayout = new QVBoxLayout(this);

    // Информация о маршруте
    infoLabel = new QLabel(
        QString("<b>%1 №%2</b><br>"
                "Начало: %3 | Дни: %4")
            .arg(route.getTransport().getType().getName())
            .arg(route.getRouteNumber())
            .arg(startTime.toString())
            .arg(route.getDays().join(", ")));
    mainLayout->addWidget(infoLabel);

    // Панель редактирования (скрыта по умолчанию)
    editPanel = new QWidget;
    auto* editLayout = new QHBoxLayout(editPanel);
    editPanel->setVisible(false);

    editLayout->addWidget(new QLabel("Время начала:"));
    startHourSpin = new QSpinBox;
    startHourSpin->setRange(0, 23);
    startHourSpin->setValue(startTime.hours);
    startHourSpin->setSuffix(" ч");
    connect(startHourSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &RouteDetailsDialog::updateArrivalTimes);
    editLayout->addWidget(startHourSpin);

    startMinuteSpin = new QSpinBox;
    startMinuteSpin->setRange(0, 59);
    startMinuteSpin->setValue(startTime.minutes);
    startMinuteSpin->setSuffix(" мин");
    connect(startMinuteSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &RouteDetailsDialog::updateArrivalTimes);
    editLayout->addWidget(startMinuteSpin);

    editLayout->addWidget(new QLabel("Дни работы:"));
    daysCombo = new QComboBox;
    daysCombo->addItems(QStringList() << "пн" << "вт" << "ср" << "чт" << "пт" << "сб" << "вс");
    daysCombo->setCurrentText(route.getDays().join(","));
    daysCombo->setEditable(true);
    editLayout->addWidget(daysCombo);

    editLayout->addStretch();

    addStopButton = new QPushButton("+ Добавить остановку");
    connect(addStopButton, &QPushButton::clicked, this, &RouteDetailsDialog::addStop);
    editLayout->addWidget(addStopButton);

    removeStopButton = new QPushButton("- Удалить остановку");
    connect(removeStopButton, &QPushButton::clicked, this, &RouteDetailsDialog::removeStop);
    editLayout->addWidget(removeStopButton);

    mainLayout->addWidget(editPanel);

    // Таблица остановок
    stopsTable = new QTableWidget;
    stopsTable->setColumnCount(4);
    stopsTable->setAlternatingRowColors(true);

    QStringList headers;
    headers << "№" << "Остановка" << "Время прибытия" << "Время движения (мин)";
    stopsTable->setHorizontalHeaderLabels(headers);

    stopsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    stopsTable->horizontalHeader()->setStretchLastSection(true);
    stopsTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    stopsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    populateStopsTable();
    mainLayout->addWidget(stopsTable);

    // Кнопки управления
    auto* buttonLayout = new QHBoxLayout;

    editButton = new QPushButton("Редактировать");
    connect(editButton, &QPushButton::clicked, this, &RouteDetailsDialog::editRoute);

    saveButton = new QPushButton("Сохранить");
    saveButton->setVisible(false);
    connect(saveButton, &QPushButton::clicked, this, &RouteDetailsDialog::saveRoute);

    cancelButton = new QPushButton("Отмена");
    cancelButton->setVisible(false);
    connect(cancelButton, &QPushButton::clicked, this, &RouteDetailsDialog::cancelEdit);

    auto* closeButton = new QPushButton("Закрыть");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    mainLayout->addLayout(buttonLayout);
}

void RouteDetailsDialog::populateStopsTable() {
    const auto& route = currentSchedule.getRoute();
    auto stops = route.getStops();
    auto travelTimes = route.getTravelTimes();

    stopsTable->setRowCount(stops.size());

    for (auto i = 0; i < stops.size(); ++i) {
        const auto& stop = stops[i];

        // Номер остановки
        auto* numberItem = new QTableWidgetItem(QString::number(i + 1));
        stopsTable->setItem(i, 0, numberItem);

        // Название остановки
        auto stopName = stop.getName();
        if (i == 0) {
            stopName = "▶ " + stopName + " (начало)";
        } else if (i == stops.size() - 1) {
            stopName = "⏹ " + stopName + " (конец)";
        } else {
            stopName = "● " + stopName;
        }
        auto* nameItem = new QTableWidgetItem(stopName);
        stopsTable->setItem(i, 1, nameItem);

        // Время прибытия
        auto* arrivalItem = new QTableWidgetItem(stop.arrivalTime.toString());
        stopsTable->setItem(i, 2, arrivalItem);

        // Время движения
        QTableWidgetItem* travelItem;
        if (i > 0 && i - 1 < travelTimes.size()) {
            travelItem = new QTableWidgetItem(QString::number(travelTimes[i - 1]));
        } else {
            travelItem = new QTableWidgetItem("0");
        }
        stopsTable->setItem(i, 3, travelItem);

        // Устанавливаем флаги редактирования
        for (auto col = 0; col < 4; ++col) {
            auto* item = stopsTable->item(i, col);
            if (item) {
                item->setTextAlignment(Qt::AlignCenter);
                if (!isEditing) {
                    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                } else {
                    // В режиме редактирования разрешаем редактировать только названия промежуточных остановок и время движения
                    if (col == 1 && i > 0 && i < stops.size() - 1) {
                        item->setFlags(item->flags() | Qt::ItemIsEditable);
                        // Убираем символы ● для редактирования
                        auto text = item->text();
                        if (text.startsWith("● ")) {
                            item->setText(text.mid(2));
                        }
                    } else if (col == 3 && i > 0) {
                        item->setFlags(item->flags() | Qt::ItemIsEditable);
                    } else {
                        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                    }
                }
            }
        }
    }

    stopsTable->resizeColumnsToContents();
}

void RouteDetailsDialog::editRoute() {
    isEditing = true;

    // Показываем панель редактирования
    editPanel->setVisible(true);

    // Показываем/скрываем кнопки
    editButton->setVisible(false);
    saveButton->setVisible(true);
    cancelButton->setVisible(true);

    // Обновляем таблицу с включенным редактированием
    populateStopsTable();

    setWindowTitle(windowTitle() + " [РЕДАКТИРОВАНИЕ]");
}

void RouteDetailsDialog::addStop() {
    auto currentRow = stopsTable->currentRow();
    if (currentRow == -1) currentRow = stopsTable->rowCount() - 1;

    // Нельзя добавлять после конечной остановки
    if (currentRow >= stopsTable->rowCount() - 1) {
        QMessageBox::information(this, "Информация", "Невозможно добавить остановку после конечной");
        return;
    }

    // Запрашиваем название новой остановки
    bool ok;
    auto stopName = QInputDialog::getText(this, "Новая остановка",
                                          "Введите название новой остановки:",
                                          QLineEdit::Normal, "", &ok);
    if (!ok || stopName.isEmpty()) return;

    // Запрашиваем время движения до новой остановки
    auto travelTime = QInputDialog::getInt(this, "Время движения",
                                           "Время движения до новой остановки (мин):",
                                           5, 1, 120, 1, &ok);
    if (!ok) return;

    // Вставляем новую строку
    auto newRow = currentRow + 1;
    stopsTable->insertRow(newRow);

    // Номер остановки
    stopsTable->setItem(newRow, 0, new QTableWidgetItem(QString::number(newRow + 1)));

    // Название остановки
    stopsTable->setItem(newRow, 1, new QTableWidgetItem("● " + stopName));

    // Время прибытия (пока пустое, пересчитается позже)
    stopsTable->setItem(newRow, 2, new QTableWidgetItem("00:00"));

    // Время движения
    stopsTable->setItem(newRow, 3, new QTableWidgetItem(QString::number(travelTime)));

    // Устанавливаем флаги редактирования для новой строки
    for (auto col = 0; col < 4; ++col) {
        auto* item = stopsTable->item(newRow, col);
        if (item) {
            item->setTextAlignment(Qt::AlignCenter);
            if (col == 1 || col == 3) {
                item->setFlags(item->flags() | Qt::ItemIsEditable);
            } else {
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            }
        }
    }

    // Обновляем нумерацию
    for (auto i = newRow + 1; i < stopsTable->rowCount(); ++i) {
        stopsTable->item(i, 0)->setText(QString::number(i + 1));
    }

    // Пересчитываем время прибытия
    calculateArrivalTimes();
}

void RouteDetailsDialog::removeStop() {
    auto currentRow = stopsTable->currentRow();
    if (currentRow == -1) {
        QMessageBox::information(this, "Информация", "Выберите остановку для удаления");
        return;
    }

    // Нельзя удалить начальную или конечную остановку
    if (currentRow == 0 || currentRow == stopsTable->rowCount() - 1) {
        QMessageBox::information(this, "Информация", "Нельзя удалить начальную или конечную остановку");
        return;
    }

    auto reply = QMessageBox::question(this, "Подтверждение",
                                       "Вы уверены, что хотите удалить эту остановку?",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) return;

    stopsTable->removeRow(currentRow);

    // Обновляем нумерацию
    for (auto i = currentRow; i < stopsTable->rowCount(); ++i) {
        stopsTable->item(i, 0)->setText(QString::number(i + 1));
    }

    // Пересчитываем время прибытия
    calculateArrivalTimes();
}

void RouteDetailsDialog::updateArrivalTimes() {
    if (!isEditing) return;
    calculateArrivalTimes();
}

void RouteDetailsDialog::calculateArrivalTimes() {
    if (!isEditing) return;

    TimeTransport currentTime(startHourSpin->value(), startMinuteSpin->value());

    for (auto i = 0; i < stopsTable->rowCount(); ++i) {
        auto arrivalTime = calculateArrivalTime(i);
        stopsTable->item(i, 2)->setText(arrivalTime.toString());
    }
}

TimeTransport RouteDetailsDialog::calculateArrivalTime(int stopIndex) {
    if (stopIndex == 0) {
        return TimeTransport(startHourSpin->value(), startMinuteSpin->value());
    }

    auto previousTime = calculateArrivalTime(stopIndex - 1);

    // Получаем время движения от предыдущей остановки
    auto travelTime = 0;
    auto* travelItem = stopsTable->item(stopIndex, 3);
    if (travelItem) {
        travelTime = travelItem->text().toInt();
    }

    return previousTime.addMinutes(travelTime);
}

void RouteDetailsDialog::saveRoute() {
    try {
        // Собираем данные из таблицы
        QVector<QSharedPointer<Stop>> stops;
        QVector<int> travelTimes;

        for (auto row = 0; row < stopsTable->rowCount(); ++row) {
            auto stopName = stopsTable->item(row, 1)->text();

            // Убираем символы оформления если они есть
            if (stopName.startsWith("▶ ") || stopName.startsWith("● ") || stopName.startsWith("⏹ ")) {
                auto spaceIndex = stopName.indexOf(" ");
                if (spaceIndex != -1) {
                    stopName = stopName.mid(spaceIndex + 1);
                }
            }

            // Убираем текст в скобках
            auto bracketIndex = stopName.indexOf(" (");
            if (bracketIndex != -1) {
                stopName = stopName.left(bracketIndex);
            }

            // Создаем или находим остановку
            auto stop = transportSchedule->findOrCreateStop(stopName.trimmed());
            stops.append(stop);

            // Время движения (для всех кроме первой строки)
            if (row > 0) {
                bool ok;
                auto travelTime = stopsTable->item(row, 3)->text().toInt(&ok);
                if (ok && travelTime > 0) {
                    travelTimes.append(travelTime);
                } else {
                    QMessageBox::warning(this, "Ошибка",
                                         QString("Некорректное время движения для строки %1").arg(row + 1));
                    return;
                }
            }
        }

        // Проверяем, что есть хотя бы 2 остановки
        if (stops.size() < 2) {
            QMessageBox::warning(this, "Ошибка", "Маршрут должен содержать хотя бы 2 остановки");
            return;
        }

        // Проверяем соответствие количества остановок и времен движения
        if (travelTimes.size() != stops.size() - 1) {
            QMessageBox::warning(this, "Ошибка",
                                 "Количество времен движения должно быть на 1 меньше количества остановок");
            return;
        }

        // Парсим дни работы
        auto days = daysCombo->currentText().split(",", Qt::SkipEmptyParts);
        for (auto& day : days) {
            day = day.trimmed();
        }

        if (days.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Укажите дни работы маршрута");
            return;
        }

        // Создаем новый маршрут
        const auto& originalTransport = originalRoute.getTransport();
        Route newRoute(originalTransport, stops[0], stops[stops.size() - 1]);

        // Добавляем промежуточные остановки
        const int intermediateStopCount = stops.size() >= 2 ? static_cast<int>(stops.size()) - 2 : 0;
        for (int i = 0; i < intermediateStopCount; ++i) {
            const int stopIndex = i + 1;
            newRoute.addStop(stops[stopIndex], travelTimes[i]);
        }

        // Добавляем время до конечной остановки
        newRoute.addFinalTravelTime(travelTimes[travelTimes.size() - 1]);

        // Сохраняем дни работы
        newRoute.setDays(days);

        // Создаем время начала
        TimeTransport startTime(startHourSpin->value(), startMinuteSpin->value());

        // Рассчитываем время прибытия
        newRoute.calculateArrivalTimes(startTime);

        // Создаем новое расписание
        Schedule newSchedule(newRoute, startTime);

        // Обновляем маршрут в расписании
        transportSchedule->removeRoute(originalRoute.getRouteNumber());

        // Добавляем обновленный маршрут
        transportSchedule->addRoute(originalTransport, stops[0], stops[stops.size() - 1],
                                    stops.mid(1, stops.size() - 2), travelTimes,
                                    days, startTime);

        // Обновляем текущее расписание
        currentSchedule = newSchedule;
        originalRoute = newRoute;

        QMessageBox::information(this, "Успех", "Маршрут успешно сохранен");
        cancelEdit();

    } catch (const std::invalid_argument& e) {
        QMessageBox::critical(this, "Ошибка ввода данных",
                              QString("Ошибка при сохранении маршрута: %1").arg(e.what()));
    } catch (const std::out_of_range& e) {
        QMessageBox::critical(this, "Ошибка диапазона",
                              QString("Ошибка при сохранении маршрута: %1").arg(e.what()));
    } catch (const std::logic_error& e) {
        QMessageBox::critical(this, "Логическая ошибка",
                              QString("Ошибка при сохранении маршрута: %1").arg(e.what()));
    } catch (const std::runtime_error& e) {
        QMessageBox::critical(this, "Ошибка выполнения",
                              QString("Ошибка при сохранении маршрута: %1").arg(e.what()));
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Неизвестная ошибка",
                              QString("Ошибка при сохранении маршрута: %1").arg(e.what()));
    }
}

void RouteDetailsDialog::cancelEdit() {
    isEditing = false;

    // Скрываем панель редактирования
    editPanel->setVisible(false);

    // Восстанавливаем оригинальные данные
    populateStopsTable();

    // Показываем/скрываем кнопки
    editButton->setVisible(true);
    saveButton->setVisible(false);
    cancelButton->setVisible(false);

    // Восстанавливаем заголовок
    setWindowTitle(QString("Маршрут %1 №%2")
                       .arg(originalRoute.getTransport().getType().getName())
                       .arg(originalRoute.getRouteNumber()));
}
