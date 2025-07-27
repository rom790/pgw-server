#include "utils/bcd_converter.h"
#include <stdexcept>
#include <algorithm>

constexpr size_t MAX_IMSI_LENGTH = 15;

std::vector<uint8_t> BCDConverter::imsi_to_bcd(const std::string& imsi) {
    if (!validate_imsi(imsi)) {
        throw std::invalid_argument("Invalid IMSI format");
    }

    std::vector<uint8_t> bcd_data;
    const size_t length = imsi.size();
    const bool is_odd_length = (length % 2 != 0);

    bcd_data.reserve((length + 1) / 2);

    // Обрабатываем по две цифры
    for (size_t i = 0; i < length; i += 2) {
        uint8_t byte = 0;
        
        // Младшие 4 бита - текущая цифра
        byte = char_to_nibble(imsi[i]);

        // Старшие 4 бита - следующая цифра или 0xF 
        if (i + 1 < length) {
            byte |= (char_to_nibble(imsi[i+1]) << 4);
        } else if (is_odd_length) {
            byte |= 0xF0; 
        }

        bcd_data.push_back(byte);
    }

    return bcd_data;
}

std::string BCDConverter::bcd_to_imsi(const std::vector<uint8_t>& bcd_data) {
    std::string imsi;
    imsi.reserve(bcd_data.size() * 2);

    for (uint8_t byte : bcd_data) {
        // Младшие 4 бита
        char digit1 = nibble_to_char(byte & 0x0F);
        if (digit1 == 'F') break;
        imsi += digit1;

        // Старшие 4 бита
        char digit2 = nibble_to_char((byte >> 4) & 0x0F);
        if (digit2 == 'F') break; 
        imsi += digit2;
    }

    return imsi;
}

bool BCDConverter::validate_imsi(std::string_view imsi) noexcept {
    return imsi.length() >= 10 && imsi.length() <= 15 &&
        std::all_of(imsi.begin(), imsi.end(), ::isdigit);
}


uint8_t BCDConverter::char_to_nibble(char c) {
    if (c >= '0' && c <= '9') {
        return static_cast<uint8_t>(c - '0');
    }
    throw std::invalid_argument("Invalid digit in IMSI");
}

char BCDConverter::nibble_to_char(uint8_t nibble) {
    if (nibble <= 9) {
        return static_cast<char>('0' + nibble);
    } else if (nibble == 0xF) {
        return 'F'; 
    }
    throw std::invalid_argument("Invalid BCD nibble");
}