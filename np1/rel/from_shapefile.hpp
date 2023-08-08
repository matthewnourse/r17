// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_FROM_SHAPEFILE_HPP
#define NP1_REL_FROM_SHAPEFILE_HPP

#include <arpa/inet.h>
#include "np1/assert.hpp"
#include "np1/io/shpfile.hpp"
#include "np1/rel/detail/helper.hpp"
#include "np1/rel/rlang/dt.hpp"
#include "np1/rel/rlang/rlang.hpp"
#include "np1/str.hpp"

#define NP1_REL_FROM_SHAPEFILE_GEOM_OUTPUT_HEADING_NAME "string:_geom"

namespace np1 {
namespace rel {
  

class from_shapefile {
public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(Input_Stream &input, Output_Stream &output, const rstd::vector<rlang::token> &args) {
    // Sort out the arguments.
    rstd::vector<rstd::string> compiled_args = rlang::compiler::eval_to_strings_only(args);
    //TODO: deal with the dbf that contains the field values too.
    NP1_ASSERT(compiled_args.size() == 1,
                "rel.from_shapefiles's arguments is the shapefile .shp name.");

    rstd::string shp_name = compiled_args[0];
    //TODO: get the dbf file name.
    io::shpfile shp(shp_name);

    // Write out the headings.
    record_ref::write(output, NP1_REL_FROM_SHAPEFILE_GEOM_OUTPUT_HEADING_NAME);

    // Write out the records.
    while (shp.has_next()) {
      rstd::vector<rstd::string> fields;
      fields.push_back("");
      rstd::string field;
      shp.next(fields[0]);
      record_ref::write(output, fields);
    }
  }
};


} // namespaces
}

#endif

