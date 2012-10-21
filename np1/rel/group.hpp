// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_NP1_REL_GROUP_HPP
#define NP1_NP1_REL_GROUP_HPP


#include "np1/rel/detail/record_multihashmap.hpp"
#include "np1/rel/rlang/rlang.hpp"

namespace np1 {
namespace rel {


namespace detail {
  //TODO: should this be used outside of this class?
  template <size_t N>
  struct field_id_is_in {
    static bool f(const size_t *ids, size_t id) {
      return (*ids == id) ? true : field_id_is_in<N-1>::f(ids+1, id);
    }
  };

  template <> struct field_id_is_in<0> {
    static bool f(const size_t *ids, size_t id) { return false; }
  };
} // namespace detail

// Aggregation functions.
#define NP1_REL_GROUP_AGGREGATOR_COUNT "count"
#define NP1_REL_GROUP_AGGREGATOR_MIN "min"
#define NP1_REL_GROUP_AGGREGATOR_MAX "max"
#define NP1_REL_GROUP_AGGREGATOR_AVG "avg"
#define NP1_REL_GROUP_AGGREGATOR_SUM "sum"
#define NP1_REL_GROUP_AGGREGATOR_SUM_COUNT "sum_count"

// Output heading names.
#define NP1_REL_GROUP_OUTPUT_HEADING_COUNT "uint:_count"
#define NP1_REL_GROUP_OUTPUT_HEADING_AVG "_avg"
#define NP1_REL_GROUP_OUTPUT_HEADING_SUM "_sum"



class group {
public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(Input_Stream &input, Output_Stream &output,
                  const rstd::vector<rel::rlang::token> &tokens) {    
    // Get the argument(s).
    const char *aggregator; 
    const char *aggregator_heading_name;

    parse_arguments(tokens, &aggregator, &aggregator_heading_name);
    
    // Read the headings from input.
    record input_headings(input.parse_headings());

    // Figure out what the output headings are.
    str::ref aggregator_type_tag =
      aggregator_heading_name
        ? mandatory_get_aggregator_heading_type_tag(input_headings, aggregator_heading_name) : str::ref();
        
    record output_headings(
      get_output_headings(aggregator, aggregator_heading_name, aggregator_type_tag, input_headings));

    // Do the real work.    
    if (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_COUNT) == 0) {
      group_count(input_headings, output_headings, input, output);
    } else if (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_AVG) == 0) {
      group_avg(input_headings, output_headings, aggregator_heading_name, input, output);
    } else if (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_SUM) == 0) {
      group_sum(input_headings, output_headings, aggregator_heading_name, input, output);
    } else if (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_SUM_COUNT) == 0) {                   
      group_sum_count(input_headings, output_headings, aggregator_heading_name, input, output);
    } else if (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_MIN) == 0) {                   
      group_min(input_headings, output_headings, aggregator_heading_name, input, output);
    } else if (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_MAX) == 0) {                   
      group_max(input_headings, output_headings, aggregator_heading_name, input, output);
    } else {
      NP1_ASSERT(false, "Unknown aggregator: " + rstd::string(aggregator));
    }
  }


  // SELECT COUNT(1)...GROUP BY... 
  template <typename Input_Stream, typename Output_Stream>
  static void group_count(const record &input_headings, const record &output_headings,
                          Input_Stream &input, Output_Stream &output) {
    rstd::vector<rstd::string> input_heading_names = input_headings.fields();
    detail::compare_specs specs(input_headings, input_heading_names);
    validate_specs(specs);
    detail::record_multihashmap<uint64_t> group_map(specs);
    input.parse_records(count_record_callback(group_map));
    output_headings.write(output);
    group_map.for_each(
      output_count_aggregated_record_callback<Output_Stream>(output));    
  }


  // SELECT AVG(column_name)...GROUP BY... 
  template <typename Input_Stream, typename Output_Stream>
  static void group_avg(const record &input_headings, const record &output_headings,
                        const char *aggregator_heading_name, Input_Stream &input, Output_Stream &output) {
    str::ref aggregator_type_tag = mandatory_get_aggregator_heading_type_tag(input_headings, aggregator_heading_name);
    rlang::dt::data_type aggregator_type = rlang::dt::mandatory_from_string(aggregator_type_tag);
    if ((rlang::dt::TYPE_INT == aggregator_type) || (rlang::dt::TYPE_UINT == aggregator_type)) {
      avg_sum_helper<int64_t>(
        input_headings, output_headings, aggregator_heading_name, input, output, 
        output_sum_or_avg_aggregated_record_callback<Output_Stream,int64_t, 1>(
          output, field_id_list<1>(input_headings.mandatory_find_heading(aggregator_heading_name)),
          input_headings.number_fields(), false));
    } else {
      validate_type_is_double(aggregator_type, NP1_REL_GROUP_AGGREGATOR_AVG, aggregator_type_tag);

      avg_sum_helper<double>(
        input_headings, output_headings, aggregator_heading_name, input, output, 
        output_sum_or_avg_aggregated_record_callback<Output_Stream, double, 1>(
          output, field_id_list<1>(input_headings.mandatory_find_heading(aggregator_heading_name)),
          input_headings.number_fields(), false));      
    }
  }


  // SELECT SUM(column_name)...GROUP BY... 
  template <typename Input_Stream, typename Output_Stream>
  static void group_sum(const record &input_headings, const record &output_headings,
                        const char *aggregator_heading_name, Input_Stream &input, Output_Stream &output) {
    str::ref aggregator_type_tag = mandatory_get_aggregator_heading_type_tag(input_headings, aggregator_heading_name);
    rlang::dt::data_type aggregator_type = rlang::dt::mandatory_from_string(aggregator_type_tag);
    if ((rlang::dt::TYPE_INT == aggregator_type) || (rlang::dt::TYPE_UINT == aggregator_type)) {
      avg_sum_helper<int64_t>(
        input_headings, output_headings, aggregator_heading_name, input, output, 
        output_sum_or_avg_aggregated_record_callback<Output_Stream, int64_t, 1>(
          output, field_id_list<1>(input_headings.mandatory_find_heading(aggregator_heading_name)),
          input_headings.number_fields(), true));
    } else {
      validate_type_is_double(aggregator_type, NP1_REL_GROUP_AGGREGATOR_SUM, aggregator_type_tag);

      avg_sum_helper<double>(
        input_headings, output_headings, aggregator_heading_name, input, output, 
        output_sum_or_avg_aggregated_record_callback<Output_Stream, double, 1>(
          output, field_id_list<1>(input_headings.mandatory_find_heading(aggregator_heading_name)),
          input_headings.number_fields(), true));
    }
  }

  // sum_count (intermediate caluclation, useful for distributed AVG).
  template <typename Input_Stream, typename Output_Stream>
  static void group_sum_count(const record &input_headings, const record &output_headings,
                              const char *aggregator_heading_name, Input_Stream &input, Output_Stream &output) {
    str::ref aggregator_type_tag = mandatory_get_aggregator_heading_type_tag(input_headings, aggregator_heading_name);
    rlang::dt::data_type aggregator_type = rlang::dt::mandatory_from_string(aggregator_type_tag);
    if ((rlang::dt::TYPE_INT == aggregator_type) || (rlang::dt::TYPE_UINT == aggregator_type)) {
      avg_sum_helper<int64_t>(
        input_headings, output_headings, aggregator_heading_name, input, output, 
        output_sum_count_aggregated_record_callback<Output_Stream, int64_t, 1>(
          output,
          field_id_list<1>(input_headings.mandatory_find_heading(aggregator_heading_name)),
          input_headings.number_fields()));
    } else {
      validate_type_is_double(aggregator_type, NP1_REL_GROUP_AGGREGATOR_SUM_COUNT, aggregator_type_tag);

      avg_sum_helper<double>(
        input_headings, output_headings, aggregator_heading_name, input, output, 
        output_sum_count_aggregated_record_callback<Output_Stream, double, 1>(
          output,
          field_id_list<1>(input_headings.mandatory_find_heading(aggregator_heading_name)),
          input_headings.number_fields()));
    }
  }
  
  // Take the sum_count intermediate calculation and create an AVG.
  template <typename Input_Stream, typename Output_Stream>
  static void group_sum_count_to_avg(const record &input_headings, const record &output_headings, Input_Stream &input,
                                      Output_Stream &output) {
    rstd::vector<rstd::string> input_heading_names = input_headings.fields();
  
    input_heading_names.erase(
      input_heading_names.begin() + input_headings.mandatory_find_heading(NP1_REL_GROUP_OUTPUT_HEADING_SUM));
    
    input_heading_names.erase(
      input_heading_names.begin()
        + record(input_heading_names, 0).mandatory_find_heading(NP1_REL_GROUP_OUTPUT_HEADING_COUNT));

    detail::compare_specs specs(input_headings, input_heading_names);
    validate_specs(specs);

    size_t sum_field_id = input_headings.mandatory_find_heading(NP1_REL_GROUP_OUTPUT_HEADING_SUM);
    size_t count_field_id = input_headings.mandatory_find_heading(NP1_REL_GROUP_OUTPUT_HEADING_COUNT);
    size_t number_fields = input_headings.number_fields();
    field_id_list<2> field_list(field_id_list<2>(sum_field_id, count_field_id));
    rlang::dt::data_type sum_type =
      rlang::dt::mandatory_from_string(
        detail::helper::mandatory_get_heading_type_tag(input_headings.mandatory_field(sum_field_id)));

    if ((rlang::dt::TYPE_INT == sum_type) || (rlang::dt::TYPE_UINT == sum_type)) {
      detail::record_multihashmap<rstd::pair<int64_t, int64_t> > sum_map(specs);      
      input.parse_records(sum_sum_count_record_callback<int64_t>(sum_map, sum_field_id, count_field_id));
      output_headings.write(output);
      sum_map.for_each(output_sum_or_avg_aggregated_record_callback<Output_Stream, int64_t, 2>(
                        output, field_list, number_fields, false));
    } else {
      validate_type_is_double(sum_type, NP1_REL_GROUP_AGGREGATOR_SUM_COUNT, str::ref(rlang::dt::to_string(sum_type)));

      detail::record_multihashmap<rstd::pair<double, int64_t> > sum_map(specs);      
      input.parse_records(sum_sum_count_record_callback<double>(sum_map, sum_field_id, count_field_id));
      output_headings.write(output);
      sum_map.for_each(output_sum_or_avg_aggregated_record_callback<Output_Stream, double, 2>(
                        output, field_list, number_fields, false));
    }
  }


  
  // SELECT MIN(column_name)...GROUP BY...
  template <typename Input_Stream, typename Output_Stream>
  static void group_min(const record &input_headings, const record &output_headings,
                        const char *aggregator_heading_name, Input_Stream &input, Output_Stream &output) {
    str::ref aggregator_type_tag = mandatory_get_aggregator_heading_type_tag(input_headings, aggregator_heading_name);
    rlang::dt::data_type aggregator_type = rlang::dt::mandatory_from_string(aggregator_type_tag);
    if ((rlang::dt::TYPE_INT == aggregator_type) || (rlang::dt::TYPE_UINT == aggregator_type)) {
      min_max_helper_struct<int64_t, min_int64_record_callback> helper;
      helper(input_headings, output_headings, aggregator_heading_name, input, output);
    } else {
      validate_type_is_double(aggregator_type, NP1_REL_GROUP_AGGREGATOR_MIN, aggregator_type_tag);
      min_max_helper_struct<double, min_double_record_callback> helper;
      helper(input_headings, output_headings, aggregator_heading_name, input, output);
    }
  }


  // SELECT MAX(column_name)...GROUP BY...
  template <typename Input_Stream, typename Output_Stream>
  static void group_max(const record &input_headings, const record &output_headings,
                        const char *aggregator_heading_name, Input_Stream &input, Output_Stream &output) {
    str::ref aggregator_type_tag = mandatory_get_aggregator_heading_type_tag(input_headings, aggregator_heading_name);
    rlang::dt::data_type aggregator_type = rlang::dt::mandatory_from_string(aggregator_type_tag);
    if ((rlang::dt::TYPE_INT == aggregator_type) || (rlang::dt::TYPE_UINT == aggregator_type)) {
      min_max_helper_struct<int64_t, max_int64_record_callback> helper;
      helper(input_headings, output_headings, aggregator_heading_name, input, output);
    } else {
      validate_type_is_double(aggregator_type, NP1_REL_GROUP_AGGREGATOR_MAX, aggregator_type_tag);        
      min_max_helper_struct<double, max_double_record_callback> helper;
      helper(input_headings, output_headings, aggregator_heading_name, input, output);
    }
  }
  


  // Helper for AVG, SUM and SUM_COUNT
  template <typename Number_Type, typename Input_Stream, typename Output_Stream, typename Sum_Map_Callback>
  static void avg_sum_helper(const record &input_headings, const record &output_headings,
                              const char *aggregator_heading_name, Input_Stream &input, Output_Stream &output,
                              Sum_Map_Callback sum_map_callback) {
    rstd::vector<rstd::string> input_heading_names = input_headings.fields();
    size_t aggregator_heading_id = input_headings.mandatory_find_heading(aggregator_heading_name);
    input_heading_names.erase(input_heading_names.begin() + aggregator_heading_id);
    detail::compare_specs specs(input_headings, input_heading_names);
    validate_specs(specs);
    detail::record_multihashmap<rstd::pair<Number_Type, int64_t> > sum_map(specs);
    input.parse_records(sum_record_callback<Number_Type>(
                          sum_map, 
                          detail::compare_spec(input_headings, aggregator_heading_name)));

    output_headings.write(output);
    sum_map.for_each(sum_map_callback);
  }


  // Helper for MIN and MAX.
  template <typename Number_Type, typename Parse_Callback>
  struct min_max_helper_struct {
    template <typename Input_Stream, typename Output_Stream>
    void operator()(const record &input_headings, const record &output_headings,
                    const char *aggregator_heading_name, Input_Stream &input, Output_Stream &output) {
      rstd::vector<rstd::string> input_heading_names = input_headings.fields();
      size_t aggregator_heading_id = input_headings.mandatory_find_heading(aggregator_heading_name);
      input_heading_names.erase(input_heading_names.begin() + aggregator_heading_id);
      detail::compare_specs specs(input_headings, input_heading_names);
      validate_specs(specs);
      detail::record_multihashmap<Number_Type> group_map(specs);
      input.parse_records(Parse_Callback(
                            group_map, 
                            detail::compare_spec(input_headings, aggregator_heading_name)));
  
      output_headings.write(output);
      group_map.for_each(output_record_callback<Output_Stream>(output));
    }
  };  
    

  static void parse_arguments(const rstd::vector<rel::rlang::token> &tokens,
                              const char **aggregator_p,
                              const char **aggregator_heading_name_p) {
    NP1_ASSERT(tokens.size() > 0, "No aggregator supplied to rel.group");

    // Get the name of the aggregator function and the field if appropriate.
    const char *aggregator = tokens[0].text();
    const char *aggregator_heading_name = NULL;
    if (tokens.size() > 1) {
      aggregator_heading_name = tokens[1].text();
    }
    
    // Check that the arguments are valid.
    if ((str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_COUNT) != 0)
        && (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_AVG) != 0)
        && (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_MIN) != 0)
        && (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_MAX) != 0)
        && (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_SUM) != 0)
        && (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_SUM_COUNT) != 0)) {       
      NP1_ASSERT(false, "Unknown aggregator: " + rstd::string(aggregator));
    }
    
    if (((str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_AVG) == 0)
          || (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_MIN) == 0)
          || (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_MAX) == 0)
          || (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_SUM) == 0)
          || (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_SUM_COUNT) == 0))
        && !aggregator_heading_name) {
      NP1_ASSERT(false, "The aggregator '" + rstd::string(aggregator)
                          + "' requires a heading name");
    }

    *aggregator_p = aggregator;
    *aggregator_heading_name_p = aggregator_heading_name;
  }


  static record get_output_headings(const char *aggregator,
                                    const char *aggregator_heading_name,
                                    const str::ref &aggregator_type_tag,
                                    const record &input_headings) {
    rstd::vector<str::ref> header_fields;         

    // Get the headings.
    if ((str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_AVG) == 0)
        || (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_SUM) == 0)
        || (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_SUM_COUNT) == 0)) {
      get_fields_except(
        header_fields, input_headings, input_headings.number_fields(),
        field_id_list<1>(detail::compare_spec(input_headings, aggregator_heading_name).field_number()));
    } else {
      get_fields(header_fields, input_headings, input_headings.number_fields());
    }
    
    rstd::string aggregator_type_name = aggregator_type_tag.to_string();
    
    // This variable exists so that all str::refs in header_fields point to valid memory until the end
    // of the function.
    rstd::string typed_heading_name;
    
    if (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_COUNT) == 0) {
      header_fields.push_back(str::ref(NP1_REL_GROUP_OUTPUT_HEADING_COUNT));
    } else if (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_AVG) == 0) {
      typed_heading_name =
        detail::helper::make_typed_heading_name(aggregator_type_name, NP1_REL_GROUP_OUTPUT_HEADING_AVG);
      header_fields.push_back(str::ref(typed_heading_name));        
    } else if (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_SUM) == 0) {
      typed_heading_name =
        detail::helper::make_typed_heading_name(aggregator_type_name, NP1_REL_GROUP_OUTPUT_HEADING_SUM);
      header_fields.push_back(str::ref(typed_heading_name));      
    } else if (str::cmp(aggregator, NP1_REL_GROUP_AGGREGATOR_SUM_COUNT) == 0) {
      typed_heading_name = 
        detail::helper::make_typed_heading_name(aggregator_type_name, NP1_REL_GROUP_OUTPUT_HEADING_SUM);
      header_fields.push_back(str::ref(typed_heading_name));        
      header_fields.push_back(str::ref(NP1_REL_GROUP_OUTPUT_HEADING_COUNT));        
    }
      
    return record(header_fields, 0);
  }

  static str::ref mandatory_get_aggregator_heading_type_tag(const record &input_headings,
                                                            const char *aggregator_heading_name) {
    return detail::helper::mandatory_get_heading_type_tag(
            input_headings.mandatory_field(
              input_headings.mandatory_find_heading(aggregator_heading_name)));
  }


