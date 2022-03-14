// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_META_STREAM_OP_TABLE_HPP
#define NP1_META_STREAM_OP_TABLE_HPP

#include "config.h"
#include "np1/simple_types.hpp"

#include "rstd/swap.hpp"
#include "rstd/list.hpp"
#include "rstd/vector.hpp"
#include "rstd/stack.hpp"
#include "np1/io/mandatory_output_stream.hpp"
#include "np1/io/mandatory_record_input_stream.hpp"
#include "np1/io/file.hpp"
#include "np1/rel/rlang/token.hpp"

namespace np1 {
namespace meta {

/// Helper functions to avoid circular #includes.
size_t stream_op_table_size();
const char *stream_op_table_name(size_t n);
const char *stream_op_table_since(size_t n);
const char *stream_op_table_description(size_t n);

typedef enum {
  STREAM_OP_TABLE_IO_TYPE_R17_NATIVE,
  STREAM_OP_TABLE_IO_TYPE_TSV,
  STREAM_OP_TABLE_IO_TYPE_CSV,
  STREAM_OP_TABLE_IO_TYPE_USV,
  STREAM_OP_TABLE_IO_TYPE_TEXT_UTF8,
  STREAM_OP_TABLE_IO_TYPE_TEXT_UTF16,
  STREAM_OP_TABLE_IO_TYPE_NONE,
  STREAM_OP_TABLE_IO_TYPE_ANY
} stream_op_table_io_type_type;

stream_op_table_io_type_type stream_op_table_input_type(size_t n);
stream_op_table_io_type_type stream_op_table_output_type(size_t n);

static const char *stream_op_table_io_type_to_text(stream_op_table_io_type_type type) {
  switch (type) {
  case STREAM_OP_TABLE_IO_TYPE_R17_NATIVE:
    return "r17 native record format";

  case STREAM_OP_TABLE_IO_TYPE_TSV:
    return "TAB-separated value format with typed headings";

  case STREAM_OP_TABLE_IO_TYPE_CSV:
    return "Comma-separated value format with typed headings";

  case STREAM_OP_TABLE_IO_TYPE_USV:
    return "Unit-separated value format with typed headings";

  case STREAM_OP_TABLE_IO_TYPE_TEXT_UTF8:
    return "UTF-8 text";

  case STREAM_OP_TABLE_IO_TYPE_TEXT_UTF16:
    return "UTF-16 text";

  case STREAM_OP_TABLE_IO_TYPE_NONE:
    return "None";

  case STREAM_OP_TABLE_IO_TYPE_ANY:
    return "Any";
  }

  NP1_ASSERT(false, "Unreachable");
  return "[undefined]";
}


size_t stream_op_table_find(const char *name);
void script_run(io::unbuffered_stream_base &input, io::unbuffered_stream_base &output,
                const rstd::string &args);



} // namespaces
}


#include "np1/assert.hpp"
#include "np1/str.hpp"
#include "np1/environment.hpp"
#include "np1/io/string_input_stream.hpp"
#include "np1/rel/record.hpp"
#include "np1/rel/group.hpp"
#include "np1/rel/order_by.hpp"
#include "np1/rel/join_natural.hpp"
#include "np1/rel/join_left.hpp"
#include "np1/rel/join_anti.hpp"
#include "np1/rel/join_consistent_hash.hpp"
#include "np1/rel/select.hpp"
#include "np1/rel/record_count.hpp"
#include "np1/rel/record_split.hpp"
#include "np1/rel/unique.hpp"
#include "np1/rel/str_split.hpp"
#include "np1/rel/where.hpp"
#include "np1/rel/assert.hpp"
#include "np1/rel/tsv_translate.hpp"
#include "np1/rel/csv_translate.hpp"
#include "np1/rel/usv_translate.hpp"
#include "np1/rel/from_text.hpp"
#include "np1/rel/generate_sequence.hpp"
#include "np1/text/utf16_to_utf8.hpp"
#include "np1/text/strip_cr.hpp"
#include "np1/meta/remote.hpp"
#include "np1/meta/shell.hpp"
#include "np1/meta/parallel_explicit_mapping.hpp"
#include "np1/lang/python.hpp"
#include "np1/lang/r.hpp"
#include "np1/io/directory.hpp"


namespace np1 {
namespace meta {


// The base class for all the wrappers around stream operators.
// If we make any function pure virtual then we'll suddenly depend on
// libstdc++....sadness.
struct stream_op_wrap_base {
  typedef io::mandatory_record_input_stream<io::unbuffered_stream_base, rel::record, rel::record_ref>
      mandatory_delimited_input_type;

  typedef io::buffered_output_stream<io::unbuffered_stream_base> buffered_output_type;
  typedef io::mandatory_output_stream<buffered_output_type> mandatory_buffered_output_type;

  virtual const char *name() const {
    NP1_ASSERT(false, "Stream operator has no name method!");
    return "";
  }

  virtual const char *since() const { return "1.0"; }

  virtual const char *description() const {
    NP1_ASSERT(false, "Stream operator has no description method!");
    return "";
  }

  virtual stream_op_table_io_type_type input_type() const {
    NP1_ASSERT(false, "Stream operator " + rstd::string(name()) + " has no input_type method!");
    return STREAM_OP_TABLE_IO_TYPE_ANY;
  }

  virtual stream_op_table_io_type_type output_type() const {
    NP1_ASSERT(false, "Stream operator " + rstd::string(name()) + " has no output_type method!");
    return STREAM_OP_TABLE_IO_TYPE_ANY;
  }

