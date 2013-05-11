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

      std::vector<base_t::ptr_t> children;
   };

   struct sequence_t
      : base_t
   {};

   struct expression_t
      : base_t
   {};

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
   private:
      T value_;
   };

   struct statement_t
      : base_t
   {};

   struct variable_t
      : statement_t
   {
      variable_t(std::string const & type, std::string const & name)
         : name_(name)
         , type_(type)
      {
      }

      std::string const & name();
      std::string const & type();
   private:
      std::string name_;
      std::string type_;
   };

   struct binop_t
      : expression_t
   {};

   struct assignment_t
      : binop_t
   {
   };
}
