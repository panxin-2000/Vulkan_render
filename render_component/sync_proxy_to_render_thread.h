//
// Created by 潘鑫 on 2026/3/8.
//

#ifndef HELLO_MAC_SYNC_PROXY_TO_RENDER_THREAD_H
#define HELLO_MAC_SYNC_PROXY_TO_RENDER_THREAD_H

struct UI_transform_dirty {
};

struct Object_transform_dirty {
};

struct Camera_transform_dirty {
};

struct Camera_optical_specifications_dirty {
};

struct add_to_render_tag {
};

struct uniform_buffer_update {
};

struct global_uniform_buffer_update {
};

struct descriptor_set_update {
};

//
void sync_render_data_to_render_thread();


#endif //HELLO_MAC_SYNC_PROXY_TO_RENDER_THREAD_H
