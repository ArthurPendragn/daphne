/*
 * Copyright 2024 The DAPHNE Consortium
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
/**
 * @brief TODO Description
 * what about destructor?
 */

struct DictionaryEncodedString {
private:

    // Global Vector that Maps Index -> String
    static std::vector<std::string> index_string_vec;

    // Global Unordered Map that Maps String -> Index
    static std::unordered_map<std::string, std::size_t> string_index_map;

    // Index for the given String in "index_string_vec"
    std::size_t index;

    /**
     * @brief Get the Index associated to the string. 
     * If it doesn't exist in the dictionary yet, it is first inserted.
     *
     * @param str The string to look up in the dictionary.
     * @return The index associated to str.
     */
    static std::size_t getIndex(const std::string &str) {
        auto search = string_index_map.find(str);
        if (search != string_index_map.end()) {
            // Element found
            return search->second;
        } 
        else {
            // Get Index for new string entry
            std::size_t newIndex = string_index_map.size();

            // Append string to end of vector
            index_string_vec.push_back(str);

            // Insert str into the Map
            string_index_map[str] = newIndex;
            return newIndex;
        }
    }

public:
    // Default constructor: points to an empty string.
    DictionaryEncodedString() : index(getIndex("")) {}

    // Constructor from a C-style string
    DictionaryEncodedString(const char *str) : index(getIndex(str ? str : "")) {}

    // Constructor from a std::string
    DictionaryEncodedString(const std::string &other) : index(getIndex(other)) {}

    // Copy constructor
    DictionaryEncodedString(const DictionaryEncodedString &other) : index(other.index) {}

    // Assignment operator
    DictionaryEncodedString &operator=(const DictionaryEncodedString &other) {
        // TODO is the if necessary? (Copied from Fixedstring.h)
        if (this != &other) {
            index = other.index;
        }
        return *this;
    }

    // Equality operator compares by dictionary index
    bool operator==(const DictionaryEncodedString &other) const {
        return index == other.index;
    }

    // Inequality operator
    bool operator!=(const DictionaryEncodedString &other) const {
        return index != other.index;
    }

    // Overriding the Less than operator
    bool operator<(const DictionaryEncodedString &other) const {
        return index_string_vec[index] < index_string_vec[other.index];
    }

    // Overriding the Greater than operator
    bool operator>(const DictionaryEncodedString &other) const {
        return index_string_vec[index] > index_string_vec[other.index];
    }

    // Concatenation operator
    DictionaryEncodedString operator+(const DictionaryEncodedString &other) const {
        return DictionaryEncodedString(index_string_vec[index] + index_string_vec[other.index]);
    }
    
    //TODO Serialize

    //TODO Overload the output stream operator

    // Size method
    size_t size() const
    {
        return index_string_vec[index].size();
    }


    // Set
    void set(const std::string &str) {
        index = getIndex(str);
    }

    // To String method
    std::string to_string() const {
        return index_string_vec[index];
    }

    //TODO Compare method similar to std::string::compare

    //TODO Convert to lowercase

    //TODO Convert to uppercase

    //TODO hash
};
