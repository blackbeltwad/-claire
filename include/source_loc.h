#ifndef ECLAIRE_SOURCE_LOC_H
#define ECLAIRE_SOURCE_LOC_H

#include <cstdint>

namespace eclaire {

struct loc {
  std::uint32_t row = 1;
  std::uint32_t col = 1;
};

} // namespace eclaire

#endif
