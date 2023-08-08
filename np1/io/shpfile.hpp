// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_SHPFILE_HPP
#define NP1_IO_SHPFILE_HPP

#include <arpa/inet.h>
#include "np1/assert.hpp"
#include "np1/io/file_mapping.hpp"
#include "np1/str.hpp"

#define NP1_REL_FROM_SHAPEFILE_GEOM_OUTPUT_HEADING_NAME "string:_geom"

namespace np1 {
namespace io {
  

class shpfile {
public:
  explicit shpfile(const rstd::string &shpfile_name) 
    : m_mapping(open_file(m_file, shpfile_name)), m_p((const uint8_t *)m_mapping.ptr()), m_end(m_p + m_mapping.size()) {
    m_p = shp_header_validate(m_p, m_end, m_end - m_p);
  }

  //TODO: make an iterator-based interface.
  bool has_next() { return m_p < m_end; }
  void next(rstd::string &field) {
    m_p = shp_record_copy(m_p, m_end, field);
  }


private:
  typedef struct {
    double x;
    double y;
  } __attribute__((packed)) shp_point_type;

  // Minimum Bounding Rectangle
  typedef struct {
    shp_point_type min;
    shp_point_type max;
  } __attribute__((packed)) shp_mbr_type;

  typedef struct {
    double min;
    double max;
  } __attribute__((packed)) shp_range_type;

  typedef struct {
    uint32_t file_code;
    uint32_t unused[5];
    uint32_t file_length_16bit_words;
    uint32_t version;
    uint32_t shape_type;   
    shp_mbr_type mbr;
    shp_range_type z_range;
    shp_range_type m_range;
  } __attribute__((packed)) shp_header_type;

  static_assert(sizeof(shp_header_type) == 100, "sizeof(shp_header_type) does not match ESRI spec");

  typedef struct {
    // 1-based
    uint32_t record_number;
    uint32_t record_length_16bit_words;
  } __attribute__((packed)) shp_record_header_type;

  static_assert(sizeof(shp_record_header_type) == 8, "sizeof(shp_record_header_type) does not match ESRI spec");

  typedef struct {
    shp_mbr_type mbr;
    uint32_t num_parts;
    uint32_t num_points;
  } __attribute__((packed)) shp_polygon_header_type;


  static const uint8_t *shp_header_read(const uint8_t *p, const uint8_t *p_end, shp_header_type &h) {
    size_t p_len = p_end - p;
    NP1_ASSERT(sizeof(h) <= p_len, ".shp is not long enough to hold the file header");
    //TODO: this code assumes little-endianness.
    h = *((shp_header_type *)p);
    h.file_code = ntohl(h.file_code);
    h.file_length_16bit_words = ntohl(h.file_length_16bit_words);
    NP1_ASSERT(h.file_code == 0x0000270a, ".shp is not in the expected format. Header file_code=0x" + str::to_hex_str(h.file_code) 
                + " expected=0x0000270a");
    NP1_ASSERT(h.version == 1000, ".shp version is not 1000");
    NP1_ASSERT(shp_length_bytes(h) == p_len, ".shp is not the expected length");
    return p + sizeof(h);
  }

  static uint64_t shp_length_bytes(const shp_header_type &h) {
    uint64_t length = h.file_length_16bit_words;
    length *= 2;
    return length;
  }

  static const uint8_t *shp_header_validate(const uint8_t *p, const uint8_t *p_end, size_t mapped_size) {
    shp_header_type h;
    p = shp_header_read(p, p_end, h);
    NP1_ASSERT(shp_length_bytes(h) == mapped_size, "shp_header.file_length_16bit_words does not match file mapped size");
    return p;
  }

  static const uint8_t *shp_record_header_read(const uint8_t *p, const uint8_t *p_end, shp_record_header_type &rh) {
    size_t p_len = p_end - p;
    NP1_ASSERT(sizeof(rh) < p_len, ".shp is not long enough to hold the record header");
    rh = *((shp_record_header_type *)p);
    //TODO this code assumes little-endianness.
    rh.record_number = ntohl(rh.record_number);
    rh.record_length_16bit_words = ntohl(rh.record_length_16bit_words);
    return p + sizeof(rh);
  }

  static uint64_t shp_record_length_bytes(shp_record_header_type &rh) {
    uint64_t length = rh.record_length_16bit_words;
    length *= 2;
    return length;
  }

