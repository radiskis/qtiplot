//  Boost general library logging.hpp header file  ---------------------------//

//  (C) Copyright Jean-Daniel Michaud 2007. Permission to copy, use, modify, 
//  sell and distribute this software is granted provided this copyright notice 
//  appears in all copies. This software is provided "as is" without express or 
//  implied warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org/LICENSE_1_0.txt for licensing.
//  See http://code.google.com/p/loglite/ for library home page.

#ifndef BOOST_LOGGING_HPP
#define BOOST_LOGGING_HPP

#include <list>
#include <stack>
#include <string>
#include <ostream>
#include <sstream>
#include <stdio.h>
#include <algorithm>
#include <exception>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif
#if defined(BOOST_HAS_THREADS)
#  include <boost/thread/thread.hpp>
#  include <boost/thread/condition.hpp>
#endif // BOOST_HAS_THREADS
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>

#ifndef BOOST_NO_CODE_GENERATION_FOR_LOG
#define BOOST_LOG_INIT( format )                                               \
{                                                                              \
  boost::logging::logger *l = boost::logging::logger::get_instance();          \
  assert(l);                                                                   \
  l->add_format(format);                                                       \
}

#define BOOST_LOG_ADD_OUTPUT_STREAM( sink )                                    \
{                                                                              \
  boost::logging::logger *l = boost::logging::logger::get_instance();          \
  assert(l);                                                                   \
  l->add_sink(sink);                                                           \
}

#define BOOST_LOG(level, qualifier, _trace)                                    \
{                                                                              \
  boost::logging::logger *l = boost::logging::logger::get_instance();          \
  assert(l);                                                                   \
  if (l->get_global_max_log_level() >= level)                                  \
  {                                                                            \
    if (l->m_string_stream.str() != "")                                        \
      l->m_string_stack.push(l->m_string_stream.str());                        \
                                                                               \
    l->m_string_stream.str("");                                                \
    l->m_string_stream << _trace;                                              \
    l->trace(level, qualifier, l->m_string_stream.str(), __FILE__, __LINE__);  \
    if (!l->m_string_stack.empty())                                            \
    {                                                                          \
      l->m_string_stream.str(l->m_string_stack.top());                         \
      l->m_string_stack.pop();                                                 \
    }                                                                          \
  }                                                                            \
}

#define BOOST_LOG_( level, _trace )                                            \
  { BOOST_LOG(level, boost::logging::log, _trace) }

#define BOOST_LOG_UNFORMATTED(level, qualifier, _trace)                        \
{                                                                              \
  boost::logging::logger *l = boost::logging::logger::get_instance();          \
  assert(l);                                                                   \
  if (l->get_global_max_log_level() >= level)                                  \
  {                                                                            \
    if (l->m_string_stream.str() != "")                                        \
      l->m_string_stack.push(l->m_string_stream.str());                        \
                                                                               \
    l->m_string_stream.str("");                                                \
    l->m_string_stream << _trace;                                              \
    l->unformatted_trace(level, qualifier,                                     \
                         l->m_string_stream.str(), __FILE__, __LINE__);        \
    if (!l->m_string_stack.empty())                                            \
    {                                                                          \
      l->m_string_stream.str(l->m_string_stack.top());                         \
      l->m_string_stack.pop();                                                 \
    }                                                                          \
  }                                                                            \
}

#define BOOST_LOG_FINALIZE()												   \
{                                                                              \
	boost::logging::logger *l = boost::logging::logger::get_instance();        \
	assert(l);                                                                 \
	l->clear();																   \
}
#else // !BOOST_NO_CODE_GENERATION_FOR_LOG
#define BOOST_LOG_INIT( format )
#define BOOST_LOG_ADD_OUTPUT_STREAM( sink )
#define BOOST_LOG(level, qualifier, _trace)
#define BOOST_LOG_( level, _trace )
#define BOOST_LOG_UNFORMATTED(level, qualifier, _trace)
#endif // BOOST_NO_CODE_GENERATION_FOR_LOG

#define BOOST_MAX_LINE_STR_SIZE 20 // log(2^64)
#define BOOST_LEVEL_UP_LIMIT    999

namespace boost {

