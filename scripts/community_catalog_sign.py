#!/usr/bin/env python3
"""Sign the Bifrost community catalog without putting the private key in Git.

Requires cryptography. Keep the private key outside version control and back it up
in an owner-controlled secret store before publishing a release that trusts it.
"""

import argparse
import base64
import json
from pathlib import Path

from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey, Ed25519PublicKey


def compact_catalog(path: Path) -> bytes:
    catalog = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(catalog, list) or not catalog:
        raise ValueError("Catalog must be a nonempty JSON array")
    return json.dumps(catalog, ensure_ascii=False, separators=(",", ":")).encode("utf-8")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--catalog", type=Path, default=Path("profiles/catalog-v3.json"))
    parser.add_argument("--output", type=Path, default=Path("profiles/catalog-v3.signed.json"))
    parser.add_argument("--private-key", type=Path)
    parser.add_argument("--public-key", type=Path, default=Path("profiles/catalog-v3.public.pem"))
    parser.add_argument("--generate-key", action="store_true")
    parser.add_argument("--verify", action="store_true")
    args = parser.parse_args()

    payload = compact_catalog(args.catalog)
    if len(payload) > 256 * 1024:
        raise ValueError("Compact catalog exceeds the app's 256 KiB limit")
    if args.verify:
        envelope = json.loads(args.output.read_text(encoding="utf-8"))
        if envelope.get("format") != 1 or envelope.get("key_id") != "troa-community-2026-10":
            raise ValueError("Unexpected signed catalog format or key ID")
        signed_payload = base64.b64decode(envelope["payload"], validate=True)
        signature = base64.b64decode(envelope["signature"], validate=True)
        public_key = serialization.load_pem_public_key(args.public_key.read_bytes())
        if not isinstance(public_key, Ed25519PublicKey) or signed_payload != payload:
            raise ValueError("Signed payload differs from the source catalog")
        public_key.verify(signature, signed_payload)
        print(f"Verified {len(payload)} catalog bytes in {args.output}")
        return

    if args.private_key is None:
        parser.error("--private-key is required when signing or generating")

    if args.generate_key:
        args.private_key.parent.mkdir(parents=True, exist_ok=True)
        key = Ed25519PrivateKey.generate()
        private_bytes = key.private_bytes(
            serialization.Encoding.PEM,
            serialization.PrivateFormat.PKCS8,
            serialization.NoEncryption(),
        )
        with args.private_key.open("xb") as private_file:
            private_file.write(private_bytes)
    else:
        key = serialization.load_pem_private_key(args.private_key.read_bytes(), password=None)
        if not isinstance(key, Ed25519PrivateKey):
            raise ValueError("Private key must be Ed25519")

    public_key = key.public_key()
    public_bytes = public_key.public_bytes(serialization.Encoding.Raw, serialization.PublicFormat.Raw)
    public_pem = public_key.public_bytes(serialization.Encoding.PEM, serialization.PublicFormat.SubjectPublicKeyInfo)
    if args.generate_key:
        with args.public_key.open("xb") as public_file:
            public_file.write(public_pem)
    elif args.public_key.read_bytes() != public_pem:
        raise ValueError("Private key does not match the checked-in public key")
    signature = key.sign(payload)
    Ed25519PublicKey.from_public_bytes(public_bytes).verify(signature, payload)
    envelope = {
        "format": 1,
        "key_id": "troa-community-2026-10",
        "payload": base64.b64encode(payload).decode("ascii"),
        "signature": base64.b64encode(signature).decode("ascii"),
    }
    args.output.write_text(json.dumps(envelope, separators=(",", ":")) + "\n", encoding="utf-8")
    print(f"Public key (raw hex): {public_bytes.hex()}")
    print(f"Signed {len(payload)} catalog bytes into {args.output}")


if __name__ == "__main__":
    main()
