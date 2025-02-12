#include <array>
#include <memory>
#include <openssl/evp.h>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>

class StringPRP {
public:
  static constexpr size_t BLOCK_SIZE = 16;

  explicit StringPRP(std::string_view key) {
    if (key.size() != BLOCK_SIZE) {
      throw std::invalid_argument("Key must be exactly 16 bytes");
    }
    std::copy(key.begin(), key.end(), key_.begin());
  }

  // Core PRP function: string -> string
  std::string permute(std::string_view input) const {
    if (input.size() > BLOCK_SIZE) {
      throw std::invalid_argument("Input must not exceed 16 bytes");
    }

    // Create cipher context with RAII
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx(
        EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

    if (!ctx) {
      throw std::runtime_error("Failed to create cipher context");
    }

    // Initialize encryption (AES-128 in CTR mode)
    if (1 !=
        EVP_EncryptInit_ex(ctx.get(), EVP_aes_128_ctr(), nullptr,
                           reinterpret_cast<const unsigned char *>(key_.data()),
                           nullptr)) {
      throw std::runtime_error("Failed to initialize encryption");
    }

    // Prepare buffers
    std::array<unsigned char, BLOCK_SIZE> padded_input{0};
    std::array<unsigned char, BLOCK_SIZE> output{0};

    // Copy input with zero padding
    std::copy(input.begin(), input.end(), padded_input.begin());

    int out_len1 = 0;
    if (1 != EVP_EncryptUpdate(ctx.get(), output.data(), &out_len1,
                               padded_input.data(), padded_input.size())) {
      throw std::runtime_error("Failed to encrypt data");
    }

    int out_len2 = 0;
    if (1 !=
        EVP_EncryptFinal_ex(ctx.get(), output.data() + out_len1, &out_len2)) {
      throw std::runtime_error("Failed to finalize encryption");
    }

    return std::string(reinterpret_cast<char *>(output.data()), BLOCK_SIZE);
  }

private:
  std::array<char, BLOCK_SIZE> key_;
};

// Example usage:
// int main() {
//     try {
//         // Create a PRP with a 16-byte key
//         std::string key = "0123456789abcdef";  // 16 bytes
//         StringPRP prp(key);

//         // Permute a string (must be ≤ 16 bytes)
//         std::string input = "test_string";
//         std::string permuted = prp.permute(input);

//         // The permuted string will always be exactly 16 bytes
//         // You can now use this permuted string in your Bloom filter
//     } catch (const std::exception& e) {
//         std::cerr << "Error: " << e.what() << std::endl;
//         return 1;
//     }
//     return 0;
// }
