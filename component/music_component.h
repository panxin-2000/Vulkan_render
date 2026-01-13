//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_MUSIC_COMPONENT_H
#define HELLO_MAC_MUSIC_COMPONENT_H
#include <iostream>
#include <string>


class AudioComponent {
public:
    AudioComponent() {
    }

    // 自定义功能：播放音效
    void PlaySound(const std::string &soundName) {
        if (bIsActive) {
            std::cout << "音频组件 [" << component_name << "] 正在播放音效：" << soundName << std::endl;
        }
    }
};

#endif //HELLO_MAC_MUSIC_COMPONENT_H
