#pragma once

#include <algorithm>
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
 * @brief A string value type with a fixed memory size of 16 bytes, where the first 4 bytes
 * are reserved for the length of the string. When the lenght does not exceeds 12 characters
 * the string is stored in the remainig 12 bytes. Otherwise, 4 of the remaining 12 bytes
 * store a prefix of the string and the other 8 bytes a pointer to the string.
 */

#pragma pack(push, 1) // Solution for it to have no padding, and thus occupy 16 bytes. Im not well aware of it might
                      // have some negative side effects
class Umbra_t {
    uint32_t length; // 4 bytes
    union            // Remaining 12 Bytes. Also not aware of possible memory issues.
    {
        char short_str[12];
        struct {
            char prefix[4];
            char *ptr;
        } long_str;
    };

  public:
    // Default constructor
    Umbra_t() : length(0) { std::fill(short_str, short_str + 12, '\0'); }

    // Constructor from a C-style string
    Umbra_t(const char *str) {
        size_t len = std::strlen(str);
        if (len > UINT32_MAX) {
            throw std::length_error("String length exceeds maximum allowed");
        }
        length = static_cast<uint32_t>(len);
        if (length <= 12) {
            std::memcpy(short_str, str, length);
            std::fill(short_str + length, short_str + 12, '\0');
        } else {
            std::memcpy(long_str.prefix, str, 4);
            long_str.ptr = new char[length + 1];
            std::memcpy(long_str.ptr, str, length);
            long_str.ptr[length] = '\0';
        }

        static_assert(sizeof(Umbra_t) == 16, "Umbra_t size is not 16 bytes");
    }

    // Constructor from a std::string
    Umbra_t(const std::string &str) {
        size_t len = str.size();
        if (len > UINT32_MAX) {
            throw std::length_error("String length exceeds maximum allowed");
        }
        length = static_cast<uint32_t>(len);
        if (length <= 12) {
            str.copy(short_str, length);
            std::fill(short_str + length, short_str + 12, '\0');
        } else {
            str.copy(long_str.prefix, 4);
            long_str.ptr = new char[length + 1];
            str.copy(long_str.ptr, length);
            long_str.ptr[length] = '\0';
        }
        static_assert(sizeof(Umbra_t) == 16, "Umbra_t size is not 16 bytes");
    }

    // Copy constructor
    Umbra_t(const Umbra_t &other) {
        length = other.length;
        if (length <= 12) {
            std::memcpy(short_str, other.short_str, length);
            std::fill(short_str + length, short_str + 12, '\0');
        } else {
            std::memcpy(long_str.prefix, other.long_str.prefix, 4);
            long_str.ptr = new char[length + 1];
            std::memcpy(long_str.ptr, other.long_str.ptr, length);
            long_str.ptr[length] = '\0';
        }
        static_assert(sizeof(Umbra_t) == 16, "Umbra_t size is not 16 bytes");
    }

    // Destructor
    ~Umbra_t() {
        if (length > 12) {
            delete[] long_str.ptr;
            long_str.ptr = nullptr;
        }
    }

    // Assignment operator
    Umbra_t &operator=(const Umbra_t &other) {
        if (this != &other) {
            if (length > 12) {
                delete[] long_str.ptr;
            }
            length = other.length;
            if (length <= 12) {
                std::memcpy(short_str, other.short_str, length);
                std::fill(short_str + length, short_str + 12, '\0');
            } else {
                std::memcpy(long_str.prefix, other.long_str.prefix, 4);
                long_str.ptr = new char[length + 1];
                std::memcpy(long_str.ptr, other.long_str.ptr, length);
                long_str.ptr[length] = '\0';
            }
        }
        return *this;
    }

    // Equality comparison with other Umbra Strings
    bool operator==(const Umbra_t &other) const {
        if (length != other.length) {
            return false;
        }
        if (length <= 12) {
            return std::memcmp(short_str, other.short_str, length) == 0;
        } else {
            if (std::memcmp(long_str.prefix, other.long_str.prefix, 4) != 0) {
                return false;
            }
            return std::memcmp(long_str.ptr, other.long_str.ptr, length) == 0;
        }
    }

    // Equality comparison with other C-style strings
    bool operator==(const char *str) const {
        if (length != std::strlen(str)) {
            return false;
        }
        if (length <= 12) {
            return std::memcmp(short_str, str, length) == 0;
        } else {
            if (std::memcmp(long_str.prefix, str, 4) != 0) {
                return false;
            }
            return std::memcmp(long_str.ptr, str, length) == 0;
        }
    }

    // Inequality comparison with other Umbra Strings
    bool operator!=(const Umbra_t &other) const { return !(*this == other); }

    // Inequality comparison with other C-style strings
    bool operator!=(const char *str) const { return !(*this == str); }

    // Less-than comparison with other Umbra Strings
    bool operator<(const Umbra_t &other) const { return true; }

    // Less-than comparison with other C-style strings
    bool operator<(const char *str) const {
        uint32_t str_len = std::strlen(str);
        uint32_t min_length = std::min(length, str_len);
        int cmp;
        if (length <= 12 && str_len <= 12) {
            cmp = std::memcmp(short_str, str, min_length);
        } else {
            cmp = std::memcmp(long_str.prefix, str, 4);
            if (cmp == 0) {
                cmp = std::memcmp(long_str.ptr, str, min_length);
            }
        }
        if (cmp == 0) {
            return length < str_len;
        }
        return cmp < 0;
    }

