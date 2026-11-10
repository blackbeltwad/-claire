#ifndef MAIN_FLOW_H
#define MAIN_FLOW_H
#include "lexer.h"
#include "parser.h"
#include <string>
namespace eclaire {
bool tokenize(const std::string &source, std::vector<lexer::token> &tokens);
} // namespace eclaire
#endif
