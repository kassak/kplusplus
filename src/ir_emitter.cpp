#include "ir_emitter.hpp"

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/ValueSymbolTable.h>

namespace
{
   struct ast_visitor_t
   {
      ast_visitor_t(ast::base_t::ptr_t const & tree)
         : root_(tree)
         , module_("kassak's c++ compiler'", llvm::getGlobalContext())
         , builder_(llvm::getGlobalContext())
      {
      }

      void process()
      {
         visit(to<ast::nt_float_value>(root_));
      }

      llvm::Value* visit(const ast::value_t<double> * node)
      {
         return llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(node->value()));
      }

      template<ast::node_t N>
      static const typename ast::ast_node_type_f<N>::type *
      to(ast::base_t::ptr_t const & n)
      {
         return ast::ast_cast<N>(n.get());
      }
   private:
      ast::base_t::ptr_t root_;
      llvm::Module module_;
      llvm::IRBuilder<> builder_;
      llvm::ValueSymbolTable symbols_;
   };
}

namespace ir
{
   void emit(ast::base_t::ptr_t const & tree)
   {
      ast_visitor_t processor(tree);
      processor.process();
   }
}