  virtual void call(io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    mandatory_delimited_input_type mandatory_delimited_input(input);
    buffered_output_type buffered_output(output);
    mandatory_buffered_output_type mandatory_output(buffered_output);
    call(mandatory_delimited_input, mandatory_output, tokens);
  }

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    NP1_ASSERT(false, "Stream operator wrapper has no call(delimited, mandatory, tokens) method!");
  }
};



/// All the stream operator wrappers.

struct rel_group_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.group"; }
  virtual const char *description() const {
    return
      "`rel.group(count)` is equivalent to SQL's `SELECT COUNT(1) FROM ... GROUP BY ...`.  The new heading is called `int:_count`.  \n"
      "`rel.group(avg header_name)` is equivalent to SQL's `SELECT AVG(header_name) FROM ... GROUP BY ...`.  The new heading is called `int:_avg`.  \n"
      "`rel.group(min header_name)` is equivalent to SQL's `SELECT MIN(header_name) FROM ... GROUP BY ...`.  No new column is created, the `header_name` column is used to hold the minimum value.  \n"
      "`rel.group(max header_name)` is equivalent to SQL's `SELECT MAX(header_name) FROM ... GROUP BY ...`.  No new column is created, the `header_name` column is used to hold the maximum value.  \n"
      "`rel.group(sum header_name)` is equivalent to SQL's `SELECT SUM(header_name) FROM ... GROUP BY ...`.  The new heading is called `int:_sum`.  \n"
      "`rel.group(median header_name)` finds the median in the same way that `rel.group(avg header_name)` finds the average.  The new heading is called `int:_median`.  The `median` aggregator is only in r17 1.8.0 and later.  \n";
  };
  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::group op;
    op(mandatory_delimited_input, mandatory_output, tokens);    
  }  
} rel_group_instance;



struct rel_join_natural_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.join.natural"; }
  virtual const char *description() const { return "`rel.join.natural('other_file_name')` joins the input to `other_file_name`."; };
  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::join_natural op;
    op(mandatory_delimited_input, mandatory_output, tokens);    
  }  
} rel_join_natural_instance;


struct rel_join_left_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.join.left"; }
  virtual const char *description() const { return "`rel.join.left('other_file_name')` left-joins the input to `other_file_name`."; };
  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::join_left op;
    op(mandatory_delimited_input, mandatory_output, tokens);    
  }  
} rel_join_left_instance;



struct rel_join_anti_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.join.anti"; }
  virtual const char *description() const { return "`rel.join.anti('other_file_name')` antijoins the input to `other_file_name`."; };
  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::join_anti op;
    op(mandatory_delimited_input, mandatory_output, tokens);    
  }  
} rel_join_anti_instance;


struct rel_join_consistent_hash : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.join.consistent_hash"; }
  virtual const char *description() const { return "`rel.join.consistent_hash('other_file_name')` joins the input to `other_file_name` using a consistent hash (http://en.wikipedia.org/wiki/Consistent_hashing).  This operator will refuse to join two streams with common header names.  Streams may contain duplicate records.  The more times that a record appears in a stream, the more likely it is to be matched & included in the join."; };
  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::join_consistent_hash op;
    op(mandatory_delimited_input, mandatory_output, tokens);    
  }  
} rel_join_consistent_hash_instance;





#define NP1_GENERIC_SORT_DESCRIPTION " will sort by heading a then b, then c"
#define NP1_GENERIC_SORT_MEMORY_USAGE "Currently the size of the sort input is limited to the available virtual memory minus 30 bytes per record overhead."

struct rel_order_by_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.order_by"; }
  virtual const char *description() const {
    return "`rel.order_by(a, b, c)`" NP1_GENERIC_SORT_DESCRIPTION " using the default search strategy: a stable merge sort.  " NP1_GENERIC_SORT_MEMORY_USAGE;
  };
  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::order_by op;
    op(mandatory_delimited_input, mandatory_output, tokens,
        rel::order_by::TYPE_MERGE_SORT, rel::order_by::ORDER_ASCENDING);    
  }  
} rel_order_by_instance;

struct rel_order_by_desc_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.order_by.desc"; }
  virtual const char *description() const {
    return "`rel.order_by.desc(a, b, c)` will sort in the opposite order to `rel.order_by(a, b, c)`.";
  };
  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::order_by op;
    op(mandatory_delimited_input, mandatory_output, tokens,
        rel::order_by::TYPE_MERGE_SORT, rel::order_by::ORDER_DESCENDING);    
  }  
} rel_order_by_desc_instance;



struct rel_order_by_mergesort_wrap : public rel_order_by_wrap {
  virtual const char *name() const { return "rel.order_by.mergesort"; }
  virtual const char *description() const {
    return "`rel.order_by.mergesort(a, b, c)`" NP1_GENERIC_SORT_DESCRIPTION " using a stable merge sort.  " NP1_GENERIC_SORT_MEMORY_USAGE;
  };
} rel_order_by_mergesort_instance;

struct rel_order_by_mergesort_desc_wrap : public rel_order_by_desc_wrap {
  virtual const char *name() const { return "rel.order_by.mergesort.desc"; }
  virtual const char *description() const {
    return "`rel.order_by.mergesort.desc(a, b, c)` will sort in the opposite order to `rel.order_by.mergesort(a, b, c)`.";
  };
} rel_order_by_mergesort_desc_instance;



struct rel_order_by_quicksort_wrap : public rel_order_by_wrap {
  virtual const char *name() const { return "rel.order_by.quicksort"; }
  virtual const char *description() const {
    return "`rel.order_by.quicksort(a, b, c)`" NP1_GENERIC_SORT_DESCRIPTION " using a stable quicksort variant.  The quicksort variant has the Sedgewick optimizations but still has the same poor worst-case running time as other quicksorts.  " NP1_GENERIC_SORT_MEMORY_USAGE;
  };

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::order_by op;
    op(mandatory_delimited_input, mandatory_output, tokens,
        rel::order_by::TYPE_QUICK_SORT, rel::order_by::ORDER_ASCENDING);    
  }  
} rel_order_by_quicksort_instance;


