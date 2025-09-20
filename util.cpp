#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

// --- Base64 Encoding/Decoding Library (by René Nyffenegger) ---
// This is a self-contained, header-style implementation for convenience.
// Original source: https://github.com/ReneNyffenegger/cpp-base64
static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

/**
 * @brief Converts a string or binary data to Base64.
 * @param input The raw data to encode.
 * @return The Base64 encoded string.
 */
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

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';
    }

    return ret;
}


/**
 * @brief Decodes a Base64 string.
 * @param encoded_string The Base64 string to decode.
 * @return The decoded raw data as a string.
 */
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
            for (i = 0; i <4; i++)
                char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = 0; j < i; j++)
            char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}
// --- End of Base64 Library ---


// --- Main Logic Functions ---

/**
 * @brief Encrypts a plaintext string using a 128-bit XOR cipher.
 * Note: This is a simple XOR cipher, NOT the AES algorithm.
 * @param plaintext The text to encrypt.
 * @param key A 16-character (128-bit) secret key.
 * @return The Base64 encoded ciphertext.
 * @throws std::invalid_argument if the key is not 16 characters long.
 */
std::string xorEncrypt(const std::string& plaintext, const std::string& key) {
    const size_t BLOCK_SIZE = 16;

    if (key.length() != BLOCK_SIZE) {
        throw std::invalid_argument("Key must be exactly 16 characters long.");
    }

    // 1. Add PKCS#7 Padding
    size_t padding_amount = BLOCK_SIZE - (plaintext.length() % BLOCK_SIZE);
    std::string padded_data = plaintext;
    padded_data.append(padding_amount, static_cast<char>(padding_amount));

    // 2. Perform XOR Operation
    std::string encrypted_data(padded_data.size(), '\0');
    for (size_t i = 0; i < padded_data.length(); ++i) {
        encrypted_data[i] = padded_data[i] ^ key[i % BLOCK_SIZE];
    }
    
    std::cout<<"Enrypted msg before base 64 : "<<encrypted_data<<std::endl;

    // 3. Encode to Base64 for output
    return base64_encode(encrypted_data);
}


/**
 * @brief Decrypts a Base64 ciphertext string using a 128-bit XOR cipher.
 * @param base64_ciphertext The Base64 encoded text to decrypt.
 * @param key A 16-character (128-bit) secret key.
 * @return The original decrypted plaintext.
 * @throws std::invalid_argument on invalid key, ciphertext length, or padding.
 */
std::string xorDecrypt(const std::string& base64_ciphertext, const std::string& key) {
    const size_t BLOCK_SIZE = 16;

    if (key.length() != BLOCK_SIZE) {
        throw std::invalid_argument("Key must be exactly 16 characters long.");
    }

    // 1. Decode from Base64
    std::string encrypted_bytes = base64_decode(base64_ciphertext);
    std::cout<<"Decoded msg before decryption :"<<encrypted_bytes<<std::endl;
    if (encrypted_bytes.length() % BLOCK_SIZE != 0) {
        throw std::runtime_error("Invalid ciphertext length. Must be a multiple of 16 bytes.");
    }

    // 2. Perform XOR Operation
    std::string decrypted_padded_bytes(encrypted_bytes.size(), '\0');
    for (size_t i = 0; i < encrypted_bytes.length(); ++i) {
        decrypted_padded_bytes[i] = encrypted_bytes[i] ^ key[i % BLOCK_SIZE];
    }

    // 3. Remove PKCS#7 Padding
    // Cast to unsigned char to correctly interpret values > 127
    size_t padding_amount = static_cast<unsigned char>(decrypted_padded_bytes.back());

    if (padding_amount == 0 || padding_amount > BLOCK_SIZE || decrypted_padded_bytes.length() < padding_amount) {
        throw std::runtime_error("Invalid padding amount.");
    }
    
    // Verify all padding bytes are correct
    for(size_t i = 1; i <= padding_amount; ++i) {
        if (static_cast<unsigned char>(decrypted_padded_bytes[decrypted_padded_bytes.length() - i]) != padding_amount) {
            throw std::runtime_error("Invalid padding bytes found.");
        }
    }

    return decrypted_padded_bytes.substr(0, decrypted_padded_bytes.length() - padding_amount);
}


// // --- Demonstration ---
// int main() {
//     std::string my_secret_message = "This is a test of the simple XOR cipher!";
//     std::string my_secret_key = "MySuperSecretKey"; // Must be 16 chars

//     std::cout << "Original Message: " << my_secret_message << std::endl;
//     std::cout << "Secret Key:       " << my_secret_key << std::endl;
//     std::cout << "------------------------------------------" << std::endl;

//     try {
//         // Encrypt the message
//         std::string encrypted = xorEncrypt(my_secret_message, my_secret_key);
//         std::cout << "Encrypted (Base64): " << encrypted << std::endl;

//         // Decrypt the message
//         std::string decrypted = xorDecrypt(encrypted, my_secret_key);
//         std::cout << "Decrypted Message:  " << decrypted << std::endl;

//         // Verify correctness
//         if (my_secret_message == decrypted) {
//             std::cout << "\n✅ Success! The decrypted message matches the original." << std::endl;
//         } else {
//             std::cout << "\n❌ Failure! The messages do not match." << std::endl;
//         }

//     } catch (const std::exception& e) {
//         std::cerr << "An error occurred: " << e.what() << std::endl;
//         return 1;
//     }

//     return 0;
// }