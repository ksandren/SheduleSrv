#include "webloader.h"
#include <QTextCodec>

WebLoader::WebLoader(const char *encoding)
{
    codec = QTextCodec::codecForName(encoding);
    timer = new QTimer(this);
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    loadError = false;
    timer->setInterval(30000);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

WebLoader::~WebLoader()
{
    delete manager;
    delete timer;
}

void WebLoader::loadPage(const QString &url, const QString &post)
{
    currentReply = manager->post(QNetworkRequest(QUrl(url.toStdString().c_str())),
                                 QByteArray(codec->fromUnicode(post).data()));
    loadError = false;
    timer->start();
}

QString WebLoader::getText()
{
    return codec->toUnicode(buffer);
}

bool WebLoader::hasError()
{
    return loadError;
}

void WebLoader::replyFinished(QNetworkReply *reply)
{
    timer->stop();
    if(reply->error() != QNetworkReply::NoError)
    {
        loadError = true;
        emit onError(QString("WebLoader::replyFinished(): ") + reply->errorString());
    }
    else
    {
        buffer.clear();
        buffer = reply->readAll();
        reply->deleteLater();
        emit loadDone();
    }
}

void WebLoader::onTimeout()
{
    loadError = true;
    emit onError("WebLoader::onTimeout(): Время ожидания превышено");
    currentReply->abort();
}

