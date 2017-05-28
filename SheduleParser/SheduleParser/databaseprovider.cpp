#include "databaseprovider.h"
#include <QSqlQuery>
#include <QDebug>
#include <QRegExp>
#include <QSqlError>
#include <QCoreApplication>

#define EXIT_IF_EXEC_FAIL(x, y, r)\
if(!x.exec(y))\
{\
    qFatal("ERROR: %s", \
    x.lastError().text().toLocal8Bit().data());\
    QCoreApplication::exit(-1);\
    return r;\
}

DataBaseProvider::DataBaseProvider()
{
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setPort(3306);
    db.setDatabaseName("tt");
    db.setUserName("tt");
    db.setPassword("cern99");
    if(!db.open())
    {
        qDebug() << QString("db error ") + db.lastError().text();
        return;
    }
    else
        qDebug() << "connection to database. success!";
}

void DataBaseProvider::putShedulePageToBase(int shedule_mode, const QString &endPointPost, QVector<QString> data)
{
    QString str;
    QSqlQuery query;
    putPostDetailsToBase(shedule_mode, endPointPost);
    query.exec("SELECT LAST_INSERT_ID()");
    query.next();
    int id = query.value(0).toInt();
    QVector<QString> columns;
    str = "SELECT col_name FROM `mode_page_struct` WHERE mode_id=%1";
    str = str.arg(shedule_mode);
    EXIT_IF_EXEC_FAIL(query, str, );
    while(query.next())
        columns.push_back(query.value(0).toString());
    QRegExp re("(\\d{2}\\.\\d{2}\\.\\d{4})");
    QString date = "FAIL";
    for(int i = 0; i < data.size(); i++)
    {

        if(re.indexIn(data[i]) != -1)
        {
            date = re.cap(1);
            i++;
        }
        QString set = "`%1` = '%2', `";
        set = set.arg(columns[0]).arg(date);
        for(int j = 0; j < columns.size() - 1; j++, i++)
        {
            set += columns[j + 1] + "` = '" + data[i] + "', `";
        }
        i--;
        set = set.left(set.length() - 3);
        str = "UPDATE `mode_%1_session_mirror` SET %2 WHERE `id` = %3";
        str = str.arg(shedule_mode).arg(set).arg(id);
        EXIT_IF_EXEC_FAIL(query, str, );
    }
}

void DataBaseProvider::putParamNamesToBase(int shedule_mode, const QString &endPointPost, QVector<QString> data)
{
    QVector<QPair<QString, QString> > post = parseEndPoint(endPointPost);
    QSqlQuery query;
    QString str;
    if(post.size() != data.size())
    {
        qDebug() << "DataBaseProvider::putParamNamesToBase: postset and dataset don`t match";
        return;
    }
    for(int i = 0; i < post.size(); i++)
    {
        str = "UPDATE `param_mirror` SET text='%1' WHERE name='%2' and mode_id=%3";
        str = str.arg(data[i], post[i].first).arg(shedule_mode);
        EXIT_IF_EXEC_FAIL(query, str, );
    }
}
void DataBaseProvider::putSelectorParamToBase(int shedule_mode, const QString &paramName, const QString &value, const QString &text)
{
    QSqlQuery query;
    QString str;
    str = "SELECT id FROM `param_mirror` WHERE mode_id=%1 AND name='%2'";
    str = str.arg(shedule_mode).arg(paramName);
    EXIT_IF_EXEC_FAIL(query, str, );
    int param_id;
    if(query.size() == 1)
    {
        query.next();
        param_id = query.value(0).toInt();
    }
    else
    {
        str = "INSERT INTO `param_mirror` (mode_id, name) values(%1, '%2')";
        str = str.arg(shedule_mode).arg(paramName);
        EXIT_IF_EXEC_FAIL(query, str, );
        query.exec("SELECT LAST_INSERT_ID()");
        query.next();
        param_id = query.value(0).toInt();
    }
    str = "SELECT id FROM `value_mirror` WHERE param_id=%1 AND name='%2'";
    str = str.arg(param_id).arg(value);
    EXIT_IF_EXEC_FAIL(query, str, );
    if(query.size() < 1)
    {
        str = "INSERT INTO `value_mirror` (param_id, name, text) values(%1, '%2', '%3')";
        str = str.arg(param_id).arg(value, text);
        EXIT_IF_EXEC_FAIL(query, str, );
    }
}

