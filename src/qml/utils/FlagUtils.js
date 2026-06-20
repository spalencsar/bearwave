// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

.pragma library

function flagEmoji(countryCode) {
    if (!countryCode || countryCode.length !== 2) {
        return "🌎"
    }
    var codeUpper = countryCode.toUpperCase()
    var codePoints = []
    for (var i = 0; i < codeUpper.length; i++) {
        codePoints.push(127397 + codeUpper.charCodeAt(i))
    }
    return String.fromCodePoint.apply(null, codePoints)
}