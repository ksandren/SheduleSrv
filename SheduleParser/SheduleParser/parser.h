#ifndef PARSER_H
#define PARSER_H
#include <QString>
#include <QVector>

/*!
    \brief Набор методов для парсинга html

    Все методы данного класса статические. Создание экземпляра невозможно.
*/
class Parser
{
public:
    //! Возвращает содержимое выпадающнго сприска со страницы
    static QVector<QString> parseSelector(QString html);
    //! Возвращает содержимое таблицы с переданной страницы
    static QVector<QString> parseTable(QString html);
    //! Возвращает строковые имена параметров post запроса
    static QVector<QString> parseParamNames(QString html);
private:
    Parser();
    //! Вырезает отдельный html тэг, по имени и части строки атрибутов
    static QString extractXmlObject(QString source, QString tag, QString param);
    //! Вырезает тэг select, результат является xml-валидной строкой
    static QString extractSelector(QString html);
    //! Вырезает и нормализует таблицы расписания,
    //! результат является xml-валидной строкой
    static QString extractTable(QString html);
    //! Вырезает параграфы, содержащие имена параметров
    static QString extractParamNames(QString html);
};

#endif // PARSER_H
