#include "util.h"
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>

// --- Base64 Encoding/Decoding Library (by René Nyffenegger) ---
static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(const std::string& input) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    const char* bytes_to_encode = input.c_str();
    size_t in_len = input.length();
    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for(i = 0; (i <4) ; i++) ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    if (i) {
        for(j = i; j < 3; j++) char_array_3[j] = '\0';
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        for (j = 0; (j < i + 1); j++) ret += base64_chars[char_array_4[j]];
        while((i++ < 3)) ret += '=';
    }
    return ret;
}

std::string base64_decode(const std::string& encoded_string) {
    size_t in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;
    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++) char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            for (i = 0; (i < 3); i++) ret += char_array_3[i];
            i = 0;
        }
    }
    if (i) {
        for (j = 0; j < i; j++) char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));
        for (j = i; j <4; j++) char_array_4[j] = 0;
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }
    return ret;
}

// --- Main Logic Functions ---
std::string xorEncrypt(const std::string& plaintext, const std::string& key) {
    const size_t BLOCK_SIZE = 16;
    if (key.length() != BLOCK_SIZE) { throw std::invalid_argument("Key must be exactly 16 characters long."); }
    size_t padding_amount = BLOCK_SIZE - (plaintext.length() % BLOCK_SIZE);
    std::string padded_data = plaintext;
    padded_data.append(padding_amount, static_cast<char>(padding_amount));
    std::string encrypted_data(padded_data.size(), '\0');
    for (size_t i = 0; i < padded_data.length(); ++i) {
        encrypted_data[i] = padded_data[i] ^ key[i % BLOCK_SIZE];
    }
    return base64_encode(encrypted_data);
}

std::string xorDecrypt(const std::string& base64_ciphertext, const std::string& key) {
    const size_t BLOCK_SIZE = 16;
    if (key.length() != BLOCK_SIZE) { return ""; } // Invalid key
    std::string encrypted_bytes = base64_decode(base64_ciphertext);
    if (encrypted_bytes.length() % BLOCK_SIZE != 0) { return ""; } // Invalid length
    std::string decrypted_padded_bytes(encrypted_bytes.size(), '\0');
    for (size_t i = 0; i < encrypted_bytes.length(); ++i) {
        decrypted_padded_bytes[i] = encrypted_bytes[i] ^ key[i % BLOCK_SIZE];
    }
    if (decrypted_padded_bytes.empty()) { return ""; }
    size_t padding_amount = static_cast<unsigned char>(decrypted_padded_bytes.back());
    if (padding_amount == 0 || padding_amount > BLOCK_SIZE || decrypted_padded_bytes.length() < padding_amount) {
        return ""; // Invalid padding amount
    }
    for(size_t i = 1; i <= padding_amount; ++i) {
        if (static_cast<unsigned char>(decrypted_padded_bytes[decrypted_padded_bytes.length() - i]) != padding_amount) {
            return ""; // Invalid padding bytes
        }
    }
    return decrypted_padded_bytes.substr(0, decrypted_padded_bytes.length() - padding_amount);
}
