import glob
import os
import subprocess
import sys

# 1. 对应 CMake 的 set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/render/shader)
CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
SHADER_SOURCE_DIR = os.path.join(CURRENT_DIR, "render", "shader")

# 2. 动态扫描所有着色器文件
EXTENSIONS = ["*.vert", "*.frag", "*.geom", "*.comp"]
SHADER_FILES = []
for ext in EXTENSIONS:
    search_path = os.path.join(SHADER_SOURCE_DIR, ext)
    SHADER_FILES.extend(glob.glob(search_path))

# 3. 获取 Mac 的标准临时目录
TMP_DIR = os.environ.get("TMPDIR", "/tmp")

# 4. 🔴 定义三个互斥的宏及其对应输出文件名的后缀
MACRO_CONFIGS = [
    {"macro": "PASS_COLOR=1",  "suffix": "COLOR"},
    {"macro": "PASS_DEPTH=1",  "suffix": "DEPTH"},
    {"macro": "PASS_PICKUP=1", "suffix": "PICKUP"}
]

print(f"Found {len(SHADER_FILES)} shaders. Each will be compiled 3 times with different macros.\n")

# 5. 开始循环遍历文件
for shader_file in SHADER_FILES:
    shader_name = os.path.basename(shader_file)

    # 6. 🔴 针对每一个文件，分别循环应用三个互斥宏
    for config in MACRO_CONFIGS:
        macro_param = f"-D{config['macro']}"   # 生成类似 -DPASS_COLOR=1
        suffix = config['suffix']               # 获取后缀，如 COLOR

        # 重新拼接输出文件名，形如: shader.vert_COLOR.spv
        output_name = f"{shader_name}_{suffix}.spv"
        output_path = os.path.join(TMP_DIR, output_name)

        print(f"compiler {shader_file} with [{config['macro']}]")

        # 7. 组装 glslc 命令，将宏定义参数加入其中
        cmd = [
            "glslc",
            "--target-env=vulkan1.3",
            macro_param,        # 👈 核心：在此处注入宏定义参数
            shader_file,
            "-o",
            output_path
        ]

        # 执行编译
        result = subprocess.run(cmd, capture_output=True, text=True)

        # 检查是否编译成功
        if result.returncode != 0:
            print(f"❌ Failed [{suffix}] for {shader_name}:\n{result.stderr}", file=sys.stderr)
        else:
            print(f"✅ Success [{suffix}]: -> {output_path}")

print("\nAll shaders processed for all passes.")
