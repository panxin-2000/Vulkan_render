#include <iostream>
#include <regex>
#include <string>

int main() {
    std::string glsl = R"(
        // Binding 0 : Position storage buffer
        layout(std140, binding = 0) buffer Pos
        {
           Particle particles[ ];
        };

        layout (local_size_x = 256) in;

        layout (binding = 1) uniform UBO
        {
            float deltaT;
            int particleCount;
            float gravity;
            float power;
            float soften;
        } ubo;
    )";

    /**
     * Regex Breakdown:
     * (?:buffer|uniform)     : Matches either buffer or uniform keyword
     * \s+(\w+)               : Group 2: The Block Type Name (Pos, UBO)
     * [\s\S]*?\{             : Skip to the opening brace
     * ([\s\S]*?)             : Group 3: The Content inside {}
     * \}\s*(\w+)?            : Group 4: The Instance Name (Optional, e.g., ubo)
     * \s*;                   : Matches the trailing semicolon
     */
    std::regex
            block_re(R"(layout\s*\([\s\S]*?binding\s*=\s*(\d+)[\s\S]*?\)\s*(?:buffer|uniform)\s+(\w+)[\s\S]*?\{([\s\S]*?)\}\s*(\w+)?\s*;)");

    auto blocks_begin = std::sregex_iterator(glsl.begin(), glsl.end(), block_re);
    auto blocks_end   = std::sregex_iterator();

    for (std::sregex_iterator i = blocks_begin; i != blocks_end; ++i) {
        std::smatch match      = *i;
        std::string binding_id = match[1];
        std::string block_type = match[2];
        std::string content    = match[3];
        std::string instance   = match[4].matched ? match[4].str() : "(Anonymous)";

        std::cout << ">>> Found Resource <<<" << std::endl;
        std::cout << "Binding:  " << binding_id << std::endl;
        std::cout << "Block:    " << block_type << std::endl;
        std::cout << "Instance: " << instance << std::endl;
        std::cout << "Content: " << std::endl << content << std::endl;
        std::cout << "-----------------------" << std::endl;
    }

    return 0;
}
