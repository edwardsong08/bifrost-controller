// SPDX-License-Identifier: GPL-3.0-or-later
#include "catalogsignature.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include <openssl/evp.h>
#include <openssl/pem.h>

namespace Troa {
QByteArray verifiedCatalogPayload(const QByteArray &envelope)
{
    if (envelope.isEmpty() || envelope.size() > 384 * 1024)
        return {};
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(envelope, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject())
        return {};
    const auto object = document.object();
    if (object.value("format").toInt() != 1 || object.value("key_id").toString() != "troa-community-2026-10")
        return {};
    const auto encodedPayload = object.value("payload").toString().toLatin1();
    const auto encodedSignature = object.value("signature").toString().toLatin1();
    const auto payload = QByteArray::fromBase64(encodedPayload);
    const auto signature = QByteArray::fromBase64(encodedSignature);
    if (payload.isEmpty() || payload.size() > 256 * 1024 || signature.size() != 64 || payload.toBase64() != encodedPayload ||
        signature.toBase64() != encodedSignature)
        return {};

    QFile publicKey(":/troa/catalog-public.pem");
    if (!publicKey.open(QIODevice::ReadOnly))
        return {};
    const auto pem = publicKey.readAll();
    BIO *bio = BIO_new_mem_buf(pem.constData(), pem.size());
    EVP_PKEY *key = bio ? PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr) : nullptr;
    BIO_free(bio);
    if (!key)
        return {};
    EVP_MD_CTX *context = EVP_MD_CTX_new();
    const bool verified =
        EVP_PKEY_base_id(key) == EVP_PKEY_ED25519 && context &&
        EVP_DigestVerifyInit(context, nullptr, nullptr, nullptr, key) == 1 &&
        EVP_DigestVerify(context, reinterpret_cast<const unsigned char *>(signature.constData()),
                         static_cast<size_t>(signature.size()), reinterpret_cast<const unsigned char *>(payload.constData()),
                         static_cast<size_t>(payload.size())) == 1;
    EVP_MD_CTX_free(context);
    EVP_PKEY_free(key);
    return verified ? payload : QByteArray{};
}
} // namespace Troa