struct rel_order_by_quicksort_desc_wrap : public rel_order_by_wrap {
  virtual const char *name() const { return "rel.order_by.quicksort.desc"; }
  virtual const char *description() const {
    return "`rel.order_by.quicksort.desc(a, b, c)` will sort in the opposite order to `rel.order_by.quicksort(a, b, c)`.";
  };

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::order_by op;
    op(mandatory_delimited_input, mandatory_output, tokens,
        rel::order_by::TYPE_QUICK_SORT, rel::order_by::ORDER_DESCENDING);    
  }  
} rel_order_by_quicksort_desc_instance;




struct rel_select_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.select"; }
  virtual const char *description() const {
    return "`rel.select(expr1 as [type:]header1, expr2 as [type:]header2, ...)` will transform incoming records as specified by "
            "the expression(s).  Approximately equivalent to SQL's SELECT.  The `prev.` prefix may be used to "
            "refer to a field from the transformed previous record.  If the header name is prefixed with a type name then the "
            "data will be coerced to that type without any extra type checking.";
  };

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::select op;
    op(mandatory_delimited_input, mandatory_output, tokens);
  }  
} rel_select_instance;


struct rel_record_count_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.record_count"; }
  virtual const char *description() const {
    return "`rel.record_count()` will count the number of incoming records.  NOTE that this operator writes an unadorned decimal number to the output stream.";
  };

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::record_count op;
    op(mandatory_delimited_input, mandatory_output, tokens);
  }  
} rel_record_count_instance;


struct rel_record_split_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.record_split"; }
  virtual const char *description() const {
    return "`rel.record_split(N, file_name_stub)` will split the incoming records into files of at most N records with names starting with `file_name_stub`.  "
            "It will gzip the files.  It does not write to the output stream.";
  };

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::record_split op;
    op(mandatory_delimited_input, mandatory_output, tokens);
  }  
} rel_record_split_instance;


struct rel_where_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.where"; }
  virtual const char *description() const {
    return "`rel.where(expression)` will include records in the output if `expression` returns true.  "
            "Approximately equivalent to SQL's WHERE.";
  };

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::where op;
    op(mandatory_delimited_input, mandatory_output, tokens);
  }  
} rel_where_instance;


struct rel_unique_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.unique"; }
  virtual const char *description() const {
    return "`rel.unique()` includes only a single copy of duplicate records in the output stream.  Approximately equivalent to SQL's DISTINCT clause.";
  };

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::unique op;
    op(mandatory_delimited_input, mandatory_output, tokens);    
  }  
} rel_unique_instance;


struct rel_str_split : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.str_split"; }
  virtual const char *description() const {
    return "`rel.str_split(header_name, 'regex')` splits the string in `header_name` using the `regex` regular "
            "expression.  It creates a new `" NP1_REL_STR_SPLIT_COUNTER_HEADING_NAME "` column to count the "
            "newly-split string components."; 
  };

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::str_split op;
    op(mandatory_delimited_input, mandatory_output, tokens);
  }  
} rel_str_split;



struct rel_assert_empty_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.assert.empty"; }
  virtual const char *description() const {
    return "`rel.assert.empty()` prints an error message and exits if the input stream is not empty.  "
           "In 1.4.3+, `rel.assert.empty` does not pass through any data or headers.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_ANY; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_NONE; }

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::assert op;
    op(mandatory_delimited_input, mandatory_output, tokens, true);    
  }  
} rel_assert_empty_instance;



struct rel_assert_nonempty_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.assert.nonempty"; }
  virtual const char *description() const {
    return "`rel.assert.nonempty()` prints error message and exits if the input stream is empty.\n"
           "Will pass through any incoming data.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_ANY; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_ANY; }


  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::assert op;
    op(mandatory_delimited_input, mandatory_output, tokens, false);    
  }  
} rel_assert_nonempty_instance;



struct rel_from_tsv_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.from_tsv"; }
  virtual const char *description() const {
    return "`rel.from_tsv()` translates the input stream from TAB-separated-value format to native record format.  "
            "In 1.4.3 and earlier, the input stream must have headings.  In 1.2.2+, if a heading has no type tag then a type of 'string' is assumed.  "
            "In 1.4.3+, non-alphanumeric characters are replaced with an _ character.  "
            "In 1.4.4+, `rel.from_tsv(\"heading_name_1\", \"heading_name_2\")` will ignore any heading names in the input stream and use the supplied heading names instead.  This allows parsing of input streams that have no heading names.  ";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_TSV; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }


  virtual void call(io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    buffered_output_type buffered_output(output);
    mandatory_buffered_output_type mandatory_output(buffered_output);
    
    rel::tsv_translate translator;
    translator.from_tsv(input, mandatory_output, tokens);
  }
} rel_from_tsv_instance;


struct rel_to_tsv_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.to_tsv"; }
  virtual const char *description() const {
    return "`rel.to_tsv()` translates the input stream from native record format to TAB-separated-value format.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_TSV; }

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::tsv_translate translator;
    translator.to_tsv(mandatory_delimited_input, mandatory_output, tokens);
  }
} rel_to_tsv_instance;


struct rel_from_csv_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.from_csv"; }
  virtual const char *since() const { return "1.6.0"; }
  virtual const char *description() const {
    return "`rel.from_csv()` translates the input stream from comma-separated-value format to native record format.  "
            "If no arguments are supplied then the input stream must have headings.  "
            "If a heading has no type tag then a type of 'string' is assumed.  "
            "Non-alphanumeric characters are replaced with an _ character.  "
            "`rel.from_csv(\"heading_name_1\", \"heading_name_2\")` will ignore any heading names in the input stream and use the supplied heading names instead.  "
            "This allows parsing of input streams that have no heading names.  "
            "Values that contain commas must be enclosed in double quotes (\"s).  "
            "Escape \" characters with a second \" character, like this: \"\".  "
            "Use \\n to represent newlines.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_CSV; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }


  virtual void call(io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    buffered_output_type buffered_output(output);
    mandatory_buffered_output_type mandatory_output(buffered_output);
    
    rel::csv_translate translator;
    translator.from_csv(input, mandatory_output, tokens);
  }
} rel_from_csv_instance;


