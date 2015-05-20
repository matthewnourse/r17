// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RLANG_COMPILER_HPP
#define NP1_REL_RLANG_COMPILER_HPP


#include "np1/rel/rlang/vm.hpp"
#include "np1/rel/record.hpp"
#include "rstd/pair.hpp"

namespace np1 {
namespace rel {
namespace rlang {

/// A compiler for rlang.
class compiler {
public:
  enum { MAX_NUMBER_FUNCTION_CALLS_PER_VM = vm::MAX_NUMBER_FUNCTION_CALLS };

public:
  class vm_info {
  public:
    vm_info() {}
    vm_info(const vm &vm, const rstd::string &typed_heading_name)
      : m_vm(vm), m_typed_heading_name(typed_heading_name) {}

    vm &get_vm() { return m_vm; }
    const vm &get_vm() const { return m_vm; }
    const rstd::string &get_typed_heading_name() const { return m_typed_heading_name; }

  private:
    vm m_vm;
    rstd::string m_typed_heading_name;
  };

public:
  // Compile a single coherent expression and return a VM that's ready to run.
  template <typename Source_Input_Stream>
  static vm compile_single_expression(Source_Input_Stream &source,
                                      const record_ref &this_headings,
                                      const record_ref &other_headings) {
    rstd::vector<token> expression;
    compile_single_expression_to_prefix(source, expression);
    return do_compile_single_expression(expression, this_headings, other_headings);
  }    

  static vm compile_single_expression(const rstd::vector<token> &expression,
                                      const record_ref &this_headings,
                                      const record_ref &other_headings) {
    return do_compile_single_expression(expression, this_headings, other_headings);
  }

  
  // Compile a single coherent expression to the list-of-tokens stage, where
  // the tokens are still in prefix notation.
  template <typename Source_Input_Stream>
  static void compile_single_expression_to_prefix(Source_Input_Stream &source,
                                                  rstd::vector<token> &prefix) {
    io::token_input_stream<Source_Input_Stream, fn::fn_table> token_input(source);
    read_all(token_input, prefix);  
  }


  // Is there a token in here that refers to the 'other' record?
  static bool any_references_to_other_record(const rstd::vector<token> &prefix) {
    size_t i;
    for (i = 0; i < prefix.size(); ++i) {
      if ((prefix[i].type() == token::TYPE_IDENTIFIER_VARIABLE)
          && !is_function(prefix, i)) {
        str::ref heading_name_without_record_identifier;    
          
        rel::detail::helper::record_identifier_type record_identifier
          = rel::detail::helper::get_heading_record_identifier(
              str::ref(prefix[i].text()), heading_name_without_record_identifier);
    
        switch (record_identifier) {
        case rel::detail::helper::RECORD_IDENTIFIER_THIS:
          break;
        
        case rel::detail::helper::RECORD_IDENTIFIER_OTHER:
          return true;
          break;
        }
      }
    }

    return false;
  }

  // Compile a list of tokens in prefix order and then evaluate them as a string.
  static rstd::pair<rstd::string, rlang::dt::data_type>
  eval_to_string(const rstd::vector<rel::rlang::token> &tokens) {
    NP1_ASSERT(
      tokens.size() > 0,
      "Unexpected empty stream operator argument list, expected string expression");
    
    rel::record_ref empty_r;

    rel::rlang::vm vm(rel::rlang::compiler::compile_single_expression(
                        tokens, empty_r, empty_r));

    rel::rlang::vm_heap heap;
    rlang::vm_stack &stack = vm.run_heap_reset(heap, empty_r, empty_r);

    switch (vm.return_type()) {
    case rlang::dt::TYPE_STRING:
    case rlang::dt::TYPE_ISTRING:
    case rlang::dt::TYPE_IPADDRESS:
      {
        str::ref s;
        stack.pop(s);
        return rstd::make_pair(s.to_string(), vm.return_type());
      }
      break;

    case rlang::dt::TYPE_INT:
      {
        int64_t i;
        stack.pop(i);
        return rstd::make_pair(str::to_dec_str(i), vm.return_type());
      }
      break;

    case rlang::dt::TYPE_UINT:
      {
        uint64_t ui;
        stack.pop(ui);            
        return rstd::make_pair(str::to_dec_str(ui), vm.return_type());
      }
      break;

    case rlang::dt::TYPE_DOUBLE:
      {
        double d;
        stack.pop(d);            
        return rstd::make_pair(str::to_dec_str(d), vm.return_type());
      }
      break;

    case rlang::dt::TYPE_BOOL:
      {
        bool b;
        stack.pop(b);
        return rstd::make_pair(str::from_bool(b).to_string(), vm.return_type());
      }
      break;
    }

    NP1_ASSERT(false, "Unreachable: unknown type");
    return rstd::make_pair(rstd::string(), rlang::dt::TYPE_STRING);
  }


