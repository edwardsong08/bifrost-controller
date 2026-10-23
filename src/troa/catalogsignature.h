// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QByteArray>

namespace Troa {
// Empty means invalid. The signature covers the exact UTF-8 JSON payload bytes.
QByteArray verifiedCatalogPayload(const QByteArray &envelope);
} // namespace Troa
