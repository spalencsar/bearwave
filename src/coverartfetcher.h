#ifndef COVERARTFETCHER_H
#define COVERARTFETCHER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class CoverArtFetcher : public QObject
{
    Q_OBJECT

public:
    explicit CoverArtFetcher(QObject *parent = nullptr);

    void fetch(const QString &artist, const QString &title);

signals:
    void coverUrlReady(const QString &url);

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_currentReply = nullptr;
};

#endif // COVERARTFETCHER_H
