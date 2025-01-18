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

// Define constants for buffer layout
#define BUFFER_SIZE 16
#define LENGTH_OFFSET 0
#define LENGTH_SIZE 4
#define PREFIX_OFFSET 4
#define PREFIX_SIZE 4
#define LONG_PTR_OFFSET 8
#define LONG_PTR_SIZE 8
#define SHORT_STR_LEN 12

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
    std::array<uint8_t, BUFFER_SIZE> buffer;

    // Helper to get the length
    uint32_t get_length() const {
        uint32_t length;
        std::memcpy(&length, buffer.data() + LENGTH_OFFSET, LENGTH_SIZE);
        return length;
    }

    void set_length(uint32_t length) { std::memcpy(buffer.data() + LENGTH_OFFSET, &length, LENGTH_SIZE); }

    // Helper to get the prefix
    void get_prefix(char *dest) const { std::memcpy(dest, buffer.data() + PREFIX_OFFSET, PREFIX_SIZE); }

    void set_prefix(const char *src) { std::memcpy(buffer.data() + PREFIX_OFFSET, src, PREFIX_SIZE); }

    // Helper to get the long string pointer
    char *get_long_ptr() const {
        char *ptr = nullptr;
        std::memcpy(&ptr, buffer.data() + LONG_PTR_OFFSET, LONG_PTR_SIZE);
        return ptr;
    }

    void set_long_ptr(char *ptr) { std::memcpy(buffer.data() + LONG_PTR_OFFSET, &ptr, LONG_PTR_SIZE); }

    // Helper to access short string
    char *short_str() { return reinterpret_cast<char *>(buffer.data() + PREFIX_OFFSET); }

    const char *short_str() const { return reinterpret_cast<const char *>(buffer.data() + PREFIX_OFFSET); }

    // Clear the buffer
    void clear_buffer() { buffer.fill(0); }

  public:
    // Default constructor
    NewUmbra_t() {
        clear_buffer();
        set_length(0);
    }

    // Constructor from a C-style string
    NewUmbra_t(const char *str) { set(str); }

    // Constructor from a std::string
    NewUmbra_t(const std::string &str) { set(str.c_str()); }

    // Copy constructor
    NewUmbra_t(const NewUmbra_t &other) {
        set_length(other.get_length());
        if (get_length() <= SHORT_STR_LEN) {
            std::memcpy(short_str(), other.short_str(), SHORT_STR_LEN);
        } else {
            char prefix[PREFIX_SIZE];
            other.get_prefix(prefix);
            set_prefix(prefix);
            if (other.get_long_ptr()) {
                char *new_long_ptr = new char[get_length() + 1];
                std::memcpy(new_long_ptr, other.get_long_ptr(), get_length());
                new_long_ptr[get_length()] = '\0';
                set_long_ptr(new_long_ptr);
            } else {
                set_long_ptr(nullptr);
            }
        }
    }

    // Destructor
    ~NewUmbra_t() {
        if (is_long() && get_long_ptr()) {
            delete[] get_long_ptr();
            set_long_ptr(nullptr);
        }
    }

    // Copy assignment operator using copy-and-swap idiom
    NewUmbra_t &operator=(NewUmbra_t other) {
        swap(*this, other);
        return *this;
    }

    // Move constructor
    NewUmbra_t(NewUmbra_t &&other) noexcept {
        set_length(other.get_length());
        if (get_length() <= SHORT_STR_LEN) {
            std::memcpy(short_str(), other.short_str(), SHORT_STR_LEN);
        } else {
            char prefix[PREFIX_SIZE];
            other.get_prefix(prefix);
            set_prefix(prefix);
            set_long_ptr(other.get_long_ptr());
            other.set_long_ptr(nullptr);
            other.set_length(0);
        }
        other.clear_buffer();
    }

    // Move assignment operator
    NewUmbra_t &operator=(NewUmbra_t &&other) noexcept {
        if (this != &other) {
            // Clean up existing data
            if (is_long() && get_long_ptr()) {
                delete[] get_long_ptr();
            }

            set_length(other.get_length());
            if (get_length() <= SHORT_STR_LEN) {
                std::memcpy(short_str(), other.short_str(), SHORT_STR_LEN);
            } else {
                char prefix[PREFIX_SIZE];
                other.get_prefix(prefix);
                set_prefix(prefix);
                set_long_ptr(other.get_long_ptr());
                other.set_long_ptr(nullptr);
            }
            other.clear_buffer();
        }
        return *this;
    }

    // Swap function for copy-and-swap
    friend void swap(NewUmbra_t &first, NewUmbra_t &second) noexcept { std::swap(first.buffer, second.buffer); }

    // Equality comparison with other Umbra Strings
    bool operator==(const NewUmbra_t &other) const {
        if (get_length() != other.get_length())
            return false;
        if (get_length() <= SHORT_STR_LEN) {
            return std::memcmp(short_str(), other.short_str(), get_length()) == 0;
        } else {
            if (get_long_ptr() == nullptr || other.get_long_ptr() == nullptr)
                return false;
            return std::memcmp(get_long_ptr(), other.get_long_ptr(), get_length()) == 0;
        }
    }

    // Equality comparison with C-style strings
    bool operator==(const char *str) const {
        uint32_t str_len = static_cast<uint32_t>(std::strlen(str));
        if (str_len != get_length())
            return false;
        if (get_length() <= SHORT_STR_LEN) {
            return std::memcmp(short_str(), str, get_length()) == 0;
        } else {
            if (get_long_ptr() == nullptr)
                return false;
            return std::memcmp(get_long_ptr(), str, get_length()) == 0;
        }
    }

    // Inequality comparisons
    bool operator!=(const NewUmbra_t &other) const { return !(*this == other); }
    bool operator!=(const char *str) const { return !(*this == str); }

    // Less-than comparison with other Umbra Strings
    bool operator<(const NewUmbra_t &other) const {
        return std::lexicographical_compare(get(), get() + size(), other.get(), other.get() + other.size());
    }

    // Less-than comparison with C-style strings
    bool operator<(const char *str) const {
        return std::lexicographical_compare(get(), get() + size(), str, str + std::strlen(str));
    }

    // Greater-than comparisons
    bool operator>(const NewUmbra_t &other) const { return other < *this; }
    bool operator>(const char *str) const {
        return std::lexicographical_compare(str, str + std::strlen(str), get(), get() + size());
    }

    // Convert to std::string
    std::string to_string() const {
        if (get_length() <= SHORT_STR_LEN) {
            return std::string(short_str(), get_length());
        }
        if (get_long_ptr() == nullptr)
            return std::string();
        return std::string(get_long_ptr(), get_length());
    }

    // Concatenation with other Umbra Strings
    NewUmbra_t operator+(const NewUmbra_t &other) const { return NewUmbra_t(this->to_string() + other.to_string()); }

    // Concatenation with C-style strings
    NewUmbra_t operator+(const char *str) const { return NewUmbra_t(this->to_string() + std::string(str)); }

    // Serialize to a byte buffer
    void serialize(std::vector<char> &outBuffer) const {
        outBuffer.reserve(outBuffer.size() + LENGTH_SIZE + get_length());

        uint32_t len = get_length();
        // Serialize length in little-endian format
        for (int i = 0; i < 4; ++i) {
            outBuffer.push_back(static_cast<char>((len >> (i * 8)) & 0xFF));
        }

        if (get_length() <= SHORT_STR_LEN) {
            outBuffer.insert(outBuffer.end(), short_str(), short_str() + get_length());
        } else {
            if (get_long_ptr()) {
                outBuffer.insert(outBuffer.end(), get_long_ptr(), get_long_ptr() + get_length());
            }
        }
    }

    // Get the size of the string
    inline size_t size() const { return get_length(); }

    // Check if the string is stored in long format
    inline bool is_long() const { return get_length() > SHORT_STR_LEN; }

    // Get C-style string
    inline const char *get() const { return is_long() ? (get_long_ptr() ? get_long_ptr() : "") : short_str(); }

    // Set the string value
    void set(const char *str) {
        size_t len = std::strlen(str);
        if (len > UINT32_MAX) {
            throw std::length_error("String length exceeds maximum allowed");
        }

        // If currently storing a long string, clean it up
        if (is_long() && get_long_ptr()) {
            delete[] get_long_ptr();
            set_long_ptr(nullptr);
        }

        set_length(static_cast<uint32_t>(len));

        if (get_length() <= SHORT_STR_LEN) {
            std::memcpy(short_str(), str, get_length());
            std::fill(short_str() + get_length(), short_str() + PREFIX_SIZE + (SHORT_STR_LEN - PREFIX_SIZE), '\0');
        } else {
            // Store prefix
            set_prefix(str);
            // Allocate and store long string
            char *new_long_ptr = new char[get_length() + 1];
            std::memcpy(new_long_ptr, str, get_length());
            new_long_ptr[get_length()] = '\0';
            set_long_ptr(new_long_ptr);
        }
    }

    // Output stream operator
    friend std::ostream &operator<<(std::ostream &os, const NewUmbra_t &str) {
        os.write(str.get(), str.size());
        return os;
    }

    // Compare method similar to std::string::compare
    int compare(const char *str) const {
        uint32_t str_len = static_cast<uint32_t>(std::strlen(str));
        uint32_t min_length = std::min(get_length(), str_len);
        int cmp = std::memcmp(get(), str, min_length);
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
        char *target = result.is_long() ? result.get_long_ptr() : result.short_str();
        for (uint32_t i = 0; i < result.size(); ++i) {
            target[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(target[i])));
        }
        return result;
    }

    // Convert to uppercase
    NewUmbra_t upper() const {
        NewUmbra_t result(*this);
        char *target = result.is_long() ? result.get_long_ptr() : result.short_str();
        for (uint32_t i = 0; i < result.size(); ++i) {
            target[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(target[i])));
        }
        return result;
    }

    // Access the underlying buffer (read-only)
    const std::array<uint8_t, BUFFER_SIZE> &get_buffer() const { return buffer; }
};

// Hash specialization for NewUmbra_t
namespace std {
template <> struct hash<NewUmbra_t> {
    size_t operator()(const NewUmbra_t &key) const {
        return std::hash<std::string_view>()(std::string_view(key.get(), key.size()));
    }
};
} // namespace std