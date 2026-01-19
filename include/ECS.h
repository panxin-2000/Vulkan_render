//
// Created by 潘鑫 on 2026/1/13.
//

#ifndef HELLO_MAC_ECS_H
#define HELLO_MAC_ECS_H
#include <entt/entt.hpp>


class entt_user {
public:
    // 获取全局唯一的注册表引用
    static entt::registry &get() {
        static entt::registry instance;
        return instance;
    }

private:
    entt_user() = default; // 禁用构造
};


static entt::registry &g_entt() {
    return entt_user::get();
}


static inline int get_win_WIDTH() {
    return 800;
}

static inline int get_win_HEIGHT() {
    return 600;
}

#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/sinks/FileSink.h"
#include "quill/sinks/ConsoleSink.h"


class quill_log {
public:
    static quill::Logger *get() {
        static quill::Logger *logger = nullptr;
        static std::once_flag flag;
        std::call_once(flag, []() {
            std::setlocale(LC_ALL, "en_US.UTF-8");
            quill::BackendOptions backend_options;
            backend_options.check_printable_char = {}; // Disable sanitization
            quill::Backend::start(backend_options);
            logger = quill::Frontend::create_or_get_logger(
                "root", quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_id_1"));
        });
        return logger;
    }
};

static quill::Logger *g_log() {
    return quill_log::get();
}


#endif //HELLO_MAC_ECS_H