struct rel_to_csv_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.to_csv"; }
  virtual const char *description() const {
    return "`rel.to_csv()` translates the input stream from native record format to comma-separated-value format.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_CSV; }

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::csv_translate translator;
    translator.to_csv(mandatory_delimited_input, mandatory_output, tokens);
  }
} rel_to_csv_instance;


struct rel_from_usv_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.from_usv"; }
  virtual const char *description() const {
    return "`rel.from_usv()` translates the input stream from \"unit-separated-value\" format to native record format.  "
            "Fields are separated by the US character (ASCII 31).  Records are separated by the NUL character (ASCII 0).  "
            "This format is designed to be faster to parse than TSV because the NUL character can't appear in UTF-8 strings and the US character is extremely unlikely to be used anywhere.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_USV; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }


  virtual void call(io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    buffered_output_type buffered_output(output);
    mandatory_buffered_output_type mandatory_output(buffered_output);
    
    rel::usv_translate translator;
    translator.from_usv(input, mandatory_output, tokens);
  }
} rel_from_usv_instance;


struct rel_to_usv_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.to_usv"; }
  virtual const char *description() const {
    return "`rel.to_usv()` translates the input stream from native record format to unit-separated-value format.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_USV; }

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::usv_translate translator;
    translator.to_usv(mandatory_delimited_input, mandatory_output, tokens);
  }
} rel_to_usv_instance;


struct rel_from_text_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.from_text"; }
  virtual const char *description() const {
    return "`rel.from_text(regular_expression, heading1, ...headingN)` translates the input stream from newline-separated 'rows' to native record format.  "
            "Lines are divided into records using captures from the regular expression.  If the input does not end with a newline then the last line will be ignored.  "
            "Non-matching lines will generate an error.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_TEXT_UTF8; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }


  virtual void call(io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    buffered_output_type buffered_output(output);
    mandatory_buffered_output_type mandatory_output(buffered_output);
    
    rel::from_text op;
    op(input, mandatory_output, tokens, rel::from_text::ERROR_ON_NON_MATCHING);
  }
} rel_from_text_instance;



struct rel_from_text_ignore_non_matching_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.from_text_ignore_non_matching"; }
  virtual const char *since() const { return "1.2.0"; }
  virtual const char *description() const {
    return "`rel.from_text_ignore_non_matching(regular_expression, heading1, ...headingN)` is equivalent to `rel.from_text` except that non-matching lines are ignored rather than generating an error.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_TEXT_UTF8; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }


  virtual void call(io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    buffered_output_type buffered_output(output);
    mandatory_buffered_output_type mandatory_output(buffered_output);
    
    rel::from_text op;
    op(input, mandatory_output, tokens, rel::from_text::IGNORE_NON_MATCHING);
  }
} rel_from_text_ignore_non_matching_wrap_instance;




struct rel_generate_sequence_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "rel.generate_sequence"; }
  virtual const char *description() const {
    return "`rel.generate_sequence(start, end)` generates a sequence of integers starting at `start` and ending at `end-1` under the heading " NP1_REL_GENERATE_SEQUENCE_OUTPUT_HEADING_NAME ".  The input stream is ignored.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_NONE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rel::generate_sequence op;
    op(mandatory_delimited_input, mandatory_output, tokens);
  }
} rel_generate_sequence_instance;



struct text_utf16_to_utf8_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "text.utf16_to_utf8"; }
  virtual const char *since() const { return "1.4.0"; }
  virtual const char *description() const {
    return "`text.utf16_to_utf8()` translates the UTF-16 input stream into UTF-8 so that it's suitable for reading by "
            "other r17 stream operators.  The UTF-16 stream must start with a Byte Order Mark: 0xfffe for little-endian, "
            "0xfeff for big-endian.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_TEXT_UTF16; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_TEXT_UTF8; }


  virtual void call(io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    buffered_output_type buffered_output(output);
    mandatory_buffered_output_type mandatory_output(buffered_output);
    
    text::utf16_to_utf8 op;
    op(input, mandatory_output, tokens);
  }
} text_utf16_to_utf8_instance;


struct text_strip_cr_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "text.strip_cr"; }
  virtual const char *since() const { return "1.4.0"; }
  virtual const char *description() const {
    return "`text.strip_cr()` copies the input stream to the output stream, omitting CR (ASCII 13) characters.  It's useful for transforming text from Windows line-endings to the Unix line-endings expected by other r17 text operators.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_TEXT_UTF8; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_TEXT_UTF8; }


  virtual void call(io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    buffered_output_type buffered_output(output);
    mandatory_buffered_output_type mandatory_output(buffered_output);
    
    text::strip_cr op;
    op(input, mandatory_output, tokens);
  }
} text_strip_cr_instance;



struct io_file_read_wrap : public stream_op_wrap_base {
  enum { MAX_HEADERS_SNIFF_LENGTH = 10 * 1024 }; // 10K.

