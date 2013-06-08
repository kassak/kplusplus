#pragma once
namespace ir
{
   enum type_t
   {
      t_other = 4,
      t_struct = 3,
      t_void = 2,
      t_float = 1,
      t_int = 0
   };

   std::string type2str(type_t tp)
   {
      switch(tp)
      {
      case t_int:    return "int";
      case t_float:  return "float";
      case t_void:   return "void";
      case t_struct: return "struct";
      case t_other:  return "other";
      default:       return "--";
      }
   }

   type_t to_type(llvm::Type* t)
   {
      if(t->isFloatingPointTy())
         return t_float;
      else if(t->isIntegerTy())
         return t_int;
      else if(t->isVoidTy())
         return t_void;
      else if(t->isStructTy())
         return t_struct;
      return t_other;
   }

   type_t type_of(llvm::Value* v)
   {
      return to_type(v->getType());
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

   bool is_castable(type_t from, type_t to)
   {
      return from == to || ((from == t_int || from == t_float) && (to == t_int || to == t_float));
   }

   bool is_arithmetic_possible(type_t t1, type_t t2)
   {
      return t1 <= t_float && t2 <= t_float;
   }

   bool is_assignable(type_t t1, type_t t2)
   {
      return t1 == t2 || is_arithmetic_possible(t1, t2);
   }

   void check_castable(type_t from, type_t to)
   {
      if(!is_castable(from, to))
         invalid_conversion(from, to);
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

   llvm::Value* to_int32(llvm::Value* val, llvm::IRBuilder<> & builder)
   {
      return builder.CreateIntCast(val, llvm::Type::getInt32Ty(llvm::getGlobalContext()), true, "boolint");
   }
}
