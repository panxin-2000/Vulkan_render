//
// Created by 潘鑫 on 2025/3/24.
//
#include "gtest/gtest.h"


#include <Eigen/Eigen>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


/**
 *  函数的功能确定好了，之后就是怎么来求了
 *  首先需要确定的一点是坐标系，OpenGL的坐标系，屏幕向右为x，屏幕上为y，视线看向屏幕为 负的z方向 标准的右手坐标系
 *  如果 用 左手坐标系的话，z的方向是相反的。
 *  既然确定了右手坐标系，那么，对于输入的参数首先需要normalize 单位化
 *  照着比较细节的说明书做出来了，但是有写内容还是不太理解
 * @param eye
 * @param center
 * @param up
 * @return
 */
Eigen::Matrix4f lookAt(const Eigen::Vector3f &eye, const Eigen::Vector3f &center, const Eigen::Vector3f &up) {
    Eigen::Vector3f eye_normalized = eye.normalized();
    Eigen::Vector3f center_normalized = center.normalized();
    Eigen::Vector3f camera_direction_normal = (eye - center).normalized();
    Eigen::Vector3f up_normalized = up.normalized();
    // 下面两行的叉乘 不能够更换顺序  可能是跟 坐标系有关吧
    Eigen::Vector3f left_direction_normal = up_normalized.cross(camera_direction_normal).normalized();
    up_normalized = camera_direction_normal.cross(left_direction_normal).normalized();
    // 有必要再更新这一下吗？ 为什么有必要
    Eigen::Matrix4f transform_position = Eigen::Matrix4f::Identity();
    transform_position(0, 3) = -eye[0];
    transform_position(1, 3) = -eye[1];
    transform_position(2, 3) = -eye[2];
    Eigen::Matrix4f rotate_position = Eigen::Matrix4f::Identity();
    // 为什么 是 left 在最开始， 与 xyz 三个坐标轴的顺序有关吗？
    rotate_position(0, 0) = left_direction_normal[0];
    rotate_position(0, 1) = left_direction_normal[1];
    rotate_position(0, 2) = left_direction_normal[2];
    rotate_position(1, 0) = up_normalized[0];
    rotate_position(1, 1) = up_normalized[1];
    rotate_position(1, 2) = up_normalized[2];
    rotate_position(2, 0) = camera_direction_normal[0];
    rotate_position(2, 1) = camera_direction_normal[1];
    rotate_position(2, 2) = camera_direction_normal[2];
    return rotate_position * transform_position;
}

// 最后考虑一下比较想过的事情，正零和负零是相等的，比较的时候估计只能按照float去比较了，那么是不是需要写一个特别的测试函数呢？

/**
 * 矩阵应该如何推呢？ 首先是哥哥参数的意思，
 *  由下面的四个参数，应该能够确定 l,r,u,d, zNear,zFar ,这个六个值
 *  u = tan(foc) * zNear
 *  tan = 对边 / 邻边
 *  zNear就是邻边 所以可以求的对边 u 的长度
 *  aspect = r/u    所以 r =  aspect * u 这样6个参数就都求出来了
 *  之后就是这个六个参数应该如何组织的事情了，有一点需要考虑，归一化的问题
 *  在所以的点的向量都会除于w分量使 w分量等于1 ,目的是什么？或者w分量在之后有什么样的应用需要看OpenGL的说明书
 *  因为这里都会除于w分量，所以我们可以令w分量为-Ze，怎么做呢？令最后一行为 0， 0，-1， 0 即可 Ze代表什么？ 经过相机矩阵之后的z值
 *  为什么是令w分量为-Ze 呢？ 不知道
 *  接着向下走，需要求第一行的值，第二行的值，以及第三行的值。分别应该怎么求呢？
 *  第一行和第二行的求法是比较类似的 可以理解为解方程，已知Z轴的缩放比例，x轴上最左边的那个点会投影到-1，x轴上最右边的那个点会投影到+1
 *  然后求相应的解
 *
 *  然后是Z轴应该怎么求？Z轴就是第三行，近平面上的点会投影到0，远平面的点会投影到1，然后 分别求第三行的第三个和第四个参数。
 *  为什么会带第四个参数呢？ 不清楚，至少我现在解释不清楚（所以目前一个猜测是偏移值，因为你不能假设两个零点刚好都为零）
 *                                                 两个零点都为零的时候，偏移值一定是零
 *
 *  怎么求应该还是能解出来吧，
 *
 *  如果没有什么问题的话，按照上面的内容就能够把矩阵给求出来
 *
 * @param fovy  FOV 这里词代表的内容确实不知道  假设FOV等于45度，那么如果近平面是一，那么近平面上超过1的值将不可见
 *                                          假设FOV等于60度，那么如果近平面是一，那么近平面上超过根号3度值将不可见
 * @param aspect 长与宽之比  需要注意这个比值，是多少长比宽，那么高
 * @param zNear  近平面的值，小于这个平面不可见
 * @param zFar   远平面的值，大于这个平面不可见
 * @return
 */


