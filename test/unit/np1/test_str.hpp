// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_TEST_STR_HPP
#define NP1_TEST_UNIT_NP1_TEST_STR_HPP


namespace test {
namespace unit {
namespace np1 {



void test_to_hex_str_pad_2() {
  char result[128];

  ::np1::str::to_hex_str_pad_2(result, -1);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "ff") == 0);

  ::np1::str::to_hex_str_pad_2(result, 0);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "00") == 0);

  ::np1::str::to_hex_str_pad_2(result, 2);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "02") == 0);

  ::np1::str::to_hex_str_pad_2(result, 15);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "0f") == 0);

  ::np1::str::to_hex_str_pad_2(result, 31);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "1f") == 0);

  ::np1::str::to_hex_str_pad_2(result, 255);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "ff") == 0);

  ::np1::str::to_hex_str_pad_2(result, 256);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "00") == 0);
}


void test_to_hex_str_pad_4() {
  char result[128];

  ::np1::str::to_hex_str_pad_4(result, -1);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "ffff") == 0);

  ::np1::str::to_hex_str_pad_4(result, 0);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "0000") == 0);

  ::np1::str::to_hex_str_pad_4(result, 2);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "0002") == 0);

  ::np1::str::to_hex_str_pad_4(result, 15);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "000f") == 0);

  ::np1::str::to_hex_str_pad_4(result, 31);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "001f") == 0);

  ::np1::str::to_hex_str_pad_4(result, 255);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "00ff") == 0);

  ::np1::str::to_hex_str_pad_4(result, 256);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "0100") == 0);

  ::np1::str::to_hex_str_pad_4(result, 65535);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "ffff") == 0);

  ::np1::str::to_hex_str_pad_4(result, 65536);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "0000") == 0);
}


void test_to_hex_str_pad_8() {
  char result[128];

  ::np1::str::to_hex_str_pad_8(result, -1);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "ffffffff") == 0);

  ::np1::str::to_hex_str_pad_8(result, 0);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "00000000") == 0);

  ::np1::str::to_hex_str_pad_8(result, 2);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "00000002") == 0);

  ::np1::str::to_hex_str_pad_8(result, 15);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "0000000f") == 0);

  ::np1::str::to_hex_str_pad_8(result, 31);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "0000001f") == 0);

  ::np1::str::to_hex_str_pad_8(result, 255);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "000000ff") == 0);

  ::np1::str::to_hex_str_pad_8(result, 256);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "00000100") == 0);

  ::np1::str::to_hex_str_pad_8(result, 65535);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "0000ffff") == 0);

  ::np1::str::to_hex_str_pad_8(result, 65536);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "00010000") == 0);

  ::np1::str::to_hex_str_pad_8(result, 0xffffffff);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "ffffffff") == 0);  

  ::np1::str::to_hex_str_pad_8(result, 0x100000000LL);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "00000000") == 0);  
}


void test_to_hex_str_pad_12() {
  char result[128];

  ::np1::str::to_hex_str_pad_12(result, -1);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "ffffffffffff") == 0);

  ::np1::str::to_hex_str_pad_12(result, 0);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "000000000000") == 0);

  ::np1::str::to_hex_str_pad_12(result, 2);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "000000000002") == 0);

  ::np1::str::to_hex_str_pad_12(result, 15);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "00000000000f") == 0);

  ::np1::str::to_hex_str_pad_12(result, 31);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "00000000001f") == 0);

  ::np1::str::to_hex_str_pad_12(result, 255);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "0000000000ff") == 0);

  ::np1::str::to_hex_str_pad_12(result, 256);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "000000000100") == 0);

  ::np1::str::to_hex_str_pad_12(result, 65535);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "00000000ffff") == 0);

  ::np1::str::to_hex_str_pad_12(result, 65536);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "000000010000") == 0);

  ::np1::str::to_hex_str_pad_12(result, 0xffffffff);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "0000ffffffff") == 0);  

  ::np1::str::to_hex_str_pad_12(result, 0x100000000LL);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "000100000000") == 0);  

  ::np1::str::to_hex_str_pad_12(result, 0xffffffffffffLL);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "ffffffffffff") == 0);  

  ::np1::str::to_hex_str_pad_12(result, 0x1000000000000LL);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "000000000000") == 0);  
}


