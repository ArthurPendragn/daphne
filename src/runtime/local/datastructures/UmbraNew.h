#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

class NewUmbra_t {
  private:
    static constexpr std::size_t SHORT_CAPACITY = 12;

    uint32_t length = 0;

    char inlineData[SHORT_CAPACITY];

    std::unique_ptr<char[]> allocated;
    char *data = inlineData;

    inline void updateDataPtr() {
        if (length <= SHORT_CAPACITY) {
            data = inlineData;
        } else {
            data = allocated.get();
        }
    }

  public:
    NewUmbra_t() { inlineData[0] = '\0'; }

    NewUmbra_t(const char *str) { set(str); }

    NewUmbra_t(const std::string &s) { set(s.c_str()); }

    NewUmbra_t(const NewUmbra_t &other) : length(other.length) {
        if (length <= SHORT_CAPACITY) {
            std::memcpy(inlineData, other.inlineData, length);
            inlineData[length] = '\0';
            data = inlineData;
        } else {
            allocated = std::make_unique<char[]>(length + 1);
            std::memcpy(allocated.get(), other.data, length);
            allocated[length] = '\0';
            data = allocated.get();
        }
    }

    NewUmbra_t(NewUmbra_t &&other) noexcept : length(other.length), allocated(std::move(other.allocated)) {
        if (length <= SHORT_CAPACITY) {
            std::memcpy(inlineData, other.inlineData, length);
            inlineData[length] = '\0';
            data = inlineData;
        } else {
            data = allocated.get();
        }
        other.length = 0;
        other.data = other.inlineData;
        other.inlineData[0] = '\0';
    }

    ~NewUmbra_t() = default;

    NewUmbra_t &operator=(const NewUmbra_t &other) {
        if (this == &other) {
            return *this;
        }
        length = other.length;
        if (length <= SHORT_CAPACITY) {
            allocated.reset();
            std::memcpy(inlineData, other.inlineData, length);
            inlineData[length] = '\0';
            data = inlineData;
        } else {
            if (!allocated || (length > capacity())) {
                allocated = std::make_unique<char[]>(length + 1);
            }
            std::memcpy(allocated.get(), other.data, length);
            allocated[length] = '\0';
            data = allocated.get();
        }
        return *this;
    }

    NewUmbra_t &operator=(NewUmbra_t &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        length = other.length;
        allocated = std::move(other.allocated);

        if (length <= SHORT_CAPACITY) {
            std::memcpy(inlineData, other.inlineData, length);
            inlineData[length] = '\0';
            data = inlineData;
        } else {
            data = allocated.get();
        }

        other.length = 0;
        other.data = other.inlineData;
        other.inlineData[0] = '\0';
        return *this;
    }

    static constexpr std::size_t inlineCapacity() { return SHORT_CAPACITY; }

    inline std::size_t capacity() const {
        if (!allocated) {
            return SHORT_CAPACITY;
        }
        return length + 1;
    }

    void set(const char *str) {
        if (!str) {
            clear();
            return;
        }
        std::size_t len = std::strlen(str);
        if (len > UINT32_MAX) {
            throw std::length_error("NewUmbra_t: length exceeds UINT32_MAX");
        }
        length = static_cast<uint32_t>(len);
        if (length <= SHORT_CAPACITY) {
            allocated.reset(); // free any old buffer
            std::memcpy(inlineData, str, length);
            inlineData[length] = '\0';
            data = inlineData;
        } else {
            if (!allocated || length > capacity()) {
                allocated = std::make_unique<char[]>(length + 1);
            }
            std::memcpy(allocated.get(), str, length);
            allocated[length] = '\0';
            data = allocated.get();
        }
    }

    void clear() {
        allocated.reset();
        inlineData[0] = '\0';
        length = 0;
        data = inlineData;
    }

    inline std::size_t size() const noexcept { return length; }
    inline bool empty() const noexcept { return length == 0; }
    inline const char *c_str() const noexcept { return data; }

    inline const char *get() const noexcept { return data; }

    inline std::string to_string() const { return std::string(data, length); }

    operator std::string() const { return to_string(); }

    char operator[](std::size_t idx) const { return data[idx]; }

    bool operator==(const NewUmbra_t &other) const {
        if (length != other.length) {
            return false;
        }
        return (std::memcmp(data, other.data, length) == 0);
    }
    bool operator!=(const NewUmbra_t &other) const { return !(*this == other); }

    bool operator<(const NewUmbra_t &other) const {
        auto min_len = std::min(length, other.length);
        int cmp = std::memcmp(data, other.data, min_len);
        if (cmp == 0) {
            return length < other.length;
        }
        return (cmp < 0);
    }
    bool operator>(const NewUmbra_t &other) const { return (other < *this); }

    bool operator==(const char *str) const { return std::strcmp(data, str) == 0; }
    bool operator!=(const char *str) const { return !(*this == str); }
    bool operator<(const char *str) const { return std::strcmp(data, str) < 0; }
    bool operator>(const char *str) const { return std::strcmp(data, str) > 0; }

    NewUmbra_t operator+(const NewUmbra_t &other) const {
        NewUmbra_t result;
        std::size_t new_len = static_cast<std::size_t>(length) + other.length;
        if (new_len <= SHORT_CAPACITY) {
            result.length = static_cast<uint32_t>(new_len);
            result.allocated.reset();
            std::memcpy(result.inlineData, data, length);
            std::memcpy(result.inlineData + length, other.data, other.length);
            result.inlineData[new_len] = '\0';
            result.data = result.inlineData;
        } else {
            result.length = static_cast<uint32_t>(new_len);
            result.allocated = std::make_unique<char[]>(new_len + 1);
            std::memcpy(result.allocated.get(), data, length);
            std::memcpy(result.allocated.get() + length, other.data, other.length);
            result.allocated[new_len] = '\0';
            result.data = result.allocated.get();
        }
        return result;
    }

    NewUmbra_t operator+(const char *str) const {
        NewUmbra_t temp(str);
        return *this + temp;
    }

    NewUmbra_t lower() const {
        NewUmbra_t result(*this);
        for (uint32_t i = 0; i < result.length; ++i) {
            result.data[i] = static_cast<char>(std::tolower(result.data[i]));
        }
        return result;
    }

    NewUmbra_t upper() const {
        NewUmbra_t result(*this);
        for (uint32_t i = 0; i < result.length; ++i) {
            result.data[i] = static_cast<char>(std::toupper(result.data[i]));
        }
        return result;
    }

    friend std::ostream &operator<<(std::ostream &os, const NewUmbra_t &um) { return os.write(um.data, um.length); }
};

namespace std {
template <> struct hash<NewUmbra_t> {
    size_t operator()(const NewUmbra_t &key) const {
        return std::hash<std::string_view>()(std::string_view(key.get(), key.size()));
    }
};
} 