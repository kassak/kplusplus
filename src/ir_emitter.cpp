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
         return llvm::ConstantFP::get(llvm::getGlobalContext(),
                                      llvm::APFloat(node->value()));
      }

      llvm::Value* visit(const ast::value_t<long> * node)
      {
         return llvm::ConstantInt::get(llvm::getGlobalContext(),
                                       llvm::APInt(sizeof(long)*CHAR_BIT, node->value(), true));
      }

      enum type_t
      {
         t_float = 1,
         t_int = 0
      };

      static type_t type_of(llvm::Value* v)
      {
         if(v->isFloatingPointTy())
            return t_float;
         else if(v->isIntegerTy())
            return t_int;
         error("type failed");
      }

      static type_t type_of(llvm::Value* v1, llvm::Value* v2, ast::binop_t::bo_t op)
      {
         type_t t1 = type_of(v1);
         type_t t2 = type_of(v2);
         switch(op)
         {
         case ast::binop_t::bo_plus:
         case ast::binop_t::bo_minus:
         case ast::binop_t::bo_mul:
         case ast::binop_t::bo_div:
            return std::max(t1, t2);
         case ast::binop_t::bo_lt:
         case ast::binop_t::bo_gt:
         case ast::binop_t::bo_eq:
         case ast::binop_t::bo_le:
         case ast::binop_t::bo_ge:
            return t_int:
         default:
            throw std::logic_error("should not be");
         }
      }

      llvm::Value* visit(const ast::binop_t * node)
      {
         //TODO: handle unary minus
         llvm::Value* v1 = value_atom(node->children[0].get());
         llvm::Value* v2 = value_atom(node->children[1].get());

         switch (node->type())
         {
         case ast::binop_t::bo_plus:
            return builder_.CreateFAdd(v1, v2, "addtmp");
         case ast::binop_t::bo_minus:
            return builder_.CreateFSub(v1, v2, "subtmp");
         case ast::binop_t::bo_mult:
            return builder_.CreateFMul(v1, v2, "multmp");
         case ast::binop_t::bo_div:
            return builder_.CreateFDiv(v1, v2, "divtmp");
         case '<':
            L = Builder.CreateFCmpULT(L, R, "cmptmp");
            // Convert bool 0/1 to double 0.0 or 1.0
            return Builder.CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()),
                                        "booltmp");
         default:
            throw std::logic_error("should not be");
         }
      }

      llvm::Value* value_atom(const ast::base_t * node)
      {
         switch(node->node_type())
         {
         case ast::nt_float_value:
            return visit(to<ast::nt_float_value>(node));
         case ast::nt_int_value:
            return visit(to<ast::nt_int_value>(node));
            //         case ast::nt_variable:
            // return visit(to<ast::nt_variable>(node));
            //case ast::nt_function_call:
            //return visit(to<ast::nt_function_call>(node));
         default:
            unexpected_node(node);
         }
      }

      llvm::Value* lookup_variable(std::string const & name) const
      {
         llvm::Value* v = symbols_[name];
         if(!v)
            error("undefined variable `" + name + "`");
         return v;
      }

      llvm::Value* define_variable(std::string const & name)
      {
         return nullptr;
      }

      template<ast::node_t N>
      static const typename ast::ast_node_type_f<N>::type *
      to(ast::base_t::ptr_t const & n)
      {
         return ast::ast_cast<N>(n.get());
      }

      template<ast::node_t N>
      static const typename ast::ast_node_type_f<N>::type *
      to(const ast::base_t * n)
      {
         return ast::ast_cast<N>(n);
      }

      void error(std::string const & e) const
      {
         throw std::runtime_error("Error: " + e);
      }
      void unexpected_node(const ast::base_t * n) const
      {
         throw std::runtime_error("Error: unexpected node `" + n->repr() + "`");
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
