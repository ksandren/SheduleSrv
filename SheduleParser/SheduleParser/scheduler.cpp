#include "scheduler.h"
#include <QTextCodec>
#include <QQueue>
#include <QVector>
#include <QRegExp>
#include <QCoreApplication>

#define LCR_SCAN_SELECTORS 1
#define LCR_LOAD_SHEDULE_PAGES 2

Scheduler::Scheduler(QObject *parent) : QObject(parent)
{
    loader = new WebLoader("Windows-1251");
    connect(loader, SIGNAL(loadDone()), SLOT(onLoadDone()));
    provider = new DataBaseProvider();
    timer.setSingleShot(true);
    timer.setInterval(500);
    connect(&timer, SIGNAL(timeout()), SLOT(onTimeOut()));
    connect(this, SIGNAL(loadShedulePagesDone()),
            SLOT(onUpdateSheduleDataDone()));
    connect(this, SIGNAL(scanSelectorsDone()), SLOT(onScanSelectorsDone()));
    connect(loader, SIGNAL(onError(QString)), SLOT(onLoadError(QString)));
    loadErrorCount = 0;
}

Scheduler::~Scheduler()
{
    delete loader;
    delete provider;
}

void Scheduler::scanSelectors(const QPair<QString, int> &url)
{
    static QPair<QString, int> currentUrl; // текущий адрес
    static QString currentPost; // текущий запрос
    static QQueue<QString> postQueue; // очередь запросов
                                      // для обхода дерева раздела
    /* машина состояний: если аргумент - не пустая строка,
     * то это запуск нового сканирования.
     */
    if(!(url.second == -1))
    {
        currentUrl = url; // запоминаем текущий адрес
        currentPost = ""; // чистим запрос
    }
    /* машина состояний: если аргумент - пустая строка,
     * то это вызов от слота окончания загрузки.
     * У нас есть загруженный документ!
     */
    else
    {
        // проверяем наличие ошибок загрузки
        if(checkLoadError(__FUNCTION__))
        {
            // обьявляем метод заказчиком загрузки
            loadCustomer = LCR_SCAN_SELECTORS;
            // запрашиваем загрузку
            loader->loadPage(currentUrl.first, currentPost);
            return;
        }
        /* пытаемся извлечь данные селектора промежуточного
         * узла - параметры для дальнейшего обхода
         */
        QVector<QString> selectorData =
                Parser::parseSelector(loader->getText());
        /* машина состояний: если нет селектора, значит мы достигли листа,
         * все оставшиеся в очереди запросы являются листьями
         */
        if(selectorData.isEmpty())
        {
            // кэшируем загруженную страницу
            // расписания для отложенного парсинга
            QPair<QString, QString> p(currentPost, loader->getText());
            pageCache[currentUrl.second].push_back(p);

            /* помещаем все конечные точки (комбинации POST
             * параметров страниц расписания) в коллекцию
             */
            endPoints[currentUrl].push_back(currentPost);
            endPoints[currentUrl].append(postQueue.toVector());

            postQueue.clear(); // чистим очередь
            // создаем таблицу параметров POST для текущего раздела
            //provider->createPostStruct(currentUrl.second, currentPost);
            // создаем основную таблицу расписания
            //provider->createModePageTable(currentUrl);

            provider->createModeSessoinTable(currentUrl, currentPost);

            emit scanSelectorsDone();
            return;
        }
        // если селектор есть
        else for(int i = 1; i < selectorData.size(); i += 2)
        {
            /* создаем из каждого варианта новый запрос и
             * кладем его в очередь
             */
            QString tmp = "";
            if(!currentPost.isEmpty()) tmp = currentPost + "&";
            postQueue.enqueue(tmp + selectorData[0] + "=" + selectorData[i]);
            emit postQueueChange(postQueue.size());
            // отправляем параметры в БД
            provider->putSelectorParamToBase(currentUrl.second,
                                             selectorData[0],
                                             selectorData[i],
                                             selectorData[i + 1]);
        }
        // выбераем очередной запрос из очереди
        currentPost = postQueue.dequeue();
        emit postQueueChange(postQueue.size());
    }
    // обьявляем этот метод заказчиком загрузки
    loadCustomer = LCR_SCAN_SELECTORS;
    loader->loadPage(currentUrl.first, currentPost); // запрашиваем загрузку
}

void Scheduler::skipScanSelectors()
{
    endPoints = provider->loadEndpointsFromBase();
    loadShedulePages();
}

void Scheduler::updateSheduleData()
{

    static bool inProcess = false;
    static QQueue<QPair<QString, int> > urlQueue;
    if(!inProcess)
    {
        // Создаем временные таблицы
        provider->createMirrorTables();
        inProcess = true;
        urlQueue.clear();
        urlQueue.append(provider->getSheduleUrlListFromBase());
    }
    if(urlQueue.isEmpty())
    {
        inProcess = false;
        // отправляем конечные точки в БД
        provider->putEndPointsToBase(endPoints);
        loadShedulePages();
        return;
    }
    QPair<QString, int> url = urlQueue.dequeue();
    scanSelectors(url);
}

