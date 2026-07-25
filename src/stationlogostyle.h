// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

namespace StationLogoStyle {
QString initials(const QString &stationName);
int paletteIndex(const QString &stationKey, int paletteSize);
}
