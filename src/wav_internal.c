#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "wav_internal.h"

uint32_t read_le32(const uint8_t* p) {
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        uint32_t val;
        memcpy(&val, p, sizeof(val));
        return val;
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        return  (uint32_t)p[0]        | ((uint32_t)p[1] << 8) |
                ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    #else
        #error "Unsupported byte order"
    #endif
}

uint16_t read_le16(const uint8_t* p) {
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        uint16_t val;
        memcpy(&val, p, sizeof(val));
        return val;
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
    #else
        #error "Unsupported byte order"
    #endif
}

void write_le16(const uint8_t* p, FILE* file) {
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        fwrite((const uint16_t*)p, 1, sizeof(uint16_t), file);
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        uint16_t le16 = (uint16_t)p[0] | ((uint16_t)p[1] << 8);
        fwrite(&le16, 1, sizeof(uint16_t), file);
    #else
        #error "Unsupported byte order"
    #endif
}

void write_le32(const uint8_t* p, FILE* file) {
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        fwrite((const uint32_t*)p, 1, sizeof(uint32_t), file);
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        uint32_t le32 = (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
        fwrite(&le32, 1, sizeof(uint32_t), file);
    #else
        #error "Unsupported byte order"
    #endif
}
