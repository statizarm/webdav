#include "cpp/include/webdav.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QAuthenticator>

WebDavManager::WebDavManager (QObject *parent) :
    QNetworkAccessManager (parent),
    m_username (),
    m_password (),
    m_rootPath (),
    m_baseUrl (),
    m_connectionType (WebDavManager::ConnectionType::HTTP),
    m_authenticatorLastReply (nullptr)
{
    qRegisterMetaType<QNetworkReply *>("QNetworkReply*");
    connect (this, SIGNAL (finished (QNetworkReply *)), this, SLOT (replyFinished (QNetworkReply *)));
    connect (this, SIGNAL (authenticationRequired (QNetworkReply *, QAuthenticator *)), this, SLOT (provideAuthentication (QNetworkReply *, QAuthenticator *)));
}

WebDavManager::~WebDavManager()
{
}

const QString &
WebDavManager::username () const
{
    return m_username;
}

const QString &
WebDavManager::password () const
{
    return m_password;
}

const QString &
WebDavManager::rootPath () const
{
    return m_rootPath;
}

const QUrl &
WebDavManager::baseUrl () const
{
    return m_baseUrl;
}

WebDavManager::ConnectionType
WebDavManager::connectionType () const
{
    return m_connectionType;
}

int
WebDavManager::port () const
{
    return m_baseUrl.port ();
}

const QString
WebDavManager::hostName () const
{
    return m_baseUrl.host ();
}

bool
WebDavManager::isSSL () const
{
    return (m_connectionType == ConnectionType::HTTPS);
}

void
WebDavManager::setConnectionSettings (const ConnectionType type,
                                      const QString &hostname,
                                      const QString &rootPath,
                                      const QString &username,
                                      const QString &password,
                                      int port)
{
    m_rootPath = rootPath;

    if ((m_rootPath.size () > 0) && (m_rootPath.endsWith ('/')))
    {
        m_rootPath.chop (1);
    }

    QString uriScheme;

    switch (type)
    {
        case HTTP:
            uriScheme = "http";
            break;
        case HTTPS:
            uriScheme = "https";
            break;
    }

    m_connectionType = type;

    m_baseUrl.setScheme (uriScheme);
    m_baseUrl.setHost (hostname);
    m_baseUrl.setPort (port);

    if (port != 0)
    {
        if (! (((port == 80) && (m_connectionType == HTTP)) ||
               ((port == 443) && (m_connectionType == HTTPS))))
        {
            m_baseUrl.setPort (port);
        }
    }

    m_username = username;
    m_password = password;
}

void
WebDavManager::replyReadyRead ()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *> (QObject::sender());

    if (reply->bytesAvailable() < 256000)
    {
        return;
    }

    QIODevice *dataIO = m_inDataDevices.value (reply, nullptr);

    if (dataIO == nullptr)
    {
        return;
    }
    dataIO->write (reply->readAll());
}

void
WebDavManager::replyFinished(QNetworkReply *reply)
{
    disconnect (reply, SIGNAL (readyRead()), this, SLOT (replyReadyRead()));
    disconnect (reply, SIGNAL (error (QNetworkReply::NetworkError)),
                this, SLOT (replyError (QNetworkReply::NetworkError)));

    QIODevice *dataIO = m_inDataDevices.value (reply, nullptr);

    if (dataIO != nullptr)
    {
        dataIO->write (reply->readAll());

        static_cast<QFile *> (dataIO)->flush();

        dataIO->close ();

        delete dataIO;
    }

    m_inDataDevices.remove (reply);

    QMetaObject::invokeMethod (this, "replyDeleteLater", Qt::QueuedConnection,
                               Q_ARG (QNetworkReply *, reply));
}

void
WebDavManager::replyDeleteLater (QNetworkReply *reply)
{
    QIODevice *outDataDevice = m_outDataDevices.value (reply, nullptr);

    if (outDataDevice != nullptr)
    {
        outDataDevice->deleteLater();
    }
    m_outDataDevices.remove (reply);
}

void
WebDavManager::replyError (QNetworkReply::NetworkError)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *> (QObject::sender ());

    if (reply == nullptr)
    {
        return;
    }

    if (reply->error() == QNetworkReply::OperationCanceledError)
    {
        QIODevice *dataIO = m_inDataDevices.value (reply, 0);
        if (dataIO != nullptr)
        {
            delete dataIO;
            m_inDataDevices.remove (reply);
        }

        return;
    }

    emit errorChanged (reply->errorString ());
}

void
WebDavManager::provideAuthentication (QNetworkReply *reply, QAuthenticator *authenticator)
{
    if (reply == m_authenticatorLastReply)
    {
        reply->abort ();
        emit errorChanged ("Chech WebDav share settings");

        reply->deleteLater ();

        reply = nullptr;
    }

    m_authenticatorLastReply = reply;

    authenticator->setUser (m_username);
    authenticator->setPassword (m_password);
}