void test_to_hex_str_pad_16() {
  char result[128];

  ::np1::str::to_hex_str_pad_16(result, -1);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "ffffffffffffffff") == 0);

  ::np1::str::to_hex_str_pad_16(result, 0);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "0000000000000000") == 0);

  ::np1::str::to_hex_str_pad_16(result, 2);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "0000000000000002") == 0);

  ::np1::str::to_hex_str_pad_16(result, 15);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "000000000000000f") == 0);

  ::np1::str::to_hex_str_pad_16(result, 31);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "000000000000001f") == 0);

  ::np1::str::to_hex_str_pad_16(result, 255);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "00000000000000ff") == 0);

  ::np1::str::to_hex_str_pad_16(result, 256);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "0000000000000100") == 0);

  ::np1::str::to_hex_str_pad_16(result, 65535);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "000000000000ffff") == 0);

  ::np1::str::to_hex_str_pad_16(result, 65536);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "0000000000010000") == 0);

  ::np1::str::to_hex_str_pad_16(result, 0xffffffff);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "00000000ffffffff") == 0);  

  ::np1::str::to_hex_str_pad_16(result, 0x100000000LL);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "0000000100000000") == 0);  

  ::np1::str::to_hex_str_pad_16(result, 0xffffffffffffLL);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "0000ffffffffffff") == 0);  

  ::np1::str::to_hex_str_pad_16(result, 0x1000000000000LL);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "0001000000000000") == 0);  

  ::np1::str::to_hex_str_pad_16(result, 0xffffffffffffffffLL);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "ffffffffffffffff") == 0);  
}


