#ifndef HASHER_H
#define HASHER_H
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <cstdint>
#include "sha256.h"

class Hasher {
public:
    Hasher() = default;

    void begin();

    void addChunk(const uint8_t* data, size_t length);

    std::string finish();

    std::string hashBuffer(const uint8_t* data, size_t length);

    std::string hashString(const std::string& input);

private:
    SHA256_CTX ctx;
    uint8_t hash[32];
    bool isFinalized = false;

    std::string sha256ToHex(const uint8_t* hash);
};



#endif