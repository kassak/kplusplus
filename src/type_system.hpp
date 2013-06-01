#pragma once
namespace ir
{
   enum type_t
   {
      t_user = 3,
      t_void = 2,
      t_float = 1,
      t_int = 0
   };

   type_t type_of(llvm::Value* v)
   {
      if(v->getType()->isFloatingPointTy())
         return t_float;
      else if(v->getType()->isIntegerTy())
         return t_int;
      else if(v->getType()->isVoidTy())
         return t_void;
      return t_user;
   }

   type_t type_of(llvm::Value* v1, llvm::Value* v2, ast::binop_t::bo_t op)
   {
      type_t t1 = type_of(v1);
      type_t t2 = type_of(v2);
      switch(op)
      {
      case ast::binop_t::bo_plus:
      case ast::binop_t::bo_minus:
      case ast::binop_t::bo_mult:
      case ast::binop_t::bo_div:
         return std::max(t1, t2);
      case ast::binop_t::bo_lt:
      case ast::binop_t::bo_gt:
      case ast::binop_t::bo_eq:
      case ast::binop_t::bo_le:
      case ast::binop_t::bo_ge:
         return t_int;
      default:
         throw std::logic_error("should not be");
      }
   }

   void invalid_conversion(type_t from, type_t to)
   {
      throw std::runtime_error("invalid type conversion");
   }

   llvm::Value * cast(llvm::Value * val, type_t to, llvm::IRBuilder<> & builder)
   {
      type_t from = type_of(val);
      if(from == to)
         return val;
      switch(from)
      {
      case t_int:
         switch(to)
         {
         case t_float:
            return builder.CreateSIToFP(val, llvm::Type::getDoubleTy(llvm::getGlobalContext()), "i_to_fl");
         default:
            invalid_conversion(from, to);
         }
         break;
      case t_float:
         switch(to)
         {
         case t_int:
            return builder.CreateFPToSI(val, llvm::Type::getInt32Ty(llvm::getGlobalContext()), "fl_to_i");
         default:
            invalid_conversion(from, to);
         }
         break;
      default:
         invalid_conversion(from, to);
      }
      return nullptr;
   }
}
