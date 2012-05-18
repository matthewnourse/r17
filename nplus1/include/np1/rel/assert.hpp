// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_ASSERT_HPP
#define NP1_REL_ASSERT_HPP

#include "np1/rel/rlang/rlang.hpp"


namespace np1 {
namespace rel {



class assert {
public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(Input_Stream &input, Output_Stream &output,
                    const std::vector<rlang::token> &tokens,
                    bool assert_empty) {      
    /* Do the work. */
    bool result;
    if (assert_empty) {
      input.parse_headings();
      run(input, output, empty_callback(result));
    } else {
      input.parse_headings().write(output);
      run(input, output, nonempty_callback<Output_Stream>(result, output));
    }
  }

private:
  template <typename Input, typename Output, typename Callback>
  void run(Input &input, Output &output, Callback callback) {
    input.parse_records(callback);
    NP1_ASSERT(callback.result(), "Assertion failed!");
  }


  class callback_base {
  protected:
    callback_base(bool &result) : m_result(result) { fail(); }
    void fail() { m_result = false; }
    void success() { m_result = true; }
  public:
    bool result() const { return m_result; }

  private:
    bool &m_result;
  };

  class empty_callback : public callback_base {
  public:
    explicit empty_callback(bool &result) : callback_base(result) { success(); }

    bool operator()(const record_ref &r) {
      fail();
      return false;    
    }
  };

  template <typename Output>
  class nonempty_callback : public callback_base {
  public:
    nonempty_callback(bool &result, Output &o) : callback_base(result), m_output(o) {}
    bool operator()(const record_ref &r) {
      success();
      r.write(m_output);
      return true;    
    }

  private:
    Output &m_output;
  };

};


} // namespaces
}


#endif
