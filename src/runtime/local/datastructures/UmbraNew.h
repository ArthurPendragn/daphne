#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * @brief A string value type with a fixed memory size of 16 bytes.
 *
 * Layout:
 * - Bytes 0-3: uint32_t length
 * - Bytes 4-15:
 *   - If length <= 12: short string stored directly.
 *   - If length > 12:
 *     - Bytes 4-7: Prefix (first 4 characters)
 *     - Bytes 8-15: Pointer to dynamically allocated long string.
 */
struct NewUmbra_t {
  private:
    std::array<uint8_t, 16> buffer;

    // Helper to get a pointer to the length field
    uint32_t &get_length() { return *reinterpret_cast<uint32_t *>(buffer.data()); }

    const uint32_t &get_length() const { return *reinterpret_cast<const uint32_t *>(buffer.data()); }

    // Helper to access short string
    char *short_str() { return reinterpret_cast<char *>(buffer.data() + 4); }

    const char *short_str() const { return reinterpret_cast<const char *>(buffer.data() + 4); }

    // Helper to access prefix
    char *prefix() { return reinterpret_cast<char *>(buffer.data() + 4); }

    const char *prefix() const { return reinterpret_cast<const char *>(buffer.data() + 4); }

    // Helper to access pointer to long string
    char *&long_ptr() { return *reinterpret_cast<char **>(buffer.data() + 8); }

    char *long_ptr() const { return *reinterpret_cast<char *const *>(buffer.data() + 8); }

    // Clear the buffer
    void clear_buffer() { buffer.fill(0); }

  public:
    // Default constructor
    NewUmbra_t() {
        clear_buffer();
        get_length() = 0;
    }

    // Constructor from a C-style string
    NewUmbra_t(const char *str) { set(str); }

    // Constructor from a std::string
    NewUmbra_t(const std::string &str) { set(str.c_str()); }

    // Copy constructor
    NewUmbra_t(const NewUmbra_t &other) {
        get_length() = other.get_length();
        if (get_length() <= 12) {
            std::memcpy(short_str(), other.short_str(), 12);
        } else {
            std::memcpy(prefix(), other.prefix(), 4);
            if (other.long_ptr()) {
                long_ptr() = new char[get_length() + 1];
                std::memcpy(long_ptr(), other.long_ptr(), get_length());
                long_ptr()[get_length()] = '\0';
            } else {
                long_ptr() = nullptr;
            }
        }
    }

    // Destructor
    ~NewUmbra_t() {
        if (is_long() && long_ptr()) {
            delete[] long_ptr();
            long_ptr() = nullptr;
        }
    }

    // Copy assignment operator
    NewUmbra_t &operator=(const NewUmbra_t &other) {
        if (this != &other) {
            // Clean up existing data
            if (is_long() && long_ptr()) {
                delete[] long_ptr();
            }

            get_length() = other.get_length();
            if (get_length() <= 12) {
                std::memcpy(short_str(), other.short_str(), 12);
            } else {
                std::memcpy(prefix(), other.prefix(), 4);
                if (other.long_ptr()) {
                    long_ptr() = new char[get_length() + 1];
                    std::memcpy(long_ptr(), other.long_ptr(), get_length());
                    long_ptr()[get_length()] = '\0';
                } else {
                    long_ptr() = nullptr;
                }
            }
        }
        return *this;
    }

    // Move constructor
    NewUmbra_t(NewUmbra_t &&other) noexcept {
        get_length() = other.get_length();
        if (get_length() <= 12) {
            std::memcpy(short_str(), other.short_str(), 12);
        } else {
            std::memcpy(prefix(), other.prefix(), 4);
            long_ptr() = other.long_ptr();
            other.long_ptr() = nullptr;
            other.get_length() = 0;
        }
        other.clear_buffer();
    }

