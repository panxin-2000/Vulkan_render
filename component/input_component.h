//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_INPUT_COMPONENT_H
#define HELLO_MAC_INPUT_COMPONENT_H
#include <iostream>
#include <utility>

#include "base_event.h"
#include "base_observer.h"
#include "component.h"
#include "observer_manage.h"


class Input_Component : public Actor_component {
public:
    Input_Component(Actor *owner, const std::string &compName)
        : Actor_component(owner, compName) {
        std::cout << "输入组件 [" << component_name << "] 已初始化（关联全局分发器）" << std::endl;
    }

    ~Input_Component() override {
        for (auto observer: _observers) {
            observe_manage_instance::instance().removeObserver(observer);
        }
    }


    // 其实我很不喜欢把两个单词连起来，只通过大小写来区分的，太难读了，
    // 如果所有的英语书都是通过大小写来区分单词连接的
    // 那么英语这门语言早就消息了
    // 订阅输入事件（转发到全局分发器）
    // 已经订阅的事件需要保存，在整个组件消失的时候，能够控制事件去取消订阅
    void Subscribe_Event(const EventType t, const std::string &event_name,
                         std::function<bool (base_event_with_stamp)> function) {
        base_observer<base_event> observer{t, event_name};
        observer.set_deal_function(std::move(function));
        _observers.push_back(observer);
        observe_manage_instance::instance().addObserver(observer);
    }

    // 取消订阅（转发到全局分发器）// 并将本地保存的事件删除掉
    void Unsubscribe_Event(const EventType t, const std::string &event_name,
                           std::function<bool (base_event_with_stamp)> function) {
        base_observer<base_event> observer{t, event_name};
        observer.set_deal_function(std::move(function));

        observe_manage_instance::instance().removeObserver(observer);
        auto &temp = _observers;
        temp.erase(
            std::remove(temp.begin(), temp.end(), observer), temp.end()
        );
    }

    void Unsubscribe_Event_all() {
        for (auto observer: _observers) {
            observe_manage_instance::instance().removeObserver(observer);
        }
        _observers.clear();
    }

private:
    std::vector<base_observer<base_event> > _observers;
};


#endif //HELLO_MAC_INPUT_COMPONENT_H