  virtual const char *name() const { return "io.file.read"; }
  virtual const char *description() const {
    return "`io.file.read(file_name)` reads `file_name` and writes it to stdout.  "
            "If the file is gzipped, io.file.read will ungzip it before writing to stdout.  "
            "In 1.4.0 and later, `io.file.read` accepts multiple `file_name` arguments.  "
            "Each file is read in the same order as it appears in the argument list.  "
            "`io.file.read` sniffs the contents of the files.  "
            "If the first file is an r17 native file then all files must be r17 native files with the same headers, and `io.file.read` will omit all headers from the second and subsequent files."
            "To read a file once per input record, use the `io.file.read` function.  ";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_NONE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_ANY; }

  virtual void call(io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    io::mandatory_output_stream<io::unbuffered_stream_base> mandatory_output(output);
    rstd::vector<rstd::string> file_names(rel::rlang::compiler::eval_to_strings_only(tokens));
    NP1_ASSERT(file_names.size() > 0, "io.file.read expects at least one file name argument.");
    if (is_r17_native_file(file_names[0])) {
      NP1_ASSERT(are_all_r17_native_files(file_names),
                  "If one file argument to io.file.read is an r17 native file, all files must be r17 native files.");
      NP1_ASSERT(are_all_headings_equal(file_names),
                  "If one file argument to io.file.read is an r17 native file, all files must be r17 native files with the same set of headings.");

      copy_native_files(file_names, mandatory_output);            
    } else {
      copy_non_native_files(file_names, mandatory_output);
    }
  }

  static void copy_native_files(const rstd::vector<rstd::string> &file_names,
                                io::mandatory_output_stream<io::unbuffered_stream_base> &mandatory_output) {
    bool written_first_file = false;
    rstd::vector<rstd::string>::const_iterator i = file_names.begin();
    rstd::vector<rstd::string>::const_iterator iz = file_names.end();
    for (; i != iz; ++i) {
      rstd::string file_name = *i;
      if (io::gzfile::is_gzfile(file_name.c_str())) {
        io::gzfile file;
        NP1_ASSERT(file.open_ro(file_name.c_str()), "Unable to open & map compressed input file " + file_name);
        io::mandatory_input_stream<io::gzfile> mandatory_input(file);
        io::mandatory_record_input_stream<io::gzfile, rel::record, rel::record_ref> record_input(file);
        record_input.parse_records(record_callback(mandatory_output, written_first_file));
      } else {
        io::file file;
        NP1_ASSERT(file.open_ro(file_name.c_str()), "Unable to open & map input file " + file_name);
        io::mandatory_input_stream<io::file> mandatory_input(file);
        io::mandatory_record_input_stream<io::file, rel::record, rel::record_ref> record_input(file);
        record_input.parse_records(record_callback(mandatory_output, written_first_file));
      }
      
      written_first_file = true;
    }
  }

  struct record_callback {
    record_callback(io::mandatory_output_stream<io::unbuffered_stream_base> &output, bool written_first_file) 
      : m_output(output), m_written_first_file(written_first_file), m_seen_first_record(false) {}
      
    bool operator()(const rel::record_ref &r) {
      if (!m_written_first_file || m_seen_first_record) {
        r.write(m_output);
      }

      m_seen_first_record = true;      
      return true;
    }
    
    io::mandatory_output_stream<io::unbuffered_stream_base> &m_output;
    bool m_written_first_file;
    bool m_seen_first_record;
  };


  static void copy_non_native_files(const rstd::vector<rstd::string> &file_names,
                                    io::mandatory_output_stream<io::unbuffered_stream_base> &mandatory_output) {      
    rstd::vector<rstd::string>::const_iterator i = file_names.begin();
    rstd::vector<rstd::string>::const_iterator iz = file_names.end();
    for (; i != iz; ++i) {
      rstd::string file_name = *i;
      if (io::gzfile::is_gzfile(file_name.c_str())) {
        io::gzfile file;
        NP1_ASSERT(file.open_ro(file_name.c_str()), "Unable to open & map compressed input file " + file_name);
        io::mandatory_input_stream<io::gzfile> mandatory_input(file);
        mandatory_input.copy(mandatory_output);
      } else {
        io::file file;
        NP1_ASSERT(file.open_ro(file_name.c_str()), "Unable to open & map input file " + file_name);
        io::mandatory_input_stream<io::file> mandatory_input(file);
        mandatory_input.copy(mandatory_output);
      }          
    }
  }

  static bool is_r17_native_file(const rstd::string &file_name) {
    rstd::vector<unsigned char> buffer;
    buffer.resize(MAX_HEADERS_SNIFF_LENGTH);
    io::gzfile file;
    open_for_sniffing(file, file_name);
    io::mandatory_input_stream<io::gzfile> sniff_input(file);
    size_t bytes_read = sniff_input.read(&buffer[0], buffer.size());
    uint64_t number_fields;
    return rel::record_ref::contains_record(&buffer[0], bytes_read, number_fields);
  }


  static bool are_all_r17_native_files(const rstd::vector<rstd::string> &file_names) {
    rstd::vector<rstd::string>::const_iterator i = file_names.begin();
    rstd::vector<rstd::string>::const_iterator iz = file_names.end();
    for (; i != iz; ++i) {
      if (!is_r17_native_file(*i)) {
        return false;
      }
    }

    return true;
  }

  static rel::record parse_headings(const rstd::string &file_name) {
    io::gzfile file;
    open_for_sniffing(file, file_name);
    io::mandatory_record_input_stream<io::gzfile, rel::record, rel::record_ref> record_input(file);
    return record_input.parse_headings();    
  }

  static bool are_all_headings_equal(const rstd::vector<rstd::string> &file_names) {
    rstd::vector<rstd::string>::const_iterator i = file_names.begin();
    rstd::vector<rstd::string>::const_iterator iz = file_names.end();
    rel::record first_file_headings;
    bool has_first_file_headings = false;

    for (; i != iz; ++i) {
      rel::record headings = parse_headings(*i);
      if (has_first_file_headings) {
        if (!headings.ref().is_equal(first_file_headings.ref())) {
          return false;
        }
      } else {
        has_first_file_headings = true;
        first_file_headings = headings;
      }
    }
  
    return true;
  }

