#include <iostream>

#include "labyrinth.h"
#include "windows.h"
#include "component/component.h"
#include "component/music_component.h"
#include "component/render_component.h"

#include "event/base_event.h"
#include "event/base_observer.h"
#include "event/observer_manage.h"
#include "interactable_object/actor.h"


int main() {

    // // 1. 创建宿主对象（模拟 UE 的 Actor）
    // Actor player("玩家角色");
    //
    // // 2. 核心操作：Add Component（添加自定义组件）
    // RenderComponent* playerRender = player.AddComponent<RenderComponent>("玩家渲染组件");
    // AudioComponent* playerAudio = player.AddComponent<AudioComponent>("玩家音频组件");
    //
    // // 3. 调用组件自定义功能
    // playerRender->SetVisibility(true);
    // playerAudio->PlaySound("脚步声.wav");
    //
    // // 4. 禁用某个组件
    // playerAudio->SetActive(false);
    // playerAudio->PlaySound("攻击声.wav"); // 禁用后不执行功能
    //
    // // 5. 宿主更新：触发所有组件 Tick
    // player.Tick(0.016f); // 模拟 60 帧（DeltaTime ≈ 0.016）
    //
    // // 6. 根据名称查找组件
    // ActorComponent* findComp = player.FindComponentByName("玩家渲染组件");
    // if (findComp)
    // {
    //     std::cout << "\n找到组件：" << findComp->GetComponentName() << std::endl;
    // }

    // std::cout << "\n===== 程序结束，宿主开始销毁 =====" << std::endl;


    auto labyrinth = new Labyrinth();
    add_render_windows();
    delete labyrinth;
}