  namespace logging {

//  Logging forward declarations ---------------------------------------------//
    class log_element;
    class level_element;
    class qualifier;
    class trace_element;
    class format;
    class sink;
    class logger;
    
//  Logging typedefs declarations --------------------------------------------//
    typedef enum { LEVEL = 0, QUALIFIER, TRACE, FILENAME, LINE }   param_e;
    typedef enum { SINK = 0, FORMAT }                   sink_format_assoc_e;
    typedef std::list<boost::shared_ptr<log_element> >  element_list_t;
    typedef std::list<boost::shared_ptr<std::ostream> > stream_list_t;
    typedef unsigned short                              level_t;
    typedef tuple<level_t, 
                  const qualifier *,
                  std::string, 
                  std::string, 
                  unsigned int>                         log_param_t;
    typedef std::list<format>                           format_list_t;
    typedef tuple<sink, format&>                        sink_format_assoc_t;
    typedef std::list<sink_format_assoc_t>             sink_format_assoc_list_t;
    typedef std::list<qualifier *>                      qualifier_list_t;

//  Used for shared_ptr() on statically allocated log_element ----------------//
    struct null_deleter
    { void operator()(void const *) const {} };

//  Qualifier class declaration ----------------------------------------------//
    class qualifier
    {
    public:
      qualifier() {}
      inline std::string to_string() const { return m_identifier; }
      virtual bool operator==(const qualifier &q) { return false; }
    protected:
      std::string m_identifier;
    };

    class log_qualifier : public qualifier 
    {
    public:
      log_qualifier() { m_identifier = "log"; }
      bool operator==(const qualifier &q) 
      { return (dynamic_cast<const log_qualifier *>(&q) != NULL); }
    };

    class notice_qualifier : public qualifier
    {
    public:
      notice_qualifier() { m_identifier = "notice"; }
      bool operator==(const qualifier &q) 
      { return (dynamic_cast<const notice_qualifier *>(&q) != NULL); }
    };

    class warning_qualifier : public qualifier
    {
    public:
      warning_qualifier() { m_identifier = "warning"; }
      bool operator==(const qualifier &q) 
      { return (dynamic_cast<const warning_qualifier *>(&q) != NULL); }
    };

    class error_qualifier : public qualifier
    {
    public:
      error_qualifier() { m_identifier = "error"; }
      bool operator==(const qualifier &q) 
      { return (dynamic_cast<const error_qualifier *>(&q) != NULL); }
    };

//  Element classes declaration  ---------------------------------------------//
    class log_element
    {
    public:
      virtual std::string to_string() { assert(0); return ""; };

      virtual std::string visit(format &f, const log_param_t &log_param);
    };
    
    class level_element : public log_element
    {
    public:
      std::string to_string(level_t l) 
      { 
        return str(boost::format("%i") % l);
      };

      std::string visit(format &f, const log_param_t &log_param);
    };
    
    class filename_element : public log_element
    {
    public:
      std::string to_string(const std::string &f) { return f; }
      std::string visit(format &f, const log_param_t &log_param);
    };
    
    class line_element : public log_element
    {
    public:
      std::string to_string(unsigned int l) 
      {
        return str(boost::format("%i") % l);
      }
      std::string visit(format &f, const log_param_t &log_param);
    };
    
    class date_element : public log_element
    {
    public:
      std::string to_string()
      {
        boost::gregorian::date d(boost::gregorian::day_clock::local_day());
        return boost::gregorian::to_iso_extended_string(d);
      }
    };
    
    class time_element : public log_element
    {
    public:
      std::string to_string() 
      { 
        boost::posix_time::ptime 
          t(boost::posix_time::microsec_clock::local_time());
        return boost::posix_time::to_simple_string(t); 
      };
    };
    
    class trace_element : public log_element
    {
    public:
      std::string to_string(const std::string& s) { return s; };

      std::string visit(format &f, const log_param_t &log_param);
    };

    class eol_element : public log_element
    {
    public:
      std::string to_string() { return "\n"; };
    };

    class literal_element : public log_element
    {
    public:
      explicit literal_element(const std::string &l) : m_literal(l) {}
      std::string to_string() { return m_literal; };
    private:
      std::string m_literal;
    };

    class qualifier_element : public log_element
    {
    public:
      qualifier_element(const qualifier &lq) 
      {
        m_qualifier_identifier = lq.to_string();
      }
      std::string to_string() { return m_qualifier_identifier; };
    private:
      std::string m_qualifier_identifier;
    };

//  Format class declatation -------------------------------------------------//
    class format
    {
    public:
      format(log_element &e)
        : m_identifier("unnamed") 
      {
        boost::shared_ptr<boost::logging::log_element> p(&e, null_deleter());
        m_element_list.push_back(p);
      }

      format(log_element &e, const std::string &identifier) 
        : m_identifier(identifier) 
      {
        boost::shared_ptr<boost::logging::log_element> p(&e, null_deleter());
        m_element_list.push_back(p);
      }

      format(element_list_t e) 
        : m_element_list(e), m_identifier("unnamed") {}

      format(element_list_t e, const std::string &identifier) 
        : m_element_list(e), m_identifier(identifier) {}