  // Compile a list of tokens in prefix order and then evaluate them as a string.
  // Crashes is the result is not a string.
  static rstd::string eval_to_string_only(const rstd::vector<rel::rlang::token> &tokens) {
    rstd::pair<rstd::string, rlang::dt::data_type> result = eval_to_string(tokens);
    tokens[0].assert(rlang::dt::TYPE_STRING == result.second, "Expression is not a string expression");
    return result.first;
  }


  // Compile a comma-seperated list of expressions in prefix order and then
  // evaluate them as strings.
  static rstd::vector<rstd::pair<rstd::string, rlang::dt::data_type> >
  eval_to_strings(const rstd::vector<rel::rlang::token> &tokens) {
    NP1_ASSERT(
      tokens.size() > 0,
      "Unexpected empty stream operator argument list, expected string expressions");

    rstd::vector<token> expression;
    rstd::vector<rel::rlang::token>::const_iterator token_i = tokens.begin();
    rstd::vector<rel::rlang::token>::const_iterator token_end = tokens.end();
    rstd::vector<rstd::pair<rstd::string, dt::data_type> > results;

    while (read_expression(token_i, token_end, expression)) {
      results.push_back(eval_to_string(expression));
    }

    return results;
  }


  static rstd::vector<rstd::string> eval_to_strings_only(const rstd::vector<rel::rlang::token> &tokens) {
    rstd::vector<rstd::pair<rstd::string, rlang::dt::data_type> > expressions = eval_to_strings(tokens);
    rstd::vector<rstd::pair<rstd::string, rlang::dt::data_type> >::const_iterator i = expressions.begin();
    rstd::vector<rstd::pair<rstd::string, rlang::dt::data_type> >::const_iterator iz = expressions.end();
    rstd::vector<rstd::string> results;
    for (; i != iz; ++i) {
      tokens[0].assert(rlang::dt::TYPE_STRING == i->second, "Expression is not a string expression");
      results.push_back(i->first);
    }

    return results;
  }
  

  /// Split a comma-separated list of expressions into a vector of expressions.
  static rstd::vector<rstd::vector<rel::rlang::token> > split_expressions(const rstd::vector<rel::rlang::token> &tokens) {
    rstd::vector<token> expression;
    rstd::vector<rel::rlang::token>::const_iterator token_i = tokens.begin();
    rstd::vector<rel::rlang::token>::const_iterator token_end = tokens.end();
    rstd::vector<rstd::vector<rel::rlang::token> > results;

    while (read_expression(token_i, token_end, expression)) {
      results.push_back(expression);
    }

    return results;    
  }

  /// Compile a comma-separated list of heading names.
  static void compile_heading_name_list(const rstd::vector<rel::rlang::token> &tokens,
                                        const record_ref &this_headings,
                                        rstd::vector<rstd::string> &untyped_heading_names) {
    untyped_heading_names.clear();
  
    rstd::vector<token> expression;
    rstd::vector<rel::rlang::token>::const_iterator token_i = tokens.begin();
    rstd::vector<rel::rlang::token>::const_iterator token_end = tokens.end();

    while (read_expression(token_i, token_end, expression)) {
      token tok = expression[0];
      tok.assert(expression.size() == 1,
                  "Complex expressions are not allowed here- only heading names");

      str::ref heading_name_without_record_identifier;    
        
      rel::detail::helper::record_identifier_type record_identifier
        = rel::detail::helper::get_heading_record_identifier(
            str::ref(tok.text()), heading_name_without_record_identifier);
  
      tok.assert(rel::detail::helper::RECORD_IDENTIFIER_THIS == record_identifier,
                  "Only headings from the 'this' record are allowed here.");

      this_headings.mandatory_find_heading(heading_name_without_record_identifier);

      untyped_heading_names.push_back(heading_name_without_record_identifier.to_string());
    }
  }

