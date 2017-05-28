#ifndef DATABASEPROVIDER_H
#define DATABASEPROVIDER_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QList>
#include <QPair>
#include <QSqlDatabase>

typedef QList<QPair<QString, int> > StrIdList;


/*!
 * \brief Интерфейс базы данных
 * Реализует методв доступа и упроавления БД проекта
 */
class DataBaseProvider
{
public:
    DataBaseProvider();
    //! Записать данные одной страницы расписания
    void putShedulePageToBase(int shedule_mode,
                              const QString &endPointPost,
                              QVector<QString> data);
    //! Записать параметры навигации и их значения
    void putSelectorParamToBase(int shedule_mode,
                                const QString &paramName,
                                const QString &value,
                                const QString &text);
    //! Добавить текстовые имена параметров навигации
    void putParamNamesToBase(int shedule_mode,
                             const QString &endPointPost,
                             QVector<QString> data);
    //! Записать строки POST запросов ко всем конечным страницам
    void putEndPointsToBase(QMap<QPair<QString, int>, QVector<QString> > &endPoints);
    //! Извлечь адреса разделов, подлежащих парсингу
    StrIdList getSheduleUrlListFromBase();
    //! Создать временные копии таблиц
    void createMirrorTables();
    //! Заменить данные основных таблиц на новые
    void swapTables();
    //! Сформировать таблицу набора параметров раздела
    void createPostStruct(int mode, const QString &post);
    //! Сформировать основную таблицу расписания раздела
    void createModePageTable(const QPair<QString, int> &url);

    //! Сформировать основную таблицу расписания раздела
    void createModeSessoinTable(const QPair<QString, int> &url, const QString &post);

    QMap<QPair<QString, int>, QVector<QString> > loadEndpointsFromBase();
private:
    QSqlDatabase db;
    //QMap<QString, QString> definitions;
    // Создать временную копию
    void createMirror(const QString &origin);
    // Обновить данные и удалить временную таблицу
    void swapTable(const QString &origin);
    // Распарсить строку параметров POST
    QVector<QPair<QString, QString> > parseEndPoint(const QString &post);
    // Записать детализацию параметров POST в таблицу набора параметров
    void putPostDetailsToBase(int mode, const QString &post);
};

#endif // DATABASEPROVIDER_H
