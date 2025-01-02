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

class Umbra_t {
  private:
    // We store up to this many characters inline.
    // Adjust SHORT_CAPACITY as needed (was 12 in the original).
    static constexpr std::size_t SHORT_CAPACITY = 12;

    // Current length of the string (in characters).
    uint32_t length_ = 0;

    // For short strings, data is stored in this inline buffer.
    // For long strings, we allocate a buffer on the heap
    // and store it in `allocated_`. We always keep `data_`
    // pointing to the valid buffer (inlineData_ or allocated_.get()).
    char inlineData_[SHORT_CAPACITY];

    // If the string is longer than SHORT_CAPACITY, we own a heap buffer here.
    std::unique_ptr<char[]> allocated_;

    // Points to either inlineData_ (short string) or allocated_.get() (long string).
    // This allows us to unify most operations under a single pointer,
    // instead of branching on `length_ <= 12`.
    char *data_ = inlineData_;

    // Helper: re-point data_ to inlineData_ or allocated_.get() based on length_.
    inline void updateDataPtr() {
        if (length_ <= SHORT_CAPACITY) {
            data_ = inlineData_;
        } else {
            data_ = allocated_.get();
        }
    }

  public:
    //---------------------
    // Constructors
    //---------------------
    Umbra_t() { inlineData_[0] = '\0'; }

    // Construct from C-string
    Umbra_t(const char *str) { set(str); }

    // Construct from std::string
    Umbra_t(const std::string &s) { set(s.c_str()); }

    // Copy constructor
    Umbra_t(const Umbra_t &other) : length_(other.length_) {
        if (length_ <= SHORT_CAPACITY) {
            // Copy short inline data
            std::memcpy(inlineData_, other.inlineData_, length_);
            inlineData_[length_] = '\0';
            data_ = inlineData_;
        } else {
            // Copy from other's allocated buffer
            allocated_ = std::make_unique<char[]>(length_ + 1);
            std::memcpy(allocated_.get(), other.data_, length_);
            allocated_[length_] = '\0';
            data_ = allocated_.get();
        }
    }

    // Move constructor
    Umbra_t(Umbra_t &&other) noexcept : length_(other.length_), allocated_(std::move(other.allocated_)) {
        if (length_ <= SHORT_CAPACITY) {
            // The moved-from object was short; copy its inline data
            std::memcpy(inlineData_, other.inlineData_, length_);
            inlineData_[length_] = '\0';
            data_ = inlineData_;
        } else {
            // The moved-from object was long; we take its allocated buffer
            data_ = allocated_.get();
        }
        // Reset 'other' to a valid empty state
        other.length_ = 0;
        other.data_ = other.inlineData_;
        other.inlineData_[0] = '\0';
    }

    //---------------------
    // Destructor
    //---------------------
    ~Umbra_t() = default; // unique_ptr handles allocated memory if any

    //---------------------
    // Assignment operators
    //---------------------
    Umbra_t &operator=(const Umbra_t &other) {
        if (this == &other) {
            return *this;
        }
        length_ = other.length_;
        if (length_ <= SHORT_CAPACITY) {
            // Copy short data
            allocated_.reset(); // free any old allocation
            std::memcpy(inlineData_, other.inlineData_, length_);
            inlineData_[length_] = '\0';
            data_ = inlineData_;
        } else {
            // Copy long data
            if (!allocated_ || (length_ > capacity())) {
                allocated_ = std::make_unique<char[]>(length_ + 1);
            }
            std::memcpy(allocated_.get(), other.data_, length_);
            allocated_[length_] = '\0';
            data_ = allocated_.get();
        }
        return *this;
    }

    // Move assignment
    Umbra_t &operator=(Umbra_t &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        length_ = other.length_;
        allocated_ = std::move(other.allocated_);

        if (length_ <= SHORT_CAPACITY) {
            // Copy inline data from 'other'
            std::memcpy(inlineData_, other.inlineData_, length_);
            inlineData_[length_] = '\0';
            data_ = inlineData_;
        } else {
            // We take the allocated buffer
            data_ = allocated_.get();
        }

        // Reset 'other'
        other.length_ = 0;
        other.data_ = other.inlineData_;
        other.inlineData_[0] = '\0';
        return *this;
    }

    //---------------------
    // Capacity helpers
    //---------------------
    // How many characters can we store inline?
    static constexpr std::size_t inlineCapacity() { return SHORT_CAPACITY; }

    // If we have an allocated buffer, how large is it?
    // (This is optional in a minimal design, but can be useful.)
    inline std::size_t capacity() const {
        if (!allocated_) {
            return SHORT_CAPACITY;
        }
        // allocated_.get() points to a buffer that is length_+1 in size
        return length_ + 1;
    }

