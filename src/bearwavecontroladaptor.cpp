#include "bearwavecontroladaptor.h"

#include "radiobackend.h"

BearWaveControlAdaptor::BearWaveControlAdaptor(RadioBackend *backend)
    : QDBusAbstractAdaptor(backend)
    , m_backend(backend)
{
    connect(m_backend, &RadioBackend::listsChanged, this, &BearWaveControlAdaptor::StationsChanged);
}

QVariantList BearWaveControlAdaptor::GetFavorites() const
{
    return m_backend ? m_backend->getFavoriteStations() : QVariantList();
}

QVariantList BearWaveControlAdaptor::GetRecentStations() const
{
    return m_backend ? m_backend->getRecentStations() : QVariantList();
}

bool BearWaveControlAdaptor::PlayFavoriteByUuid(const QString &uuid, const QString &urlResolved)
{
    return m_backend ? m_backend->playFavoriteByUuid(uuid, urlResolved) : false;
}

bool BearWaveControlAdaptor::PlayRecentByUuid(const QString &uuid, const QString &urlResolved)
{
    return m_backend ? m_backend->playRecentByUuid(uuid, urlResolved) : false;
}

bool BearWaveControlAdaptor::HasNext() const
{
    return m_backend ? m_backend->hasNextStation() : false;
}

bool BearWaveControlAdaptor::HasPrevious() const
{
    return m_backend ? m_backend->hasPreviousStation() : false;
}

void BearWaveControlAdaptor::PlayNext()
{
    if (m_backend) {
        m_backend->playNextStation();
    }
}

void BearWaveControlAdaptor::PlayPrevious()
{
    if (m_backend) {
        m_backend->playPreviousStation();
    }
}

void BearWaveControlAdaptor::PlayPause()
{
    if (m_backend && m_backend->player()) {
        m_backend->player()->togglePlayPause();
    }
}

void BearWaveControlAdaptor::Stop()
{
    if (m_backend && m_backend->player()) {
        m_backend->player()->stop();
    }
}