  static const uint8_t *shp_record_copy(const uint8_t *p, const uint8_t *p_end, rstd::string &dest) {
    shp_record_header_type rh;
    const uint8_t *record_p = shp_record_header_read(p, p_end, rh);
    uint64_t record_len = shp_record_length_bytes(rh);
    size_t remaining_length = p_end - record_p;
    NP1_ASSERT(record_len <= remaining_length, ".shp is not long enough to hold record number " + str::to_dec_str(rh.record_number) 
                + " (1-based). record_len=" + str::to_dec_str(record_len) + " (0x" + str::to_hex_str(record_len) + ") remaining=" 
                + str::to_dec_str(remaining_length));
    
    // Every record contains a shape type but the spec doesn't consider it part of the header.  Read it now.
    uint32_t shape_type;
    NP1_ASSERT(remaining_length > sizeof(uint32_t), ".shp corrupt, file remaining length is not enough to hold the record shape type");
    shape_type = *((const uint32_t *)record_p);
    const uint8_t *content_p = record_p + sizeof(uint32_t);
    const uint8_t *content_end = record_p + record_len;


    // The spec says that the shape type must be the same as the shape type in the header, or the null shape type.  
    // But we don't enforce this.
    //TODO: complete the rest of the data types.
    switch (shape_type) {
    case 0:
      dest = "";
      break;

    case 1:
      dest = get_point(content_p, content_end);
      break;

    case 3:
      dest = get_polyline(content_p, content_end);
      break;

    case 5:
      dest = get_polygon(content_p, content_end);
      break;

    case 8:
      dest = "MULTIPOINT";
      break;

    case 11:
      dest = "POINTZ";
      break;

    case 13:
      dest = "POLYLINEZ";
      break;

    case 15:
      dest = "POLYGONZ";
      break;

    case 18:
      dest = "MULTIPOINTZ";
      break;

    case 21:
      dest = "POINTM";
      break;

    case 23:
      dest = "POLYLINEM";
      break;

    case 25:
      dest = "POLYGONM";
      break;

    case 28:
      dest = "MULTIPOINTM";
      break;

    case 31:
      dest = "MULTIPATCH";
      break;

    default:
      NP1_ASSERT(false, "Unsupported shape type");
      break;
    }

    return content_end;
  }

  //TODO don't concatenate strings!
  static rstd::string get_point(const uint8_t *content_p, const uint8_t *content_end) {
    NP1_ASSERT((content_p < content_end) && (((size_t)(content_end - content_p)) <= sizeof(shp_point_type)), 
              ".shp point record content is not long enough to hold a point");
    const shp_point_type *point = (const shp_point_type *)content_p;
    return "POINT (" + get_point_coordinates(point) + ")";
  }

  static rstd::string get_point_coordinates(const shp_point_type *point) {
    return str::to_dec_str(point->x) + " " + str::to_dec_str(point->y);
  }

  static rstd::string get_polyline(const uint8_t *content_p, const uint8_t *content_end) {
    return "MULTILINESTRING" + get_multi_coordinates(content_p, content_end);
  }

  static rstd::string get_polygon(const uint8_t *content_p, const uint8_t *content_end) {
    return "MULTIPOLYGON" + get_multi_coordinates(content_p, content_end);
  }

  static rstd::string get_multi_coordinates(const uint8_t *content_p, const uint8_t *content_end) {
    size_t content_length = content_end - content_p;
    NP1_ASSERT(sizeof(shp_polygon_header_type) <= content_length, ".shp record content is not long enough to hold a polygon header");
    const shp_polygon_header_type *header = (const shp_polygon_header_type *)content_p;

    const uint8_t *parts_and_points = content_p + sizeof(shp_polygon_header_type);
    NP1_ASSERT(((size_t)(content_end - parts_and_points)) == (sizeof(uint32_t) * header->num_parts) + (sizeof(shp_point_type) * header->num_points), 
                ".shp record content is length is not equal to the specified parts and points size of this polygon");
    const uint32_t *parts_start = (const uint32_t *)parts_and_points;
    const uint32_t *parts_end = parts_start + header->num_parts;
    const shp_point_type *points_start = (const shp_point_type *)parts_end;
    const shp_point_type *points_end = points_start + header->num_points;

    return get_multi_coordinates(parts_start, parts_end, points_start, points_end);
  }


  static rstd::string get_multi_coordinates(const uint32_t *parts_start, const uint32_t *parts_end, 
                                            const shp_point_type *points_start, const shp_point_type *points_end) {
 
    NP1_ASSERT((parts_start < parts_end) && (points_start < points_end), ".shp file is corrupt, parts & points are wierd"); 
    const uint32_t *parts_p = parts_start;

    rstd::string result = "(";
    bool is_first = true;
    for (; parts_p < parts_end; ++parts_p) {
      if (!is_first) {
        result.append(",");
      }

      is_first = false;

      const shp_point_type *part_points_start = points_start + parts_p[0];

      NP1_ASSERT((part_points_start >= points_start) && (part_points_start < points_end), 
                  ".shp file is corrupt, start part offset is outside the bounds of the points of this polygon");
      const shp_point_type *part_points_end = (parts_p + 1 < parts_end) ? (points_start + parts_p[1]) : points_end;
      NP1_ASSERT((part_points_end >= points_start) && (part_points_end <= points_end), 
                  ".shp file is corrupt, end part offset is outside the bounds of the points of this polygon");
      
      result.append(get_line_coordinates(part_points_start, part_points_end));
    }

    result.append(")");

    return result;
  }


  static rstd::string get_line_coordinates(const shp_point_type *point_p, const shp_point_type *point_end) {
    NP1_ASSERT(point_p < point_end, ".shp file is corrupt");

    rstd::string result = "(";
    bool is_first = true;
    for (; point_p < point_end; ++point_p) {
      if (!is_first) {
        result.append(",");
      }

      is_first = false;
      result.append(get_point_coordinates(point_p));
    }

    result.append(")");
    return result;
  }

  static file::handle_type open_file(file &f, const rstd::string &shp_name) {
    NP1_ASSERT(f.open_ro(shp_name.c_str()), "Unable to open shapefile " + shp_name);
    return f.handle();
  }


private:
  file m_file;
  file_mapping m_mapping;
  const uint8_t *m_p;
  const uint8_t *m_end;
};


} // namespaces
}

#endif

