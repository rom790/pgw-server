#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <string_view>
class BCDConverter {
public:

    static std::vector<uint8_t> imsi_to_bcd(const std::string& imsi);

    static std::string bcd_to_imsi(const std::vector<uint8_t>& bcd_data);

    static bool validate_imsi(std::string_view imsi) noexcept;

private:
    static inline uint8_t char_to_nibble(char c);
    static inline char nibble_to_char(uint8_t nibble);
};