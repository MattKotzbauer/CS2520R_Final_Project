// CityHash.cpp
#include "CityHash.h"
#include <cstring>
#include <algorithm>
#include <utility>
#include <stdint.h>

using namespace std;

// Define constants
const uint64_t k0 = 0xc3a5c85c97cb3127ULL;
const uint64_t k1 = 0xb492b66fbe98f273ULL;
const uint64_t k2 = 0x9ae16a3b2f90404fULL;

// Implement helper functions
static uint64_t UNALIGNED_LOAD64(const char* p) {
    uint64_t result;
    memcpy(&result, p, sizeof(result));
    return result;
}

static uint32_t UNALIGNED_LOAD32(const char* p) {
    uint32_t result;
    memcpy(&result, p, sizeof(result));
    return result;
}

#if defined(_MSC_VER)
#include <stdlib.h>
#define bswap_32(x) _byteswap_ulong(x)
#define bswap_64(x) _byteswap_uint64(x)
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define bswap_32(x) OSSwapInt32(x)
#define bswap_64(x) OSSwapInt64(x)
#elif defined(__sun) || defined(sun)
#include <sys/byteorder.h>
#define bswap_32(x) BSWAP_32(x)
#define bswap_64(x) BSWAP_64(x)
#elif defined(__FreeBSD__)
#include <sys/endian.h>
#define bswap_32(x) bswap32(x)
#define bswap_64(x) bswap64(x)
#elif defined(__OpenBSD__)
#include <sys/types.h>
#define bswap_32(x) swap32(x)
#define bswap_64(x) swap64(x)
#elif defined(__NetBSD__)
#include <sys/types.h>
#include <machine/bswap.h>
#if defined(__BSWAP_RENAME) && !defined(__bswap_32)
#define bswap_32(x) bswap32(x)
#define bswap_64(x) bswap64(x)
#endif
#else
#include <byteswap.h>
#endif

#ifdef WORDS_BIGENDIAN
#define uint32_in_expected_order(x) (bswap_32(x))
#define uint64_in_expected_order(x) (bswap_64(x))
#else
#define uint32_in_expected_order(x) (x)
#define uint64_in_expected_order(x) (x)
#endif

#if !defined(LIKELY)
#if HAVE_BUILTIN_EXPECT
#define LIKELY(x) (__builtin_expect(!!(x), 1))
#else
#define LIKELY(x) (x)
#endif
#endif

static uint64_t Fetch64(const char* p) {
    return uint64_in_expected_order(UNALIGNED_LOAD64(p));
}

static uint32_t Fetch32(const char* p) {
    return uint32_in_expected_order(UNALIGNED_LOAD32(p));
}

static uint64_t Rotate(uint64_t val, int shift) {
    // Avoid shifting by 64: doing so yields an undefined result.
    return shift == 0 ? val : ((val >> shift) | (val << (64 - shift)));
}

static uint64_t ShiftMix(uint64_t val) {
    return val ^ (val >> 47);
}

// Hash128to64 should be defined before its usage in other functions
static uint64_t Hash128to64(unsigned __int128 x) {
    const uint64_t kMul = 0x9ddfea08eb382d69ULL;
    uint64_t a = static_cast<uint64_t>(x);
    uint64_t b = static_cast<uint64_t>(x >> 64);
    return (a ^ b) * kMul;
}

static uint64_t HashLen16(uint64_t u, uint64_t v) {
    // Combine u and v into a 128-bit integer
    unsigned __int128 combined = ((unsigned __int128)u << 64) | v;
    return Hash128to64(combined);
}


