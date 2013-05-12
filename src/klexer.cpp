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
   value_ = 0.0;
}

config_t const & klexer_t::config() const
{
   return cfg_;
}

void klexer_t::configure(config_t const & cfg)
{
   cfg_ = cfg;
}

std::string klexer_t::text() const
{
   if(!out_.empty())
      return out_.front().text;
   return std::string(YYText());
}

token_t klexer_t::token() const
{
   if(!out_.empty())
      return out_.front().token;
   return token_;
}

bool klexer_t::next()
{
   if(tracking_)
      track_.emplace_back(text(), token(), pvalue(), line());
   if(!out_.empty())
   {
      out_.pop_front();
      return true;
   }
   token_ = (token_t)yylex();
   if(config().verbose_lexer)
      std::cerr << repr(token_) << "[" << YYText() << "]" << std::endl;
   return token_ != TOK_EOF;
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
   if(!out_.empty())
      return out_.front().lineno;
   return lineno();
}

const klexer_t::value_t & klexer_t::pvalue() const
{
   if(!out_.empty())
      return out_.front().value;
   return value_;
}
