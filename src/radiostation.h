// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RADIOSTATION_H
#define RADIOSTATION_H

#include <QObject>
#include <QString>
#include <QVariant>

class RadioStation : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString uuid READ uuid CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString url READ url CONSTANT)
    Q_PROPERTY(QString urlResolved READ urlResolved CONSTANT)
    Q_PROPERTY(QString homepage READ homepage CONSTANT)
    Q_PROPERTY(QString favicon READ favicon CONSTANT)
    Q_PROPERTY(QString country READ country CONSTANT)
    Q_PROPERTY(QString tags READ tags CONSTANT)
    Q_PROPERTY(QString codec READ codec CONSTANT)
    Q_PROPERTY(int bitrate READ bitrate CONSTANT)
    Q_PROPERTY(int votes READ votes CONSTANT)
    Q_PROPERTY(bool isOnline READ isOnline CONSTANT)
    Q_PROPERTY(bool isFavorite READ isFavorite WRITE setIsFavorite NOTIFY isFavoriteChanged)

public:
    explicit RadioStation(QObject *parent = nullptr);

    QString uuid() const;
    void setUuid(const QString &uuid);

    QString name() const;
    void setName(const QString &name);

    QString url() const;
    void setUrl(const QString &url);

    QString urlResolved() const;
    void setUrlResolved(const QString &urlResolved);

    QString homepage() const;
    void setHomepage(const QString &homepage);

    QString favicon() const;
    void setFavicon(const QString &favicon);

    QString country() const;
    void setCountry(const QString &country);

    QString tags() const;
    void setTags(const QString &tags);

    QString codec() const;
    void setCodec(const QString &codec);

    int bitrate() const;
    void setBitrate(int bitrate);

    int votes() const;
    void setVotes(int votes);

    bool isOnline() const;
    void setIsOnline(bool isOnline);

    bool isFavorite() const;
    void setIsFavorite(bool isFavorite);

    static RadioStation* fromJson(const QJsonObject &json);

signals:
    void isFavoriteChanged();

private:
    QString m_uuid;
    QString m_name;
    QString m_url;
    QString m_urlResolved;
    QString m_homepage;
    QString m_favicon;
    QString m_country;
    QString m_tags;
    QString m_codec;
    int m_bitrate = 0;
    int m_votes = 0;
    bool m_isOnline = false;
    bool m_isFavorite = false;
};

#endif
