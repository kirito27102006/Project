#ifndef NOTIFICATIONSERVICE_H
#define NOTIFICATIONSERVICE_H

#include <QString>
#include <QObject>
#include <QMessageBox>

class NotificationService : public QObject
{
    Q_OBJECT

public:
    explicit NotificationService(QObject* parent = nullptr);

    void showInfo(const QString& title, const QString& message);
    void showWarning(const QString& title, const QString& message);
    void showError(const QString& title, const QString& message);
    void showSuccess(const QString& title, const QString& message);
    void showQuestion(const QString& title, const QString& message);

    bool getLastAnswer() const { return lastAnswer; }

signals:
    void notificationRequested(const QString& type, const QString& title, const QString& message);
    void questionAnswered(bool answer);

private:
    bool lastAnswer = false;
};

#endif // NOTIFICATIONSERVICE_H
