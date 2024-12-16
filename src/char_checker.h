#pragma once

#include <string>

static bool IsTartgetChar(char ch) {
    return (ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z');
}

static std::string_view::iterator GetTargetIterator(std::string_view::iterator start, std::string_view::iterator end) {
    if (start > end) {
        return end;
    }
    for (auto it = start; it != end; ++it) {
        if (IsTartgetChar(*it)) {
            return it;
        }
    }
    return end;
}

static std::string_view::iterator GetNonTargetIterator(std::string_view::iterator start, std::string_view::iterator end) {
    if (start > end) {
        return end;
    }
    for (auto it = start; it != end; ++it) {
        if (!IsTartgetChar(*it)) {
            return it;
        }
    }
    return end;
}

static std::string ToLowerCase(std::string_view::iterator start, std::string_view::iterator end) {
    std::string result;
    result.resize(end-start);
    for (auto it = start; it != end; ++it) {
        if (*it < 'a') {
            result[it - start] = *it - 'A' + 'a';
        } else {
            result[it - start] = *it;
        }
    }
    return result;
}