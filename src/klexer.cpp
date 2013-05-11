#include "klexer.hpp"

track_guard_t::track_guard_t(klexer_t & lex)
   : lex_(&lex)
{
   lex_->set_tracking(true);
}

track_guard_t::~track_guard_t()
{
   lex_->set_tracking(false);
}

klexer_t::klexer_t(std::istream& input)
   : input_(&input)
   , tracking_(false)
{
}

std::string klexer_t::text() const
{
   if(!out_.empty())
      return track_.front().text;
   return std::string(YYText());
}

token_t klexer_t::token() const
{
   if(!out_.empty())
      return track_.front().token;
   return token_;
}

bool klexer_t::next()
{
   if(tracking_)
      track_.emplace_back(text(), token(), pvalue(), line());
   if(!out_.empty())
   {
      track_.pop_front();
      return true;
   }
   return (token_ = (token_t)yylex()) != TOK_EOF;
}

void klexer_t::set_tracking(bool t)
{
   if(tracking_ == t)
      throw std::logic_error("Recursive tracking is forbidden");
   tracking_ = t;
   if(!t)
   {
      out_.insert(out_.end(), track_.begin(), track_.end());
      track_.clear();
   }
}

int klexer_t::line() const
{
   if(!tracking_ && !track_.empty())
      return track_.front().lineno;
   return lineno();
}

const klexer_t::value_t & klexer_t::pvalue() const
{
   if(!out_.empty())
      return track_.front().value;
   return value_;
}
