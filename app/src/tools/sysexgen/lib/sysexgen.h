#pragma once

#include <cstdint>
#include <vector>

namespace sysexgen
{
    std::vector<uint8_t> generate(const std::vector<uint8_t>& firmware, uint32_t targetUid);
}