  static void open_for_sniffing(io::gzfile &file, const rstd::string &file_name) {
    NP1_ASSERT(file.open_ro(file_name.c_str(), MAX_HEADERS_SNIFF_LENGTH), "Unable to open input file " + file_name);
  }
} io_file_read_instance;



struct io_file_append_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "io.file.append"; }
  virtual const char *description() const {
    return "`io.file.append(file_name)` reads input and appends it all to `file_name`.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_ANY; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_NONE; }

  virtual void call(io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    io::mandatory_input_stream<io::unbuffered_stream_base> mandatory_input(input);
    rstd::string file_name(rel::rlang::compiler::eval_to_string_only(tokens));
    mandatory_input.copy_append(file_name);
  }
} io_file_append_instance;


struct io_file_overwrite_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "io.file.overwrite"; }
  virtual const char *description() const {
    return "`io.file.overwrite(file_name)` reads input and writes it all to `file_name`, overwriting the file.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_ANY; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_NONE; }

  virtual void call(io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    io::mandatory_input_stream<io::unbuffered_stream_base> mandatory_input(input);
    rstd::string file_name(rel::rlang::compiler::eval_to_string_only(tokens));
    mandatory_input.copy_overwrite(file_name);
  }
} io_file_overwrite_instance;



struct io_directory_list_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "io.directory.list"; }
  virtual const char *description() const {
    return "`io.directory.list(directory_name)` lists all files and subdirectories in the supplied `directory_name`.  "
            "The output stream contains these headings: `string:directory_name`, `string:file_name`, `string:relative_path`, `uint:size_bytes`, `uint:mtime_usec`, `bool:is_directory`.  "
            "The input stream is ignored.";
  };

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_NONE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }


  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rstd::string directory_name(rel::rlang::compiler::eval_to_string_only(tokens));
    write_headings(mandatory_output);
    io::directory::mandatory_iterate(directory_name, entry_iterator(mandatory_output));
  }  

  static void write_headings(mandatory_buffered_output_type &mandatory_output) {
    rel::record_ref::write(mandatory_output, "string:directory_name", "string:file_name", "string:relative_path",
                          "uint:size_bytes", "uint:mtime_usec", "bool:is_directory");
  }

  static void write_entry(mandatory_buffered_output_type &mandatory_output, const io::directory::entry &e) {
    char size_bytes_str[str::MAX_NUM_STR_LENGTH + 1];
    char mtime_usec_str[str::MAX_NUM_STR_LENGTH + 1];
    str::to_dec_str(size_bytes_str, e.size_bytes());
    str::to_dec_str(mtime_usec_str, e.mtime_usec());
    rel::record_ref::write(mandatory_output, e.directory_name(), e.file_name(), e.relative_path(), size_bytes_str,
                            mtime_usec_str, str::from_bool(e.is_directory()));
  }

  struct entry_iterator {
    explicit entry_iterator(mandatory_buffered_output_type &mandatory_output) : m_mandatory_output(mandatory_output) {}
    void operator()(const io::directory::entry &e) {
      write_entry(m_mandatory_output, e);
    }

    mandatory_buffered_output_type &m_mandatory_output;
  };
} io_directory_list_instance;


struct io_ls_wrap : public io_directory_list_wrap {
  virtual const char *name() const { return "io.ls"; }
  virtual const char *description() const {
    return "Synonym for `io.directory.list`.";
  };

  virtual const char *since() const { return "1.4.2"; }
} io_ls_instance;



struct io_directory_list_recurse_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "io.directory.list_recurse"; }
  virtual const char *description() const {
    return "`io.directory.list_recurse(directory_name)` lists all files and subdirectories as does `io.directory.list`, recursing into subdirectories.";
  };

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_NONE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rstd::string directory_name(rel::rlang::compiler::eval_to_string_only(tokens));
    io_directory_list_wrap::write_headings(mandatory_output);
    io::directory::mandatory_iterate(directory_name, entry_iterator(mandatory_output));
  }  


  struct entry_iterator {
    explicit entry_iterator(mandatory_buffered_output_type &mandatory_output) : m_mandatory_output(mandatory_output) {}
    void operator()(const io::directory::entry &e) {
      io_directory_list_wrap::write_entry(m_mandatory_output, e);
      if (e.is_directory()) {
        io::directory::mandatory_iterate(e.relative_path(), entry_iterator(m_mandatory_output));
      }
    }

    mandatory_buffered_output_type &m_mandatory_output;
  };
} io_directory_list_recurse_instance;


struct io_ls_r_wrap : public io_directory_list_recurse_wrap {
  virtual const char *name() const { return "io.ls_r"; }
  virtual const char *description() const {
    return "Synonym for `io.directory.list_recurse`.";
  };

  virtual const char *since() const { return "1.4.2"; }
} io_ls_r_instance;



struct help_markdown_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "help.markdown"; }
  virtual const char *description() const { return "`help.markdown()` writes out help in Markdown format."; }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_NONE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_TEXT_UTF8; }

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    help::markdown::all(mandatory_output);    
  }  
} help_markdown_instance;


struct help_version_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "help.version"; }
  virtual const char *description() const { return "`help.version()` writes out the current r17 version number."; }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_NONE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_TEXT_UTF8; }

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    mandatory_output.write(PACKAGE_VERSION "\n");    
  }  
} help_version_instance;


