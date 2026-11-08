//
// Created by 潘鑫 on 2026/11/7.
//

#ifndef VULKAN_RENDER_BOUNDING_BOX_2D_H
#define VULKAN_RENDER_BOUNDING_BOX_2D_H

#include <Eigen/Eigen>


struct Bounding_Box_2D {
    Eigen::Vector4f box;

    float &min_x() {
        return box[0];
    }

    float &min_y() {
        return box[1];
    }

    float &max_x() {
        return box[2];
    }

    float &max_y() {
        return box[3];
    }
};




#endif //VULKAN_RENDER_BOUNDING_BOX_2D_H
