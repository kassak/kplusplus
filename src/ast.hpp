#pragma once

//#include <memory>
//#include <vector>
//#include <string>

namespace ast
{
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

      std::vector<base_t::ptr_t> children;
   };

   struct sequence_t
      : base_t
   {
      std::string repr() const
      {
         return "seq";
      }
   };

   struct expression_t
      : base_t
   {};

   struct variable_t
      : expression_t
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
      : expression_t
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
      : expression_t
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

   struct statement_t
      : base_t
   {
      std::string repr() const
      {
         return "stmt";
      }
   };

   struct variable_def_t
      : statement_t
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

   struct binop_t
      : expression_t
   {
      enum bo_t
      {
         bo_assign, bo_plus, bo_minus, bo_mult, bo_div
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
         default:
            return "unk_op";
         }
      }

   private:
      bo_t type_;
   };

}