      std::string produce_trace(const log_param_t &log_param)
      {
        element_list_t::iterator e_it = m_element_list.begin();
        std::stringstream str_stream;
        for (; e_it != m_element_list.end(); ++e_it)
        {
	  str_stream << (*e_it)->visit(*this, log_param);
        }

        return str_stream.str();
      }

      // Visitors for the log elements
      std::string accept(log_element &e)
      {
	      return e.to_string();
      }
      std::string accept(level_element &e, level_t l)
      {
	      return e.to_string(l);
      }
      std::string accept(trace_element &e, const std::string& s)
      {
        return e.to_string(s);
      }
      std::string accept(filename_element &e, const std::string& s)
      {
        return e.to_string(s);
      }
      std::string accept(line_element &e, unsigned int l)
      {
        return e.to_string(l);
      }

    private:
      element_list_t    m_element_list;
      std::string       m_identifier;
    };

//  Sink class declaration ---------------------------------------------------//
    class sink
    {
    public:
      sink(std::ostream *s, level_t max_log_level = 1)
      {
        if (s)
          if (*s == std::cout || *s == std::cerr || *s == std::clog)
            m_output_stream.reset(s, null_deleter());
          else
            m_output_stream.reset(s);

        set_max_log_level(max_log_level);
      }

      void set_max_log_level(level_t max_log_level)
      { 
        m_max_log_level = ((BOOST_LEVEL_UP_LIMIT < max_log_level) 
          ? BOOST_LEVEL_UP_LIMIT : max_log_level);
      }

      inline level_t get_max_log_level() const { return m_max_log_level; }

      void consume_trace(format &f, const log_param_t &log_param)
      {
        /* make here check to avoid producing a useless trace */
        if (get<LEVEL>(log_param) > m_max_log_level)
          return ;

        qualifier_list_t::const_iterator it = m_qualifier_list.begin();
        bool qualifier_present = false;
        for ( ; !qualifier_present && it !=  m_qualifier_list.end(); ++it)
          qualifier_present = (**it == *get<QUALIFIER>(log_param));

        if (!qualifier_present)
          return ;

        *m_output_stream << f.produce_trace(log_param);
      }

      void attach_qualifier(qualifier &q)
      {
        m_qualifier_list.push_back(&q);
      }

    private:
      level_t                  m_max_log_level;
      shared_ptr<std::ostream> m_output_stream;
      qualifier_list_t         m_qualifier_list;      
    };
   
//  Element static instantiations --------------------------------------------//
    static level_element     level     = level_element();
    static filename_element  filename  = filename_element();
    static line_element      line      = line_element();
    static date_element      date      = date_element();
    static time_element      time      = time_element();
    static trace_element     trace     = trace_element();
    static eol_element       eol       = eol_element();

    static log_qualifier     log       = log_qualifier();
    static notice_qualifier  notice    = notice_qualifier();
    static warning_qualifier warning   = warning_qualifier();
    static error_qualifier   error     = error_qualifier();

//  Logger class declaration  ------------------------------------------------//
    class logger
    {
    public: 
      logger() : m_global_max_log_level(0) {}

	  void clear()
	  {
		  m_format_list.clear();
		  m_sink_format_assoc.clear();
	  }
        
      static logger *get_instance()
      {
#if defined(BOOST_HAS_THREADS)
        static boost::mutex m_inst_mutex;
        boost::mutex::scoped_lock scoped_lock(m_inst_mutex);
#endif // BOOST_HAS_THREADS
        static logger             *l = NULL;
        
        if (!l)
        {
          l = new logger();
          static shared_ptr<logger> s_ptr_l(l);
        }
          
        return l;
      }

      void add_format(const format &f)
      {
        m_format_list.push_back(f);
      }

      void add_sink(const sink &s)
      {
        if (m_format_list.begin() == m_format_list.end())
          throw "no format defined";

        // Updating global_max_level used for full lazy evaluation
        m_global_max_log_level = 
          (m_global_max_log_level < s.get_max_log_level()) 
          ? 
            s.get_max_log_level()
          : 
            m_global_max_log_level;

        m_sink_format_assoc.push_back
          (
            sink_format_assoc_t(s, *m_format_list.begin())
          );
      }

      void add_sink(const sink &s, format &f)
      {
        // Updating global_max_level used for full lazy evaluation
        m_global_max_log_level = 
          (m_global_max_log_level < s.get_max_log_level()) 
          ? 
            s.get_max_log_level()
          : 
            m_global_max_log_level;

        m_sink_format_assoc.push_back(sink_format_assoc_t(s, f));
      }

