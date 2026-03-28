#version 450 core
//#extension GL_GOOGLE_include_directive : enable
//
//#define PNANOVDB_GLSL
//#define PNANOVDB_ADDRESS_32
//
//// 1. 声明 SSBO，数组名必须叫 raw_data (或自定义)
//layout(std430, binding = 0) readonly buffer VdbBuffer {
//    uint raw_data[];
//} vdb_ssbo;
//
//// 2. 核心：将库内部访问宏指向这个数组
//#define pnanovdb_buf_data vdb_ssbo.raw_data
//
//// 3. 包含修改后的头文件
//#include "PNanoVDB.h"
//
void main() {
//    pnanovdb_address_t addr;
//    addr.byte_offset = 0u;
//
//    // 现在这里应该能编译过了
//    uint magic = pnanovdb_buf_read_uint32(vdb_ssbo.raw_data, addr);
//
//    // 调试：如果 magic 等于 0x4e564442 ('NVDB') 则成功
//    if (magic == 1314271302u) {
//        // 成功逻辑
//    }
}