QString
WebDavManager::absolutePath (const QString &relPath)
{
    return QString (m_rootPath + relPath);
}

QNetworkReply *
WebDavManager::createRequest (const QString &method, QNetworkRequest &req, QIODevice *outGoingData)
{
    if (outGoingData != nullptr && outGoingData->size() != 0)
    {
        req.setHeader(QNetworkRequest::ContentLengthHeader, outGoingData->size ());
        req.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml; charset=utf-8");
    }

    return sendCustomRequest (req, method.toLatin1 (), outGoingData);
}

QNetworkReply *
WebDavManager::createRequest (const QString &method, QNetworkRequest &req, const QByteArray &data)
{
    QBuffer *dataIO = new QBuffer;
    dataIO->setData (data);
    dataIO->open (QIODevice::ReadOnly);

    QNetworkReply *reply = createRequest (method, req, dataIO);

    m_outDataDevices.insert (reply, dataIO);

    return reply;
}

QNetworkReply *
WebDavManager::get (const QString &path)
{
    QNetworkRequest req;

    QUrl url(m_baseUrl);
    url.setPath(absolutePath(path));

    req.setUrl (url);

    return QNetworkAccessManager::get (req);
}

QNetworkReply *
WebDavManager::get (const QString &path, QIODevice *data)
{
    return get (path, data, 0);
}

QNetworkReply *
WebDavManager::get (const QString &path, QIODevice *data, quint64 fromRangeBytes)
{
    QNetworkRequest req;
    QUrl url (m_baseUrl);

    url.setPath (absolutePath (path));

    req.setUrl (url);

    if (fromRangeBytes > 0)
    {
        QByteArray fromRange = "bytes=" + QByteArray::number(fromRangeBytes) + "-";
        req.setRawHeader ("Range", fromRange);
    }

    QNetworkReply *reply = QNetworkAccessManager::get (req);

    m_inDataDevices.insert (reply, data);

    connect (reply, SIGNAL (readyRead ()), this, SLOT (replyReadyRead ()));
    connect (reply, SIGNAL (error (QNetworkReply::NetworkError)), this, SLOT (replyError (QNetworkReply::NetworkError)));

    return reply;
}

QNetworkReply *
WebDavManager::put (const QString &path, QIODevice *data)
{
    QNetworkRequest req;

    QUrl url (m_baseUrl);
    url.setPath (absolutePath (path));

    req.setUrl (url);

    return QNetworkAccessManager::put (req, data);
}

QNetworkReply *
WebDavManager::put (const QString &path, const QByteArray &data)
{
    QNetworkRequest req;

    QUrl reqUrl (m_baseUrl);
    reqUrl.setPath (absolutePath(path));

    req.setUrl (reqUrl);

    return QNetworkAccessManager::put (req, data);
}

QNetworkReply *
WebDavManager::mkdir (const QString &path)
{
    QNetworkRequest req;

    QUrl url (m_baseUrl);
    url.setPath (absolutePath (path));

    req.setUrl (url);

    return createRequest ("MKCOL", req);
}

QNetworkReply *
WebDavManager::copy (const QString &pathFrom, const QString &pathTo, bool overWrite)
{
    QNetworkRequest req;

    QUrl reqUrl (m_baseUrl);
    reqUrl.setPath (absolutePath (pathFrom));

    req.setUrl (reqUrl);

    QUrl dstUrl (m_baseUrl);
    dstUrl.setPath (absolutePath (pathTo));

    req.setRawHeader ("Destination", dstUrl.toString().toUtf8());

    req.setRawHeader ("Depth", "infinity");
    req.setRawHeader ("Overwrite", overWrite ? "T" : "F");

    return createRequest ("COPY", req);
}

QNetworkReply *
WebDavManager::move (const QString &pathFrom, const QString &pathTo, bool overWrite)
{
    QNetworkRequest req;

    QUrl reqUrl (m_baseUrl);
    reqUrl.setPath (absolutePath (pathFrom));

    req.setUrl (reqUrl);

    QUrl dstUrl (m_baseUrl);
    dstUrl.setPath (absolutePath (pathTo));

    req.setRawHeader ("Destination", dstUrl.toString ().toUtf8 ());

    req.setRawHeader ("Depth", "infinity");
    req.setRawHeader ("Overwrite", overWrite ? "T" : "F");

    return createRequest("MOVE", req);
}

QNetworkReply *
WebDavManager::remove (const QString &path)
{
    QNetworkRequest req;

    QUrl reqUrl (m_baseUrl);
    reqUrl.setPath (absolutePath (path));

    req.setUrl (reqUrl);

    return createRequest ("DELETE", req);
}
