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


struct SDF_Round_box_2d {
    Eigen::Vector4f box;
    float right_down, right_up, left_down, left_up;

public:
    Eigen::Vector4f get_bounding_box() const {
        return Eigen::Vector4f(box[0] + std::max(left_down, left_up),
                               box[1] + +std::max(right_up, left_up),
                               box[2] - std::max(left_down, left_up),
                               box[3] - std::max(right_up, left_up));
    }
};


#endif //VULKAN_RENDER_BOUNDING_BOX_2D_H
