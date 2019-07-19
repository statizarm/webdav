#ifndef WEBDAV_H_
#define WEBDAV_H_

#include <QNetworkAccessManager>
#include <QMap>
#include <QNetworkReply>
#include <QtCore>

class WebDavManager : public QNetworkAccessManager
{
    Q_OBJECT
public:

    enum ConnectionType {HTTP = 1, HTTPS};

    WebDavManager (QObject *parent = nullptr);
    ~WebDavManager ();

    const QString &username () const;
    const QString &password () const;
    const QString &rootPath () const;
    const QUrl &baseUrl () const;
    const QString hostName () const;
    ConnectionType connectionType () const;
    int port () const;
    bool isSSL () const;

    void setConnectionSettings (const ConnectionType type,
                                const QString &hostname,
                                const QString &rootPath = "/",
                                const QString &username = "",
                                const QString &password = "",
                                int port = 0);
    QNetworkReply *get (const QString &path);
    QNetworkReply *get (const QString &path, QIODevice *data);
    QNetworkReply *get (const QString &path, QIODevice *data, quint64 fromRangeBytes);

    QNetworkReply *put (const QString &path, QIODevice *data);
    QNetworkReply *put (const QString &path, const QByteArray &data);

    QNetworkReply *mkdir (const QString &dtr);
    QNetworkReply *copy (const QString &pathFrom, const QString &pathTo, bool overWrite = false);
    QNetworkReply *move (const QString &pathFrom, const QString &pathTo, bool overWrite = false);
    QNetworkReply *remove (const QString &path);

signals:
    void errorChanged (QString error);

private:
    QMap<QNetworkReply *, QIODevice *> m_inDataDevices;
    QMap<QNetworkReply *, QIODevice *> m_outDataDevices;

    QString m_username;
    QString m_password;
    QString m_rootPath;
    QUrl m_baseUrl;
    ConnectionType m_connectionType;

    QNetworkReply *m_authenticatorLastReply;

protected:
    QNetworkReply *createRequest (const QString &method, QNetworkRequest &req, QIODevice *outGoingData = nullptr);
    QNetworkReply *createRequest (const QString &method, QNetworkRequest &req, const QByteArray &outGoingData);

    QString absolutePath (const QString &relPath);
protected slots:
    void replyReadyRead ();
    void replyFinished (QNetworkReply *);
    void replyDeleteLater (QNetworkReply *);
    void replyError (QNetworkReply::NetworkError);
    void provideAuthentication (QNetworkReply *, QAuthenticator *authenicator);
};

#endif // WEBDAV_H_
