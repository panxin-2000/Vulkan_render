//
// Created by 潘鑫 on 2025/10/9.
//

#ifndef EAR_CLIP_H
#define EAR_CLIP_H


#include "vector_signed_area.h"
#include "RB_tree_node.h"

bool ear_clip_algorithm_no_efficient(std::vector<triangle> &result_segments,
                                     std::vector<segment_vector> &new_segments,
                                     std::vector<segment_vector> &tree_vertices);


#endif //EAR_CLIP_H
