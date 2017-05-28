#include "parser.h"
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QRegExp>
#include <QDebug>

QVector<QString> Parser::parseSelector(QString html)
{
    QXmlStreamReader xml(extractSelector(html));
    QVector<QString> result;
    // находим начало тэга select
    while(!xml.atEnd() && !xml.hasError())
    {
        xml.readNext();
        if(xml.tokenType() != QXmlStreamReader::StartElement)
            continue;
        if(xml.name() != "select")
            continue;
        else
            break;
    }
    // если нет такого тэга, возвращаем пустой результат
    if(xml.atEnd() || xml.hasError())
        return result;
    // читаем атрибут id или name - он является именем параметра post запроса
    QXmlStreamAttributes attributes = xml.attributes();
    if(attributes.hasAttribute("id"))
        result.push_back(attributes.value("id").toString());
    else if(attributes.hasAttribute("name"))
        result.push_back(attributes.value("name").toString());
    else return result;
    xml.readNext();
    // читаем содержимое select
    while(!xml.atEnd() && !xml.hasError() &&
          !(xml.tokenType() == QXmlStreamReader::EndElement &&
            xml.name() == "select"))
    {
        if(xml.tokenType() == QXmlStreamReader::StartElement)
        {
            attributes = xml.attributes();
            QString value;
            // находим очередной тэг option и получаем из него
            // атрибут value, являющийся значением параметра post запроса.
            // и текст внутри тэга, отображаемый пользователю
            if(xml.name() == "option" &&
                    attributes.hasAttribute("value") &&
                    !(value = attributes.value("value").toString()).isEmpty())
            {
                result.push_back(value);
                xml.readNext();
                result.push_back(xml.text().toString());
            }
        }
        xml.readNext();
    }
    return result;
}

QVector<QString> Parser::parseTable(QString html)
{
    QXmlStreamReader xml(extractTable(html));
    QVector<QString> result;
    QXmlStreamAttributes attributes;
    // в теле тэга table нас интересуют все тэги td
    // у которых нет атрибута align
    while(!xml.atEnd() && !xml.hasError())
    {
        xml.readNext();
        if(xml.tokenType() != QXmlStreamReader::StartElement)
            continue;
        attributes = xml.attributes();
        if(!attributes.hasAttribute("align"))
        {
            if(xml.name() == "td")
            {
                // при атрибуте colspan содержимое тэга td обернуто в <b></b>
                if(attributes.hasAttribute("colspan"))
                {
                    xml.readNextStartElement();
                    xml.readNext();
                    result.push_back(xml.text().toString());
                }
                else
                {
                    xml.readNext();
                    result.push_back(xml.text().toString());
                }
            }
        }
    }
    return result;
}

QVector<QString> Parser::parseParamNames(QString html)
{
    QXmlStreamReader xml(extractParamNames(html));
    QVector<QString> result;
    while(!xml.atEnd() && !xml.hasError())
    {
        xml.readNextStartElement();
        if(xml.name() == "p")
        {
            xml.readNext();
            QString tmp = xml.text().toString();
            if(tmp[0] == '\n') continue;
            result.push_back(tmp.left(tmp.length() - 2));
        }
    }
    return result;
}

QString Parser::extractXmlObject(QString source, QString tag, QString paramStr)
{
    QString beginsOn = QString("") + "<" + tag + paramStr;
    QString endsOn = QString("") + "</" + tag + ">";
    int index = source.indexOf(beginsOn);
    if(index == -1)
        return "";
        //throw QString("QString Parser::extractXmlObject");
    int len = source.indexOf(endsOn, index) - index;
    return source.mid(index, len + endsOn.length());
}

QString Parser::extractSelector(QString html)
{
    return extractXmlObject(html, "select", "");
}

QString Parser::extractTable(QString html)
{
    QString table = extractXmlObject(html, "table", " width");
    if(table.isEmpty()) return "";
    // приводим документ к нормальному xml

    // аттрибут colspan встречается без ковычек при значении
    QRegExp re( "colspan=\\d" );
    table = table.replace(re, "colspan=\"0\"");
    // отрезаем лишнее
    int index = table.indexOf("<td colspan=\"0\">");
    // восстанавливаем целостность
    table = QString("<table><tr>") + table.right(table.length() - index);
    // такой тэг проще удалить
    table = table.replace("<br>", "");
    // снова чиним ковычки
    return table.replace("align=center", "align=\"center\"");
}

QString Parser::extractParamNames(QString html)
{
    QString tmp = "<td colspan=\"5\">";
    int index = html.indexOf(tmp) + tmp.length();
    int end = html.indexOf("<p><a", index);
    tmp = "<r>";
    return tmp + html.mid(index, end - index) + "</r>";
}