      inline level_t get_global_max_log_level() 
      { return m_global_max_log_level; }

      void trace(unsigned short     l, 
                 const qualifier   &q, 
                 const std::string &t, 
                 const std::string &f, 
                 unsigned int      ln)
      {
#if defined(BOOST_HAS_THREADS)
        boost::mutex::scoped_lock scoped_lock(m_mutex);
#endif // BOOST_HAS_THREADS

        log_param_t log_param(l, &q, t, f, ln);
        sink_format_assoc_list_t::iterator 
          s_it = m_sink_format_assoc.begin();
        for (; s_it != m_sink_format_assoc.end(); ++s_it)
        {
          get<SINK>(*s_it).consume_trace(get<FORMAT>(*s_it), log_param);
        }
      }
      
      void unformatted_trace(unsigned short     l, 
			     const qualifier   &q, 
			     const std::string &t, 
			     const std::string &f, 
			     unsigned int      ln);

    public:
      std::stringstream       m_string_stream;
      std::stack<std::string> m_string_stack;
      
    private:
      format_list_t            m_format_list;
      sink_format_assoc_list_t m_sink_format_assoc;

      // The global max log level is the highest log level on all the link
      // added to the logger. If no sink as a log level high enougth for
      // a trace, the trace does not need to be evaluated.
      level_t                  m_global_max_log_level;
#if defined(BOOST_HAS_THREADS)
      boost::mutex             m_mutex;
#endif // BOOST_HAS_THREADS
    };  // logger
    
//  Element functions definition ---------------------------------------------//
    inline std::string log_element::visit(format &f, 
                                          const log_param_t &log_param)
    {
      return f.accept(*this);
    }

    inline std::string level_element::visit(format &f, 
                                            const log_param_t &log_param)
    {
      return f.accept(*this, get<LEVEL>(log_param));
    }

    inline std::string trace_element::visit(format &f, 
                                            const log_param_t &log_param)
    {
      return f.accept(*this, get<TRACE>(log_param));
    }

    inline std::string filename_element::visit(format &f, 
                                               const log_param_t &log_param)
    {
      return f.accept(*this, get<FILENAME>(log_param));
    }

    inline std::string line_element::visit(format &f, 
                                           const log_param_t &log_param)
    {
      return f.accept(*this, get<LINE>(log_param));
    }

  } // !namespace logging

} // !namespace boost

//  Element global operators -------------------------------------------------//
inline boost::logging::element_list_t operator>>(
  boost::logging::log_element &lhs, 
  boost::logging::log_element &rhs)
{ 
  boost::logging::element_list_t l;
  l.push_back(boost::shared_ptr<boost::logging::log_element> 
                (&lhs, boost::logging::null_deleter()));
  l.push_back(boost::shared_ptr<boost::logging::log_element> 
                (&rhs, boost::logging::null_deleter())); 
  return l;
}

inline boost::logging::element_list_t operator>>(
  boost::logging::element_list_t lhs, 
  boost::logging::log_element &rhs)
{ 
  lhs.push_back(boost::shared_ptr<boost::logging::log_element> 
                (&rhs, boost::logging::null_deleter())); 
  return lhs; 
}

inline boost::logging::element_list_t operator>>(
  const std::string &s, 
  boost::logging::log_element &rhs)
{
  boost::logging::element_list_t l;
  boost::shared_ptr<boost::logging::literal_element> 
    p(new boost::logging::literal_element(s));
  l.push_back(p);
  l.push_back(boost::shared_ptr<boost::logging::log_element> 
                (&rhs, boost::logging::null_deleter())); 
  return l;
}

inline boost::logging::element_list_t operator>>(
  boost::logging::element_list_t lhs, 
  const std::string &s)
{ 
  boost::shared_ptr<boost::logging::literal_element> 
    p(new boost::logging::literal_element(s));
  lhs.push_back(p);
  return lhs;
}

inline
void boost::logging::logger::unformatted_trace(unsigned short     l, 
					       const qualifier   &q, 
                                               const std::string &t, 
                                               const std::string &f, 
                                               unsigned int      ln)
{
#if defined(BOOST_HAS_THREADS)
  boost::mutex::scoped_lock scoped_lock(m_mutex);
#endif // BOOST_HAS_THREADS
  log_param_t log_param(l, &q, t, f, ln);
  sink_format_assoc_list_t::iterator 
    s_it = m_sink_format_assoc.begin();
  for (; s_it != m_sink_format_assoc.end(); ++s_it)
    {
      boost::logging::format f(boost::logging::trace);
      get<SINK>(*s_it).consume_trace(f, log_param);
    }
}

#endif  // !BOOST_LOGGING_HPP