    // Move assignment operator
    NewUmbra_t &operator=(NewUmbra_t &&other) noexcept {
        if (this != &other) {
            // Clean up existing data
            if (is_long() && long_ptr()) {
                delete[] long_ptr();
            }

            get_length() = other.get_length();
            if (get_length() <= 12) {
                std::memcpy(short_str(), other.short_str(), 12);
            } else {
                std::memcpy(prefix(), other.prefix(), 4);
                long_ptr() = other.long_ptr();
                other.long_ptr() = nullptr;
                other.get_length() = 0;
            }
            other.clear_buffer();
        }
        return *this;
    }

    // Equality comparison with other Umbra Strings
    bool operator==(const NewUmbra_t &other) const {
        if (get_length() != other.get_length())
            return false;
        if (get_length() <= 12) {
            return std::memcmp(short_str(), other.short_str(), get_length()) == 0;
        } else {
            return std::memcmp(long_ptr(), other.long_ptr(), get_length()) == 0;
        }
    }

    // Equality comparison with C-style strings
    bool operator==(const char *str) const {
        if (std::strlen(str) != get_length())
            return false;
        if (get_length() <= 12) {
            return std::memcmp(short_str(), str, get_length()) == 0;
        } else {
            return std::memcmp(long_ptr(), str, get_length()) == 0;
        }
    }

    // Inequality comparisons
    bool operator!=(const NewUmbra_t &other) const { return !(*this == other); }
    bool operator!=(const char *str) const { return !(*this == str); }

    // Less-than comparison with other Umbra Strings
    bool operator<(const NewUmbra_t &other) const {
        uint32_t min_length = std::min(get_length(), other.get_length());
        int cmp;
        if (get_length() <= 12 && other.get_length() <= 12) {
            cmp = std::memcmp(short_str(), other.short_str(), min_length);
        } else if (get_length() <= 12) {
            cmp = std::memcmp(short_str(), other.prefix(), 4);
            if (cmp == 0) {
                cmp = std::memcmp(short_str(), other.long_ptr(), min_length);
            }
        } else if (other.get_length() <= 12) {
            cmp = std::memcmp(prefix(), other.short_str(), 4);
            if (cmp == 0) {
                cmp = std::memcmp(long_ptr(), other.short_str(), min_length);
            }
        } else {
            cmp = std::memcmp(prefix(), other.prefix(), 4);
            if (cmp == 0) {
                cmp = std::memcmp(long_ptr(), other.long_ptr(), min_length);
            }
        }
        if (cmp == 0) {
            return get_length() < other.get_length();
        }
        return cmp < 0;
    }

    // Less-than comparison with C-style strings
    bool operator<(const char *str) const {
        uint32_t str_len = static_cast<uint32_t>(std::strlen(str));
        uint32_t min_length = std::min(get_length(), str_len);
        int cmp;
        if (get_length() <= 12 && str_len <= 12) {
            cmp = std::memcmp(short_str(), str, min_length);
        } else if (get_length() <= 12) {
            cmp = std::memcmp(short_str(), str, 4);
            if (cmp == 0) {
                cmp = std::memcmp(short_str(), str, min_length);
            }
        } else {
            cmp = std::memcmp(prefix(), str, 4);
            if (cmp == 0) {
                cmp = std::memcmp(long_ptr(), str, min_length);
            }
        }
        if (cmp == 0) {
            return get_length() < str_len;
        }
        return cmp < 0;
    }

    // Greater-than comparisons
    bool operator>(const NewUmbra_t &other) const { return !(*this < other) && !(*this == other); }
    bool operator>(const char *str) const { return !(*this < str) && !(*this == str); }

    // Conversion to std::string
    operator std::string() const { return to_string(); }

    // Concatenation with other Umbra Strings
    NewUmbra_t operator+(const NewUmbra_t &other) const {
        std::string concatenated = this->to_string() + other.to_string();
        return NewUmbra_t(concatenated);
    }

    // Concatenation with C-style strings
    NewUmbra_t operator+(const char *str) const {
        std::string concatenated = this->to_string() + std::string(str);
        return NewUmbra_t(concatenated);
    }

