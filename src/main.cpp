#include <iostream>
#include "kparser.hpp"

int main(int argc, char **argv)
{
   ast::parse(std::cin);
   std::cout << "test" << std::endl;
}