static uint64_t fmix(uint32_t h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

static uint32_t Rotate32(uint32_t val, int shift) {
    // Avoid shifting by 32: doing so yields an undefined result.
    return shift == 0 ? val : ((val >> shift) | (val << (32 - shift)));
}

#define PERMUTE3(a, b, c) do { std::swap(a, b); std::swap(a, c); } while (0)

static uint32_t Mur(uint32_t a, uint32_t h) {
    // Helper from Murmur3 for combining two 32-bit values.
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    a *= c1;
    a = Rotate32(a, 17);
    a *= c2;
    h ^= a;
    h = Rotate32(h, 19);
    return h * 5 + 0xe6546b64;
}

static uint32_t Hash32Len0to4(const char* s, size_t len) {
    uint32_t b = 0;
    uint32_t c = 9;
    for (size_t i = 0; i < len; i++) {
        signed char v = static_cast<signed char>(s[i]);
        const uint32_t c1 = 0xcc9e2d51;
        b = b * c1 + static_cast<uint32_t>(v);
        c ^= b;
    }
    return fmix(Mur(b, Mur(static_cast<uint32_t>(len), c)));
}

static uint32_t Hash32Len5to12(const char* s, size_t len) {
    uint32_t a = static_cast<uint32_t>(len), b = a * 5, c = 9, d = b;
    a += Fetch32(s);
    b += Fetch32(s + len - 4);
    c += Fetch32(s + ((len >> 1) & 4));
    return fmix(Mur(c, Mur(b, Mur(a, d))));
}

static uint32_t Hash32Len13to24(const char* s, size_t len) {
    uint32_t a = Fetch32(s - 4 + (len >> 1));
    uint32_t b = Fetch32(s + 4);
    uint32_t c = Fetch32(s + len - 8);
    uint32_t d = Fetch32(s + (len >> 1));
    uint32_t e = Fetch32(s);
    uint32_t f = Fetch32(s + len - 4);
    uint32_t h = static_cast<uint32_t>(len);

    return fmix(Mur(f, Mur(e, Mur(d, Mur(c, Mur(b, Mur(a, h)))))));
}

static uint64_t HashLen0to16(const char* s, size_t len) {
    if (len >= 8) {
        uint64_t mul = k2 + len * 2;
        uint64_t a = Fetch64(s) + k2;
        uint64_t b = Fetch64(s + len - 8);
        uint64_t c = Rotate(b, 37) * mul + a;
        uint64_t d = (Rotate(a, 25) + b) * mul;
        return Hash128to64(((unsigned __int128)(c) << 64) | d);
    }
    if (len >= 4) {
        uint64_t mul = k2 + len * 2;
        uint64_t a = Fetch32(s);
        return Hash128to64(((unsigned __int128)(len + (a << 3)) << 64) | Fetch32(s + len - 4));
    }
    if (len > 0) {
        uint8_t a = static_cast<uint8_t>(s[0]);
        uint8_t b = static_cast<uint8_t>(s[len >> 1]);
        uint8_t c = static_cast<uint8_t>(s[len - 1]);
        uint32_t y = static_cast<uint32_t>(a) + (static_cast<uint32_t>(b) << 8);
        uint32_t z = static_cast<uint32_t>(len) + (static_cast<uint32_t>(c) << 2);
        return ShiftMix(y * k2 ^ z * k0) * k2;
    }
    return k2;
}

static uint64_t HashLen17to32(const char* s, size_t len) {
    uint64_t mul = k2 + len * 2;
    uint64_t a = Fetch64(s) * k1;
    uint64_t b = Fetch64(s + 8);
    uint64_t c = Fetch64(s + len - 8) * mul;
    uint64_t d = Fetch64(s + len - 16) * k2;
    return Hash128to64(((unsigned __int128)(Rotate(a + b, 43) + Rotate(c, 30) + d) << 64) |
                        (a + Rotate(b + k2, 18) + c));
}

static pair<uint64_t, uint64_t> WeakHashLen32WithSeeds(
    const char* s, uint64_t a, uint64_t b) {
    uint64_t w = Fetch64(s);
    uint64_t x = Fetch64(s + 8);
    uint64_t y = Fetch64(s + 16);
    uint64_t z = Fetch64(s + 24);
    a += w;
    b = Rotate(b + a + z, 21);
    uint64_t c = a;
    a += x;
    a += y;
    b += Rotate(a, 44);
    return make_pair(a + z, b + c);
}

static uint64_t HashLen33to64(const char* s, size_t len) {
    uint64_t mul = k2 + len * 2;
    uint64_t a = Fetch64(s) * k2;
    uint64_t b = Fetch64(s + 8);
    uint64_t c = Fetch64(s + len - 24);
    uint64_t d = Fetch64(s + len - 32);
    uint64_t e = Fetch64(s + 16) * k2;
    uint64_t f = Fetch64(s + 24) * 9;
    uint64_t g = Fetch64(s + len - 8);
    uint64_t h = Fetch64(s + len - 16) * mul;
    uint64_t u = Rotate(a + g, 43) + (Rotate(b, 30) + c) * 9;
    uint64_t v = ((a + g) ^ d) + f + 1;
    uint64_t w = Hash128to64(((unsigned __int128)(u + v) << 64) | v);
    uint64_t x = Rotate(e + f, 42) + c;
    uint64_t y = (Hash128to64(((unsigned __int128)(v + w) << 64) | w) + g) * mul;
    uint64_t z = e + f + c;
    a = Hash128to64(((unsigned __int128)(x + z) << 64) | z) + b;
    b = ShiftMix((z + a) * mul + d + h) * mul;
    return b + x;
}

uint64_t CityHash64(const char* s, size_t len) {
    if (len <= 32) {
        if (len <= 16) {
            return HashLen0to16(s, len);
        }
        else {
            return HashLen17to32(s, len);
        }
    }
    else if (len <= 64) {
        return HashLen33to64(s, len);
    }

    // For strings over 64 bytes we hash the end first, and then as we
    // loop we keep 56 bytes of state: v, w, x, y, and z.
    uint64_t x = Fetch64(s + len - 40);
    uint64_t y = Fetch64(s + len - 16) + Fetch64(s + len - 56);
    uint64_t z = HashLen16(Fetch64(s + len - 48) + len, Fetch64(s + len - 24));
    pair<uint64_t, uint64_t> v = WeakHashLen32WithSeeds(s + len - 64, len, z);
    pair<uint64_t, uint64_t> w = WeakHashLen32WithSeeds(s + len - 32, y + k1, x);
    x = x * k1 + Fetch64(s);

    // Decrease len to the nearest multiple of 64, and operate on 64-byte chunks.
    len = (len - 1) & ~static_cast<size_t>(63);
    do {
        x = Rotate(x + y + v.first + Fetch64(s + 8), 37) * k1;
        y = Rotate(y + v.second + Fetch64(s + 48), 42) * k1;
        x ^= w.second;
        y += v.first + Fetch64(s + 40);
        z = Rotate(z + w.first, 33) * k1;
        v = WeakHashLen32WithSeeds(s, v.second * k1, x + w.first);
        w = WeakHashLen32WithSeeds(s + 32, z + w.second, y + Fetch64(s + 16));
        std::swap(z, x);
        s += 64;
        len -= 64;
    } while (len != 0);
    return HashLen16(HashLen16(v.first, w.first) + ShiftMix(y) * k1 + z,
                    HashLen16(v.second, w.second) + x);
}

uint64_t CityHash64WithSeed(const char* s, size_t len, uint64_t seed) {
    // Implement CityHash64WithSeeds if required
    // For now, we'll assume it's similar to CityHash64
    // Modify as per your provided implementation
    return CityHash64(s, len) ^ seed;
}