void DataBaseProvider::putEndPointsToBase(QMap<QPair<QString, int>, QVector<QString> > &endPoints)
{
    QSqlQuery query;
    QString str;
    QMap<QPair<QString, int>, QVector<QString> >::iterator it;
    for(it = endPoints.begin(); it != endPoints.end(); it++)
    {
        int mode = it.key().second;
        for(int i = 0; i < it.value().size(); i++)
        {
            str = "INSERT INTO `endpoint_mirror` (mode_id, post) values(%1, '%2')";
            str = str.arg(mode).arg(it.value()[i]);
            EXIT_IF_EXEC_FAIL(query, str, );
        }
    }
}

StrIdList DataBaseProvider::getSheduleUrlListFromBase()
{
    StrIdList urlList;
    QSqlQuery query;
    EXIT_IF_EXEC_FAIL(query, "SELECT `id`, `url` FROM `shedule_mode` WHERE enable=1", urlList);
    while(query.next())
    {
        QPair<QString, int> p(query.value(1).toString(), query.value(0).toInt());
        urlList.push_back(p);
    }
    return urlList;
}

void DataBaseProvider::createMirrorTables()
{
    createMirror("param");
    createMirror("value");
    createMirror("endpoint");
}

void DataBaseProvider::swapTables()
{
    swapTable("param");
    swapTable("value");
    swapTable("endpoint");
    StrIdList urlList = getSheduleUrlListFromBase();
    for(auto i : urlList)
    {
        int mode = i.second;
        swapTable(QString("").sprintf("mode_%d_page", mode));
        swapTable(QString("").sprintf("mode_%d_post", mode));
    }
}

void DataBaseProvider::createPostStruct(int mode, const QString &post)
{
    QSqlQuery query;
    QString str = "DROP TABLE IF EXISTS `mode_%1_post`";
    str = str.arg(mode);
    EXIT_IF_EXEC_FAIL(query, str, );
    QVector<QPair<QString, QString> > postList = parseEndPoint(post);
    QString colList;
    for(auto i : postList)
        colList += i.first + " CHAR(50), ";
    str = "CREATE TABLE `mode_%1_post` (id INT NOT NULL AUTO_INCREMENT, endpoint_id INT NOT NULL, %2PRIMARY KEY (id))";
    str = str.arg(mode).arg(colList);
    EXIT_IF_EXEC_FAIL(query, str, );
    str = "mode_%1_post";
    str = str.arg(mode);
    createMirror(str);
}

void DataBaseProvider::createModePageTable(const QPair<QString, int> &url)
{
    int mode = url.second;
    QSqlQuery query;
    QString str = "SELECT col_name, col_len FROM `mode_page_struct` WHERE mode_id=%1";
    str = str.arg(mode);
    EXIT_IF_EXEC_FAIL(query, str, );
    QString colList;
    while(query.next())
    {
        colList += query.value(0).toString() + " CHAR(" + query.value(1).toString() + "), ";
    }
    str = "CREATE TABLE IF NOT EXISTS `mode_%1_page` (id INT NOT NULL AUTO_INCREMENT, mode_%1_post_id INT NOT NULL, %2PRIMARY KEY (id))";
    str = str.arg(mode).arg(colList);
    EXIT_IF_EXEC_FAIL(query, str, );
    str = "mode_%1_page";
    str = str.arg(mode);
    createMirror(str);
}