    // Greater-than comparison with other Umbra Strings
    bool operator>(const Umbra_t &other) const { return *this != other && !(*this < other); }

    // Greater-than comparison with other C-style strings
    bool operator>(const char *str) const { return !(*this < str) && *this != str; }

    // TODO is this necessary?
    operator std::string() const { return this->to_string(); }

    // Concatenation Operation with other Umbra Strings.
    Umbra_t operator+(const Umbra_t &other) const {
        // Is there anything from the Umbra Strings we can take advantage of?
        std::string combined;
        combined.reserve(length + other.length);
        combined.append(this->to_string());
        combined.append(other.to_string());
        return Umbra_t(combined);
    }

    // Concatenation Operation with other C-style strings.
    Umbra_t operator+(const char *str) const {
        uint32_t str_length = std::strlen(str);
        uint32_t new_length = length + str_length;
        char *new_str = new char[new_length + 1];

        if (length <= 12) {
            std::memcpy(new_str, short_str, length);
        } else {
            std::memcpy(new_str, long_str.ptr, length);
        }

        std::memcpy(new_str + length, str, str_length);

        new_str[new_length] = '\0';
        Umbra_t result(new_str);
        delete[] new_str;
        return result;
    }

    // I dont know if this is actually right. Its in little-endian which probably goes against portability.
    void serialize(std::vector<char> &outBuffer) const {
        outBuffer.reserve(4 + length);

        outBuffer.push_back(static_cast<char>((length >> 0) & 0xFF));
        outBuffer.push_back(static_cast<char>((length >> 8) & 0xFF));
        outBuffer.push_back(static_cast<char>((length >> 16) & 0xFF));
        outBuffer.push_back(static_cast<char>((length >> 24) & 0xFF));

        if (length <= 12) {
            outBuffer.insert(outBuffer.end(), short_str, short_str + length);
        } else {
            outBuffer.insert(outBuffer.end(), long_str.ptr, long_str.ptr + length);
        }
    }

    size_t size() const { return length; }

    // Method to check if string is stored in long format
    bool is_long() const { return length > 12; }

    // TODO implement this method in all other methods where if/else is used to check for length
    const char *get() const {
        if (is_long()) {
            return long_str.ptr;
        }
        return short_str;
    }

    // Method to set the String. Check for better implementation
    void set(const char *str) {
        size_t len = std::strlen(str);

        if (len > UINT32_MAX) {
            throw std::length_error("String length exceeds maximum allowed");
        }

        if (is_long()) {
            delete[] long_str.ptr; // Clean up old string if previously long
        }

        length = static_cast<uint32_t>(len);

        if (length <= 12) {
            std::memcpy(short_str, str, length);
            std::fill(short_str + length, short_str + 12, '\0');

        } else {
            std::memcpy(long_str.prefix, str, 4);
            long_str.ptr = new char[length + 1];
            std::memcpy(long_str.ptr, str, length);
            long_str.ptr[length] = '\0';
        }
    }

    std::string to_string() const {
        if (length <= 12) {
            return std::string(short_str, length);
        }
        return std::string(long_str.ptr, size());
    }

    // Output stream operator
    friend std::ostream &operator<<(std::ostream &os, const Umbra_t &str) {
        if (str.length <= 12) {
            os.write(str.short_str, str.length);
        } else {
            os.write(str.long_str.ptr, str.length);
        }

        return os;
    }

    // Compare method similar to std::string::compare
    int compare(const char *str) const {
        uint32_t str_len = std::strlen(str);
        uint32_t min_length = std::min(length, str_len);
        int cmp;
        if (length <= 12) {
            cmp = std::memcmp(short_str, str, min_length);
        } else {
            cmp = std::memcmp(long_str.prefix, str, 4);
            if (cmp == 0) {
                cmp = std::memcmp(long_str.ptr, str, min_length);
            }
        }
        return cmp;
    }

    // Convert to lowercase
    Umbra_t lower() const {
        Umbra_t result(*this);
        if (length <= 12) {
            for (uint32_t i = 0; i < length; i++) {
                result.short_str[i] = static_cast<char>(std::tolower(result.short_str[i]));
            }
        } else {
            char *str = reinterpret_cast<char *>(result.long_str.ptr);
            for (uint32_t i = 0; i < length; i++) {
                str[i] = static_cast<char>(std::tolower(str[i]));
            }
            for (int i = 0; i < 4; i++) {
                result.long_str.prefix[i] = static_cast<char>(std::tolower(result.long_str.prefix[i]));
            }
        }
        return result;
    }

    // Convert to uppercase
    Umbra_t upper() const {
        Umbra_t result(*this);
        if (length <= 12) {
            for (uint32_t i = 0; i < length; i++) {
                result.short_str[i] = static_cast<char>(std::toupper(result.short_str[i]));
            }
        } else {
            char *str = reinterpret_cast<char *>(result.long_str.ptr);
            for (uint32_t i = 0; i < length; i++) {
                str[i] = static_cast<char>(std::toupper(str[i]));
            }
            for (int i = 0; i < 4; i++) {
                result.long_str.prefix[i] = static_cast<char>(std::toupper(result.long_str.prefix[i]));
            }
        }
        return result;
    }
};
#pragma pack(pop)

namespace std {
template <> struct hash<Umbra_t> {
    size_t operator()(const Umbra_t &u) const { return std::hash<std::string>()(u.to_string()); }
};
} // namespace std