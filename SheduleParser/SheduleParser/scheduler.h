#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "parser.h"
#include "webloader.h"
#include "databaseprovider.h"
#include <QObject>
#include <QTimer>
#include <QMap>
#include <QVector>
#include <QPair>

class Scheduler : public QObject
{
    Q_OBJECT
public:
    explicit Scheduler(QObject *parent = 0);
    ~Scheduler();
//===============================================================
/* Запуск построения и заполнения структуры таблиц расписания.
 * Cчитывает из БД адреса разделов сайта,
 * которые подлежат загрузке. Сканирует дерево расписания
 * каждого раздела. Загружает страницы расписания.
*/
    void updateSheduleData();
//===============================================================
/* Запуск сканирования раздела сайта.
 * Принимает адрес раздела, создает очередь
 * запросов. Обходит дерево сайта до
 * листьев - страниц расписания, сохраняя их
 * комбинации параметров POST запроса для дальнейшего обхода.
 * работает по принципу машины состояний, после первого
 * прохода циклически вызывается слотом завершения загрузки
 * очередного html документа.
 * Результатом работы является внесение в таблицы БД
 * параметров навигации по дереву расписания.
*/
    void scanSelectors(const QPair<QString, int> &url);
//===============================================================

    void skipScanSelectors();

signals:
    // Сканирование промежуточных узлов раздела завершено
    void scanSelectorsDone();
    // Все страницы расписания обработаны
    void loadShedulePagesDone();
    // Размер очереди запросов изменился
    void postQueueChange(int n);
    // Число известных страниц расписания изменилось
    void endpointCountChange(int n);
    // Очередная страница обработана
    void oneEndPointScanned();

private slots:
    void onLoadDone();
    void onTimeOut();
    void onUpdateSheduleDataDone();
    void onLoadError(QString errStr);
    void onScanSelectorsDone();

private:
    // Номер заказчика загрузки
    int loadCustomer;
    // Число неудачных попыток загрузки текущего даокумента
    int loadErrorCount;
    // Таймер ограничения частоты запросов
    QTimer timer;
    // Известные страницы расписания
    QMap<QPair<QString, int>, QVector<QString> > endPoints;
    // Страницы для отложенного парсинга
    QMap<int, QVector<QPair<QString, QString> > > pageCache;
    WebLoader *loader;
    DataBaseProvider *provider;
    // Скачать и обработать известные страницы расписания
    void loadShedulePages();
    bool checkLoadError(const char *functionName);
    void parseCache();
};

#endif // SCHEDULER_H
