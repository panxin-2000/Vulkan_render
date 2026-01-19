//
// Created by 潘鑫 on 2026/1/19.
//

#ifndef HELLO_MAC_UTILITY_H
#define HELLO_MAC_UTILITY_H


// 使用 template 和 underlying_type 彻底通用化
#define ENABLE_BITWISE_OPERATORS(EnumType)                                \
inline EnumType operator|(EnumType a, EnumType b) {                       \
    using T = std::underlying_type_t<EnumType>;                           \
    return static_cast<EnumType>(static_cast<T>(a) | static_cast<T>(b));  \
}                                                                         \
inline EnumType& operator|=(EnumType& a, EnumType b) {                    \
    a = a | b;                                                            \
    return a;                                                             \
}                                                                         \
inline bool operator&(EnumType a, EnumType b) {                           \
    using T = std::underlying_type_t<EnumType>;                           \
    return static_cast<bool>(static_cast<T>(a) & static_cast<T>(b));      \
}                                                                         \
inline EnumType operator~(EnumType a) {                                   \
    using T = std::underlying_type_t<EnumType>;                           \
    return static_cast<EnumType>(~static_cast<T>(a));                     \
}                                                                         \
/* 按位异或 ^ */                                                           \
inline constexpr EnumType operator^(EnumType a, EnumType b) noexcept {    \
    using T = std::underlying_type_t<EnumType>;                           \
    return static_cast<EnumType>(static_cast<T>(a) ^ static_cast<T>(b));  \
}                                                                         \
/* 复合异或 ^= */                                                          \
inline constexpr EnumType& operator^=(EnumType& a, EnumType b) noexcept { \
    a = a ^ b;                                                            \
    return a;                                                             \
}
#endif //HELLO_MAC_UTILITY_H
