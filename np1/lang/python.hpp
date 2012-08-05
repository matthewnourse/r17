// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_LANG_PYTHON_HPP
#define NP1_LANG_PYTHON_HPP

#include "np1/io/named_temp_file.hpp"
#include "np1/lang/detail/helper.hpp"

namespace np1 {
namespace lang {

// Execute an inline Python script.
class python {
public:
  static void run(io::unbuffered_stream_base &input, io::unbuffered_stream_base &output,
                  const rstd::vector<rel::rlang::token> &tokens) {
    NP1_ASSERT((tokens.size() == 1) && (tokens[0].type() == rel::rlang::token::TYPE_UNPARSED_CODE_BLOCK),
               "lang.python's argument is a Python code block enclosed in "
                  NP1_TOKEN_UNPARSED_CODE_BLOCK_DELIMITER "s.");

    rstd::string user_python_code = tokens[0].text();

    // Read the headers so we can generate our supporting code.
    io::mandatory_record_input_stream<io::unbuffered_stream_base, rel::record, rel::record_ref>
        record_input_stream(input);

    rel::record headers = record_input_stream.parse_headings();

    rstd::string combined_python_code =
      python_helper_code() + python_input_record_code(headers.ref()) + user_python_code;

    io::named_temp_file temp_file;    
  
    // Write out the python code to the temp file.
    io::mandatory_output_stream<io::file> temp_stream(temp_file.real_file());
    temp_stream.write(combined_python_code.c_str());
    temp_stream.close();

    // Run the python code and the stream operators that will translate to and from TSV.
    // It's weird that we can just pass through the input stream without headers intact but rel.to_tsv doesn't care
    // if there are headers or not.
    rstd::string r17_script =
      "rel.to_tsv() | meta.shell('python " + rstd::string(temp_file.file_name()) + "') | rel.from_tsv();";

    meta::script_run(input, output, r17_script, false);
  }

  static rstd::string python_helper_code_markdown() {
    return detail::helper::code_to_markdown(python_helper_code());
  }

private:
  static const char *python_helper_code() {
    return 
      "import sys\n"
      "import csv\n"
      "import types\n"
      "\n"
      "class R17StreamDefinition:\n"
      "    @staticmethod\n"
      "    def delimiter():\n"
      "        return '\\t'\n"
      "\n" 
      "    @staticmethod\n"
      "    def escapeChar():\n"
      "        return '\\\\'\n"
      "\n"
      "    @staticmethod\n"
      "    def lineTerminator():\n"
      "        return '\\n'\n"
      "\n"
      "\n"
      "class R17InputStream:\n"
      "    def __init__(self):\n"
      "        self.inCsvReader = csv.reader(sys.stdin, delimiter=R17StreamDefinition.delimiter(), quotechar=None,\n"
      "                                       escapechar=R17StreamDefinition.escapeChar(),\n"
      "                                       lineterminator=R17StreamDefinition.lineTerminator())\n"
      "\n" 
      "    def __iter__(self):\n"
      "        return self\n"
      "\n" 
      "    def next(self):\n"
      "        return R17InputRecord(self.inCsvReader.next())\n"
      "\n" 
      "r17InputStream = R17InputStream()\n"
      "\n"
      "\n"
      "class R17OutputStream:\n"
      "    def __init__(self):\n"
      "        self.outCsvWriter = csv.writer(sys.stdout, delimiter=R17StreamDefinition.delimiter(), quotechar=None,\n"
      "                                       escapechar=R17StreamDefinition.escapeChar(),\n"
      "                                       lineterminator=R17StreamDefinition.lineTerminator())\n"
      "        self.headerWritten = False\n"
      "\n" 
      "    def write(self, outputDict):\n"
      "        if not type(outputDict) is types.DictionaryType:\n"
      "            outputDict = outputDict.__dict__\n"
      "\n" 
      "        if (not self.headerWritten):\n"
      "            self.headerWritten = True\n"
      "            headers = []\n"
      "            for name, value in outputDict.iteritems():\n"
      "                headers.append(self.typeAsString(value) + ':' + name)\n"
      "\n"
      "            self.outCsvWriter.writerow(headers)\n"
      "\n"
      "        row = []\n"
      "        for name, value in outputDict.iteritems():\n"
      "            # Python's str(boolean_value) returns True or False but r17 expects true or false.\n"
      "            if type(value) is types.BooleanType:\n"
      "                if value:\n"
      "                    row.append('true')\n"
      "                else:\n"
      "                    row.append('false')\n"
      "            else:\n"
      "                row.append(str(value))\n"
      "\n"
      "        self.outCsvWriter.writerow(row)\n"
      "\n"
      "    def typeAsString(self, value):\n"
      "        if type(value) is types.IntType:\n"
      "            return 'int'\n"
      "\n"
      "        if type(value) is types.LongType:\n"
      "            return 'int'\n"
      "\n"
      "        if type(value) is types.StringType:\n"
      "            return 'string'\n"
      "\n"
      "        if type(value) is types.FloatType:\n"
      "            return 'double'\n"
      "\n"
      "        if type(value) is types.BooleanType:\n"
      "            return 'bool'\n"
      "\n"
      "        raise Exception('r17: unsupported type: ' + str(type(value)))\n"
      "\n"
      "r17OutputStream = R17OutputStream()\n"
      "\n";
  }
      

  static rstd::string python_input_record_code(const rel::record_ref &headings) {
    rstd::string code =
      "class R17InputRecord:\n"
      "    def __init__(self, row):\n";

    size_t field_number;
    for (field_number = 0; field_number < headings.number_fields(); ++field_number) {
      str::ref heading = headings.mandatory_field(field_number);
      code.append("        self.");
      code.append(rel::detail::helper::get_heading_without_type_tag(heading).to_string());
      code.append(" = ");

      switch (rel::rlang::dt::mandatory_from_string(rel::detail::helper::mandatory_get_heading_type_tag(heading))) {
        case rel::rlang::dt::TYPE_STRING:
        case rel::rlang::dt::TYPE_ISTRING:
        case rel::rlang::dt::TYPE_IPADDRESS:
          code.append(make_python_row_item_cast_code("str", field_number));
          break;

        case rel::rlang::dt::TYPE_INT: 
        case rel::rlang::dt::TYPE_UINT:
          code.append(make_python_row_item_cast_code("long", field_number));
          break;

        case rel::rlang::dt::TYPE_DOUBLE:
          code.append(make_python_row_item_cast_code("float", field_number));
          break;

        case rel::rlang::dt::TYPE_BOOL: 
          code.append(make_python_row_access_code(field_number) + " == 'true'");
          break;
      }

      code.append("\n");
    }

    code.append("\n");
    return code;
  }

  static rstd::string make_python_row_item_cast_code(const rstd::string &python_type, size_t field_number) {
    return python_type + "(" + make_python_row_access_code(field_number) + ")";
  }

  static rstd::string make_python_row_access_code(size_t field_number) {
    return "row[" + str::to_dec_str(field_number) + "]";
  }
};

} /// namespaces
}



#endif
