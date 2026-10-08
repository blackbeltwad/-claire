#include "main_flow.h"
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {

  if (argc != 2) {
    std::cerr
        << "Error: Number of arguments must be 2, example: -claire file.-cl\n";
    return 1;
  }

  std::ifstream file(argv[1]);

  if (!file) {
    std::cerr << "Error: Unable to open given file\n";
    return 1;
  }

  std::string source;
  char c;

  while (file.get(c)) {
    source += c;
  }

  std::cout << source;
  if (!tokenize(&source)) {
    std::cerr << "Tokenization Failed \n";
    return 1;
  }
}
