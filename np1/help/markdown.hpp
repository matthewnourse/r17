// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_HELP_MARKDOWN_HPP
#define NP1_HELP_MARKDOWN_HPP

#include "config.h"

namespace np1 {
namespace help {

/// The help content in markdown format.
class markdown {
public:
  template <typename Mandatory_Output_Stream>
  static void all(Mandatory_Output_Stream &output) {
    output.write(
      "### r17 v" PACKAGE_VERSION " ###\n\n"
      "R17 is a language for heterogeneous data querying and manipulation.  \n\n"
      "It's a syntactic cross between UNIX shell scripting and SQL.  "
      "Data flows along a pipeline of stream operators just like it does in UNIX pipelines.  "
      "Instead of UNIX's bias towards line-oriented records, most r17 operators work on a structured relational data stream optimized for parsing performance.  \n"
      "\n\n#### Examples ####\n\n"
      "The examples below assume this TAB-separated data is stored in test.tsv:\n\n"
      "`string:name\tint:birth_year`  \n"
      "`Johann Sebastian Bach\t1685`  \n"
      "`Clamor Heinrich Abel\t1634`  \n"
      "`Johann Georg Ahle\t1651`  \n"
      "`Johann Michael Bach\t1648`  \n\n"
      "**Example 1: sorting**\n\n"
      "`$ r17 'io.file.read(\"test.tsv\") | rel.from_tsv() | rel.order_by(birth_year) | rel.to_tsv();'`  \n\n"
      "`string:name\tint:birth_year`  \n"
      "`Clamor Heinrich Abel\t1634`  \n"
      "`Johann Michael Bach\t1648`  \n"
      "`Johann Georg Ahle\t1651`  \n"
      "`Johann Sebastian Bach\t1685`  \n\n"
      "**Example 2: projection, restriction, regular expression replacement**\n\n"
      "`$ r17 '`  \n`io.file.read(\"test.tsv\")`  \n`| rel.from_tsv()`  \n`| rel.select(str.regex_replace(\"([a-zA-Z]+)$\", name, \"\\1\") as last_name)`  \n`| rel.where(str.starts_with(last_name, \"A\"))`  \n`| rel.to_tsv();'`  \n\n"
      "`string:last_name`  \n"
      "`Abel`  \n"
      "`Ahle`  \n\n"
    );  //TODO: more here!

    usage(output);

    stream_operators(output);

    output.write("#### Expressions ####\n\n");
    output.write(
      "Expressions are simple infix C/Java/Python-like expressions that operate on one or at most two records.\n\n"
      "The only control expression is if/then/else, which works the same way as C/Java/PHP's ternary ?: operator, eg  \n"
      "`rel.select((if (str.starts_with(name, \"Johann\")) then (\"Another Johann\") else (\"No Johann\")) as johann_nature);`\n\n");

    data_types(output);
    builtin_variables(output);
    operators(output);
    functions(output);
    distribution(output);
    environment_variables(output);
    third_party_licenses(output); 
 }

  template <typename Mandatory_Output_Stream>
  static void usage(Mandatory_Output_Stream &output) {
    output.write(
      "#### Usage ####\n\n"
      "`r17 stream_op expression`  \n"
      "OR  \n"
      "`r17 script_file_name`  \n"
      "OR  \n"
      "`r17 'inline_script'`  \n"
      "where `stream_op` is one of:  \n");

    size_t num_ops = meta::stream_op_table_size();
    size_t i;
    for (i = 0; i < num_ops; ++i) {
      output.write("`");
      output.write(meta::stream_op_table_name(i));
      output.write("`  \n");
    }    
  }

  template <typename Mandatory_Output_Stream>
  static void stream_operators(Mandatory_Output_Stream &output) {
    output.write("#### Stream operators ####\n");
    size_t num_ops = meta::stream_op_table_size();
    size_t i;
    for (i = 0; i < num_ops; ++i) {
      output.write("**`");
      output.write(meta::stream_op_table_name(i));
      output.write("`**  \n");
      output.write("*input*: ");
      output.write(stream_op_table_io_type_to_text(meta::stream_op_table_input_type(i)));
      output.write("  \n*output*: ");
      output.write(stream_op_table_io_type_to_text(meta::stream_op_table_output_type(i)));
      output.write("  \n*since*: ");
      output.write(meta::stream_op_table_since(i));
      output.write("  \n");
      output.write(meta::stream_op_table_description(i));
      output.write("  \n  \n  \n");
    }
    output.write('\n');  
  }