Eigen::Matrix4f perspective(float fovy, float aspect, float zNear, float zFar) {
    // 简单的一点都做法就是填值就好了
    float u = tan(fovy / 2.0f) * zNear;
    float d = -u;
    float r = aspect * u;
    float l = -r;
    Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
    transform(0, 0) = zNear / r; // 1.0f / (aspect * (tan(fovy / 2.0f));      // 问题是 zNear 为什么被强制设置为1了
    transform(1, 1) = zNear / u; // 为什么算法会不一样  = 1.0f / (tan(fovy / 2.0f))  // 原因是什么？
    transform(2, 2) = -(zFar + zNear) / (zFar - zNear);
    transform(2, 3) = -2 * zFar * zNear / (zFar - zNear);
    transform(3, 2) = -1;
    transform(3, 3) = 0;
    return transform;
}


// 正交抄出来了，但是没有继续做测试
Eigen::Matrix4f Orthographic_Projection(float fovy, float aspect, float zNear, float zFar) {
    // 简单的一点都做法就是填值就好了
    float u = tan(fovy / 2.0f) * zNear;
    float d = -u;
    float r = aspect * u;
    float l = -r;
    Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
    transform(0, 0) = 1 / r; // 问题是 zNear 为什么被强制设置为1了
    transform(1, 1) = 1 / u;
    transform(2, 2) = -2.0f / (zFar - zNear);
    transform(2, 3) = -1 * (zFar + zNear) / (zFar - zNear);
    return transform;
}

/**
 * 这个函数证明了两个数值完全相等的矩阵需要改变存储方式
 * @param matrix
 * @param matrix_3
 */
void test_two_matrix(glm::mat4 &matrix, Eigen::Matrix4f matrix_3) {
    Eigen::Matrix<float, 4, 4, Eigen::RowMajor> matrix_2 = matrix_3; // 这里的意义是在存储上更换方向
    for (int i = 0; i < 16; i++) {
        float *tem = (float *) &matrix;
        float *tem_2 = (float *) &matrix_2;
        EXPECT_EQ(*(tem + i), *(tem_2 + i)) << "failed " << i << std::endl;
    }
}

// 上面这个问题解决后还有哪些问题呢？ 比如四元数的问题，为什么需要用到四元数呢？因为欧拉角描述的旋转是有死锁的
// 然后还有旋转动作的插值问题，也就是旋转中的动作是应该如何插值的？ 为什么不是再算而是采用插值呢？ 再算和插值有什么区别呢？
// 如何把Eigen 用于 OpenGL呢？ 直接赋值，然后测试，减少中间过程，检查最后的结果是否是正确的。
// 昨天看视频，有另一个可能的实现，也就是如何切换，比如从透视切换到正交，然后再从正交回到透视，中间的插值应该怎么搞？
// 哪些内容是不变的，哪些内容是改变的？
// 先从两个平头截体来看，变化的时候，进出口不用变化太多，但是远处一定会减少
// 然后他的展示其实还有一个问题，那就是原本是正视图，需要变成侧视图，中间变化的时候，需要怎么配合摄像机进行旋转呢？


void print_matrix(glm::mat4 &matrix2) {
    std::cout << matrix2[0][0] << ' ' << matrix2[1][0] << ' ' << matrix2[2][0] << ' ' << matrix2[3][0] << ' ' <<
            std::endl;
    std::cout << matrix2[0][1] << ' ' << matrix2[1][1] << ' ' << matrix2[2][1] << ' ' << matrix2[3][1] << ' ' <<
            std::endl;
    std::cout << matrix2[0][2] << ' ' << matrix2[1][2] << ' ' << matrix2[2][2] << ' ' << matrix2[3][2] << ' ' <<
            std::endl;
    std::cout << matrix2[0][3] << ' ' << matrix2[1][3] << ' ' << matrix2[2][3] << ' ' << matrix2[3][3] << ' ' <<
            std::endl;
    std::cout << std::endl;
}

