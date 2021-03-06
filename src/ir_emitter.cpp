#include "ir_emitter.hpp"

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/ValueSymbolTable.h>
#include <llvm/Support/raw_os_ostream.h>

#include "type_system.hpp"
#include "util.hpp"

#include <iostream>

namespace
{
   using namespace ir;

   struct named_struct_t
   {
      llvm::StructType * type;
      std::vector<std::string> field_names;
   };

   struct ast_visitor_t
   {
      typedef
         std::vector<std::pair<size_t, llvm::Value*>>
         scoped_variables_t;
      typedef
         std::unordered_map<std::string, scoped_variables_t>
         symbols_map_t;
      typedef
         std::unordered_map<std::string, named_struct_t>
         structs_map_t;

      struct scope_guard_t
      {
         scope_guard_t(scope_guard_t const &) = delete;
         scope_guard_t& operator=(scope_guard_t const &) = delete;

         scope_guard_t(ast_visitor_t * av)
            : av_(av)
         {
            av_->change_scope(true);
         }
         ~scope_guard_t()
         {
            av_->change_scope(false);
         }
      private:
         ast_visitor_t * av_;
      };

      ast_visitor_t(ast::base_t::ptr_t const & tree)
         : root_(tree)
         , module_("kassak's c++ compiler'", llvm::getGlobalContext())
         , builder_(llvm::getGlobalContext())
         , current_function_(nullptr)
         , scope_num_(0)
      {
      }

      void process()
      {
         visit(to<ast::nt_stmt_sequence>(root_));
      }

      llvm::Value* visit(const ast::value_t<double> * node)
      {
         return llvm::ConstantFP::get(llvm::getGlobalContext(),
                                      llvm::APFloat(node->value()));
      }

      llvm::Value* visit(const ast::value_t<long> * node)
      {
         return llvm::ConstantInt::get(llvm::getGlobalContext(),
                                       llvm::APInt(32, node->value(), true));
      }

      llvm::Value* visit(const ast::class_def_t * node)
      {
         named_struct_t & strt = structs_.insert(std::make_pair(node->name(), named_struct_t())).first->second;
         strt.type = llvm::StructType::create(llvm::getGlobalContext(), node->name());
         std::vector<llvm::Type*> tps;
         for(ast::base_t::ptr_t const & c : node->children)
            if(c->node_type() == ast::nt_var_sequence)
            {
               for(ast::base_t::ptr_t const & v : c->children)
               {
                  const ast::variable_def_t * vd = to<ast::nt_variable_def>(v);
                  tps.push_back(lookup_type(vd->type()));
                  strt.field_names.push_back(vd->name());
               }
            }
         strt.type->setBody(tps);

         for(ast::base_t::ptr_t const & c : node->children)
            if(c->node_type() == ast::nt_function_def)
            {
               visit(to<ast::nt_function_def>(c));
            }

         return nullptr;
      }

      llvm::Value* visit(const ast::binop_t * node)
      {
         //TODO: handle unary minus
         llvm::Value* v1 = visit_expression(node->children[0]);
         if(node->children.size() == 1 && node->type() == ast::binop_t::bo_minus)
         {
            return builder_.CreateNeg(v1);
         }
         llvm::Value* v2 = visit_expression(node->children[1]);

         type_t t1 = type_of(v1);
         type_t t2 = type_of(v2);
         type_t max_tp = std::max(t1, t2);
         if(!(is_arithmetic_possible(t1, t2) || (node->type() == ast::binop_t::bo_assign && is_assignable(t1, t2))))
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
         case ast::binop_t::bo_assign:
            if(node->children[0]->node_type() != ast::nt_variable)
               error("lvalue expected");
            return builder_.CreateStore(tv2, lookup_variable(to<ast::nt_variable>(node->children[0]), NULL));
         case ast::binop_t::bo_lt:
            if(max_tp == t_int)
               return to_int32(builder_.CreateICmpSLT(tv1, tv1, "lttmp"), builder_);
            else
               return to_int32(builder_.CreateFCmpULT(tv1, tv2, "lttmp"), builder_);
         case ast::binop_t::bo_le:
            if(max_tp == t_int)
               return to_int32(builder_.CreateICmpSLE(tv1, tv1, "letmp"), builder_);
            else
               return to_int32(builder_.CreateFCmpULE(tv1, tv2, "letmp"), builder_);
         case ast::binop_t::bo_gt:
            if(max_tp == t_int)
               return to_int32(builder_.CreateICmpSGT(tv1, tv1, "gttmp"), builder_);
            else
               return to_int32(builder_.CreateFCmpUGT(tv1, tv2, "gttmp"), builder_);
         case ast::binop_t::bo_ge:
            if(max_tp == t_int)
               return to_int32(builder_.CreateICmpSGE(tv1, tv1, "getmp"), builder_);
            else
               return to_int32(builder_.CreateFCmpUGE(tv1, tv2, "getmp"), builder_);
         case ast::binop_t::bo_eq:
            if(max_tp == t_int)
               return to_int32(builder_.CreateICmpEQ(tv1, tv1, "eqtmp"), builder_);
            else
               return to_int32(builder_.CreateFCmpUEQ(tv1, tv2, "eqtmp"), builder_);
         case ast::binop_t::bo_neq:
            if(max_tp == t_int)
               return to_int32(builder_.CreateICmpNE(tv1, tv1, "neqtmp"), builder_);
            else
               return to_int32(builder_.CreateFCmpUNE(tv1, tv2, "neqtmp"), builder_);
            //            L = Builder.CreateFCmpULT(L, R, "cmptmp");
            // Convert bool 0/1 to double 0.0 or 1.0
            //            return Builder.CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()),
            //                                        "booltmp");
         default:
            throw std::logic_error("should not be");
         }
      }