  template <typename Mandatory_Output_Stream>
  static void builtin_variables(Mandatory_Output_Stream &output) {
    output.write("#### Builtin variables ####\n");
    output.write("`");
    output.write(NP1_REL_RLANG_FN_ROWNUM_SPECIAL_VARIABLE_NAME);
    output.write("`: the current row number.  \n");
  }

  template <typename Mandatory_Output_Stream>
  static void data_types(Mandatory_Output_Stream &output) {
    output.write("##### Data types #####\n");
    data_type_iterator<Mandatory_Output_Stream> iter(output);
    rel::rlang::dt::for_each(iter);
    output.write('\n');  
  }

  template <typename Mandatory_Output_Stream>
  static void operators(Mandatory_Output_Stream &output) {
    function_or_operator_iterator<Mandatory_Output_Stream> iter(output, true);
    rel::rlang::fn::fn_table::for_each(iter);  
  }

  template <typename Mandatory_Output_Stream>
  static void functions(Mandatory_Output_Stream &output) {
    function_or_operator_iterator<Mandatory_Output_Stream> iter(output, false);
    rel::rlang::fn::fn_table::for_each(iter);  
  }

  template <typename Mandatory_Output_Stream>
  static void function_or_operator_overloads(Mandatory_Output_Stream &output,
                                              const char *name) {
    overload_iterator<Mandatory_Output_Stream> iter(output, name);
    rel::rlang::fn::fn_table::for_each(iter);      
  }

