//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_MUSIC_COMPONENT_H
#define HELLO_MAC_MUSIC_COMPONENT_H
#include "component.h"


class AudioComponent : public Actor_component {
public:
    AudioComponent(Actor *owner, const std::string &compName)
        : Actor_component(owner, compName) {
    }

    // 自定义功能：播放音效
    void PlaySound(const std::string &soundName) {
        if (bIsActive) {
            std::cout << "音频组件 [" << component_name << "] 正在播放音效：" << soundName << std::endl;
        }
    }
};

#endif //HELLO_MAC_MUSIC_COMPONENT_H
