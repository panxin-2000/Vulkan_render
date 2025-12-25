//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_MUSIC_COMPONENT_H
#define HELLO_MAC_MUSIC_COMPONENT_H
#include "component.h"


class AudioComponent : public ActorComponent {
public:
    AudioComponent(Actor *owner, const std::string &compName)
        : ActorComponent(owner, compName) {
    }

    // 自定义功能：播放音效
    void PlaySound(const std::string &soundName) {
        if (bIsActive) {
            std::cout << "音频组件 [" << ComponentName << "] 正在播放音效：" << soundName << std::endl;
        }
    }
};

#endif //HELLO_MAC_MUSIC_COMPONENT_H
