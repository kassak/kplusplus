#include <iostream>
#include "kparser.hpp"

int main(int argc, char **argv)
{
   config_t cfg(argc, argv);
   ast::parse(std::cin, cfg);
   std::cout << "test" << std::endl;
}