  // Compile a "select" statement, returning separate VMs for each column.
  template <typename Source_Stream>
  static void compile_select(Source_Stream &source,
                              const record_ref &this_headings,                              
                              rstd::vector<vm_info> &vm_infos) {
    rstd::vector<token> tokens;
    compile_single_expression_to_prefix(source, tokens);
    do_compile_select(tokens, this_headings, vm_infos);
  }

  static void compile_select(const rstd::vector<token> &tokens,
                                const record_ref &this_headings,                              
                                rstd::vector<vm_info> &vm_infos) {
    do_compile_select(tokens, this_headings, vm_infos);
  }

  // Update tokens in infix format with information that's not available to the
  // token parser.
  static void update_tokens_with_info_from_surrounding_tokens(
                                                  rstd::vector<token> &infix) {
    size_t i;
    for (i = 0; i < infix.size(); ++i) {
      update_if_unary_minus(infix, i);
      update_if_function(infix, i);
    }          
  }
  
private:
  // For use within compile_select.
  struct expression_info {
    rstd::vector<shunting_yard::parsed_token_info> postfix;    
    rstd::string heading_name;
    bool is_complete;
  };

private:
  static void do_compile_select(const rstd::vector<token> &tokens,
                                const record_ref &this_headings,                              
                                rstd::vector<vm_info> &vm_infos) {
    vm_infos.clear();

    rstd::vector<expression_info> expression_infos;

    // Figuring out the return type of the expressions can be a litle tricky
    // because the expressions are allowed to refer to the "previous" record
    // which is the last-written record.  So we can't just compile each of
    // the expressions from left to right, we might have to make several passes
    // as we learn more about the types of the output record.

    // First, read in all the expressions.
    rstd::vector<token> expression;
    rstd::vector<rel::rlang::token>::const_iterator token_i = tokens.begin();
    rstd::vector<rel::rlang::token>::const_iterator token_end = tokens.end();

    while (read_expression(token_i, token_end, expression)) {  
      rstd::string heading_name = get_and_remove_result_heading_name(expression);
      rstd::vector<shunting_yard::parsed_token_info> postfix;
      infix_to_postfix(expression, postfix);
      expression_info exinfo = { postfix, heading_name, false };
      expression_infos.push_back(exinfo);
    }

    // We now know how many expressions there are so set up some other stuff.
    rstd::vector<rstd::string> typed_output_heading_names;
    typed_output_heading_names.resize(expression_infos.size());
    vm_infos.resize(expression_infos.size());

    // Next make a pass over the expressions, setting all those heading
    // type tags that are explicitly provided.
    size_t i;
    for (i = 0; i < expression_infos.size(); ++i) {
      const rstd::string &heading_name = expression_infos[i].heading_name;

      str::ref heading_type_tag =
        np1::rel::detail::helper::get_heading_type_tag(heading_name);

      if (!heading_type_tag.is_null()) {
        // Check that the heading type tag is valid.
        //TODO: we also need to check that the name itself is a valid heading
        // name without the type tag.
        dt::mandatory_from_string(heading_type_tag);        
      
        // Set the typed heading.
        typed_output_heading_names[i] = heading_name;
      }
    }

    // Now keep making compiling passes until we can't figure out anything else
    // or we finish.
    bool all_complete = true;
    bool made_progress = false;
    
    do {
      made_progress = false;
      all_complete = true;

      for (i = 0; i < expression_infos.size(); ++i) {
        if (!expression_infos[i].is_complete) {
          const rstd::vector<shunting_yard::parsed_token_info> &postfix
            = expression_infos[i].postfix;

          // If the expression refers only to output headings for which a type
          // is known, then we can compile it.
          if (refers_only_to_known_output_headings(postfix, typed_output_heading_names)) {
            record temp_other_headings(typed_output_heading_names, 0);
            vm new_vm(do_compile_single_expression(postfix, this_headings,
                                                    temp_other_headings.ref()));
            rstd::string typed_heading_name = typed_output_heading_names[i];

            // If we don't have a typed heading name for this expression already
            // then derive the expression's type.
            if (typed_heading_name.empty()) {
              typed_heading_name =
                np1::rel::detail::helper::make_typed_heading_name(
                              dt::to_string(new_vm.return_type()),
                              expression_infos[i].heading_name);

              typed_output_heading_names[i] = typed_heading_name;
            }
            vm_infos[i] = vm_info(new_vm, typed_heading_name);
            expression_infos[i].is_complete = true;
            made_progress = true;
          } else {
            all_complete = false;          
          }
        }
      }
    } while (!all_complete && made_progress);
    
    NP1_ASSERT(
      all_complete,
      "Unable to derive types for all output columns.  Please explicitly specify a column type using 'as [typename]:[headingname]'.  "
      "If you're using the 'prev.' construct to refer to a field in the previous record, remember to include that field too.");
  }

