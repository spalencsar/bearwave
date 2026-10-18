// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "networkurlguard.h"

#include <QHostInfo>
#include <QNetworkRequest>
#include <QObject>

namespace {
bool hasAllowedShape(const QUrl &url, bool allowHttp)
{
    const QString scheme = url.scheme().toLower();
    return url.isValid() && !url.host().isEmpty()
           && (scheme == QStringLiteral("https")
               || (allowHttp && scheme == QStringLiteral("http")))
           && url.userInfo().isEmpty();
}
}

bool NetworkUrlGuard::isPublicAddress(const QHostAddress &address)
{
    if (address.isNull() || address.isLoopback() || address.isLinkLocal()
        || address.isSiteLocal() || address.isMulticast()) {
        return false;
    }

    bool isMappedIpv4 = false;
    const quint32 ipv4 = address.toIPv4Address(&isMappedIpv4);
    if (address.protocol() == QAbstractSocket::IPv4Protocol || isMappedIpv4) {
        return (ipv4 & 0xff000000u) != 0x00000000u       // current network
               && (ipv4 & 0xff000000u) != 0x0a000000u   // private
               && (ipv4 & 0xffc00000u) != 0x64400000u   // carrier-grade NAT
               && (ipv4 & 0xff000000u) != 0x7f000000u   // loopback
               && (ipv4 & 0xffff0000u) != 0xa9fe0000u   // link-local
               && (ipv4 & 0xfff00000u) != 0xac100000u   // private
               && (ipv4 & 0xffffff00u) != 0xc0000000u   // IETF protocol use
               && (ipv4 & 0xffffff00u) != 0xc0000200u   // documentation
               && (ipv4 & 0xffffff00u) != 0xc0586300u   // deprecated relay
               && (ipv4 & 0xffff0000u) != 0xc0a80000u   // private
               && (ipv4 & 0xfffe0000u) != 0xc6120000u   // benchmarking
               && (ipv4 & 0xffffff00u) != 0xc6336400u   // documentation
               && (ipv4 & 0xffffff00u) != 0xcb007100u   // documentation
               && (ipv4 & 0xf0000000u) != 0xe0000000u   // multicast/reserved
               && address.isGlobal();
    }

    const Q_IPV6ADDR ipv6 = address.toIPv6Address();
    const bool uniqueLocal = (ipv6.c[0] & 0xfe) == 0xfc;
    const bool documentation = ipv6.c[0] == 0x20 && ipv6.c[1] == 0x01
                               && ipv6.c[2] == 0x0d && ipv6.c[3] == 0xb8;
    return !uniqueLocal && !documentation && address.isGlobal();
}

bool NetworkUrlGuard::isSafeUrl(
    const QUrl &url, const QList<QHostAddress> &resolvedAddresses,
    bool allowHttp, bool allowPrivate)
{
    if (!hasAllowedShape(url, allowHttp) || resolvedAddresses.isEmpty()) {
        return false;
    }
    for (const QHostAddress &address : resolvedAddresses) {
        if (!allowPrivate && !isPublicAddress(address)) {
            return false;
        }
    }
    return true;
}

void NetworkUrlGuard::resolve(const QUrl &url, bool allowHttp,
                              bool allowPrivate, QObject *context,
                              ResolutionCallback callback)
{
    if (!hasAllowedShape(url, allowHttp)) {
        callback(false, {});
        return;
    }

    QHostAddress literal;
    if (literal.setAddress(url.host())) {
        const QList<QHostAddress> addresses{literal};
        callback(isSafeUrl(url, addresses, allowHttp, allowPrivate), addresses);
        return;
    }

    QHostInfo::lookupHost(url.host(), context,
                          [url, allowHttp, allowPrivate,
                           callback = std::move(callback)](const QHostInfo &info) {
        const QList<QHostAddress> addresses = info.addresses();
        const bool resolved = info.error() == QHostInfo::NoError;
        callback(resolved
                     && isSafeUrl(url, addresses, allowHttp, allowPrivate),
                 addresses);
    });
}

QNetworkRequest NetworkUrlGuard::requestForResolvedUrl(
    const QUrl &url, const QHostAddress &resolvedAddress)
{
    QUrl connectionUrl = url;
    const bool isHttps =
        url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0;
    connectionUrl.setHost(resolvedAddress.toString());

    QNetworkRequest request(connectionUrl);
    QByteArray hostHeader = QUrl::toAce(url.host());
    if (hostHeader.contains(':')) {
        hostHeader = '[' + hostHeader + ']';
    }
    const int port = url.port();
    if (port >= 0) {
        hostHeader += ':' + QByteArray::number(port);
    }
    request.setRawHeader(QByteArrayLiteral("Host"), hostHeader);
    // HTTP/2 derives :authority from the connection URL and may ignore a raw
    // Host header. Keep pinned requests on HTTP/1.1 so virtual hosting uses
    // the original, validated hostname.
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    if (isHttps) {
        request.setPeerVerifyName(url.host());
    }
    return request;
}