  template <typename Mandatory_Output_Stream>
  static void distribution(Mandatory_Output_Stream &output) {
    output.write("##### Distribution #####\n");
    output.write("R17 provides 2 ways of distributing load across multiple servers:  \n");
    output.write("1.  Explicit SSH-based distribution. The script writer explicitly defines which parts of the script are distributed.  This style provides more control, less setup/config and less disk write load for once-off queries.  \n");
    output.write("2.  Semi-automatic WebDAV-based distribution.  R17 figures out which parts of the script can be distributed and sends script fragments to pre-started workers via UDP.  This style provides automatic caching.  It offers better performance when running a query multiple times over a large incrementally changing data set.  May require a cluster of WebDAV servers.  \n\n");
    output.write("##### Explicit SSH-based Distribution #####\n");
    output.write("The key stream operators for SSH-based distribution are `rel.record_split`, `rel.join.consistent_hash` and `meta.parallel_explicit_mapping`.  The recommended procedure is:  \n");
    output.write("1.  Use `rel.record_split` to split a large data set into individual files.  For best results aim for chunk sizes of 100MB to 500MB.  \n");
    output.write("2.  Create a list of participating hosts in a TSV file and translate that TSV file to native R17 format with `rel.from_tsv`.  Use `localhost` to refer to the local machine.  \n");
    output.write("3.  Join the list of files from (1) to the list of hosts in (2) using `rel.join.consistent_hash`.  The consistent hashing will result in fewer large file movements as the number of hosts or files change over time.  \n");
    output.write("4.  Distribute the files using the output of (3), the `meta.shell` function and your favorite file transfer utility eg scp or rsync.  \n");
    output.write("5.  Query using the output of (3) and `meta.parallel_consistent_mapping`.  \n\n");
    output.write("Below is a complete script that does everything from splitting to the query.  In practice it's usually better to separate out steps 1-4 from 5.  \n\n");
    output.write("`# Split the input file into fragments.`  \n");
    output.write("`io.file.read('big_input.r17_native.gz') | rel.record_split(1000000, 'tmp.web_fragment.');`  \n");
    output.write("  \n");
    output.write("`# Make the mapping from hosts to files.  participating_hosts.tsv's sole column is string:host_name.`  \n");
    output.write("`io.file.read('participating_hosts.tsv') | rel.from_tsv() | io.file.overwrite('participating_hosts.r17_native');`  \n");
    output.write("  \n");
    output.write("`io.directory.list('.')`  \n");
    output.write("`| rel.select(file_name)`  \n");
    output.write("`| rel.where(str.starts_with(file_name, 'tmp.web_fragment.'))`  \n");
    output.write("`| rel.join.consistent_hash('participating_hosts.r17_native')`  \n");
    output.write("`| io.file.overwrite('host_file_mapping.r17_native');`  \n");
    output.write("  \n");
    output.write("`# Do the distribution.  Skip transferring files to localhost.`  \n");
    output.write("`io.file.read('host_file_mapping.r17_native')`  \n");
    output.write("`| rel.where(host_name != 'localhost')`  \n");
    output.write("`| rel.select(meta.shell('rsync -av ' + file_name + ' ' + host_name + ':~') as rsync_output)`  \n");
    output.write("`| io.file.append('/dev/null');`  \n");
    output.write("  \n");
    output.write("`# Do the actual query.  It's ok to pass a pipeline to meta.parallel_explicit_mapping.`  \n");
    output.write("`io.file.read('host_file_mapping.r17_native')`  \n");
    output.write("`| meta.parallel_explicit_mapping(rel.select(username))`  \n");
    output.write("`| rel.group(count)`  \n");
    output.write("`| rel.order_by(_count)`  \n");
    output.write("`| io.file.overwrite('user_by_activity.r17_native');`  \n");

    output.write("##### Semi-automatic WebDAV-based Distribution #####\n");
    output.write("R17 implements semi-automatic distributed operation by splitting work across data stream fragments that together are called a 'recordset'.  ");
    output.write("To create a recordset, pipe a data stream into `rel.recordset.create`.  ");
    output.write("If an operator is able to distribute its workload then it will do so if its input is a recordset stream (ie the output of `rel.recordset.read`).\n");
    output.write("If an operator does not support distribution then the recordset stream will be automatically translated to a normal data stream.  ");
    output.write("Recordset streams can also be manually translated using `rel.recordset.translate`.  \n   \n");
    output.write("Recordsets are stored in 'reliable storage' and are identified with a 64-digit hexadecimal id.  ");
    output.write("There's currently no command to create a recordset id.  One way to create a suitable recordset id is:  \n");
    output.write("$ `cat /dev/urandom | head -n 100 | sha512sum | head --bytes 64`  \n  \n");
    output.write("The 'reliable storage' is a WebDAV-capable web server, specified by a URL in the ");
    output.write("`" NP1_ENVIRONMENT_RELIABLE_STORAGE_REMOTE_ROOT_NAME "`");
    output.write(" environment variable.  The web server is responsible for its own scaling & replication.  The local reliable storage cache is is specified by a path in the ");
    output.write("`" NP1_ENVIRONMENT_RELIABLE_STORAGE_LOCAL_ROOT_NAME "`");
    output.write(" environment variable.  Distributed scripts also require the ");
    output.write("`" NP1_ENVIRONMENT_DISTRIBUTED_SCRIPT_IP_PORT_START_NAME"`" );
    output.write(" environment variable to specify a start point for searching for an unused IP address/port combination.  \n  \n");
    output.write("R17 expects two special files inside the reliable storage: a list of 'client' IP address/port combinations and a list of 'worker' IP address/port combinations.  ");
    output.write("R17 expects the 'client' list at  \n");
    output.write("`00/00/00/0000000000000000000000000000000000000000000000000000000000`  \n");
    output.write("and the 'worker' list at  \n");
    output.write("`00/00/00/0000000000000000000000000000000000000000000000000000000001`.  \n  \n");
    output.write("In distributed mode, r17 will not delete any of its temporary files so they can be used as a query cache later.  This is safe because reliable storage data can never be updated.  All temporary files have an id beginning with 't'.  \n  \n");
    output.write("###### Example ######\n");
    output.write("In this example a script's work is distributed across two local processes and the reliable storage is at http://very_reliable/.  \n  \n");
    output.write("Contents of `http://very_reliable/00/00/00/0000000000000000000000000000000000000000000000000000000000`:  \n");
    output.write("127.0.0.1:22222  \n  \n");
    output.write("Contents of `http://very_reliable/00/00/00/0000000000000000000000000000000000000000000000000000000001`:  \n");
    output.write("127.0.0.1:22223  \n");
    output.write("127.0.0.1:22224  \n  \n");
    output.write("To start both workers and then run the distributed script:  \n");
    output.write("`$ export " NP1_ENVIRONMENT_RELIABLE_STORAGE_REMOTE_ROOT_NAME "=http://very_reliable`  \n");
    output.write("`$ export " NP1_ENVIRONMENT_RELIABLE_STORAGE_LOCAL_ROOT_NAME "=/tmp/inf_rs`  \n");
    output.write("`$ r17 meta.worker \"'127.0.0.1:22223'\" &`  \n");
    output.write("`$ r17 meta.worker \"'127.0.0.1:22224'\" &`  \n");
    output.write("`$ export " NP1_ENVIRONMENT_DISTRIBUTED_SCRIPT_IP_PORT_START_NAME "=127.0.0.1:22222`  \n");
    output.write("`$ r17 script_file_name.r17`  \n  \n");

    output.write("  \n  \n");
  }


