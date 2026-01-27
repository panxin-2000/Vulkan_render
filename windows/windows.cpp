// //
// // Created by 潘鑫 on 2025/12/11.
// //
//
// #include <iostream>
// #include <valarray>
// #include <GLFW/glfw3.h>
//
//
// #include "labyrinth.h"
//
// #include "vK_object_manage.h"
//
//
// #include "base_event.h"
// #include "event_queue_mange.h"
// #include "base_observer.h"
// #include "input_device_manage.h"
// #include "observer_manage.h"
// #include "entity_name_component.h"
// #include "input_component.h"
// #include "model_matrix_component.h"
// #include "scene_component.h"
//
//
// void add_render_windows() {
// #ifdef WITH_VULKAN_BACKEND
//
// #endif
//
// #ifdef WITH_OPENGL_BACKEND
//
// #endif
//
//     std::thread t(start_render_manage_thread, window);
//     t.detach();
//
//     while (!glfwWindowShouldClose(window)) {
//         glfwWaitEvents();
//         if (GLFW_TRUE == glfwWindowShouldClose(window)) {
//             break;
//         }
//
//         glfwPollEvents();
//
//         dispatcher.update(); // 统一分发执行
//
//         auto view = g_entt().view<PendingDestroyTag>();
//         g_entt().destroy(view.begin(), view.end());
//
//
//         std::this_thread::sleep_for(std::chrono::milliseconds(10));
//     }
//
//
//     end_render_manage_thread();
//
//     // Terminate GLFW, clearing any resources allocated by GLFW.
//     glfwTerminate();
// }