  // Does the postfix expression refer only to headings for which we have a type?
  static bool refers_only_to_known_output_headings(
                  const rstd::vector<shunting_yard::parsed_token_info> &postfix,    
                  const rstd::vector<rstd::string> &typed_output_headings) {
    rstd::vector<shunting_yard::parsed_token_info>::const_iterator i = postfix.begin();        
    rstd::vector<shunting_yard::parsed_token_info>::const_iterator iz = postfix.end();
    for (; i < iz; ++i) {
      if (i->tok().type() == token::TYPE_IDENTIFIER_VARIABLE) {
        str::ref heading_name_without_record_identifier;    
          
        rel::detail::helper::record_identifier_type record_identifier
          = rel::detail::helper::get_heading_record_identifier(
              str::ref(i->tok().text()), heading_name_without_record_identifier);

        switch (record_identifier) {
        case rel::detail::helper::RECORD_IDENTIFIER_THIS:
          break;
        
        case rel::detail::helper::RECORD_IDENTIFIER_OTHER:
          if (!in_typed_output_headings(typed_output_headings,
                                        heading_name_without_record_identifier)) {
            return false; 
          }
          break;
        }
      }
    }

    return true;
  }

  // Is the supplied untyped heading name in the list of typed output headings?
  static bool in_typed_output_headings(
                  const rstd::vector<rstd::string> &typed_output_headings,
                  const str::ref &untyped_name) {
    rstd::vector<rstd::string>::const_iterator i = typed_output_headings.begin();
    rstd::vector<rstd::string>::const_iterator iz = typed_output_headings.end();    
    for (; i < iz; ++i) {
      if (str::cmp(rel::detail::helper::get_heading_without_type_tag(*i),
                    untyped_name) == 0) {
        return true;
      }
    }
    
    return false;
  }


  // Read the whole stream into the vector.
  template <typename Input_Stream>
  static void read_all(io::token_input_stream<Input_Stream, fn::fn_table> &token_input,
                        rstd::vector<token> &output) {
    token tok;

    output.clear();

    while (token_input.read(tok)) {
      output.push_back(tok);
    }

    update_tokens_with_info_from_surrounding_tokens(output);
  }

  // If the token is a minus and it's actually a unary minus, update it.
  static void update_if_unary_minus(rstd::vector<token> &infix, size_t offset) {
    if (is_unary_minus(infix, offset)) {
      infix[offset].first_matching_sym_op_fn_id(fn::fn_table::find_unary_minus());
    }
  }

  // If the token marked as being a variable identifier and it's actually a
  // function, update it.
  static void update_if_function(rstd::vector<token> &infix, size_t offset) {
    if (is_function(infix, offset)) {
      infix[offset].type(token::TYPE_IDENTIFIER_FUNCTION);
    }
  }


