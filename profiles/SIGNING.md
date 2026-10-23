# Community catalog signing

The app downloads `profiles/catalog-v3.signed.json` from the live profile branch.
It accepts the embedded JSON payload only after Ed25519 verification against the
public key bundled in the app, and verifies the cached envelope on every read.
The old unsigned cache is retained on disk but is no longer loaded. Personal
profiles and active mappings are unaffected.

To publish a catalog revision, use the owner-controlled private key (never put
it in Git or a build artifact):

```text
python scripts/community_catalog_sign.py --private-key PATH_TO_PRIVATE_KEY
python scripts/community_catalog_sign.py --verify
```

Commit the catalog source and signed envelope together. The Windows preview
build checks that the signed payload exactly matches the bundled source before
compiling. A signing key must be backed up in an owner-controlled secret store
before a release depends on it. If compromised, rotate the public key and key ID
in a new app release; simply replacing the remote signature is insufficient.

This candidate's private key is held locally in the ignored `.tools` folder with
current-user Windows ACLs. It is not a durable custody or recovery arrangement.
Do not publish the candidate until that key has been transferred to durable
owner custody and the Windows package has compiled successfully.
