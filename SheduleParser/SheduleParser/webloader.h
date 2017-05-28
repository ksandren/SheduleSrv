#ifndef WEBLOADER_H
#define WEBLOADER_H

#include <QObject>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QTimer>

/*!
  \brief Асинхронный загрузчик web страниц
*/

class WebLoader : public QObject
{
    Q_OBJECT
public:
    //! Принимает имя исходной кодировки для преобразования в юникод
    WebLoader(const char *encoding);
    ~WebLoader();
    //! Послать запрос
    void loadPage(const QString &url, const QString &post);
    //! Получить ответ
    QString getText();
    //! Ошибка при последней загрузке
    bool hasError();
signals:
    //! Загрузка завершена без ошибок
    void loadDone();
    //! Ошибка при загрузке
    void onError(QString errStr);
private:
    QByteArray buffer; // Приемрый буффер
    QTextCodec *codec; // Преобразователь кодировки
    QNetworkAccessManager *manager; // Менеджер достума к сети
    QTimer *timer; // Таймер времени ожидания
    QNetworkReply *currentReply; // Ответ хоста
    bool loadError; // Флаг наличия ошибки при последней загрузке
private slots:
    // Ответ получен
    void replyFinished(QNetworkReply*);
    // Время ожидания истекло
    void onTimeout();
};

#endif // WEBLOADER_H
