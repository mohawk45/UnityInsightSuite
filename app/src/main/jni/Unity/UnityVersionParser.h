#pragma once

#include <cctype>
#include <cstddef>
#include <string>

namespace UnityTool {
namespace VersionParser {

inline bool IsDigit(char c) { return c >= '0' && c <= '9'; }

inline bool IsSuffix(char c) {
    return c == 'a' || c == 'b' || c == 'f' || c == 'p' || c == 'c' || c == 'x';
}

inline bool IsValidVersion(const std::string& version) {
    if (version.size() < 9 || version.size() > 24) return false;
    std::size_t i = 0;
    while (i < 4 && IsDigit(version[i])) ++i;
    if (i != 4 || version[i++] != '.') return false;
    if (i >= version.size() || !IsDigit(version[i])) return false;
    while (i < version.size() && IsDigit(version[i])) ++i;
    if (i >= version.size() || version[i++] != '.') return false;
    const std::size_t patchStart = i;
    while (i < version.size() && IsDigit(version[i])) ++i;
    if (i == patchStart || i >= version.size() || !IsSuffix(version[i++])) return false;
    const std::size_t buildStart = i;
    while (i < version.size() && IsDigit(version[i])) ++i;
    if (i != version.size() || i == buildStart) return false;
    const int year = (version[0] - '0') * 1000 + (version[1] - '0') * 100 +
                     (version[2] - '0') * 10 + (version[3] - '0');
    return year >= 2017 && year <= 6000;
}

inline std::string ExtractVersion(const std::string& data) {
    for (std::size_t i = 0; i + 8 < data.size(); ++i) {
        if (i > 0 && IsDigit(data[i - 1])) continue;
        if (i + 4 >= data.size() || !IsDigit(data[i]) || !IsDigit(data[i + 1]) ||
            !IsDigit(data[i + 2]) || !IsDigit(data[i + 3]) || data[i + 4] != '.') continue;
        std::size_t end = i + 5;
        while (end < data.size() && (IsDigit(data[end]) || data[end] == '.')) ++end;
        while (end < data.size() && IsSuffix(data[end])) {
            ++end;
            while (end < data.size() && IsDigit(data[end])) ++end;
            break;
        }
        const std::string candidate = data.substr(i, end - i);
        if (IsValidVersion(candidate)) return candidate;
    }
    return {};
}

} // namespace VersionParser
} // namespace UnityTool