      llvm::Function * current_function() const
      {
         return builder_.GetInsertBlock()->getParent();
      }

      llvm::Value* condition(llvm::Value* val)
      {
         return builder_.CreateICmpEQ(val, llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, 0, true)), "condition");
      }

      llvm::Value* visit(const ast::if_stmt_t * node, bool last_inst)
      {
         llvm::BasicBlock * then_br = llvm::BasicBlock::Create(llvm::getGlobalContext(), "then", current_function());
         llvm::BasicBlock * else_br = llvm::BasicBlock::Create(llvm::getGlobalContext(), "else", current_function());
         llvm::BasicBlock * after_if_br;
         if(!last_inst)
            after_if_br = llvm::BasicBlock::Create(llvm::getGlobalContext(), "after_if", current_function());

         llvm::Value * cond = cast(visit_expression(node->children[0]), t_int, builder_);
         cond = condition(cond);
         builder_.CreateCondBr(cond, else_br, then_br);

         builder_.SetInsertPoint(then_br);
         visit(to<ast::nt_stmt_sequence>(node->children[1]));
         if(!last_inst)
            builder_.CreateBr(after_if_br);

         if(node->children[2])
         {
            builder_.SetInsertPoint(else_br);
            visit(to<ast::nt_stmt_sequence>(node->children[2]));
            if(!last_inst)
               builder_.CreateBr(after_if_br);
         }

         if(!last_inst)
            builder_.SetInsertPoint(after_if_br);
         return nullptr;
      }

      llvm::Value* visit(const ast::while_stmt_t * node, bool last_inst)
      {
         llvm::BasicBlock * loop = llvm::BasicBlock::Create(llvm::getGlobalContext(), "loop", current_function());
         llvm::BasicBlock * after_loop = llvm::BasicBlock::Create(llvm::getGlobalContext(), "after_loop", current_function());

         llvm::Value * cond = cast(visit_expression(node->children[0]), t_int, builder_);
         cond = condition(cond);
         builder_.CreateCondBr(cond, after_loop, loop);

         builder_.SetInsertPoint(loop);
         visit(to<ast::nt_stmt_sequence>(node->children[1]));

         cond = cast(visit_expression(node->children[0]), t_int, builder_);
         cond = condition(cond);
         builder_.CreateCondBr(cond, after_loop, loop);

         builder_.SetInsertPoint(after_loop);
         return nullptr;
      }

      llvm::Value* visit(const ast::for_stmt_t * node, bool last_inst)
      {
         if(node->children[0])
         {
            ast::base_t::ptr_t vdef_or_expr = node->children[0];
            if(vdef_or_expr->node_type() == ast::nt_var_sequence)
               visit(to<ast::nt_var_sequence>(vdef_or_expr));
            else
               visit_expression(vdef_or_expr);
         }
         llvm::BasicBlock * loop = llvm::BasicBlock::Create(llvm::getGlobalContext(), "loop", current_function());
         llvm::BasicBlock * after_loop = llvm::BasicBlock::Create(llvm::getGlobalContext(), "after_loop", current_function());

         if(node->children[1])
         {
            llvm::Value * cond = cast(visit_expression(node->children[1]), t_int, builder_);
            cond = condition(cond);
            builder_.CreateCondBr(cond, after_loop, loop);
         }
         else
            builder_.CreateBr(loop);
         builder_.SetInsertPoint(loop);

         visit(to<ast::nt_stmt_sequence>(node->children[3]));

         if(node->children[2])
         {
            visit_expression(node->children[2]);
         }
         if(node->children[1])
         {
            llvm::Value * cond = cast(visit_expression(node->children[1]), t_int, builder_);
            cond = condition(cond);
            builder_.CreateCondBr(cond, after_loop, loop);
         }
         else
            builder_.CreateBr(loop);

         builder_.SetInsertPoint(after_loop);
         return nullptr;
      }

      named_struct_t const & lookup_struct(std::string const & name) const
      {
         structs_map_t::const_iterator it = structs_.find(name);
         if(it == structs_.end())
            error("no such struct `" + name + "`");
         return it->second;
      }

      size_t field_idx(named_struct_t const & strt, std::string const & fname)
      {
         std::vector<std::string>::const_iterator it2 = std::find(
             strt.field_names.begin(),
             strt.field_names.end(), fname);
         if(it2 == strt.field_names.end())
            error("unknown field `" + fname + "`");
         return it2 - strt.field_names.begin();
      }

      llvm::Value* visit(const ast::variable_t * node, const ast::variable_t * node_end = nullptr)
      {
         llvm::Value* a = lookup_variable(node, node_end);
         return builder_.CreateLoad(a, "var");
      }

      llvm::Type* lookup_type(std::string const & name) const
      {
         if(name == "double")
            return llvm::Type::getDoubleTy(llvm::getGlobalContext());
         if(name == "int")
            return llvm::Type::getInt32Ty(llvm::getGlobalContext());
         if(name == "void")
            return llvm::Type::getVoidTy(llvm::getGlobalContext());
         structs_map_t::const_iterator it = structs_.find(name);
         if(it != structs_.end())
            return it->second.type;
         error("unknown type `" + name + "`");
         return nullptr;
      }

      llvm::Value* lookup_variable(const ast::variable_t * begin, const ast::variable_t * end, llvm::Value* parent = nullptr)
      {
         if(!parent)
         {
            symbols_map_t::const_iterator it = symbols_.find(begin->name());
            if(it == symbols_.end() || it->second.empty())
               error("undefined variable `" + begin->name() + "`");
            parent = it->second.back().second;
         }
         else
         {
            type_t tp = to_type(parent->getType()->getPointerElementType());
            if(tp != t_struct)
               error("should be struct `" + begin->name() + "`");
            size_t idx = field_idx(lookup_struct(parent->getType()->getPointerElementType()->getStructName()), begin->name());
            parent = builder_.CreateStructGEP(parent, idx, "fld");
         }
         if(begin->children.empty() || begin->children[0].get() == end)
            return parent;
         return lookup_variable(to<ast::nt_variable>(begin->children[0]), end, parent);
      }

      llvm::Value* define_variable(std::string const & type, std::string const & name)
      {
         llvm::Value * res;
         res = builder_.CreateAlloca(lookup_type(type), 0, name.c_str());
         return put_variable(name, res);
      }

      llvm::Value* put_variable(std::string const & name, llvm::Value* val)
      {
         scoped_variables_t & sv = symbols_[name];
         if(!sv.empty() && sv.back().first == scope_num_)
            error("redefinition of `" + name + "` in same scope");
         sv.push_back(std::make_pair(scope_num_, val));
         return val;
      }

      llvm::Type* assure_struct_by_ref(llvm::Type* tp)
      {
         if(tp->isStructTy())
            return tp->getPointerTo();
         return tp;
      }

      llvm::Function* visit(const ast::function_def_t * node)
      {
         const ast::arg_sequence_t * args = to<ast::nt_arg_sequence>(node->children[0]);
         const ast::stmt_sequence_t * body = to<ast::nt_stmt_sequence>(node->children[1]);

         std::vector<llvm::Type*> arg_types(args->children.size());
         for(size_t i = 0; i < args->children.size(); ++i)
            arg_types[i] = assure_struct_by_ref(lookup_type(to<ast::nt_variable_def>(args->children[i])->type()));
         llvm::FunctionType * ftype = llvm::FunctionType::get(lookup_type(node->type()), arg_types, false);

         llvm::Function *foo = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, node->name(), &module_);
         if(foo->getName() != node->name())
         {
            foo->eraseFromParent();
            foo = module_.getFunction(node->name());
            if(foo->getFunctionType() != ftype || (body && !foo->empty()))
               error("redefinition of " + node->name());
         }

         if(body)
         {
            llvm::BasicBlock * b = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", foo);
            {
               size_t i = 0;
               for (llvm::Function::arg_iterator ait = foo->arg_begin(); i != arg_types.size(); ++ait, ++i)
               {
                  ait->setName(to<ast::nt_variable_def>(args->children[i])->name());
               }
            }

            current_function_ = foo;
            scope_exit([&current_function_](){current_function_ = nullptr;});

            builder_.SetInsertPoint(b);

            {
               size_t i = 0;
               for (llvm::Function::arg_iterator ait = foo->arg_begin(); i != arg_types.size(); ++ait, ++i)
               {
                  if(!arg_types[i]->isPointerTy())
                  {
                     llvm::Value* a = define_variable(
                        to<ast::nt_variable_def>(args->children[i])->type(),
                        to<ast::nt_variable_def>(args->children[i])->name()
                     );
                     builder_.CreateStore(ait, a);
                  }
                  else
                     put_variable(to<ast::nt_variable_def>(args->children[i])->name(), ait);
               }
            }

            visit(body, true);
         //         llvm::verifyFunction(*foo);
         }
         return foo;
      }

      llvm::Value* argument(ast::base_t::ptr_t const & node, type_t exp_type, const ast::variable_t * enode = nullptr)
      {
         if(node->node_type() == ast::nt_variable)
         {
            const ast::variable_t * vnode = to<ast::nt_variable>(node);
            llvm::Value* v = lookup_variable(vnode, enode);
            if(v->getType()->getPointerElementType()->isStructTy())
               return v;
         }
         return cast(visit_expression(node), exp_type, builder_);
      }

      llvm::Value* visit(const ast::function_call_t * node)
      {
         const ast::variable_t * name_node = to<ast::nt_variable>(node->name());
         const ast::variable_t * lowest = name_node;
         while(!lowest->children.empty())
            lowest = to<ast::nt_variable>(lowest->children[0]);
         bool class_method = (lowest != name_node);
         std::string name = lowest->name();
         llvm::Function * foo = module_.getFunction(name.c_str());
         if(!foo)
            error("undefined function `" + name + "`");
         const ast::base_t * args = node;
         if((!class_method && args->children.size() != foo->arg_size())
             || (class_method && args->children.size()+1 != foo->arg_size()))
         {
            std::stringstream ss;
            ss     << class_method << "|"
                   << "wrong number of arguments for `"
                   << name
                   << "`, expected "
                   << foo->arg_size()
                   << " got "
                   << args->children.size()
               ;
            error(ss.str());
         }
         std::vector<llvm::Value*> vargs(foo->arg_size());
         size_t skew = 0;
         if(class_method)
         {
            vargs[0] = argument(node->name(), t_void, lowest);
            skew = 1;
         }
         for(size_t i = 0; i < args->children.size(); ++i)
            vargs[i + skew] = argument(args->children[i], to_type(foo->getFunctionType()->getParamType(i+skew)));
         return builder_.CreateCall(foo, vargs, foo->getReturnType()->isVoidTy() ? "" : "foocall");
      }

      llvm::Value* visit(const ast::variable_def_t * node)
      {
         if(!current_function_)
         {
            llvm::Type * tp = lookup_type(node->type());
            if(!node->children.empty())
               error("global variable initializers are unsupported");
            llvm::Value * res = new llvm::GlobalVariable(module_, tp, false, llvm::GlobalValue::PrivateLinkage, llvm::UndefValue::get(tp), node->name().c_str());
            put_variable(node->name(), res);
            return res;
         }
         llvm::Value * a = define_variable(node->type(), node->name());
         if(!node->children.empty())
            builder_.CreateStore(cast(visit_expression(node->children[0]), to_type(a->getType()->getPointerElementType()), builder_), a);
         return a;
      }

      llvm::Value* visit(const ast::var_sequence_t * node)
      {
         for(ast::base_t::ptr_t const & c : node->children)
            visit(to<ast::nt_variable_def>(c));
         return nullptr;
      }

      llvm::Value* visit(const ast::stmt_sequence_t * node, bool foo_body = false)
      {
         scope_guard_t _(this);

         llvm::Value* res = nullptr;
         for(size_t i = 0; i < node->children.size(); ++i)
         {
            ast::base_t::ptr_t const & c = node->children[i];
            bool last_inst = current_function_
               && !current_function()->getReturnType()->isVoidTy()
               && i == node->children.size() - 1;
            switch(c->node_type())
            {
            case ast::nt_class_def:
               res = visit(to<ast::nt_class_def>(c));
               break;
            case ast::nt_function_def:
               res = visit(to<ast::nt_function_def>(c));
               break;
            case ast::nt_return:
               res = visit(to<ast::nt_return>(c));
               break;
            case ast::nt_var_sequence:
               res = visit(to<ast::nt_var_sequence>(c));
               break;
            case ast::nt_stmt_sequence:
               res = visit(to<ast::nt_stmt_sequence>(c));
               break;
            case ast::nt_for:
               res = visit(to<ast::nt_for>(c), last_inst);
               break;
            case ast::nt_while:
               res = visit(to<ast::nt_while>(c), last_inst);
               break;
            case ast::nt_if:
               res = visit(to<ast::nt_if>(c), last_inst);
               break;
            default:
               res = visit_expression(c);
               break;
            }
         }
         if(foo_body && current_function_->getReturnType()->isVoidTy())
         {
            builder_.CreateRetVoid();
         }
         return res;
      }

      llvm::Value* visit_expression(ast::base_t::ptr_t const & node)
      {
         switch(node->node_type())
         {
         case ast::nt_int_value:
            return visit(to<ast::nt_int_value>(node));
         case ast::nt_float_value:
            return visit(to<ast::nt_float_value>(node));
         case ast::nt_binop:
            return visit(to<ast::nt_binop>(node));
         case ast::nt_variable:
            return visit(to<ast::nt_variable>(node));
         case ast::nt_function_call:
            return visit(to<ast::nt_function_call>(node));
         default:
            unexpected_node(node);
         }
         return nullptr;
      }

      llvm::ReturnInst * visit(const ast::return_stmt_t * node)
      {
         llvm::Value * ret_val = nullptr;
         type_t ret_tp = t_void;
         if(!node->children.empty())
         {
            ret_val = visit_expression(node->children[0]);
            ret_tp = type_of(ret_val);
         }
         type_t foo_tp = to_type(current_function_->getReturnType());
         check_castable(ret_tp, foo_tp);
         if(node->children.empty())
            return builder_.CreateRetVoid();
         else
            return builder_.CreateRet(cast(ret_val, foo_tp, builder_));
      }

      template<ast::node_t N>
      static const typename ast::ast_node_type_f<N>::type *
      to(ast::base_t::ptr_t const & n)
      {
         if(!n)
            return nullptr;
         return ast::ast_cast<N>(n.get());
      }

      template<ast::node_t N>
      static const typename ast::ast_node_type_f<N>::type *
      to(const ast::base_t * n)
      {
         if(!n)
            return nullptr;
         return ast::ast_cast<N>(n);
      }

      void error(std::string const & e) const
      {
         throw std::runtime_error("Error: " + e);
      }
      void not_appliable(ast::binop_t::bo_t op, type_t t1, type_t t2) const
      {
         error("operator is not appliable for this argument types[`" + type2str(t1) + "`, `" + type2str(t2) + "`]");
      }
      void unexpected_node(ast::base_t::ptr_t const & n) const
      {
         error("unexpected node `" + n->repr() + "`");
      }

      void write_out(llvm::raw_ostream & os)
      {
         os << module_;
      }

      void change_scope(bool inc)
      {
         if(inc)
         {
            ++scope_num_;
            return;
         }
         --scope_num_;
         for(symbols_map_t::value_type & vars : symbols_)
            vars.second.erase(std::remove_if(vars.second.begin(), vars.second.end(),
               [scope_num_](scoped_variables_t::value_type const & v)
               {
                  return v.first > scope_num_;
               }), vars.second.end());
      }
   private:
      ast::base_t::ptr_t root_;
      llvm::Module module_;
      llvm::IRBuilder<> builder_;
      symbols_map_t symbols_;
      llvm::Function * current_function_;
      structs_map_t structs_;
      size_t scope_num_;
      //      llvm::ValueSymbolTable symbols_;
   };
}

namespace ir
{
   void emit(ast::base_t::ptr_t const & tree)
   {
      ast_visitor_t processor(tree);
      processor.process();
      llvm::raw_os_ostream os(std::cout);
      processor.write_out(os);
   }
}
