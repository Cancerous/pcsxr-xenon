#include <stdint.h>

// byteswappings
#include <byteswap.h>
#define SWAP16(x) bswap_16(x)
#define SWAP32(x) __builtin_bswap32(x)

// big endian config
#define HOST2LE32(x) SWAP32(x)
#define HOST2BE32(x) (x)
#define LE2HOST32(x) SWAP32(x)
#define BE2HOST32(x) (x)

#define HOST2LE16(x) SWAP16(x)
#define HOST2BE16(x) (x)
#define LE2HOST16(x) SWAP16(x)
#define BE2HOST16(x) (x)

#define GETLEs16(X) ((int16_t)GETLE16((uint16_t *)X))
#define GETLEs32(X) ((int16_t)GETLE32((uint16_t *)X))


// GCC style
static __inline__ uint16_t GETLE16(uint16_t *ptr) {
    uint16_t ret; __asm__ ("lhbrx %0, 0, %1" : "=r" (ret) : "r" (ptr));
    return ret;
}
static __inline__ uint32_t GETLE32(uint32_t *ptr) {
    uint32_t ret;
    __asm__ ("lwbrx %0, 0, %1" : "=r" (ret) : "r" (ptr));
    return ret;
}
static __inline__ uint32_t GETLE16D(uint32_t *ptr) {
    uint32_t ret;
    __asm__ ("lwbrx %0, 0, %1\n"
             "rlwinm %0, %0, 16, 0, 31" : "=r" (ret) : "r" (ptr));
    return ret;
}

static __inline__ void PUTLE16(uint16_t *ptr, uint16_t val) {
    __asm__ ("sthbrx %0, 0, %1" : : "r" (val), "r" (ptr) : "memory");
}
static __inline__ void PUTLE32(uint32_t *ptr, uint32_t val) {
    __asm__ ("stwbrx %0, 0, %1" : : "r" (val), "r" (ptr) : "memory");
}