struct lang_python_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "lang.python"; }
  virtual const char *since() const { return "1.7.0"; }
  virtual const char *description() const {
    static const rstd::string desc =
      "`lang.python(" NP1_TOKEN_UNPARSED_CODE_BLOCK_DELIMITER " python " NP1_TOKEN_UNPARSED_CODE_BLOCK_DELIMITER ")` "
      "executes Python code using the python interpreter in the shell's path.  "
      "The Python script's standard input is the input stream.  "
      "The Python script's standard output is the output stream.  "
      "R17 prepends helper Python code to the Python script before passing it to the system's Python interpreter.  "
      "R17's helper code supplies 2 stream-like global variables: `r17InputStream` and `r17OutputStream`.  "
      "Each call to `r17InputStream.next()` returns an object with member variables of the same type and name as "
      "the r17 input columns.  `r17OutputStream.write(v)` writes an r17 record row with column names and types "
      "inferred from `v` where `v` is an object or dictionary.  \n"
      "The simplest possible example is copying the input stream to the output stream:  \n\n"
      "    lang.python(@@@  \n"
      "    for inputR in r17InputStream:  \n"
      "        r17OutputStream.write(inputR)  \n"
      "    @@@);  \n  \n"
      "Note that r17 does not add or remove whitespace to the Python script because indentation is so important "
      "in Python.  So you need to indent inline Python code as if the Python code was in its own file.  \n\n"
      "This example assumes that the input stream contains a `value` column that is some kind of number:  \n\n"
      "    lang.python(@@@  \n"
      "    for inputR in r17InputStream:  \n"
      "        r17OutputStream.write({value: inputR.value + 1})  \n"
      "    @@@);  \n  \n"
      "Below is almost all the Python code that r17 prepends to the inline script before passing to Python.  \n\n"
      + lang::python::python_helper_code_markdown() +
      "Underneath this code, r17 creates an R17InputRecord class that's tailored to the input stream columns.  "
      "For example, the R17InputRecord class for an r17 input stream with columns...  \n"
      "`string:v1 istring:v2 int:v3 uint:v4 double:v5 bool:v6 ipaddress:v7`  \n"
      "...is  \n\n"
      "    class R17InputRecord:  \n"
      "        def __init__(self, row):  \n"
      "        self.v1 = str(row[0])      # r17 istring -> Python string  \n"
      "        self.v2 = str(row[1])      # r17 string -> Python string  \n"
      "        self.v3 = long(row[2])     # r17 int -> Python long  \n"
      "        self.v4 = long(row[3])     # r17 uint -> Python long  \n"
      "        self.v5 = float(row[4])    # r17 double -> Python float  \n"
      "        self.v6 = row[5] == 'true' # r17 bool -> Python boolean  \n"
      "        self.v7 = str(row[6])      # r17 ipaddress -> Python string  \n  \n  \n";

    return desc.c_str();
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }

  virtual void call(io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    lang::python::run(input, output, tokens);
  }
} lang_python_instance;


struct lang_r_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "lang.R"; }
  virtual const char *since() const { return "1.7.1"; }
  virtual const char *description() const {
    static const rstd::string desc =
      "`lang.R(" NP1_TOKEN_UNPARSED_CODE_BLOCK_DELIMITER " R code " NP1_TOKEN_UNPARSED_CODE_BLOCK_DELIMITER ")` "
      "executes R code using the Rscript interpreter in the shell's path.  "
      "The R script's standard input is the input stream.  "
      "The R script's standard output is the output stream.  "
      "R17 prepends helper R code to the R script before passing it to the system's R interpreter.  "
      "R17's helper code supplies an `r17InputTable` table and an `r17WriteTable` function.  "
      "`r17InputTable` contains the entire stream contents.  The table's column names match the r17 stream headings.  "
      "The definition of `r17WriteTable` is:  \n\n"
      + lang::r::r_helper_code_markdown() +
      "  \n"
      "R17 can't infer the output table column types so you need to include the r17 types in the column names, for example: \n"
      "`r17WriteTable(c(\"string:name\", \"int:value\"), table)`  \n";

    return desc.c_str();
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }

  virtual void call(io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    lang::r::run(input, output, tokens);
  }
} lang_r_instance;



#define NP1_META_SCRIPT_NAME "meta.script"

struct meta_script_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return NP1_META_SCRIPT_NAME; }
  virtual const char *description() const {
    return "`meta.script(file_name)` executes an r17 script.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_NONE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_ANY; }

  virtual void call(io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    rstd::string file_name(rel::rlang::compiler::eval_to_string_only(tokens));
    script_run(input, output, file_name);    
  }  
} meta_script_instance;



struct meta_remote_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "meta.remote"; }
  virtual const char *description() const {
    return "`meta.remote(host_name, inline_script)` executes an r17 script on a remote machine.  "
            "The current user must be able to SSH to the remote machine without a password.  "
            "r17 must be in the PATH on the remote machine.  "
            "If the `host_name` argument is 'localhost' then the script will be executed on the local machine without SSH.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_ANY; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_ANY; }

  //NOTE that currently the only input and output supported are stdin and stdout.
  virtual void call(io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    remote::run(input, output, tokens);
  }  
} meta_remote_instance;



struct meta_shell_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "meta.shell"; }
  virtual const char *description() const {
    return "`meta.shell(command)` executes a shell command on the local machine once.  "
            "The input stream becomes the shell command's standard input.  "
            "The output of the shell command is written to the output stream.  "
            "To execute a shell command once per input record, use the `meta.shell` function.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_ANY; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_ANY; }

  virtual void call(io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    shell::run(input, output, tokens);
  }
} meta_shell_instance;



struct meta_parallel_explicit_mapping_wrap : public stream_op_wrap_base {
  virtual const char *name() const { return "meta.parallel_explicit_mapping"; }
  virtual const char *description() const {
    return "`meta.parallel_explicit_mapping(inline_script)` executes an r17 script on remote & local machine(s).  "
            "It reads the [`" NP1_META_PARALLEL_EXPLICIT_MAPPING_HEADING_FILE_NAME "`, `" NP1_META_PARALLEL_EXPLICIT_MAPPING_HEADING_HOST_NAME "`] mapping from the input.  "
            "The current user must be able to SSH to the remote machine(s) without a password.  "
            "r17 must be in the PATH on all remote machine(s).  "
            "The output of inline_script must be a normal record stream.";
  }

