   template<node_t N>
   struct ast_node_type_f;

   struct bad_ast_cast : std::logic_error
   {
      bad_ast_cast(std::string const & w) : std::logic_error(w){}
   };

   template<node_t N>
   typename ast_node_type_f<N>::type* ast_cast(base_t * n)
   {
      if(N != n->node_type())
         throw bad_ast_cast(std::string("casting ") + typeid(n).name() + " to "
                            + typeid(typename ast_node_type_f<N>::type).name());
      return static_cast<typename ast_node_type_f<N>::type*>(n);
   };

   template<node_t N>
   const typename ast_node_type_f<N>::type* ast_cast(const base_t * n)
   {
      if(N != n->node_type())
         throw bad_ast_cast(std::string("casting ") + typeid(n).name() + " to "
                            + typeid(typename ast_node_type_f<N>::type).name());
      return static_cast<const typename ast_node_type_f<N>::type*>(n);
   };

#define AST_HELPER(T, N)\
   template<>                                             \
   struct ast_node_type_f<N>{typedef T type;};

#define AST_NODE(T, N) \
   struct T;                                                     \
   AST_HELPER(T, N)                                              \
   struct T : base_mixin_t<N>

#define AST_NODE_TEMP_FWD(T, TA)                                         \
   template<class TA> struct T;

#define AST_NODE_TEMP(T, TA, N)                                            \
   template<class TA> struct T : base_mixin_t<N>

   template<class T>
   struct value_nt_f{};
   template<>
   struct value_nt_f<long>{static const node_t value = nt_int_value;};
   template<>
   struct value_nt_f<double>{static const node_t value = nt_float_value;};