  template <typename Mandatory_Output_Stream>
  static void environment_variables(Mandatory_Output_Stream &output) {
    output.write("##### Environment Variables #####\n");
    output.write("`" NP1_ENVIRONMENT_RELIABLE_STORAGE_LOCAL_ROOT_NAME "` (mandatory for workers & clients): the directory for the local copy of the reliable storage, without trailing slash.  \n  \n");
    output.write("`" NP1_ENVIRONMENT_RELIABLE_STORAGE_REMOTE_ROOT_NAME "` (mandatory for workers & clients): the URL for the canonical remote copy of the reliable storage, without trailing slash.  \n  \n");
    output.write("`" NP1_ENVIRONMENT_DISTRIBUTED_SCRIPT_IP_PORT_START_NAME "` (mandatory for clients): the local IP address and starting port for the client in IP:port format eg 127.0.0.1:22222.  The client will search for a free port starting with this port.  \n  \n");    
    output.write("`" NP1_ENVIRONMENT_MAX_RECORD_HASH_TABLE_SIZE "` (optional): The maximum number of slots in the record hash table that's used for rel.join.*, rel.unique and rel.group.  Default is " NP1_ENVIRONMENT_DEFAULT_MAX_RECORD_HASH_TABLE_SIZE " slots.  \n  \n");
    output.write("`" NP1_ENVIRONMENT_SORT_CHUNK_SIZE_NAME "` (optional): The size of the chunks used for sorting, in bytes.  The default is " NP1_ENVIRONMENT_DEFAULT_SORT_CHUNK_SIZE " bytes.\n  \n");
    output.write("`" NP1_ENVIRONMENT_SORT_INITIAL_NUMBER_THREADS "` (optional): The initial number of threads used for parallel sorting.  Each thread will sort a single chunk.  R17 will adjust the actual number of threads based on throughput after sorting each chunk.  The default is " NP1_ENVIRONMENT_DEFAULT_SORT_INITIAL_NUMBER_THREADS " threads.\n");
  }

private:
  template <typename Mandatory_Output_Stream>
  struct data_type_iterator {
    explicit data_type_iterator(Mandatory_Output_Stream &o) : m_output(o) {}

    void operator()(rel::rlang::dt::data_type type, const char *name,
                    const char *description) {
      m_output.write("* ");
      m_output.write(name);
      m_output.write(": ");
      m_output.write(description);
      m_output.write('\n');
    }

    Mandatory_Output_Stream &m_output;
  };


  template <typename Mandatory_Output_Stream>
  struct function_or_operator_iterator {
    explicit function_or_operator_iterator(Mandatory_Output_Stream &o, bool is_operator)
      : m_output(o), m_is_operator(is_operator) {}

