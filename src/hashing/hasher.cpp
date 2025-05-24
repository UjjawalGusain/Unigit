#include "hasher.h"
#include "sha256.h"
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

void Hasher::begin() {
    sha256_init(&ctx);
    isFinalized = false;
}

void Hasher::addChunk(const uint8_t *data, size_t length) {
    if (!isFinalized) {
        sha256_update(&ctx, data, length);
    } else {
        std::cerr << "Error: Hash already finalized.\n";
    }
}

std::string Hasher::finish() {
    if (!isFinalized) {
        sha256_final(&ctx, hash);
        isFinalized = true;
    }
    return sha256ToHex(hash);
}

std::string Hasher::hashBuffer(const uint8_t *data, size_t length) {
    begin();
    addChunk(data, length);
    return finish();
}

std::string Hasher::hashString(const std::string &input) {
    return hashBuffer(reinterpret_cast<const uint8_t *>(input.c_str()), input.size());
}

std::string Hasher::sha256ToHex(const uint8_t *hash) {
    std::ostringstream oss;
    for (int i = 0; i < 32; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return oss.str();
}
