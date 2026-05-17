//
// Created by 潘鑫 on 2026/5/15.
//


#include <iostream>
#include <vector>

template<typename E>
class VecExpression {
public:
    const E &cast() const {
        return static_cast<const E &>(*this);
    }
};


template<typename LHS, typename RHS>
class VecAdd : public VecExpression<VecAdd<LHS, RHS> > {
    const LHS &lhs;
    const RHS &rhs;

public:
    VecAdd(const LHS &lhs, const RHS &rhs) : lhs(lhs), rhs(rhs) {
    }

    double operator[ ](const size_t i) const {
        return lhs[i] + rhs[i];
    }
};

template<typename LHS, typename RHS>
class VecMultiply : public VecExpression<VecMultiply<LHS, RHS> > {
    const LHS &lhs;
    const RHS &rhs;

public:
    VecMultiply(const LHS &lhs, const RHS &rhs) : lhs(lhs), rhs(rhs) {
    }

    double operator[ ](const size_t i) const {
        return lhs[i] * rhs[i];
    }
};

template<typename LHS, typename RHS>
VecAdd<LHS, RHS> operator +(const VecExpression<LHS> &lhs, const VecExpression<RHS> &rhs) {
    return VecAdd<LHS, RHS>(lhs.cast(), rhs.cast());
}

template<typename LHS, typename RHS>
VecMultiply<LHS, RHS> operator *(const VecExpression<LHS> &lhs, const VecExpression<RHS> &rhs) {
    return VecMultiply<LHS, RHS>(lhs.cast(), rhs.cast());
}

class Vector : public VecExpression<Vector> {
    std::vector<double> data_;

public:
    explicit Vector(const size_t n, const double val = 0.0) : data_(n, val) {
    }

    double operator[ ](const size_t i) const {
        return data_[i];
    }

    double &operator[](const size_t i) {
        return data_[i];
    }

    size_t size() const { return data_.size(); }

    template<typename E>
    Vector &operator=(const VecExpression<E> &expr) {
        const E &real_expr = expr.cast();
        for (size_t i = 0; i < data_.size(); ++i) {
            data_[i] = real_expr[i];
        }
        return *this;
    }
};


#include "gtest/gtest.h"


TEST(lazy_evaluation, evaluation) {
    Vector A(5, 1);
    Vector B(5, 2);
    Vector C(5, 0);
    C = A + B;
    Vector D(5, 0);
    D = (A + B) + C;
    Vector E(5, 0);
    // 上面只是一个示例， 33 行的代码 AI 说有问题
}
