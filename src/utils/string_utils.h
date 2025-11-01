#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>

namespace esp32_ide {
namespace utils {

/**
 * @brief String utility functions
 */
class StringUtils {
public:
    // String manipulation
    static std::string Trim(const std::string& str);
    static std::string TrimLeft(const std::string& str);
    static std::string TrimRight(const std::string& str);
    
    // Case conversion
    static std::string ToLower(const std::string& str);
    static std::string ToUpper(const std::string& str);
    
    // String splitting and joining
    static std::vector<std::string> Split(const std::string& str, char delimiter);
    static std::string Join(const std::vector<std::string>& strings, const std::string& delimiter);
    
    // String searching
    static bool StartsWith(const std::string& str, const std::string& prefix);
    static bool EndsWith(const std::string& str, const std::string& suffix);
    static bool Contains(const std::string& str, const std::string& substr);
    
    // String replacement
    static std::string Replace(const std::string& str, const std::string& from, const std::string& to);
    static std::string ReplaceAll(const std::string& str, const std::string& from, const std::string& to);
};

} // namespace utils
} // namespace esp32_ide

#endif // STRING_UTILS_H