void test_to_hex_str_pad_64() {
  char result[128];

  {
    unsigned char bytes[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    ::np1::str::to_hex_str_pad_64(result, bytes);
    NP1_TEST_ASSERT(::np1::str::cmp(result, "0000000000000000000000000000000000000000000000000000000000000000") == 0);
  }

  {
    unsigned char bytes[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
    ::np1::str::to_hex_str_pad_64(result, bytes);
    NP1_TEST_ASSERT(::np1::str::cmp(result, "0000000000000000000000000000000000000000000000000000000000000001") == 0);
  }

  {
    unsigned char bytes[32] = { 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
    ::np1::str::to_hex_str_pad_64(result, bytes);
    const char *expected = "0100000000000001000000000000000000000000000000000000000000000001";
    NP1_TEST_ASSERT(::np1::str::cmp(result, expected) == 0);
  }
}

void test_find_last() {
  NP1_TEST_ASSERT(::np1::str::find_last(::rstd::string(""), '/') == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(::np1::str::find_last(::rstd::string("/"), '/'), "/") == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(::np1::str::find_last(::rstd::string("/fred"), '/'), "/fred") == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(::np1::str::find_last(::rstd::string("fred/"), '/'), "/") == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(::np1::str::find_last(::rstd::string("f/red/"), '/'), "/") == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(::np1::str::find_last(::rstd::string("f/red/jane"), '/'), "/jane") == 0);
}


void test_trim() {
  NP1_TEST_ASSERT(::np1::str::cmp(::np1::str::trim(::rstd::string("")), "") == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(::np1::str::trim(::rstd::string(" ")), "") == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(::np1::str::trim(::rstd::string("  ")), "") == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(::np1::str::trim(::rstd::string(" a")), "a") == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(::np1::str::trim(::rstd::string("a ")), "a") == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(::np1::str::trim(::rstd::string(" ab ")), "ab") == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(::np1::str::trim(::rstd::string("  a b c")), "a b c") == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(::np1::str::trim(::rstd::string("a b c  ")), "a b c") == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(::np1::str::trim(::rstd::string(" a b c ")), "a b c") == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(::np1::str::trim(::rstd::string("  a b c  ")), "a b c") == 0);
}

void test_is_valid_utf8() {
  NP1_TEST_ASSERT(::np1::str::is_valid_utf8("fred"));
  NP1_TEST_ASSERT(::np1::str::is_valid_utf8("jane"));
  NP1_TEST_ASSERT(::np1::str::is_valid_utf8("Fred\t\n \v\b"));
  NP1_TEST_ASSERT(::np1::str::is_valid_utf8("\xc2\xa9"));  // Copyright sign
  NP1_TEST_ASSERT(::np1::str::is_valid_utf8("\xe2\x89\xa0"));  // "not equal" symbol

  // Embedded null
  NP1_TEST_ASSERT(!::np1::str::is_valid_utf8("t\0st", 4));

  // String too short
  NP1_TEST_ASSERT(!::np1::str::is_valid_utf8("\xe2\x89"));

  // Non-shortest forms of A, B and C.
  // TODO: more non-shortest-form testing.
  NP1_TEST_ASSERT(!::np1::str::is_valid_utf8("\xc1\x81"));
  NP1_TEST_ASSERT(!::np1::str::is_valid_utf8("\xc1\x82"));
  NP1_TEST_ASSERT(!::np1::str::is_valid_utf8("\xc1\x83"));  
}


void test_replace_invalid_utf8_sequences() {
  char buffer[128];

  strcpy(buffer, "fred");
  ::np1::str::replace_invalid_utf8_sequences(buffer, strlen(buffer), '?');
  NP1_TEST_ASSERT(::np1::str::cmp(buffer, "fred") == 0);

  strcpy(buffer, "jane");
  ::np1::str::replace_invalid_utf8_sequences(buffer, strlen(buffer), '?');
  NP1_TEST_ASSERT(::np1::str::cmp(buffer, "jane") == 0);

  strcpy(buffer, "\xc2\xa9");  // Copyright sign
  ::np1::str::replace_invalid_utf8_sequences(buffer, strlen(buffer), '?');
  NP1_TEST_ASSERT(::np1::str::cmp(buffer, "\xc2\xa9") == 0);

  strcpy(buffer, "\xe2\x89\xa0");  // "not equal" symbol
  ::np1::str::replace_invalid_utf8_sequences(buffer, strlen(buffer), '?');
  NP1_TEST_ASSERT(::np1::str::cmp(buffer, "\xe2\x89\xa0") == 0);

  memcpy(buffer, "t\0st", 4);  
  ::np1::str::replace_invalid_utf8_sequences(buffer, 4, '?');
  NP1_TEST_ASSERT(memcmp(buffer, "t?st", 4) == 0);

  strcpy(buffer, "test\xe2\x89");  // too short
  ::np1::str::replace_invalid_utf8_sequences(buffer, strlen(buffer), '?');
  NP1_TEST_ASSERT(::np1::str::cmp(buffer, "test??") == 0);

  strcpy(buffer, "test\xc1\x81y");  // non-shortest form
  ::np1::str::replace_invalid_utf8_sequences(buffer, strlen(buffer), '?');
  NP1_TEST_ASSERT(::np1::str::cmp(buffer, "test??y") == 0);

  //TODO: more non-shortest-form testing.
}


void test_to_dec_str_unsigned() {
  char buf[::np1::str::MAX_NUM_STR_LENGTH] = "deadbeefdeadbeefdeadbeef";
  char sprintf_buf[::np1::str::MAX_NUM_STR_LENGTH];
      
  ::np1::str::to_dec_str(buf, (uint64_t)0);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "0") == 0);

  ::np1::str::to_dec_str(buf, (uint64_t)1);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "1") == 0);

  ::np1::str::to_dec_str(buf, (uint64_t)9);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "9") == 0);

  ::np1::str::to_dec_str(buf, (uint64_t)10);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "10") == 0);

  ::np1::str::to_dec_str(buf, (uint64_t)11);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "11") == 0);

  ::np1::str::to_dec_str(buf, (uint64_t)100);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "100") == 0);

  ::np1::str::to_dec_str(buf, (uint64_t)101);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "101") == 0);
  
  ::np1::str::to_dec_str(buf, (uint64_t)100001);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "100001") == 0);
  
  ::np1::str::to_dec_str(buf, (uint64_t)210301);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "210301") == 0);
  
  ::np1::str::to_dec_str(buf, (uint64_t)-1);
  sprintf(sprintf_buf, "%llu", (unsigned long long)(uint64_t)-1);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, sprintf_buf) == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "18446744073709551615") == 0);
}

