#ifndef ICYREADER_H
#define ICYREADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

class IcyReader : public QObject
{
    Q_OBJECT
public:
    explicit IcyReader(QObject *parent = nullptr);
    ~IcyReader();

    void start(const QString &url);
    void stop();

signals:
    void metaDataReceived(const QString &artist, const QString &title);

private slots:
    void onReadyRead();
    void onFinished();

private:
    void parseMetaData(const QByteArray &metaData);

    QNetworkAccessManager *m_nam = nullptr;
    QNetworkReply *m_reply = nullptr;

    enum State {
        ReadingAudio,
        ReadingMetaLength,
        ReadingMetaData
    };

    State m_state = ReadingAudio;
    int m_metaInt = 0;
    int m_audioBytesRead = 0;
    int m_metaLength = 0;
};

#endif // ICYREADER_H
