//
// Created by 潘鑫 on 2026/1/16.
//

#ifndef HELLO_MAC_APP_UTILITY_MIXINS_H
#define HELLO_MAC_APP_UTILITY_MIXINS_H


/**
 * A type that inherits from NonCopyable cannot be copied anymore.
 */
class NonCopyable {
public:
    /* Disable copy construction and assignment. */
    NonCopyable(const NonCopyable &other) = delete;

    NonCopyable &operator=(const NonCopyable &other) = delete;

    /* Explicitly enable default construction, move construction and move assignment. */
    NonCopyable() = default;

    NonCopyable(NonCopyable &&other) = default;

    NonCopyable &operator=(NonCopyable &&other) = default;
};

/**
 * A type that inherits from NonMovable cannot be moved anymore.
 */
class NonMovable {
public:
    /* Disable move construction and assignment. */
    NonMovable(NonMovable &&other) = delete;

    NonMovable &operator=(NonMovable &&other) = delete;

    /* Explicitly enable default construction, copy construction and copy assignment. */
    NonMovable() = default;

    NonMovable(const NonMovable &other) = default;

    NonMovable &operator=(const NonMovable &other) = default;
};
#endif //HELLO_MAC_APP_UTILITY_MIXINS_H
