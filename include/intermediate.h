#ifndef INTERMEDIATE_H
#define INTERMEDIATE_H
#include <cstdint>
#include <parser.h>
#include <variant>
namespace eclaire {
namespace ir {

using eclaire::parser::binary_op;
enum block_t {
  entry,
  if_block,
  elif_block,
  else_block,
  while_block,
};

struct value_id {
  std::uint32_t id;
};

using constant = std::variant<std::int64_t, double, bool>;

using operand = std::variant<value_id, constant>;

struct bi_instruction {
  value_id result;
  operand left;
  operand right;
  binary_op op;
};

struct release_instruction {
  operand source;
};

struct call_instruction {
  // difficult for me to implement rn
};

using instruction =
    std::variant<bi_instruction, release_instruction, call_instruction>;

struct block {
  block_t type;
  std::vector<instruction> instructions;
};

struct function_t {
  std::vector<block> blocks;
};

} // namespace ir
} // namespace eclaire
#endif
