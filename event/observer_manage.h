//
// Created by 潘鑫 on 2025/12/15.
//

#ifndef LEARN_OPENGL_OBSERVE_MANAGE_H
#define LEARN_OPENGL_OBSERVE_MANAGE_H

#include <vector>
#include <mutex>
#include "base_event.h"
#include "event_queue_mange.h"
#include "input_device_manage.h"

/**
 * 这个管理器的目的是，为了管理输出设备，目前是两个，一个是键盘，另一个是鼠标或者触摸板
 * 事件由不同的设备管理器负责输入，每个设备拿到之后将事件输入到事件队列中
 * 事件队列中有事件的时候，通过同步变量进行通知，之后再进行处理
 * @tparam T
 */
template<typename T_observer, typename T_event>
class ObserverManager {
public:
    ObserverManager() = default;

    ~ObserverManager() = default;

    // 添加观察者
    ; // 关注的事件类型
    void addObserver(T_observer &observer) {
        const EventType temp_type = observer.type;
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_hasObserver(observer)) {
            _observers[static_cast<unsigned long>(temp_type)].push_back(observer);
            switch (temp_type) {
                case EventType::key_combination:
                    Keyboard_Manage::instance().register_key_combination(observer.event_name);
                    break;
                case EventType::mouse_click_left:
                    break;
                default: ;
            }
        }
    }

    // 移除观察者
    void removeObserver(T_observer *observer, EventType type) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto temp = _observers[static_cast<unsigned long>(type)];
        temp.erase(
            std::remove(temp.begin(), temp.end(), observer), temp.end()
        );
    }

    // 通知所有观察者（分发组合键事件）
    void notifyObservers(const T_event &event) {
        std::lock_guard<std::mutex> lock(_mutex);
        const EventType temp_type = event.type;
        switch (temp_type) {
            case EventType::key_combination:
                break;
            case EventType::mouse_click_left:
                // 保存首次点击的位置
                break;
            case EventType::mouse_click_right:
                // 保存首次点击的位置
                // 保存首次点击的位置
                break;
            default: ;
        }

        for (auto observer: _observers[static_cast<unsigned long>(event.type)]) {
            if (observer.event_name == event.event_name) {
                // 这里的判断少了一点内容
                observer.on_Event(event);
            }
        }
    }

    // 清空所有观察者
    void clearObservers() {
        std::lock_guard<std::mutex> lock(_mutex);
        _observers.clear();
    }

    static auto *instance_ptr() {
        static auto instance = new ObserverManager;
        return instance;
    }

    static auto &instance() {
        return *instance_ptr();
    }

    static void observer_manage_thread() {
        instance().dispatchEvents();
        return;
    }

    void observer_manage_thread_close() {
        _eventQueue.close();
    }

    static queue_thread_safe<T_event> *get_event_queue() {
        return instance()._get_event_queue();
    }

private
:
    std::mutex _mutex; // 线程安全锁
    std::vector<T_observer> _observers[static_cast<unsigned long>(EventType::EventType_max)]; // 观察者列表
    queue_thread_safe<T_event> _eventQueue; // 组合键事件队列

    queue_thread_safe<T_event> *_get_event_queue() {
        return &_eventQueue;
    }

    // 调度事件队列：将队列中的事件分发到观察者管理器
    void dispatchEvents() {
        while (1) {
            auto event = _eventQueue.pop();
            if (event) {
                auto event_value = event.value();
                // 委托观察者管理器通知所有观察者
                notifyObservers(event_value);
            } else {
                if (_eventQueue.is_closed()) {
                    // _eventQueue 已经关闭了，退出这个线程
                    break;
                }
            }
        }
    }

    // 检查观察者是否已存在
    bool _hasObserver(T_observer &observer) const {
        return std::find(_observers[static_cast<unsigned long>(EventType::key_combination)].begin(),
                         _observers[static_cast<unsigned long>(EventType::key_combination)].end(),
                         observer) != _observers[static_cast<unsigned long>(EventType::key_combination)].end();
    }
};

using observe_manage_instance = ObserverManager<base_observer<base_event>, base_event_with_stamp>;

#endif //LEARN_OPENGL_OBSERVE_MANAGE_H
