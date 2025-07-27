
#include "utils/bcd_converter.h"
#include <gtest/gtest.h>

TEST(BCDConverterTest, ValidateIMSI) {
    // Valid IMSIs
    EXPECT_TRUE(BCDConverter::validate_imsi("123456789012345"));
    EXPECT_TRUE(BCDConverter::validate_imsi("1234567890")); // min length
    
    // Invalid IMSIs
    EXPECT_FALSE(BCDConverter::validate_imsi(""));
    EXPECT_FALSE(BCDConverter::validate_imsi("12345"));
    EXPECT_FALSE(BCDConverter::validate_imsi("1234567890123456")); // too long
    EXPECT_FALSE(BCDConverter::validate_imsi("12345a678901234")); // non-digit
}

TEST(BCDConverterTest, IMSIToBCDConversion) {
    std::string imsi_even = "1234567890";
    auto bcd_even = BCDConverter::imsi_to_bcd(imsi_even);

    std::string imsi_odd = "12345678901";
    auto bcd_odd = BCDConverter::imsi_to_bcd(imsi_odd);

    EXPECT_EQ(bcd_even.size(), 5);  
    EXPECT_EQ(bcd_even[0], 0x21);   
    EXPECT_EQ(bcd_even[1], 0x43);   
    EXPECT_EQ(bcd_even[2], 0x65);   
    EXPECT_EQ(bcd_even[3], 0x87);   
    EXPECT_EQ(bcd_even[4], 0x09);   

    EXPECT_EQ(bcd_odd.size(), 6);  
    EXPECT_EQ(bcd_odd[0], 0x21);    
    EXPECT_EQ(bcd_odd[1], 0x43);     
    EXPECT_EQ(bcd_odd[2], 0x65);   
    EXPECT_EQ(bcd_odd[3], 0x87);    
    EXPECT_EQ(bcd_odd[4], 0x09);     
    EXPECT_EQ(bcd_odd[5], 0xF1);    


}

TEST(BCDConverterTest, BCDToIMSIConversion) {
    // Even length
    std::vector<uint8_t> bcd1 = {0x21, 0x43};
    std::vector<uint8_t> bcd2 = {0x21, 0x43, 0xF5};
    
    // Odd length
    EXPECT_EQ(BCDConverter::bcd_to_imsi(bcd1), "1234");
    EXPECT_EQ(BCDConverter::bcd_to_imsi(bcd2), "12345");
}

TEST(BCDConverterTest, RoundConversion) {
    std::string original = "1234567890";
    auto bcd = BCDConverter::imsi_to_bcd(original);
    auto converted = BCDConverter::bcd_to_imsi(bcd);

    EXPECT_EQ(original, converted);
}