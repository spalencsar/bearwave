// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "radiostation.h"

#include <QJsonObject>

RadioStation::RadioStation(QObject *parent)
    : QObject(parent)
{
}

QString RadioStation::uuid() const
{
    return m_uuid;
}

void RadioStation::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}

QString RadioStation::name() const
{
    return m_name;
}

void RadioStation::setName(const QString &name)
{
    m_name = name;
}

QString RadioStation::url() const
{
    return m_url;
}

void RadioStation::setUrl(const QString &url)
{
    m_url = url;
}

QString RadioStation::urlResolved() const
{
    return m_urlResolved;
}

void RadioStation::setUrlResolved(const QString &urlResolved)
{
    m_urlResolved = urlResolved;
}

QString RadioStation::homepage() const
{
    return m_homepage;
}

void RadioStation::setHomepage(const QString &homepage)
{
    m_homepage = homepage;
}

QString RadioStation::favicon() const
{
    return m_favicon;
}

void RadioStation::setFavicon(const QString &favicon)
{
    m_favicon = favicon;
}

QString RadioStation::country() const
{
    return m_country;
}

void RadioStation::setCountry(const QString &country)
{
    m_country = country;
}

QString RadioStation::tags() const
{
    return m_tags;
}

void RadioStation::setTags(const QString &tags)
{
    m_tags = tags;
}

QString RadioStation::codec() const
{
    return m_codec;
}

void RadioStation::setCodec(const QString &codec)
{
    m_codec = codec;
}

int RadioStation::bitrate() const
{
    return m_bitrate;
}

void RadioStation::setBitrate(int bitrate)
{
    m_bitrate = bitrate;
}

int RadioStation::votes() const
{
    return m_votes;
}

void RadioStation::setVotes(int votes)
{
    m_votes = votes;
}

bool RadioStation::isOnline() const
{
    return m_isOnline;
}

void RadioStation::setIsOnline(bool isOnline)
{
    m_isOnline = isOnline;
}

bool RadioStation::isFavorite() const
{
    return m_isFavorite;
}

void RadioStation::setIsFavorite(bool isFavorite)
{
    if (m_isFavorite == isFavorite) {
        return;
    }
    m_isFavorite = isFavorite;
    emit isFavoriteChanged();
}

RadioStation* RadioStation::fromJson(const QJsonObject &json)
{
    RadioStation *station = new RadioStation();

    station->setUuid(json.value("stationuuid").toString());
    station->setName(json.value("name").toString());
    station->setUrl(json.value("url").toString());
    station->setUrlResolved(json.value("url_resolved").toString());
    station->setHomepage(json.value("homepage").toString());
    station->setFavicon(json.value("favicon").toString());
    station->setCountry(json.value("country").toString());
    station->setTags(json.value("tags").toString());
    station->setCodec(json.value("codec").toString());
    station->setBitrate(json.value("bitrate").toInt());
    station->setVotes(json.value("votes").toInt());
    station->setIsOnline(json.value("lastcheckok").toInt() == 1);

    return station;
}