  static bool is_unary_minus(const rstd::vector<token> &infix, size_t offset) {    
    if ((infix[offset].type() != token::TYPE_OPERATOR) || !infix[offset].is_minus()) {
      return false; 
    }

    if (0 == offset) {
      return true;
    }

    switch (infix[offset-1].type()) {
    case token::TYPE_STRING:    //TODO: should we spit the dummy here if it's a string?
    case token::TYPE_IDENTIFIER_VARIABLE:
    case token::TYPE_IDENTIFIER_FUNCTION:
    case token::TYPE_CLOSE_PAREN:
    case token::TYPE_INT:
    case token::TYPE_UINT:
    case token::TYPE_DOUBLE:
      return false;

    default:
      break;
    }

    return true;
  }

  static bool is_function(const rstd::vector<token> &infix, size_t offset) {    
    if (token::TYPE_IDENTIFIER_FUNCTION == infix[offset].type()) {
      return true;  
    }

    return ((token::TYPE_IDENTIFIER_VARIABLE == infix[offset].type())
            && (offset < infix.size()-1)
            && (token::TYPE_OPEN_PAREN == infix[offset+1].type()));
  }



  // Read a single expression, returns false when the end of the token list is
  // reached and there is no more input.
  static bool read_expression(
                    rstd::vector<rel::rlang::token>::const_iterator &token_i,
                    rstd::vector<rel::rlang::token>::const_iterator &token_end,
                    rstd::vector<token> &output) {
    int paren_depth = 0;

    output.clear();
    bool finished = false;

    for (; !finished && token_i < token_end; ++token_i) {
      switch (token_i->type()) {
      case token::TYPE_OPEN_PAREN:
        ++paren_depth;
        output.push_back(*token_i);
        break;

      case token::TYPE_CLOSE_PAREN:
        --paren_depth;
        output.push_back(*token_i);
        break;

      case token::TYPE_COMMA:
        if (0 == paren_depth) {
          finished = true;
        } else {
          output.push_back(*token_i);
        }
        break;

      default:
        output.push_back(*token_i);
        break;
      }
    }

    update_tokens_with_info_from_surrounding_tokens(output);

    return !output.empty();
  }  

  // May return a heading name without a type if none was specified yet.
  static rstd::string get_and_remove_result_heading_name(rstd::vector<token> &expression) {    
    token last = expression[expression.size() -1];    

    // If there is an "as [name]" clause...
    if ((expression.size() >= 3)
        && (expression[expression.size() - 2].type() == token::TYPE_IDENTIFIER_VARIABLE)
        && (str::cmp(expression[expression.size() - 2].text(), "as") == 0)) {
      // ...check that it's a valid heading name...
      last.assert(last.type() == token::TYPE_IDENTIFIER_VARIABLE,
                  "Token after 'as' must be a valid heading name");
      
      // ...remove the 'as' clause.
      expression.pop_back();
      expression.pop_back();
    } else {
      // If we get to here then there is no "as [name]" clause.  If there's just
      // a single value then that's ok.
      last.assert(
        expression.size() == 1,
        "Result heading names must be explicitly specified with an 'as' clause");
    }

    return last.text(); 
  }


  static void infix_to_postfix(const rstd::vector<token> &source,
                                rstd::vector<shunting_yard::parsed_token_info> &target) {
    // Parse infix input and convert it to postfix.
    shunting_yard::parse<fn::fn_table>(source, target);

    NP1_ASSERT(target.size() != 0, "No rlang expressions found");    
  }



   // Compile a single expression from a collection of tokens.
  static vm do_compile_single_expression(const rstd::vector<token> &source,
                                          const record_ref &this_headings,
                                          const record_ref &other_headings) {
    rstd::vector<shunting_yard::parsed_token_info> parsed_tokens;    
    infix_to_postfix(source, parsed_tokens);
    return do_compile_single_expression(parsed_tokens, this_headings, other_headings);
  }

  


  static vm do_compile_single_expression(
            const rstd::vector<shunting_yard::parsed_token_info> &parsed_tokens,
            const record_ref &this_headings,
            const record_ref &other_headings) {
    simulated_stack sim_stack;
    vm_function_call_list<MAX_NUMBER_FUNCTION_CALLS_PER_VM> function_calls;
    vm_literals literals;
    bool refers_to_other_record = false;
    rstd::vector<shunting_yard::parsed_token_info>::const_iterator i = parsed_tokens.begin();
    rstd::vector<shunting_yard::parsed_token_info>::const_iterator iz = parsed_tokens.end();
    do_compile_single_expression(
      i, iz, this_headings, other_headings, sim_stack, function_calls, literals, refers_to_other_record);

    return vm(literals, function_calls, sim_stack.top(), refers_to_other_record);
  }