    // Serialize to a byte buffer
    void serialize(std::vector<char> &outBuffer) const {
        outBuffer.reserve(outBuffer.size() + 4 + get_length());

        uint32_t len = get_length();
        for (int i = 0; i < 4; ++i) {
            outBuffer.push_back(static_cast<char>((len >> (i * 8)) & 0xFF));
        }

        if (get_length() <= 12) {
            outBuffer.insert(outBuffer.end(), short_str(), short_str() + get_length());
        } else {
            outBuffer.insert(outBuffer.end(), long_ptr(), long_ptr() + get_length());
        }
    }

    // Get the size of the string
    inline size_t size() const { return get_length(); }

    // Check if the string is stored in long format
    inline bool is_long() const { return get_length() > 12; }

    // Get C-style string
    inline const char *get() const { return is_long() ? long_ptr() : short_str(); }

    // Set the string value
    void set(const char *str) {
        size_t len = std::strlen(str);
        if (len > UINT32_MAX) {
            throw std::length_error("String length exceeds maximum allowed");
        }

        // Clean up if currently long
        if (is_long() && long_ptr()) {
            delete[] long_ptr();
            long_ptr() = nullptr;
        }

        get_length() = static_cast<uint32_t>(len);

        if (get_length() <= 12) {
            std::memcpy(short_str(), str, get_length());
            std::fill(short_str() + get_length(), short_str() + 12, '\0');
        } else {
            std::memcpy(prefix(), str, 4);
            long_ptr() = new char[get_length() + 1];
            std::memcpy(long_ptr(), str, get_length());
            long_ptr()[get_length()] = '\0';
        }
    }

    // Convert to std::string
    std::string to_string() const {
        if (get_length() <= 12) {
            return std::string(short_str(), get_length());
        }
        return std::string(long_ptr(), get_length());
    }

    // Output stream operator
    friend std::ostream &operator<<(std::ostream &os, const NewUmbra_t &str) {
        if (str.is_long()) {
            if (str.long_ptr()) {
                os.write(str.long_ptr(), str.size());
            }
        } else {
            os.write(str.short_str(), str.size());
        }
        return os;
    }

    // Compare method similar to std::string::compare
    int compare(const char *str) const {
        uint32_t str_len = static_cast<uint32_t>(std::strlen(str));
        uint32_t min_length = std::min(get_length(), str_len);
        int cmp;
        if (get_length() <= 12) {
            cmp = std::memcmp(short_str(), str, min_length);
        } else {
            cmp = std::memcmp(long_ptr(), str, min_length);
        }
        if (cmp == 0) {
            if (get_length() < str_len)
                return -1;
            if (get_length() > str_len)
                return 1;
            return 0;
        }
        return cmp;
    }

    // Convert to lowercase
    NewUmbra_t lower() const {
        NewUmbra_t result(*this);
        if (result.is_long()) {
            for (uint32_t i = 0; i < result.size(); ++i) {
                result.long_ptr()[i] = static_cast<char>(std::tolower(result.long_ptr()[i]));
            }
        } else {
            for (uint32_t i = 0; i < result.size(); ++i) {
                result.short_str()[i] = static_cast<char>(std::tolower(result.short_str()[i]));
            }
        }
        return result;
    }

    // Convert to uppercase
    NewUmbra_t upper() const {
        NewUmbra_t result(*this);
        if (result.is_long()) {
            for (uint32_t i = 0; i < result.size(); ++i) {
                result.long_ptr()[i] = static_cast<char>(std::toupper(result.long_ptr()[i]));
            }
        } else {
            for (uint32_t i = 0; i < result.size(); ++i) {
                result.short_str()[i] = static_cast<char>(std::toupper(result.short_str()[i]));
            }
        }
        return result;
    }

    // Access the underlying buffer (for advanced byte manipulation)
    std::array<uint8_t, 16> &get_buffer() { return buffer; }
    const std::array<uint8_t, 16> &get_buffer() const { return buffer; }
};

// Hash specialization for NewUmbra_t
namespace std {
template <> struct hash<NewUmbra_t> {
    size_t operator()(const NewUmbra_t &key) const {
        return std::hash<std::string_view>()(std::string_view(key.get(), key.size()));
    }
};
} // namespace std