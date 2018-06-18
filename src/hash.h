// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2018 The Abcmint developers

#ifndef ABCMINT_HASH_H
#define ABCMINT_HASH_H

#include "uint256.h"
#include "serialize.h"
#include "pqcrypto/pqcrypto.h"

#include <vector>

template<typename T1>
inline uint256 Hash(const T1 pbegin, const T1 pend)
{
    static unsigned char pblank[1];
    uint256 hash1;
	pqcSha256(pbegin == pend ? pblank : (unsigned char*)&pbegin[0], (pend - pbegin) * sizeof(pbegin[0]),(unsigned char*)&hash1);
    uint256 hash2;
	pqcSha256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}

inline uint256 Hash(const std::vector<unsigned char>& vch) {
    uint256 hash1;
    pqcSha256(&vch[0], vch.size(), (unsigned char*)&hash1);
    uint256 hash2;
    pqcSha256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}

class CHashWriter
{
private:
    Sha256 ctx;

public:
    int nType;
    int nVersion;

    void Init() {
        sha256Init(&ctx);
    }

    CHashWriter(int nTypeIn, int nVersionIn) : nType(nTypeIn), nVersion(nVersionIn) {
        Init();
    }

    CHashWriter& write(const char *pch, size_t size) {
        sha256Process(&ctx, (const unsigned char *)pch, size);
        return (*this);
    }

    // invalidates the object
    uint256 GetHash() {
        uint256 hash1;
        sha256Done(&ctx, (unsigned char*)&hash1);
        uint256 hash2;
        pqcSha256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
        return hash2;
    }

    template<typename T>
    CHashWriter& operator<<(const T& obj) {
        // Serialize to this stream
        ::Serialize(*this, obj, nType, nVersion);
        return (*this);
    }
};


template<typename T1, typename T2>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end)
{
    static unsigned char pblank[1];
    uint256 hash1;
    Sha256 ctx;
    sha256Init(&ctx);
    sha256Process(&ctx, (p1begin == p1end ? pblank : (unsigned char*)&p1begin[0]), (p1end - p1begin) * sizeof(p1begin[0]));
    sha256Process(&ctx, (p2begin == p2end ? pblank : (unsigned char*)&p2begin[0]), (p2end - p2begin) * sizeof(p2begin[0]));
    sha256Done(&ctx, (unsigned char*)&hash1);
    uint256 hash2;
    pqcSha256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}

template<typename T1, typename T2, typename T3>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end,
                    const T3 p3begin, const T3 p3end)
{
    static unsigned char pblank[1];
    uint256 hash1;
    Sha256 ctx;
    sha256Init(&ctx);
    sha256Process(&ctx, (p1begin == p1end ? pblank : (unsigned char*)&p1begin[0]), (p1end - p1begin) * sizeof(p1begin[0]));
    sha256Process(&ctx, (p2begin == p2end ? pblank : (unsigned char*)&p2begin[0]), (p2end - p2begin) * sizeof(p2begin[0]));
    sha256Process(&ctx, (p3begin == p3end ? pblank : (unsigned char*)&p3begin[0]), (p3end - p3begin) * sizeof(p3begin[0]));
    sha256Done(&ctx, (unsigned char*)&hash1);
    uint256 hash2;
    pqcSha256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}

template<typename T>
uint256 SerializeHash(const T& obj, int nType=SER_GETHASH, int nVersion=PROTOCOL_VERSION)
{
    CHashWriter ss(nType, nVersion);
    ss << obj;
    return ss.GetHash();
}

unsigned int MurmurHash3(unsigned int nHashSeed, const std::vector<unsigned char>& vDataToHash);

#endif