  static void do_compile_single_expression(
                rstd::vector<shunting_yard::parsed_token_info>::const_iterator i,
                rstd::vector<shunting_yard::parsed_token_info>::const_iterator iz,
                const record_ref &this_headings,
                const record_ref &other_headings,
                simulated_stack &sim_stack,
                vm_function_call_list<MAX_NUMBER_FUNCTION_CALLS_PER_VM> &function_calls,
                vm_literals &literals,
                bool &refers_to_other_record) {

    size_t sim_stack_start_size = sim_stack.size();

    // Walk the list of postfix-parsed tokens and convert them into executable
    // functions.  

    for (; i < iz; ++i) {
      // Check to see if this is an 'if'.
      if ((i->tok().type() == token::TYPE_IDENTIFIER_FUNCTION)
          && (str::cmp(i->tok().text(), "if") == 0)) {
        // It is an 'if'.  Check that its argument is a boolean and that there
        // is only one argument.
        i->tok().assert(sim_stack.top() == dt::TYPE_BOOL, "Non-boolean argument to 'if'.");
        i->tok().assert(i->function_arg_count() == 1, "Multiple arguments supplied to 'if'.");
        sim_stack.pop();

        // Look for the matching "then" and "else" and check that they have
        // one argument each.
        rstd::vector<shunting_yard::parsed_token_info>::const_iterator then_i =
          mandatory_find_function_starting_at(i->function_end_postfix_offset()+1, i+1, iz, "then");

        rstd::vector<shunting_yard::parsed_token_info>::const_iterator else_i =
          mandatory_find_function_starting_at(then_i->function_end_postfix_offset()+1, then_i+1, iz, "else");

        then_i->tok().assert(then_i->function_arg_count() == 1, "Multiple arguments supplied to 'then'.");
        else_i->tok().assert(then_i->function_arg_count() == 1, "Multiple arguments supplied to 'else'.");

        // Save the "if" call.  We don't know the length of the 'then' branch until
        // we process it, so save the offset of the compiled 'if' so we can come
        // back to it.
        size_t compiled_if_offset = function_calls.size();
        function_calls.push_back(vm_function_call(fn::fn_table::find_if(), (size_t)-1));

        // Process the "then" branch.
        do_compile_single_expression(
          i+1, then_i, this_headings, other_headings, sim_stack, function_calls,
          literals, refers_to_other_record);

        // Now we know how long the "then" is, we can save the offset of the "else"
        // branch into the compiled "if".  The +1 is so that it's one past the
        // unconditional jump that we're about to append.
        function_calls[compiled_if_offset].data(
                                function_calls.size() - compiled_if_offset + 1);

        // Save the type of the "then" branch.
        dt::data_type then_return_type = sim_stack.top();
        sim_stack.pop();

        // Write out an unconditional jump at the end of the "then" branch
        // We don't know how long the compiled 'else' branch is yet so save
        // an offset so we can come back to it.
        size_t compiled_goto_offset = function_calls.size();
        function_calls.push_back(vm_function_call(fn::fn_table::find_goto(), (size_t)-1));

        // Processs the "else" branch.
        do_compile_single_expression(
          then_i+1, else_i, this_headings, other_headings, sim_stack, function_calls,
          literals, refers_to_other_record);

        // Now we know how long the "else" is, we can fix up the compiled goto.
        function_calls[compiled_goto_offset].data(function_calls.size() - compiled_goto_offset);

        // Check that the "else" branch returns the same type as the "then" branch.
        else_i->tok().assert(sim_stack.top() == then_return_type,
                              "'then' and 'else' branches do not evaluate to the same type.");
        i = else_i;
      } else {
        // It's not an 'if', it's just a normal thing.
        bool temp_refers_to_other_record = false;        
  
        function_calls.push_back(
          create_function_call(this_headings, other_headings, i->tok(),
                                i->function_arg_count(), sim_stack, literals,
                                temp_refers_to_other_record));
  
        if (temp_refers_to_other_record) {
          refers_to_other_record = true;
        }
      }
    }
    
    // We don't have any functions that return void, and all arguments must have
    // been consumed.
    size_t expected_sim_stack_size = sim_stack_start_size + 1;
    NP1_ASSERT(sim_stack.size() == expected_sim_stack_size,
                "Unexpected simulated stack size: " + str::to_dec_str(sim_stack.size())
                + "  Expected: " + str::to_dec_str(expected_sim_stack_size));
  }


    
  static vm_function_call create_function_call(
                                      const record_ref &this_headings,
                                      const record_ref &other_headings,
                                      const token &tok, size_t called_arg_count,
                                      simulated_stack &stk,
                                      vm_literals &literals,
                                      bool &refers_to_other_record) {
    // Literals and variables are special cases- we need to push them on to the
    // simulated stack first.
    switch (tok.type()) {
    case token::TYPE_STRING:
      stk.push(dt::TYPE_STRING);
      return vm_function_call(
              fn::fn_table::find_push_literal_string(),
              literals.push_back(dt::TYPE_STRING, tok.text()));

    case token::TYPE_INT:
      stk.push(dt::TYPE_INT);
      return vm_function_call(
              fn::fn_table::find_push_literal_integer(),
              literals.push_back(dt::TYPE_INT, tok.text()));

    case token::TYPE_UINT:
      stk.push(dt::TYPE_UINT);
      return vm_function_call(
              fn::fn_table::find_push_literal_uinteger(),
              literals.push_back(dt::TYPE_UINT, tok.text()));

    case token::TYPE_DOUBLE:
      stk.push(dt::TYPE_DOUBLE);
      return vm_function_call(
              fn::fn_table::find_push_literal_double(),
              literals.push_back(dt::TYPE_DOUBLE, tok.text()));

    case token::TYPE_BOOL_TRUE:
      stk.push(dt::TYPE_BOOL);
      return vm_function_call(fn::fn_table::find_push_literal_boolean_true(), -1);

    case token::TYPE_BOOL_FALSE:
      stk.push(dt::TYPE_BOOL);
      return vm_function_call(
              fn::fn_table::find_push_literal_boolean_false(), -1);

    case token::TYPE_IDENTIFIER_VARIABLE:
      return create_variable_push_function_call(this_headings, other_headings,
                                                tok.text(), stk,
                                                refers_to_other_record);

    default:
      break;
    }

    // It's an operator or function.
    size_t function_id = fn::fn_table::find(str::ref(tok.text()), stk, called_arg_count);
    if ((size_t)-1 == function_id) {
      tok.assert(fn::fn_table::find_first(str::ref(tok.text())) != (size_t)-1,
                  "Unknown function or operator");

      // We can find the operator or function but the overloads for it aren't
      // correct.  Get the list of overloads and show it to the user.
      rstd::string overloads;
      np1::io::string_output_stream sos(overloads);
      help::markdown::function_or_operator_overloads(sos, tok.text());
      rstd::string error_text =
        "Function or operator is known but overload is not supported.  Supported overloads:\n"
        + overloads;

      if ((stk.size() >= 2) && ((stk.top() == dt::TYPE_INT) || (stk.top_minus_1() == dt::TYPE_UINT))) {
        error_text =
          error_text + "Note that there is no automatic conversion between int and uint to avoid ambiguity and unexpected results.  To convert an integer literal to an unsigned integer literal add a trailing U, like this: 2U\n\n";
      }

      tok.assert(false, error_text.c_str());
    }

    // Apply this operator or function's changes to the simulated stack.
    fn::fn_table::fn_info finfo = fn::fn_table::get_info(function_id);
    size_t number_arguments = finfo.number_arguments();
    size_t i;
    for (i = 0; i < number_arguments; ++i) {
      stk.pop();
    }

    stk.push(finfo.return_type());

    return vm_function_call(function_id, (size_t)-1);    
  }