void Scheduler::onLoadDone()
{
    timer.start();
}

void Scheduler::onTimeOut()
{
    timer.stop();
    switch(loadCustomer)
    {
    case LCR_SCAN_SELECTORS:
    {
        QPair<QString, int> p("", -1);
        scanSelectors(p);
        break;
    }
    case LCR_LOAD_SHEDULE_PAGES:
        loadShedulePages();
        break;
    }
}

void Scheduler::onUpdateSheduleDataDone()
{
    qDebug() << "onUpdateSheduleDataDone: work done";
}

void Scheduler::onLoadError(QString errStr)
{
    qDebug() << errStr;
    loadErrorCount++;
    onTimeOut();
}

void Scheduler::onScanSelectorsDone()
{
    int epCount = 0;
    for(auto i : endPoints)
        epCount += i.size();
    emit endpointCountChange(epCount);
    updateSheduleData();
}

void Scheduler::loadShedulePages()
{
    // Флаг машины состояний "в процессе"
    static bool inProcess = false;
    // Флаг наличия кэша предварительно загруженных страниц
    static bool hasCache = false;
    // Текущий раздел
    static QMap<QPair<QString, int>, QVector<QString> >::iterator currentMode;
    // Текущая страница
    static QVector<QString>::iterator currentEndPoint;
    // Машина состояний: если цикл закгрузки не в процессе
    if(!inProcess)
    {
        // начинаем процесс
        inProcess = true;
        // итераторы на начало
        currentMode = endPoints.begin();
        currentEndPoint = currentMode.value().begin();

        // Если есть кэш
        if(!pageCache.isEmpty())
        {
            // запоминаем этот факт
            hasCache = true;
            currentEndPoint++;
            // парсим кэш
            parseCache();
        }
        else hasCache = false;
    }
    // Машина состояний: если цикл закгрузки в процессе
    // значит должна быть загружена страница
    else
    {
        // проверяем наличие ошибок загрузки
        if(checkLoadError(__FUNCTION__))
        {
            // обьявляем метод заказчиком загрузки
            loadCustomer = LCR_LOAD_SHEDULE_PAGES;
            // запрашиваем загрузку
            loader->loadPage(currentMode.key().first, *currentEndPoint);
            return;
        }
        // Парсим страницу
        provider->putShedulePageToBase(currentMode.key().second, *currentEndPoint,
                             Parser::parseTable(loader->getText()));
        // Парсим имена параметров, если это первая страница раздела
        if(currentEndPoint == currentMode.value().begin())
            provider->putParamNamesToBase(currentMode.key().second, *currentEndPoint,
                                          Parser::parseParamNames(loader->
                                                                  getText()));
        currentEndPoint++;
    }
    // Если конец раздела
    if(currentEndPoint == currentMode.value().end())
    {
        // переходим к следующему
        currentMode++;
        // если рпзделы кончились
        if(currentMode == endPoints.end())
        {
            // завершаем процесс
            inProcess = false;
            // обновляем таблицы
            provider->swapTables();
            emit loadShedulePagesDone();
            return;
        }
        // иначе встаем в начало раздела
        currentEndPoint = currentMode.value().begin();
        // если был кэш, пропускаем первую страницу
        if(hasCache) currentEndPoint++;
    }
    // обьявляем этот метод заказчиком загрузки
    loadCustomer = LCR_LOAD_SHEDULE_PAGES;
    // запрашиваем загрузку
    loader->loadPage(currentMode.key().first, *currentEndPoint);
    emit oneEndPointScanned();
}

bool Scheduler::checkLoadError(const char *functionName)
{
    if(loader->hasError())
    {
        // повторяем попытку загрузки до трех раз
        if(loadErrorCount < 3)
            return true;
        else
        {
            qFatal("ERROR: %s:%s", functionName,
                   " Cann`t load page for 3 attempts");
            QCoreApplication::exit(-1);
        }
    }
    else
        loadErrorCount = 0;
    return false;
}

void Scheduler::parseCache()
{
    QMap<int, QVector<QPair<QString, QString> > >::iterator it;
    for(it = pageCache.begin(); it != pageCache.end(); it++)
    {
        for(int i = 0; i < it.value().size(); i++)
        {
            provider->putShedulePageToBase(it.key(),
                                           it.value()[i].first,
                                           Parser::parseTable(
                                               it.value()[i].second));
            emit oneEndPointScanned();
        }
        provider->putParamNamesToBase(it.key(),
                                      it.value()[0].first,
                Parser::parseParamNames(it.value()[0].second));
    }
    pageCache.clear();
}

