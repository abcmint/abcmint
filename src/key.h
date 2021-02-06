// Copyright (c) 2018 The Abcmint developers

#ifndef ABCMINT_KEY_H
#define ABCMINT_KEY_H

#include <stdexcept>
#include <vector>

#include "allocators.h"
#include "hash.h"
#include "serialize.h"
#include "uint256.h"

/**
 * RAINBOW signature:
 */
static const unsigned int RAINBOW_PUBLIC_KEY_SIZE = 152097;
static const unsigned int RAINBOW_SIGNATURE_SIZE = 64;
const static unsigned int RAINBOW_PRIVATE_KEY_SIZE = 100209;
static const unsigned int HASH_LEN_BYTES = 32;

class key_error : public std::runtime_error {
   public:
    explicit key_error(const std::string &str) : std::runtime_error(str) {}
};

/** A reference to a CKey: the Hash of its serialized public key */
class CKeyID : public uint256 {
   public:
    CKeyID() : uint256(0) {}
    CKeyID(const uint256 &in) : uint256(in) {}
    CKeyID(const std::string &in) : uint256(in) {}
};

/** A reference to a CScript: the Hash of its serialization (see script.h) */
class CScriptID : public uint256 {
   public:
    CScriptID() : uint256(0) {}
    CScriptID(const uint256 &in) : uint256(in) {}
};

/** An encapsulated public key. */
class CPubKey {
   private:
    friend class CKey;

   public:
    std::vector<unsigned char> vchPubKey;

    CPubKey() { vchPubKey.resize(RAINBOW_PUBLIC_KEY_SIZE); }

    //! Initialize a public key using begin/end iterators to byte data.
    template <typename T>
    void Set(const T pbegin, const T pend) {
        int len = pend == pbegin ? 0 : RAINBOW_PUBLIC_KEY_SIZE;
        if (len && len == (pend - pbegin))
            memcpy(vchPubKey.data(), (unsigned char *)&pbegin[0], len);
        else
            vchPubKey.resize(RAINBOW_PUBLIC_KEY_SIZE);
    }

    //! Construct a public key using begin/end iterators to byte data.
    template <typename T>
    CPubKey(const T pbegin, const T pend) {
        vchPubKey.resize(RAINBOW_PUBLIC_KEY_SIZE);
        Set(pbegin, pend);
    }

    CPubKey(const CPubKey &pk) {
        vchPubKey.resize(RAINBOW_PUBLIC_KEY_SIZE);
        Set(pk.vchPubKey.data(), pk.vchPubKey.data() + RAINBOW_PUBLIC_KEY_SIZE);
    }

    explicit CPubKey(const std::vector<unsigned char> &vchPubKeyIn) {
        vchPubKey.resize(RAINBOW_PUBLIC_KEY_SIZE);
        memcpy(vchPubKey.data(), vchPubKeyIn.data(), vchPubKey.size());
    }
    friend bool operator==(const CPubKey &a, const CPubKey &b) {
        return a.vchPubKey == b.vchPubKey;
    }
    friend bool operator!=(const CPubKey &a, const CPubKey &b) {
        return a.vchPubKey != b.vchPubKey;
    }
    friend bool operator<(const CPubKey &a, const CPubKey &b) {
        return a.vchPubKey < b.vchPubKey;
    }
    CPubKey &operator=(const CPubKey &rhs) {
        if (this == &rhs) {
            return *this;
        }
        this->vchPubKey.resize(RAINBOW_PUBLIC_KEY_SIZE);
        memcpy(this->vchPubKey.data(), rhs.vchPubKey.data(),
               RAINBOW_PUBLIC_KEY_SIZE);
        return *this;
    }

    IMPLEMENT_SERIALIZE(READWRITE(vchPubKey);)

    CKeyID GetID() const { return CKeyID(Hash(vchPubKey)); }

    uint256 GetHash() const { return Hash(vchPubKey.begin(), vchPubKey.end()); }

    bool IsValid() const {
        bool isNULL = true;
        for (size_t i = 0; i < vchPubKey.size(); i++) {
            if ('\0' != vchPubKey[i]) {
                isNULL = false;
                break;
            }
        }
        return vchPubKey.size() == RAINBOW_PUBLIC_KEY_SIZE && !isNULL;
    }

    bool Verify(uint256 hash, const std::vector<unsigned char> &vchSig);

    std::vector<unsigned char> Raw() const { return vchPubKey; }
};

// secure_allocator is defined in allocators.h
// CPrivKey is a serialized private key, with all parameters included
typedef std::vector<unsigned char> CPrivKey;
// CSecret is a serialization of just the secret parameter
typedef std::vector<unsigned char> CSecret;

class CKey {
   protected:
    bool fSet;
    std::vector<unsigned char> privKey;

   public:
    // The public key associate to private key
    CPubKey pubKey;

    void Reset();

    CKey();
    CKey(const CKey &b);
    ~CKey() {}
    CKey &operator=(const CKey &b);
    bool IsNull() const { return !fSet; }
    void MakeNewKey();
    bool SetPrivKey(const CPrivKey &vchPrivKey);
    CPrivKey GetPrivKey() const { return privKey; }
    bool SetPubKey(const CPubKey &vchPubKey);
    bool SetPubKey(std::vector<unsigned char> pk);
    CPubKey GetPubKey() const { return (pubKey); }
    bool Sign(uint256 hash, std::vector<unsigned char> &vchSig);
};

#endif
