#include <iostream>
#include "kparser.hpp"
#include "ast2dot.hpp"
#include "ir_emitter.hpp"

int main(int argc, char **argv)
{
   config_t cfg(argc, argv);
   ast::base_t::ptr_t p = ast::parse(std::cin, cfg);
   if(cfg.dump_ast)
      ast::ast2dot(std::cout, p);
   if(cfg.emit_ir)
      try
      {
          ir::emit(p);
      }
      catch(std::exception const & e)
      {
         std::cerr << e.what() << std::endl;
         std::exit(1);
      }
   //   std::cout << "test" << std::endl;
}