    //---------------------
    // Modifiers
    //---------------------
    void set(const char *str) {
        if (!str) {
            // setting to null? interpret as empty
            clear();
            return;
        }
        std::size_t len = std::strlen(str);
        if (len > UINT32_MAX) {
            throw std::length_error("Umbra_t: length exceeds UINT32_MAX");
        }
        length_ = static_cast<uint32_t>(len);
        if (length_ <= SHORT_CAPACITY) {
            allocated_.reset(); // free any old buffer
            std::memcpy(inlineData_, str, length_);
            inlineData_[length_] = '\0';
            data_ = inlineData_;
        } else {
            // Need allocated storage
            if (!allocated_ || length_ > capacity()) {
                allocated_ = std::make_unique<char[]>(length_ + 1);
            }
            std::memcpy(allocated_.get(), str, length_);
            allocated_[length_] = '\0';
            data_ = allocated_.get();
        }
    }

    void clear() {
        allocated_.reset();
        inlineData_[0] = '\0';
        length_ = 0;
        data_ = inlineData_;
    }

    //---------------------
    // Observers
    //---------------------
    inline std::size_t size() const noexcept { return length_; }
    inline bool empty() const noexcept { return length_ == 0; }
    inline const char *c_str() const noexcept { return data_; }

    // Returns a pointer to the internal buffer. This is read-only unless you
    // carefully manage that the string length does not exceed SHORT_CAPACITY if inlined.
    inline const char *data() const noexcept { return data_; }

    // Convert to std::string
    inline std::string to_string() const { return std::string(data_, length_); }

    // Operator std::string for convenience
    operator std::string() const { return to_string(); }

    // Basic indexing (optional)
    char operator[](std::size_t idx) const { return data_[idx]; }

    //---------------------
    // Comparison
    //---------------------
    bool operator==(const Umbra_t &other) const {
        if (length_ != other.length_) {
            return false;
        }
        return (std::memcmp(data_, other.data_, length_) == 0);
    }
    bool operator!=(const Umbra_t &other) const { return !(*this == other); }

    bool operator<(const Umbra_t &other) const {
        // Compare up to the shorter length
        auto min_len = std::min(length_, other.length_);
        int cmp = std::memcmp(data_, other.data_, min_len);
        if (cmp == 0) {
            // if equal so far, shorter length is "less"
            return length_ < other.length_;
        }
        return (cmp < 0);
    }
    bool operator>(const Umbra_t &other) const { return (other < *this); }

    // Comparison with C-strings for convenience
    bool operator==(const char *str) const { return std::strcmp(data_, str) == 0; }
    bool operator!=(const char *str) const { return !(*this == str); }
    bool operator<(const char *str) const { return std::strcmp(data_, str) < 0; }
    bool operator>(const char *str) const { return std::strcmp(data_, str) > 0; }

    //---------------------
    // Concatenation
    //---------------------
    Umbra_t operator+(const Umbra_t &other) const {
        Umbra_t result;
        std::size_t new_len = static_cast<std::size_t>(length_) + other.length_;
        if (new_len <= SHORT_CAPACITY) {
            // Entirely inline
            result.length_ = static_cast<uint32_t>(new_len);
            result.allocated_.reset();
            std::memcpy(result.inlineData_, data_, length_);
            std::memcpy(result.inlineData_ + length_, other.data_, other.length_);
            result.inlineData_[new_len] = '\0';
            result.data_ = result.inlineData_;
        } else {
            // Need allocated storage
            result.length_ = static_cast<uint32_t>(new_len);
            result.allocated_ = std::make_unique<char[]>(new_len + 1);
            std::memcpy(result.allocated_.get(), data_, length_);
            std::memcpy(result.allocated_.get() + length_, other.data_, other.length_);
            result.allocated_[new_len] = '\0';
            result.data_ = result.allocated_.get();
        }
        return result;
    }

    Umbra_t operator+(const char *str) const {
        Umbra_t temp(str);
        return *this + temp;
    }

    //---------------------
    // Case conversions (example)
    //---------------------
    Umbra_t lower() const {
        Umbra_t result(*this);
        // We directly modify the result.data_
        for (uint32_t i = 0; i < result.length_; ++i) {
            result.data_[i] = static_cast<char>(std::tolower(result.data_[i]));
        }
        return result;
    }

    Umbra_t upper() const {
        Umbra_t result(*this);
        for (uint32_t i = 0; i < result.length_; ++i) {
            result.data_[i] = static_cast<char>(std::toupper(result.data_[i]));
        }
        return result;
    }

    //---------------------
    // Stream output
    //---------------------
    friend std::ostream &operator<<(std::ostream &os, const Umbra_t &um) { return os.write(um.data_, um.length_); }
};

// Example custom hash
namespace std {
template <> struct hash<Umbra_t> {
    std::size_t operator()(const Umbra_t &key) const noexcept {
        // Simple approach: we can hash a std::string_view pointing to the data
        auto sv = std::string_view(key.data(), key.size());
        return std::hash<std::string_view>()(sv);
    }
};
} // namespace std