  virtual stream_op_table_io_type_type input_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }
  virtual stream_op_table_io_type_type output_type() const { return STREAM_OP_TABLE_IO_TYPE_R17_NATIVE; }

  virtual void call(mandatory_delimited_input_type &mandatory_delimited_input,
                    mandatory_buffered_output_type &mandatory_output,
                    const rstd::vector<rel::rlang::token> &tokens) const {
    parallel_explicit_mapping<mandatory_delimited_input_type, mandatory_buffered_output_type>::run(
      mandatory_delimited_input, mandatory_output, tokens);
  }
} meta_parallel_explicit_mapping_instance;



class stream_op_table {
public:
  static size_t size() {
    size_t sz;
    stream_op_definitions(sz);
    return sz;
  }

  static const char *name(size_t n) {
    return at(n)->name();    
  }

  static const char *since(size_t n) {
    return at(n)->since();    
  }

  static const char *description(size_t n) {
    return at(n)->description();    
  }

  static stream_op_table_io_type_type input_type(size_t n) {
    return at(n)->input_type();  
  }

  static stream_op_table_io_type_type output_type(size_t n) {
    return at(n)->output_type();  
  }


  static void call(size_t n, io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rstd::string> &args,
                    const rstd::string &script_file_name,
                    size_t script_line_number) {
    global_info::stream_op_details(at(n)->name(), script_file_name.c_str(), script_line_number);
    rstd::vector<rstd::string> args_without_program_name = args;
    args_without_program_name.pop_front();
    rstd::string useful_args = str::implode(args_without_program_name, " ");
    io::string_input_stream args_stream(useful_args);
    rstd::vector<rel::rlang::token> tokens;
    np1::rel::rlang::compiler::compile_single_expression_to_prefix(args_stream, tokens);
    call(n, input, output, tokens, script_file_name, script_line_number);
    global_info::stream_op_details_reset();
  }

  static void call(size_t n, io::unbuffered_stream_base &input,
                    io::unbuffered_stream_base &output,
                    const rstd::vector<rel::rlang::token> &tokens,
                    const rstd::string &script_file_name,
                    size_t script_line_number) {
    const stream_op_wrap_base *op = at(n);
    global_info::stream_op_details(op->name(), script_file_name.c_str(), script_line_number);

    op->call(input, output, tokens);
    global_info::stream_op_details_reset();
  }  

  static size_t find(const char *needle) {
    size_t i;
    for (i = 0; i < size(); ++i) {
      if (str::cmp(name(i), needle) == 0) {
        return i;    
      }
    }
    
    return (size_t)-1;
  }

private:
  static const stream_op_wrap_base *at(size_t n) {
    size_t sz;
    const stream_op_wrap_base **definitions = stream_op_definitions(sz);
    NP1_ASSERT(n < sz, "Offset out-of-bounds on stream op definitions");
    return definitions[n];  
  }

  static const stream_op_wrap_base **stream_op_definitions(size_t &number_definitions) {
    static const stream_op_wrap_base *definitions[] = {
      &rel_group_instance,
      &rel_join_natural_instance,
      &rel_join_left_instance,
      &rel_join_anti_instance,
      &rel_join_consistent_hash_instance,
      &rel_order_by_instance,
      &rel_order_by_desc_instance,
      &rel_order_by_mergesort_instance,
      &rel_order_by_mergesort_desc_instance,
      &rel_order_by_quicksort_instance,
      &rel_order_by_quicksort_desc_instance,
      &rel_select_instance,
      &rel_record_count_instance,
      &rel_record_split_instance,
      &rel_where_instance,
      &rel_unique_instance,
      &rel_str_split,
      &rel_assert_empty_instance,
      &rel_assert_nonempty_instance,
      &rel_from_tsv_instance,
      &rel_to_tsv_instance,
      &rel_from_csv_instance,
      &rel_to_csv_instance,
      &rel_from_usv_instance,
      &rel_to_usv_instance,
      &rel_from_text_instance,
      &rel_from_text_ignore_non_matching_wrap_instance,
      &rel_generate_sequence_instance,
      &text_utf16_to_utf8_instance,
      &text_strip_cr_instance,
      &io_file_read_instance,
      &io_file_append_instance,
      &io_file_overwrite_instance,
      &io_directory_list_instance,
      &io_ls_instance,
      &io_directory_list_recurse_instance,
      &io_ls_r_instance,
      &lang_python_instance,
      &lang_r_instance,
      &meta_script_instance,
      &meta_remote_instance,
      &meta_shell_instance,
      &meta_parallel_explicit_mapping_instance,
      &help_markdown_instance,
      &help_version_instance    
    };

    number_definitions = sizeof(definitions)/sizeof(definitions[0]);
    return definitions;
  }
};


/// Helper functions to avoid circular #includes.
size_t stream_op_table_size() { return stream_op_table::size(); }
const char *stream_op_table_name(size_t n) { return stream_op_table::name(n); }
const char *stream_op_table_since(size_t n) { return stream_op_table::since(n); }
const char *stream_op_table_description(size_t n) { return stream_op_table::description(n); }
stream_op_table_io_type_type stream_op_table_input_type(size_t n) { return stream_op_table::input_type(n); }
stream_op_table_io_type_type stream_op_table_output_type(size_t n) { return stream_op_table::output_type(n); }

size_t stream_op_table_find(const char *name) { return stream_op_table::find(name); }


} /// namespaces
}



#endif
