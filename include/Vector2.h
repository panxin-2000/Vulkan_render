//
// Created by 潘鑫 on 2026/6/11.
//

#ifndef HELLO_MAC_VECTOR2_H
#define HELLO_MAC_VECTOR2_H

#pragma once

namespace quadtree {
    template<typename T>
    class Vector2 {
    public:
        T x;
        T y;

        constexpr Vector2<T>(T X = 0, T Y = 0) noexcept : x(X), y(Y) {
        }

        constexpr Vector2<T> &operator+=(const Vector2<T> &other) noexcept {
            x += other.x;
            y += other.y;
            return *this;
        }

        constexpr Vector2<T> &operator/=(T t) noexcept {
            x /= t;
            y /= t;
            return *this;
        }
    };

    template<typename T>
    constexpr Vector2<T> operator+(Vector2<T> lhs, const Vector2<T> &rhs) noexcept {
        lhs += rhs;
        return lhs;
    }

    template<typename T>
    constexpr Vector2<T> operator/(Vector2<T> vec, T t) noexcept {
        vec /= t;
        return vec;
    }
}
#endif //HELLO_MAC_VECTOR2_H