TEST(matrix, view) { {
        glm::mat4 matrix2 = glm::lookAt(glm::vec3(0, 0, -4), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
        Eigen::Vector3f eye{0.0f, 0.0f, -4.0f};
        Eigen::Vector3f center{0.0f, 0.0f, 1.0f};
        Eigen::Vector3f up{0.0f, 1.0f, 0.0f};
        Eigen::Matrix4f view = lookAt(eye, center, up).transpose(); // 需要转置？ 这里需要转置说明了什么呢？
        test_two_matrix(matrix2, view);
    } {
        glm::mat4 matrix2 = glm::lookAt(glm::vec3(1, 1, -4), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
        Eigen::Vector3f eye{1.0f, 1.0f, -4.0f};
        Eigen::Vector3f center{0.0f, 0.0f, 1.0f};
        Eigen::Vector3f up{0.0f, 1.0f, 0.0f};
        Eigen::Matrix4f view = lookAt(eye, center, up).transpose(); // 需要转置？ 这里需要转置说明了什么呢？
        test_two_matrix(matrix2, view);
    } {
        glm::mat4 matrix2 = glm::lookAt(glm::vec3(1, 1, -4), glm::vec3(1, 2, 1), glm::vec3(0, 1, 0));
        Eigen::Vector3f eye{1.0f, 1.0f, -4.0f};
        Eigen::Vector3f center{1.0f, 2.0f, 1.0f};
        Eigen::Vector3f up{0.0f, 1.0f, 0.0f};
        Eigen::Matrix4f view = lookAt(eye, center, up).transpose(); // 需要转置？ 这里需要转置说明了什么呢？
        test_two_matrix(matrix2, view);
    }


    glm::mat4 projection = glm::perspective(
        glm::radians(145.0f),
        (float) 239 / (float) 400,
        1.0f,
        10000.0f);
    print_matrix(projection);

    glm::mat4 ortho = glm::ortho(0.0f, 100.0f, 0.0f, -1.0f, 1.0f, 1000.0f);
    glm::mat4 ortho2 = glm::ortho(0.0f, 100.0f, 0.0f, -1.0f); // 省略掉的两个参数的值是多少？
    //  glm::ortho 有两个版本的参数，一个是四个参数，另一个是六个参数的，估计少的是最后两个参数 ， 设置为零和无穷


    Eigen::Matrix4f matrix_2 = perspective(glm::radians(145.0f),
                                           (float) 239 / (float) 400,
                                           1.0f,
                                           10000.0f); // 问题还是这里为什么要转置？

    std::cout << matrix_2 << std::endl;
    test_two_matrix(projection, matrix_2.transpose());
}

TEST(vector, test_equal) {
    glm::vec4 vector1 = glm::vec4(1, 2, 3, 4);
    Eigen::Vector4f adcd{1, 2, 3, 4};
    for (int i = 0; i < 4; i++) {
        float *tem = (float *) &vector1;
        float *tem_2 = (float *) &adcd;
        EXPECT_EQ(*(tem + i), *(tem_2 + i)) << "failed " << i << std::endl;
    }
}

TEST(RowVector4f, test_equal) {
    glm::vec4 vector1 = glm::vec4(1, 2, 3, 4);
    Eigen::RowVector4f adcd{1, 2, 3, 4};
    for (int i = 0; i < 4; i++) {
        float *tem = (float *) &vector1;
        float *tem_2 = (float *) &adcd;
        EXPECT_EQ(*(tem + i), *(tem_2 + i)) << "failed " << i << std::endl;
    }
}


TEST(matrix, test_equal) {
    glm::mat4 matrix{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    Eigen::Matrix4f matrix_2;
    matrix_2 << 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16;
    // std::cout << matrix_2 << std::endl;

    test_two_matrix(matrix, matrix_2);
}


// 其实行存储和列存储就是一次转置？有两个内容吧，首先是索引方式，是先索引行还是先索引列，之后再是存储方式，先存储行还是先存储列
