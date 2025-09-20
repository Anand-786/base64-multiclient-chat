#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>
#include <stdexcept>

// Declares the encryption function that takes a plaintext string and a key.
// It returns a Base64 encoded, XOR-encrypted string.
std::string xorEncrypt(const std::string& plaintext, const std::string& key);

// Declares the decryption function that takes a Base64 encoded ciphertext and a key.
// It decodes and decrypts the message, returning the original plaintext.
std::string xorDecrypt(const std::string& base64_ciphertext, const std::string& key);

#endif // UTIL_H
