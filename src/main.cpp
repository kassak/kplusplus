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
      ir::emit(p);
   //   std::cout << "test" << std::endl;
}