void test_to_dec_str_signed() {
  char buf[::np1::str::MAX_NUM_STR_LENGTH] = "deadbeefdeadbeefdeadbeef";
  char sprintf_buf[::np1::str::MAX_NUM_STR_LENGTH];
      
  ::np1::str::to_dec_str(buf, (int64_t)0);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "0") == 0);

  ::np1::str::to_dec_str(buf, (int64_t)1);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "1") == 0);

  ::np1::str::to_dec_str(buf, (int64_t)9);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "9") == 0);

  ::np1::str::to_dec_str(buf, (int64_t)9);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "9") == 0);

  ::np1::str::to_dec_str(buf, (int64_t)10);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "10") == 0);

  ::np1::str::to_dec_str(buf, (int64_t)11);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "11") == 0);

  ::np1::str::to_dec_str(buf, (int64_t)100);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "100") == 0);

  ::np1::str::to_dec_str(buf, (int64_t)101);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "101") == 0);
  
  ::np1::str::to_dec_str(buf, (int64_t)100001);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "100001") == 0);
  
  ::np1::str::to_dec_str(buf, (int64_t)210301);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "210301") == 0);
  
  ::np1::str::to_dec_str(buf, (int64_t)-9223372036854775807LL);
  sprintf(sprintf_buf, "%lld", (long long)(int64_t)-9223372036854775807LL);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, sprintf_buf) == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "-9223372036854775807") == 0);  

  ::np1::str::to_dec_str(buf, (int64_t)9223372036854775806LL);
  sprintf(sprintf_buf, "%lld", (long long)(int64_t)9223372036854775806LL);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, sprintf_buf) == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "9223372036854775806") == 0);  


  ::np1::str::to_dec_str(buf, (int64_t)-1);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "-1") == 0);

  ::np1::str::to_dec_str(buf, (int64_t)-9);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "-9") == 0);

  ::np1::str::to_dec_str(buf, (int64_t)-10);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "-10") == 0);

  ::np1::str::to_dec_str(buf, (int64_t)-11);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "-11") == 0);

  ::np1::str::to_dec_str(buf, (int64_t)-100);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "-100") == 0);

  ::np1::str::to_dec_str(buf, (int64_t)-101);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "-101") == 0);
  
  ::np1::str::to_dec_str(buf, (int64_t)-100001);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "-100001") == 0);
  
  ::np1::str::to_dec_str(buf, (int64_t)-210301);
  NP1_TEST_ASSERT(::np1::str::cmp(buf, "-210301") == 0);
}


void test_str() {
  //TODO: more tests!
  NP1_TEST_RUN_TEST(test_to_hex_str_pad_2);
  NP1_TEST_RUN_TEST(test_to_hex_str_pad_4);
  NP1_TEST_RUN_TEST(test_to_hex_str_pad_8);
  NP1_TEST_RUN_TEST(test_to_hex_str_pad_12);
  NP1_TEST_RUN_TEST(test_to_hex_str_pad_16);
  NP1_TEST_RUN_TEST(test_to_hex_str_pad_64);
  NP1_TEST_RUN_TEST(test_find_last);
  NP1_TEST_RUN_TEST(test_trim);
  NP1_TEST_RUN_TEST(test_is_valid_utf8);
  NP1_TEST_RUN_TEST(test_replace_invalid_utf8_sequences);
  NP1_TEST_RUN_TEST(test_to_dec_str_unsigned);
  NP1_TEST_RUN_TEST(test_to_dec_str_signed);
}

} // namespaces
}
}

#endif
