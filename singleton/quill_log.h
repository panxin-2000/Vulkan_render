//
// Created by 潘鑫 on 2026/1/20.
//

#ifndef HELLO_MAC_QUILL_LOG_H
#define HELLO_MAC_QUILL_LOG_H


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

#endif //HELLO_MAC_QUILL_LOG_H