    void operator()(const char *since, const char *name, const char *synonym, const char *description,
                    bool is_operator, size_t precedence, bool is_left_assoc,
                    rel::rlang::dt::data_type *arg_types, size_t number_arguments,
                    rel::rlang::dt::data_type return_type) {
      if ((is_operator != m_is_operator) || (str::cmp(name, m_previous_name) == 0)
          || ('_' == name[0])) {   // Internal names start with _
        return;
      }
 
      m_output.write("##### ");
      if (m_is_operator) {
        m_output.write("Operator: `");
      } else {
        m_output.write("Function: `");
      }
      m_output.write(name);
      m_output.write("` #####\n");

      m_output.write("Since: `");
      m_output.write(since);
      m_output.write("`  \n");


      if (synonym) {
        m_output.write("Synonym: `");
        m_output.write(synonym);
        m_output.write("`  \n");
      }

      m_output.write(description);
      m_output.write("  \n");
      
      if (m_is_operator) {
        m_output.write("Precedence: ");
        m_output.write(str::to_dec_str(precedence).c_str());
        m_output.write(" (lower is better)  \n");
      }

      m_output.write("\n\n");

      function_or_operator_overloads(m_output, name);
      m_output.write("  \n\n");
      m_output.write("- - -  \n\n");

      m_previous_name = name;
    }

    rstd::string m_previous_name;
    Mandatory_Output_Stream &m_output;
    bool m_is_operator;
  };


  template <typename Mandatory_Output_Stream>
  struct overload_iterator {
    explicit overload_iterator(Mandatory_Output_Stream &o, const char *target_name)
      : m_output(o), m_target_name(target_name) {}

    void operator()(const char *since, const char *name, const char *synonym, const char *description,
                    bool is_operator, size_t precedence, bool is_left_assoc,
                    rel::rlang::dt::data_type *arg_types, size_t number_arguments,
                    rel::rlang::dt::data_type return_type) {
      if (str::cmp(name, m_target_name) != 0) {
        return;
      }
 
      m_output.write("* ");
      if (is_operator && (2 == number_arguments) && is_left_assoc) {
        m_output.write("`");
        m_output.write(rel::rlang::dt::to_string(arg_types[0]));
        m_output.write(" ");
        m_output.write(name);
        m_output.write(" ");
        m_output.write(rel::rlang::dt::to_string(arg_types[1]));
        m_output.write("` returns `");
        m_output.write(rel::rlang::dt::to_string(return_type));
        m_output.write("`");
      } else if (is_operator && (1 == number_arguments) && !is_left_assoc) {
        m_output.write("`");
        m_output.write(name);
        m_output.write(" ");
        m_output.write(rel::rlang::dt::to_string(arg_types[0]));
        m_output.write("` returns `");
        m_output.write(rel::rlang::dt::to_string(return_type));
        m_output.write("`");
      } else if (!is_operator) {
        m_output.write("`");
        m_output.write(name);
        m_output.write('(');
        size_t i;
        for (i = 0; i < number_arguments; ++i) {
          m_output.write(rel::rlang::dt::to_string(arg_types[i]));
          if (i+1 < number_arguments) {
            m_output.write(", ");
          }
        }

        m_output.write(")` returns `");
        m_output.write(rel::rlang::dt::to_string(return_type));
        m_output.write("`");
      } else {
        m_output.write(" Error: unexpected overload type");
      }
    
      m_output.write("\n");
    }

    Mandatory_Output_Stream &m_output;
    const char *m_target_name;
  };

