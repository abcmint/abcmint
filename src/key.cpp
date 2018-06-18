// Copyright (c) 2018 The Abcmint developers


#include <map>

#include "pqcrypto/api.h"
#include "key.h"

void CKey::Reset() {
    privKey.resize(RAINBOW_PRIVATE_KEY_SIZE);
}

CKey::CKey(): pubKey() {
    privKey.resize(RAINBOW_PRIVATE_KEY_SIZE);
    fSet = false;
}

CKey::CKey(const CKey& b) : pubKey() {
    privKey.resize(RAINBOW_PRIVATE_KEY_SIZE);
    if (b.privKey.size() == RAINBOW_PRIVATE_KEY_SIZE)
        memcpy(privKey.data(), b.privKey.data(), RAINBOW_PRIVATE_KEY_SIZE);
    else
        throw key_error("CKey::CKey: invalid key size.");
    fSet = b.fSet;
}

CKey& CKey::operator=(const CKey& b) {
    if (b.privKey.size() == RAINBOW_PRIVATE_KEY_SIZE)
        memcpy(privKey.data(), b.privKey.data(), RAINBOW_PRIVATE_KEY_SIZE);   
    else
        throw key_error("CKey::CKey: invalid key size.");
    pubKey = b.pubKey;
    fSet = b.fSet;
    return (*this);
}

void CKey::MakeNewKey()
{
    int status;
    status = crypto_sign_keypair(pubKey.vchPubKey.data(), privKey.data());
    if (status != 0) {
        throw key_error("CKey::MakeNewKey, make key pair failure.");
    }
    fSet = true;
}

bool CKey::SetPrivKey(const CPrivKey& vchPrivKey)
{
    const unsigned char* pbegin = &vchPrivKey[0];
    privKey.resize(RAINBOW_PRIVATE_KEY_SIZE);
    memcpy(privKey.data(), pbegin, RAINBOW_PRIVATE_KEY_SIZE);
    return true;
}

bool CPubKey::Verify(uint256 hash, const std::vector<unsigned char>& vchSig)
{
    if (vchSig.empty()) return false;
    int status = -1;
    status = rainbow_verify((unsigned char*)&hash, &vchSig[0], vchPubKey.data());
    if (status != 0) {
        return false;
    }
    return true;
}

bool CKey::SetPubKey(const CPubKey& vchPubKey)
{
    const unsigned char* pbegin = &vchPubKey.vchPubKey[0];
    pubKey.vchPubKey.resize(RAINBOW_PUBLIC_KEY_SIZE);
    memcpy(pubKey.vchPubKey.data(), pbegin, RAINBOW_PUBLIC_KEY_SIZE);
    return true;
}

bool CKey::SetPubKey(std::vector<unsigned char> pk)
{
    if (pk.size() !=  RAINBOW_PUBLIC_KEY_SIZE)
        return false;
    pubKey.vchPubKey.resize(RAINBOW_PUBLIC_KEY_SIZE);
    memcpy(pubKey.vchPubKey.data(), pk.data(), RAINBOW_PUBLIC_KEY_SIZE);
    return true;
}


bool CKey::Sign(uint256 hash, std::vector<unsigned char>& vchSig)
{
    int status = 0;
    vchSig.resize(RAINBOW_SIGNATURE_SIZE);
    status = rainbow_sign(&vchSig[0],privKey.data(),(unsigned char*)&hash);
    if (status != 0) {
        vchSig.clear();
        return false;
    }
    return true;
}

