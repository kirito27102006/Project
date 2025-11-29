#include "NotificationService.h"

NotificationService::NotificationService(QObject* parent)
    : QObject(parent), lastAnswer(false)
{
}

void NotificationService::showInfo(const QString& title, const QString& message)
{
    emit notificationRequested("info", title, message);
    QMessageBox::information(nullptr, title, message);
}

void NotificationService::showWarning(const QString& title, const QString& message)
{
    emit notificationRequested("warning", title, message);
    QMessageBox::warning(nullptr, title, message);
}

void NotificationService::showError(const QString& title, const QString& message)
{
    emit notificationRequested("error", title, message);
    QMessageBox::critical(nullptr, title, message);
}

void NotificationService::showSuccess(const QString& title, const QString& message)
{
    emit notificationRequested("success", title, message);
    QMessageBox::information(nullptr, title, message);
}

void NotificationService::showQuestion(const QString& title, const QString& message)
{
    emit notificationRequested("question", title, message);
    QMessageBox::StandardButton reply = QMessageBox::question(nullptr, title, message);
    lastAnswer = (reply == QMessageBox::Yes);
    emit questionAnswered(lastAnswer);
}