  template <typename Mandatory_Output_Stream>
  static void third_party_licenses(Mandatory_Output_Stream &output) {
    output.write("##### Third Party Licenses #####\n");
    output.write("###### libcurl ######\n");
    output.write(
"`COPYRIGHT AND PERMISSION NOTICE`\n"
"\n"
"`Copyright (c) 1996 - 2011, Daniel Stenberg, <daniel@haxx.se>.`\n"
"\n"
"`All rights reserved.`\n"
"\n"
"`Permission to use, copy, modify, and distribute this software for any purpose`\n"
"`with or without fee is hereby granted, provided that the above copyright`\n"
"`notice and this permission notice appear in all copies.`\n"
"\n"
"`THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR`\n"
"`IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,`\n"
"`FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN`\n"
"`NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,`\n"
"`DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR`\n"
"`OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE`\n"
"`OR OTHER DEALINGS IN THE SOFTWARE.`\n"
"\n"
"`Except as contained in this notice, the name of a copyright holder shall not`\n"
"`be used in advertising or otherwise to promote the sale, use or other dealings`\n"
"`in this Software without prior written authorization of the copyright holder.`\n");

    output.write("###### zlib ######\n");
    output.write(
"`(C) 1995-2010 Jean-loup Gailly and Mark Adler`\n"
"\n"
"`This software is provided 'as-is', without any express or implied`\n"
"`warranty.  In no event will the authors be held liable for any damages`\n"
"`arising from the use of this software.`\n"
"\n"
"`Permission is granted to anyone to use this software for any purpose,`\n"
"`including commercial applications, and to alter it and redistribute it`\n"
"`freely, subject to the following restrictions:`\n"
"\n"
"`1. The origin of this software must not be misrepresented; you must not`\n"
"`   claim that you wrote the original software. If you use this software`\n"
"`   in a product, an acknowledgment in the product documentation would be`\n"
"`   appreciated but is not required.`  \n"
"`2. Altered source versions must be plainly marked as such, and must not be`\n"
"`   misrepresented as being the original software.`  \n"
"`3. This notice may not be removed or altered from any source distribution.`\n"
"\n"
"`Jean-loup Gailly        Mark Adler`\n"
"`jloup@gzip.org          madler@alumni.caltech.edu`\n");

    output.write("###### PCRE ######\n");
    output.write(
"`Written by:       Philip Hazel`\n"
"`Email local part: ph10`\n"
"`Email domain:     cam.ac.uk`\n"
"\n"
"`University of Cambridge Computing Service,`\n"
"`Cambridge, England.`\n"
"\n"
"`Copyright (c) 1997-2010 University of Cambridge`\n"
"`All rights reserved.`\n"
"\n"
"`Redistribution and use in source and binary forms, with or without`\n"
"`modification, are permitted provided that the following conditions are met:`\n"
"\n"
"`    * Redistributions of source code must retain the above copyright notice,`\n"
"`      this list of conditions and the following disclaimer.`\n"
"\n"
"`    * Redistributions in binary form must reproduce the above copyright`\n"
"`      notice, this list of conditions and the following disclaimer in the`\n"
"`      documentation and/or other materials provided with the distribution.`\n"
"\n"
"`    * Neither the name of the University of Cambridge nor the name of Google`\n"
"`      Inc. nor the names of their contributors may be used to endorse or`\n"
"`      promote products derived from this software without specific prior`\n"
"`      written permission.`\n"
"\n"
"`THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"`\n"
"`AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE`\n"
"`IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE`\n"
"`ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE`\n"
"`LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR`\n"
"`CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF`\n"
"`SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS`\n"
"`INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN`\n"
"`CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)`\n"
"`ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE`\n"
"`POSSIBILITY OF SUCH DAMAGE.`\n");

    output.write("###### 64-bit multiplication and division on 32-bit platforms ######\n");
    output.write(
"`Copyright (c) 1992, 1993`\n"
"`The Regents of the University of California.  All rights reserved.`\n"
"\n"
"`This software was developed by the Computer Systems Engineering group`\n"
"`at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and`\n"
"`contributed to Berkeley.`\n"
"\n"
"`Redistribution and use in source and binary forms, with or without`\n"
"`modification, are permitted provided that the following conditions`\n"
"`are met:`\n"
"`1. Redistributions of source code must retain the above copyright`\n"
"`   notice, this list of conditions and the following disclaimer.`  \n"
"`2. Redistributions in binary form must reproduce the above copyright`\n"
"`   notice, this list of conditions and the following disclaimer in the`\n"
"`   documentation and/or other materials provided with the distribution.`  \n"
"`3. All advertising materials mentioning features or use of this software`\n"
"`   must display the following acknowledgement:`  \n"
"` This product includes software developed by the University of`  \n"
"` California, Berkeley and its contributors.`  \n"
"`4. Neither the name of the University nor the names of its contributors`\n"
"`   may be used to endorse or promote products derived from this software`\n"
"`   without specific prior written permission.`  \n"
"\n"
"`THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS \"AS IS\" AND`\n"
"`ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE`\n"
"`IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE`\n"
"`ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE`\n"
"`FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL`\n"
"`DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS`\n"
"`OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)`\n"
"`HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT`\n"
"`LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY`\n"
"`OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF`\n"
"`SUCH DAMAGE.`\n");
  }
  
};



} // namespaces
}


#endif
