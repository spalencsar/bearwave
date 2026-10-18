// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHostAddress>
#include <QList>
#include <QUrl>

#include <functional>

class QObject;
class QNetworkRequest;

class NetworkUrlGuard
{
public:
    using ResolutionCallback =
        std::function<void(bool allowed, const QList<QHostAddress> &addresses)>;

    static bool isPublicAddress(const QHostAddress &address);
    static bool isSafeUrl(const QUrl &url,
                          const QList<QHostAddress> &resolvedAddresses,
                          bool allowHttp, bool allowPrivate = false);
    static void resolve(const QUrl &url, bool allowHttp, bool allowPrivate,
                        QObject *context, ResolutionCallback callback);
    static QNetworkRequest requestForResolvedUrl(
        const QUrl &url, const QHostAddress &resolvedAddress);
};