private:
  static void validate_specs(const detail::compare_specs &specs) {
    NP1_ASSERT(
      !specs.has_double(),
      "Group on floating-point columns is not supported because floating-point equality comparison is unreliable.");   
  }
  
  static void validate_type_is_double(rlang::dt::data_type type, const char *operator_name, const str::ref &type_tag) {
    NP1_ASSERT(
        (rlang::dt::TYPE_DOUBLE == type),
        rstd::string(operator_name) + " does not support type: " + type_tag.to_string());
  }
  
    
  static int64_t get_number(const str::ref &field, int64_t unused) {
    return str::dec_to_int64(field);
  }

  static double get_number(const str::ref &field, double unused) {
    return str::dec_to_double(field);
  }

  
  // The callback for the count aggregator.
  struct count_record_callback {
    count_record_callback(detail::record_multihashmap<uint64_t> &m) : m_map(m) {}  
    bool operator()(const record_ref &r) const {
      detail::record_multihashmap<uint64_t>::equal_list_type *eq_list;
      eq_list = m_map.find(r);
      if (eq_list) {
        eq_list->front().second++;
      } else {
        m_map.insert(r, (uint64_t)1);  
      }
      
      return true;
    }
    
    detail::record_multihashmap<uint64_t> &m_map;  
  };
  
  
  // The callback for the sum or avg aggregator.
  //TODO: this will only work with numbers that sum to less than max-int64/max-double.  
  // Surely this number is big enough?
  template <typename Number_Type>
  struct sum_record_callback {
    sum_record_callback(
      detail::record_multihashmap<rstd::pair<Number_Type, int64_t> > &m,
      const detail::compare_spec &spec)
    : m_map(m), m_spec(spec) {}
    bool operator()(const record_ref &r) const {
      // The first element in the pair is the sum, the second element is the
      // count of elements.
      typedef typename rstd::pair<Number_Type, int64_t> pair_type;
      typedef typename detail::record_multihashmap<pair_type>::equal_list_type eq_list_type;
      eq_list_type *eq_list;
        
      Number_Type num = field_to_number(r);
      eq_list = m_map.find(r);    
      if (eq_list) {
        eq_list->front().second.first += num;
        eq_list->front().second.second++;
      } else {
        m_map.insert(r, rstd::make_pair(num, (int64_t)1));
      }
      
      return true;
    }
      
    Number_Type field_to_number(const record_ref &r) const {
      Number_Type num = 0;
      num = get_number(r.mandatory_field(m_spec.field_number()), num);      
      return num;
    }        
    
    detail::record_multihashmap<rstd::pair<Number_Type, int64_t> > &m_map;
    detail::compare_spec m_spec;   
  };
  

  // Sum the sum & count intermediate results.
  template <typename Number_Type>
  struct sum_sum_count_record_callback {
    sum_sum_count_record_callback(
      detail::record_multihashmap<rstd::pair<Number_Type, int64_t> > &m,
      size_t sum_field_id,
      size_t count_field_id)
    : m_map(m), m_sum_field_id(sum_field_id), m_count_field_id(count_field_id) {}

    bool operator()(const record_ref &r) const {
      // The first element in the pair is the sum, the second element is the
      // count of elements.
      typedef typename rstd::pair<Number_Type, int64_t> pair_type;
      typedef typename detail::record_multihashmap<pair_type>::equal_list_type eq_list_type;
      eq_list_type *eq_list;
        
      Number_Type sum = 0;
      sum = get_number(r.mandatory_field(m_sum_field_id), sum);
      int64_t count = str::dec_to_int64(r.mandatory_field(m_count_field_id));
      eq_list = m_map.find(r);    
      if (eq_list) {
        eq_list->front().second.first += sum;
        eq_list->front().second.second += count;
      } else {
        m_map.insert(r, rstd::make_pair(sum, count));
      }
      
      return true;
    }
      
    detail::record_multihashmap<rstd::pair<Number_Type, int64_t> > &m_map;
    size_t m_sum_field_id;
    size_t m_count_field_id;
  };

  
  
  // The callback for an aggregator that selects just one element.
  template <typename Selector_Operator, typename Number_Type>
  struct selector_record_callback {
    selector_record_callback(detail::record_multihashmap<Number_Type> &m,
                  const detail::compare_spec &spec) 
    : m_map(m), m_spec(spec), m_selector_operator(Selector_Operator()) {}
      
    bool operator()(const record_ref &r) const {
      typedef typename detail::record_multihashmap<Number_Type>::equal_list_type eq_list_type;
      eq_list_type *eq_list;
      
      eq_list = m_map.find(r);
      if (eq_list) {
        const record &in_map_record = eq_list->front().first;
        
        str::ref in_map_field;
        str::ref field;
        
        in_map_field = in_map_record.mandatory_field(m_spec.field_number());
        field = r.mandatory_field(m_spec.field_number());
        
        if (m_selector_operator(m_spec.compare_function()(
                                field.ptr(), field.length(),
                                in_map_field.ptr(), in_map_field.length()))) {
          // Recall that the comparison spec for the map does not 
          // include the aggregation field.
          eq_list->front().first = record(r); 
        }      
      } else {
        m_map.insert(r, (Number_Type)0);  
      }
      
      return true;
    }
    
    detail::record_multihashmap<Number_Type> &m_map;
    detail::compare_spec m_spec;
    Selector_Operator m_selector_operator;
  };
  
  
  // Selector operators.
  struct min_operator { bool operator()(int i) const { return (i < 0); } };
  struct max_operator { bool operator()(int i) const { return (i > 0); } };
  
  // Selector callbacks.
  typedef selector_record_callback<min_operator, int64_t> min_int64_record_callback;
  typedef selector_record_callback<min_operator, double> min_double_record_callback;
  typedef selector_record_callback<max_operator, int64_t> max_int64_record_callback;
  typedef selector_record_callback<max_operator, double> max_double_record_callback;
  
  
  
  // The callback for writing out the records & aggregation for the count 
  // aggregator.
  template <typename Output_Stream>
  struct output_count_aggregated_record_callback {
    output_count_aggregated_record_callback(Output_Stream &output) : m_output(output) {}  
    bool operator()(const record &r, size_t val) const {
      char counter_string[32];
      str::to_dec_str(counter_string, val);
      record_ref::write(m_output, r.ref(), counter_string);
      return true;
    }
    
    Output_Stream &m_output;
  };
  
  // Get all the fields from the record.  number_fields is supplied for
  // performance reasons.
  static void get_fields(rstd::vector<str::ref> &fields, const record &r, size_t number_fields) {
    fields.resize(number_fields);
    rstd::vector<str::ref>::iterator fields_iter = fields.begin();

    size_t i;
    for (i = 0; i < number_fields; ++i, ++fields_iter) {
      *fields_iter = r.mandatory_field(i);
    }  
  }


  template <size_t N>
  class field_id_list {
  public:
    explicit field_id_list(size_t id1) {
      NP1_PREPROC_STATIC_ASSERT(N == 1);
      m_field_ids[0] = id1;
    }

    field_id_list(size_t id1, size_t id2) {
      NP1_PREPROC_STATIC_ASSERT(N == 2);
      m_field_ids[0] = id1;
      m_field_ids[1] = id2;
    }

    bool is_in(size_t id) const {
      return detail::field_id_is_in<N>::f(m_field_ids, id);
    }

    size_t size() const {
      return N;
    }

  private:
    size_t m_field_ids[N];
  };


  // Get all the fields from the record except the victim field.  number_fields
  // is supplied for performance reasons.
  template <size_t N>
  static void get_fields_except(rstd::vector<str::ref> &fields,
                                const record &r, size_t number_fields,
                                const field_id_list<N> &victim_field_numbers) {
    fields.resize(number_fields - victim_field_numbers.size());
    
    rstd::vector<str::ref>::iterator fields_iter = fields.begin();

    size_t i;
    for (i = 0; i < number_fields; ++i) {
      if (!victim_field_numbers.is_in(i)) {
        *fields_iter = r.mandatory_field(i);
        ++fields_iter;
      }
    }  
  }

  
  // The callback for writing out the records & aggregation for the 
  // sum or avg aggregator.
  template <typename Output_Stream, typename Number_Type, size_t N>
  struct output_sum_or_avg_aggregated_record_callback {
    output_sum_or_avg_aggregated_record_callback(
                        Output_Stream &output, 
                        const field_id_list<N> &field_ids_to_omit,
                        size_t number_fields,
                        bool is_sum) 
    : m_output(output), m_field_ids_to_omit(field_ids_to_omit), m_number_fields(number_fields), m_is_sum(is_sum) {}

    bool operator()(const record &r, 
                    const rstd::pair<Number_Type, int64_t> &sum_count_pair) {
      // Get all the fields except the one that was involved in the
      // averaging.
      get_fields_except(m_output_fields, r, m_number_fields, m_field_ids_to_omit);

      // Now get the average or sum.
      char num_string[32];
      if (m_is_sum) {      
        str::to_dec_str(num_string, sum_count_pair.first);
      } else {
        str::to_dec_str(num_string, sum_count_pair.first/sum_count_pair.second);
      }
      
      record_ref::write(m_output, m_output_fields, num_string);
      
      return true;
    }
            
    Output_Stream &m_output;
    field_id_list<N> m_field_ids_to_omit;
    rstd::vector<str::ref> m_output_fields;
    size_t m_number_fields;
    bool m_is_sum;
  };



  // The callback for writing out the records & aggregation for the sum_count aggregator.
  template <typename Output_Stream, typename Number_Type, size_t N>
  struct output_sum_count_aggregated_record_callback {
    output_sum_count_aggregated_record_callback(
                        Output_Stream &output, 
                        const field_id_list<N> &field_ids_to_omit,
                        size_t number_fields) 
    : m_output(output), m_field_ids_to_omit(field_ids_to_omit), m_number_fields(number_fields) {}

    bool operator()(const record &r, const rstd::pair<Number_Type, int64_t> &sum_count_pair) {
      // Get all the fields except the one that was involved in the
      // averaging.
      get_fields_except(m_output_fields, r, m_number_fields, m_field_ids_to_omit);

      // Now get the sum & count.
      char sum_string[32];
      char count_string[32];
      str::to_dec_str(sum_string, sum_count_pair.first);
      str::to_dec_str(count_string, sum_count_pair.second);
      
      record_ref::write(m_output, m_output_fields, sum_string, count_string);
      
      return true;
    }
    
    Output_Stream &m_output;
    field_id_list<N> m_field_ids_to_omit;
    rstd::vector<str::ref> m_output_fields;
    size_t m_number_fields;
  };
  
  
  // Just write out the records.
  template <typename Output_Stream>
  struct output_record_callback {
    output_record_callback(Output_Stream &output) : m_output(output) {}  
    bool operator()(const record &r, size_t val) const {
      r.write(m_output);    
      return true;
    }
    
    Output_Stream &m_output;
  };
};


} // namespaces
}


#endif