void DataBaseProvider::createModeSessoinTable(const QPair<QString, int> &url, const QString &post)
{
    int mode = url.second;
    QSqlQuery query;
    QString str = "SELECT col_name, col_len FROM `mode_page_struct` WHERE mode_id=%1";
    str = str.arg(mode);
    EXIT_IF_EXEC_FAIL(query, str, );
    QString colList;
    while(query.next())
    {
        colList += query.value(0).toString() + " CHAR(" + query.value(1).toString() + "), ";
    }
    QVector<QPair<QString, QString> > postList = parseEndPoint(post);
    for(auto i : postList)
        colList += i.first + " INT, ";
    str = "CREATE TABLE IF NOT EXISTS `mode_%1_session` "
          "(id INT NOT NULL AUTO_INCREMENT, %2PRIMARY KEY (id))";
    str = str.arg(mode).arg(colList);
    EXIT_IF_EXEC_FAIL(query, str, );
    str = "mode_%1_session";
    str = str.arg(mode);
    createMirror(str);
}

QMap<QPair<QString, int>, QVector<QString> > DataBaseProvider::loadEndpointsFromBase()
{
    StrIdList urlList = getSheduleUrlListFromBase();
    QMap<int, QString> urlById;
    for(auto i : urlList)
    {
        urlById[i.second] = i.first;
    }
    QSqlQuery query;
    QMap<QPair<QString, int>, QVector<QString> > result;
    EXIT_IF_EXEC_FAIL(query, "SELECT mode_id, post FROM endpoint_mirror", result);
    while(query.next())
    {
        int id = query.value(0).toInt();
        QPair<QString, int> p(urlById[id], id);
        result[p].push_back(query.value(1).toString());
    }
    return result;
}

void DataBaseProvider::createMirror(const QString &origin)
{
    QSqlQuery query;
    QString str;
    str = "DROP TABLE IF EXISTS `%1_mirror`";
    str = str.arg(origin);
    EXIT_IF_EXEC_FAIL(query, str, );
    str = "CREATE TABLE `%1_mirror` LIKE `%1`";
    str = str.arg(origin);
    EXIT_IF_EXEC_FAIL(query, str, );
}

void DataBaseProvider::swapTable(const QString &origin)
{
    QSqlQuery query;
    EXIT_IF_EXEC_FAIL(query, QString("TRUNCATE TABLE `") + origin + "`", );
    EXIT_IF_EXEC_FAIL(query, QString("INSERT INTO `") + origin + "` SELECT * FROM `" + origin + "_mirror`", );
    EXIT_IF_EXEC_FAIL(query, QString("DROP TABLE `") + origin + "_mirror`", );
}

QVector<QPair<QString, QString> > DataBaseProvider::parseEndPoint(const QString &post)
{
    QVector<QPair<QString, QString> > result;
    QRegExp re("(^|&)(\\w+)=([^&]+)");
    int index = 0;
    while((index = re.indexIn(post, index)) != -1)
    {
        index += re.matchedLength();
        QPair<QString, QString> pair(re.cap(2), re.cap(3));
        result.push_back(pair);
    }
    return result;
}

void DataBaseProvider::putPostDetailsToBase(int mode, const QString &post)
{
    QSqlQuery query;
    QVector<QPair<QString, QString> > postList = parseEndPoint(post);
    QString paramList, valList = "'", str;
    for(auto i : postList)
    {
        paramList += i.first + ", ";
        str = "SELECT id FROM `value_mirror` WHERE name='%1' AND param_id=("
              "SELECT id FROM `param_mirror` WHERE name='%2' AND mode_id = %3)";
        str = str.arg(i.second, i.first).arg(mode);
        EXIT_IF_EXEC_FAIL(query, str, );
        query.next();
        valList += query.value(0).toString() + "', '";
    }
    paramList = paramList.left(paramList.length() - 2);
    valList = valList.left(valList.length() - 3);
    str = "INSERT INTO `mode_%1_session_mirror` (%2) values(%4)";
    str = str.arg(mode).arg(paramList).arg(valList);
    EXIT_IF_EXEC_FAIL(query, str, );
}