  static vm_function_call create_variable_push_function_call(
                                                const record_ref &this_headings,
                                                const record_ref &other_headings,
                                                const char *tok_text,
                                                simulated_stack &stk,
                                                bool &refers_to_other_record) {
    str::ref heading_name_without_record_identifier;    
      
    rel::detail::helper::record_identifier_type record_identifier
      = rel::detail::helper::get_heading_record_identifier(
          str::ref(tok_text), heading_name_without_record_identifier);

    bool is_this = true;
    switch (record_identifier) {
    case rel::detail::helper::RECORD_IDENTIFIER_THIS:
      is_this = true;
      break;
    
    case rel::detail::helper::RECORD_IDENTIFIER_OTHER:
      is_this = false;
      break;
    }

    refers_to_other_record = !is_this;

    // Check for the special "row number" value.
    if (str::cmp(heading_name_without_record_identifier,
                  NP1_REL_RLANG_FN_ROWNUM_SPECIAL_VARIABLE_NAME) == 0) {
      stk.push(dt::TYPE_UINT);
      return vm_function_call(
              is_this ? fn::fn_table::find_push_this_rownum() : fn::fn_table::find_push_other_rownum(),
              -1);
    }

    const record_ref &headings = is_this ? this_headings : other_headings;
    size_t field_number = headings.mandatory_find_heading(
                                      heading_name_without_record_identifier);

    switch (mandatory_heading_name_to_data_type(headings.mandatory_field(field_number))) {
    case dt::TYPE_STRING:
      stk.push(dt::TYPE_STRING);
      return vm_function_call(
              is_this ? fn::fn_table::find_push_this_field_string()
                      : fn::fn_table::find_push_other_field_string(),
              field_number);

    case dt::TYPE_ISTRING:
      stk.push(dt::TYPE_ISTRING);
      return vm_function_call(
              is_this ? fn::fn_table::find_push_this_field_istring()
                      : fn::fn_table::find_push_other_field_istring(),
              field_number);

    case dt::TYPE_INT:
      stk.push(dt::TYPE_INT);
      return vm_function_call(
              is_this ? fn::fn_table::find_push_this_field_integer()
                      : fn::fn_table::find_push_other_field_integer(),
              field_number);

    case dt::TYPE_UINT:
      stk.push(dt::TYPE_UINT);
      return vm_function_call(
              is_this ? fn::fn_table::find_push_this_field_uinteger()
                      : fn::fn_table::find_push_other_field_uinteger(),
              field_number);

    case dt::TYPE_DOUBLE:
      stk.push(dt::TYPE_DOUBLE);
      return vm_function_call(
              is_this ? fn::fn_table::find_push_this_field_double()
                      : fn::fn_table::find_push_other_field_double(),
              field_number);

    case dt::TYPE_BOOL:
      stk.push(dt::TYPE_BOOL);
      return vm_function_call(
              is_this ? fn::fn_table::find_push_this_field_boolean()
                      : fn::fn_table::find_push_other_field_boolean(),
              field_number);

    case dt::TYPE_IPADDRESS:
      stk.push(dt::TYPE_IPADDRESS);
      return vm_function_call(
              is_this ? fn::fn_table::find_push_this_field_ipaddress()
                      : fn::fn_table::find_push_other_field_ipaddress(),
              field_number);
    }

    NP1_ASSERT(false, "Unreachable");
    return vm_function_call();
  }    

  // Find the function that starts at the supplied position.
  static rstd::vector<shunting_yard::parsed_token_info>::const_iterator
  mandatory_find_function_starting_at(
        size_t pos,
        rstd::vector<shunting_yard::parsed_token_info>::const_iterator start_i,
        rstd::vector<shunting_yard::parsed_token_info>::const_iterator end_i,
        const char *name) {
    rstd::vector<shunting_yard::parsed_token_info>::const_iterator i = start_i;
    for (; i < end_i; ++i) {
      if ((i->function_start_postfix_offset() == pos)
          && (str::cmp(i->text(), name) == 0)) {
        return i; 
      }
    }

    NP1_ASSERT(false, "Unable to find matching '" + rstd::string(name) + "'");
    return 0;
  }
 

  static dt::data_type mandatory_heading_name_to_data_type(const str::ref &heading) {
    str::ref type_tag =
              np1::rel::detail::helper::mandatory_get_heading_type_tag(heading);
    return dt::mandatory_from_string(type_tag);  
  }

};


} // namespaces
}
}



#endif
