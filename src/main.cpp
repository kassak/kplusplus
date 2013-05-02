#include <iostream>
#include "klexer.hpp"

int main(int argc, char **argv)
{
   klexer_t ll;
   int x;
   while(x = ll.yylex())
   {
      if(x == TOK_INT)
         std::cout << "int: " << ll.value<long>() << std::endl;
      else if(x == TOK_FLOAT)
         std::cout << "flt: " << ll.value<double>() << std::endl;
      std::cout << "lex: " << x << std::endl;
   }
   std::cout << "test" << std::endl;
}
