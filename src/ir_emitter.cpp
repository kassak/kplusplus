#include "ir_emitter.hpp"

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/ValueSymbolTable.h>

#include "type_system.hpp"

namespace
{
   using namespace ir;

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

      llvm::Value* visit(const ast::binop_t * node)
      {
         //TODO: handle unary minus
         llvm::Value* v1 = value_atom(node->children[0].get());
         llvm::Value* v2 = value_atom(node->children[1].get());

         type_t t1 = type_of(v1);
         type_t t2 = type_of(v2);
         type_t max_tp = std::max(t1, t2);
         if(max_tp != t_int && max_tp != t_float)
            not_appliable(node->type(), t1, t2);

         llvm::Value* tv1 = ir::cast(v1, max_tp, builder_);
         llvm::Value* tv2 = ir::cast(v2, max_tp, builder_);

         switch (node->type())
         {
         case ast::binop_t::bo_plus:
            if(max_tp == t_int)
               return builder_.CreateAdd(tv1, tv2, "addtmp");
            else
               return builder_.CreateFAdd(tv1, tv2, "addtmp");
         case ast::binop_t::bo_minus:
            if(max_tp == t_int)
               return builder_.CreateSub(tv1, tv2, "subtmp");
            else
               return builder_.CreateFSub(tv1, tv2, "subtmp");
         case ast::binop_t::bo_mult:
            if(max_tp == t_int)
               return builder_.CreateMul(tv1, tv2, "multtmp");
            else
               return builder_.CreateFMul(tv1, tv2, "multtmp");
         case ast::binop_t::bo_div:
            if(max_tp == t_int)
               return builder_.CreateSDiv(tv1, tv2, "divtmp");
            else
               return builder_.CreateFDiv(tv1, tv2, "divtmp");
            //         case '<':
            //            L = Builder.CreateFCmpULT(L, R, "cmptmp");
            // Convert bool 0/1 to double 0.0 or 1.0
            //            return Builder.CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()),
            //                                        "booltmp");
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

      llvm::Type* lookup_type(std::string const & name) const
      {
         return nullptr;
      }

      llvm::Value* lookup_variable(std::string const & name) const
      {
         llvm::Value* v = symbols_.lookup(name);
         if(!v)
            error("undefined variable `" + name + "`");
         return v;
      }

      llvm::Value* define_variable(std::string const & name)
      {
         return nullptr;
      }

      llvm::Function* visit(const ast::function_def_t * node)
      {
         const ast::arg_sequence_t * args = to<ast::nt_arg_sequence>(node->children[0]);
         const ast::stmt_sequence_t * body = to<ast::nt_stmt_sequence>(node->children[1]);

         std::vector<llvm::Type*> arg_types(args->children.size());
         for(size_t i = 0; i < args->children.size(); ++i)
            arg_types[i] = lookup_type(to<ast::nt_variable_def>(args->children[i])->type());
         llvm::FunctionType * ftype = llvm::FunctionType::get(lookup_type(node->type()), arg_types, false);

         llvm::Function *foo = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, node->name(), &module_);
         if(foo->getName() != node->name())
            error("redefinition of " + node->name());

         {
            size_t i = 0;
            for (llvm::Function::arg_iterator ait = foo->arg_begin(); i != arg_types.size(); ++ait, ++i)
            {
               ait->setName(to<ast::nt_variable_def>(args->children[i])->name());
               //               symbols_.lookup(to<ast::nt_variable_def>(args->children[i])->name()] = ait;
            }
         }

         llvm::BasicBlock * b = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", foo);
         visit(body, b);
         llvm::verifyFunction(*foo);
         return foo;
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
      void not_appliable(ast::binop_t::bo_t op, type_t t1, type_t t2) const
      {
         throw error("operator is not appliable for this argument types");
      }
      void unexpected_node(const ast::base_t * n) const
      {
         throw error("unexpected node `" + n->repr() + "`");
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
