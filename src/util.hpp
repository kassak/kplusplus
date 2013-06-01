#pragma once

struct scope_exit_t
{
   scope_exit_t(std::function<void()> const & foo)
      : foo_(foo)
   {}

   ~scope_exit_t()
   {
      foo_();
   }
private:
   std::function<void()> foo_;
};

#define scope_exit(F) scope_exit_t scope_exit_guard_##__COUNTER__(F)
