#pragma once

//#include <memory>
//#include <vector>
//#include <string>

namespace ast
{
   enum node_t
   {
      nt_begin,
      nt_var_sequence = nt_begin,
      nt_stmt_sequence,
      nt_arg_sequence,

      nt_variable,
      nt_function_call,
      nt_value,
      nt_binop,

      nt_while,
      nt_for,
      nt_if,
      nt_return,
      nt_continue,
      nt_break,

      nt_variable_def,
      nt_function_def,

      nt_end
   };

   struct base_t
   {
      typedef
         std::shared_ptr<base_t>
         ptr_t;

      virtual ~base_t(){};
      virtual std::string repr() const
      {
         return "base";
      }

      template<class T>
      const T * get() const
      {
         return dynamic_cast<const T *>(this);
      }

      virtual node_t node_type() const = 0;

      std::vector<base_t::ptr_t> children;
   };

   template<node_t N>
   struct base_mixin_t
      : base_t
   {
      node_t node_type() const
      {
         return N;
      }
   };

   struct var_sequence_t
      : base_mixin_t<nt_var_sequence>
   {
      std::string repr() const
      {
         return "var_seq";
      }
   };

   struct arg_sequence_t
      : base_mixin_t<nt_arg_sequence>
   {
      std::string repr() const
      {
         return "arg_seq";
      }
   };

   struct stmt_sequence_t
      : base_mixin_t<nt_stmt_sequence>
   {
      std::string repr() const
      {
         return "stmt_seq";
      }
   };

   struct variable_t
      : base_mixin_t<nt_variable>
   {
      variable_t(std::string const & name)
         : name_(name)
      {
      }

      std::string const & name() const
      {
         return name_;
      }

      std::string repr() const
      {
         if(children.empty())
            return name();
         else
            return name() + "." + children.front()->repr();
      }
   private:
      std::string name_;
   };

   struct function_call_t
      : base_mixin_t<nt_function_call>
   {
      function_call_t(base_t::ptr_t const & name)
         : name_(name)
      {
      }

      base_t::ptr_t const & name() const
      {
         return name_;
      }

      std::string repr() const
      {
         return name()->repr() + "()";
      }
   private:
      base_t::ptr_t name_;
   };

   template<class T>
   struct value_t
      : base_mixin_t<nt_value>
   {
      value_t(T const & value)
         : value_(value)
      {
      }

      T const & value() const
      {
         return value_;
      }

      std::string repr() const
      {
         std::stringstream ss;
         ss << value_;
         return ss.str();
      }

   private:
      T value_;
   };

   struct function_def_t
      : base_mixin_t<nt_function_def>
   {
      function_def_t(std::string const & type, std::string const & name)
         : name_(name)
         , type_(type)
      {
      }

      std::string const & name() const
      {
         return name_;
      }

      std::string const & type() const
      {
         return type_;
      }


      std::string repr() const
      {
         return type() + " " + name() + "()";
      }
   private:
      std::string name_;
      std::string type_;
   };

   struct variable_def_t
      : base_mixin_t<nt_variable_def>
   {
      variable_def_t(std::string const & type, std::string const & name)
         : name_(name)
         , type_(type)
      {
      }

      std::string const & name() const
      {
         return name_;
      }

      std::string const & type() const
      {
         return type_;
      }


      std::string repr() const
      {
         return type() + " " + name();
      }
   private:
      std::string name_;
      std::string type_;
   };

   struct while_stmt_t
      : base_mixin_t<nt_while>
   {
      std::string repr() const
      {
         return "while";
      }
   };

   struct for_stmt_t
      : base_mixin_t<nt_for>
   {
      std::string repr() const
      {
         return "for";
      }
   };

   struct return_stmt_t
      : base_mixin_t<nt_return>
   {
      std::string repr() const
      {
         return "return";
      }
   };

   struct break_stmt_t
      : base_mixin_t<nt_break>
   {
      std::string repr() const
      {
         return "break";
      }
   };

   struct continue_stmt_t
      : base_mixin_t<nt_continue>
   {
      std::string repr() const
      {
         return "continue";
      }
   };

   struct if_stmt_t
      : base_mixin_t<nt_if>
   {
      std::string repr() const
      {
         return "if";
      }
   };

   struct binop_t
      : base_mixin_t<nt_binop>
   {
      enum bo_t
      {
         bo_assign, bo_plus, bo_minus, bo_mult, bo_div, bo_eq, bo_le, bo_ge, bo_lt, bo_gt
      };

      binop_t(bo_t type)
         : type_(type)
      {
      }

      bo_t type() const
      {
         return type_;
      }

      std::string repr() const
      {
         switch(type())
         {
         case bo_assign:
            return "=";
         case bo_plus:
            return "+";
         case bo_minus:
            return "-";
         case bo_div:
            return "/";
         case bo_mult:
            return "*";
         case bo_lt:
            return "<";
         case bo_gt:
            return ">";
         case bo_eq:
            return "==";
         case bo_le:
            return "<=";
         case bo_ge:
            return ">=";
         default:
            return "unk_op";
         }
      }

   private:
      bo_t type_;
   };

}
