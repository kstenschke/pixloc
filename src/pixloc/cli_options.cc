/*
  Copyright (c) 2019, Kay Stenschke
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   * Neither the name of  nor the names of its contributors may be used to
     endorse or promote products derived from this software without specific
     prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include <regex>

#include "cli_options.h"
#include "pixloc/helper/helper_strings.h"

namespace pixloc {
namespace clioptions {

int get_mode_id_from_name(const std::string &mode) {
  if (strcmp(mode.c_str(), kModeNameFindBitmask)==0) return kModeIdFindBitmask;
  if (strcmp(mode.c_str(), kModeNameFindConsecutiveHorizontal)==0) return kModeIdFindConsecutiveHorizontal;
  if (strcmp(mode.c_str(), kModeNameFindConsecutiveVertical)==0) return kModeIdFindConsecutiveVertical;
  if (strcmp(mode.c_str(), kModeNameTraceBitmask)==0) return kModeIdTraceBitmask;
  if (strcmp(mode.c_str(), kModeNameTraceHorizontal)==0) return kModeIdTraceHorizontal;
  if (strcmp(mode.c_str(), kModeNameTraceMainColor)==0) return kModeIdTraceMainColor;
  if (strcmp(mode.c_str(), kModeNameTraceMouse)==0) return kModeIdTraceMouse;
  if (strcmp(mode.c_str(), kModeNameTraceVertical)==0) return kModeIdTraceVertical;
  return -1;
}

bool is_tupel_range_mode(int mode_id) {
  return mode_id==kModeIdTraceBitmask || mode_id==kModeIdFindBitmask || mode_id==kModeIdTraceMainColor;
}

bool is_horizontal_mode(int mode_id) {
  return mode_id==kModeIdTraceHorizontal || mode_id==kModeIdFindConsecutiveHorizontal;
}

bool is_trace_mode(int mode_id) {
  return mode_id==kModeIdTraceHorizontal || mode_id==kModeIdTraceVertical || mode_id==kModeIdTraceBitmask;
}

bool is_bitmask_mode(int mode_id) {
  return mode_id==kModeIdTraceBitmask || mode_id==kModeIdFindBitmask;
}

bool mode_requires_amount_px(int mode_id) {
  return mode_id==kModeIdFindConsecutiveHorizontal || mode_id==kModeIdFindConsecutiveVertical;
}

bool mode_requires_bitmask(int mode_id) {
  return mode_id==kModeIdFindBitmask;
}

bool mode_requires_color(int mode_id) {
  switch (mode_id) {
    case kModeIdFindBitmask:
    case kModeIdFindConsecutiveHorizontal:
    case kModeIdFindConsecutiveVertical:
    case kModeIdTraceBitmask:return true;
    case kModeIdTraceHorizontal:
    case kModeIdTraceMainColor:
    case kModeIdTraceVertical:
    default:return false;
  }
}

bool is_valid_range_for_mode(int mode_id, const std::string &range) {
  return is_tupel_range_mode(mode_id)
         ? is_valid_numeric_tupel(const_cast<std::string &>(range))
         : helper::strings::IsNumeric(range);
}

bool is_valid_color(const std::string &color) {
  return !color.empty() && std::regex_match(color, std::regex("[1-9]?[0-9]?[0-9],[1-9]?[0-9]?[0-9],[1-9]?[0-9]?[0-9]"));
}

bool is_valid_bitmask(const std::string &bitmask_px, int min_width, int min_height) {
  if (bitmask_px.empty() || !std::regex_match(bitmask_px, std::regex("[\\*_,]+"))) return false;
  // TODO validate bitmask dimension
  return true;
}

bool is_valid_numeric_tupel(std::string &str) {
  return !str.empty() && std::regex_match(str, std::regex("[1-9][0-9]*,[1-9][0-9]*"));
}

bool resolve_numeric_tupel(const std::string &str, int &number_1, int &number_2) {
  if (str.empty() || !is_valid_numeric_tupel(const_cast<std::string &>(str))) return false;

  std::vector<std::string> fromCoordinate = helper::strings::Explode(str, ',');

  number_1 = helper::strings::ToInt(fromCoordinate.at(0), 0);
  number_2 = helper::strings::ToInt(fromCoordinate.at(1), 0);

  return number_1 > 0 && number_2 > 0;
}

bool resolve_scanning_range(int mode_id, const std::string &range, int &range_x, int &range_y) {
  if (!is_valid_range_for_mode(mode_id, range)) return false;

  if (is_tupel_range_mode(mode_id)) {
    resolve_numeric_tupel(range, range_x, range_y);
    return range_x > 0 && range_y > 0;
  }
  if (is_horizontal_mode(mode_id)) {
    range_y = 1;
    range_x = helper::strings::ToInt(range, -1);

    return range_x > 0;
  }

  range_x = 1;
  range_y = helper::strings::ToInt(range, -1);

  return range_y > 0;
}

bool resolve_rgb_color(const std::string &color, int &red, int &green, int &blue) {
  if (!is_valid_color(color)) return false;

  std::vector<std::string> rgb = helper::strings::Explode(color, ',');

  red = helper::strings::ToInt(rgb.at(0), -1);
  green = helper::strings::ToInt(rgb.at(1), -1);
  blue = helper::strings::ToInt(rgb.at(2), -1);

  return red > -1 && green > -1 && blue > -1;
}
} // namespace cli
} // namespace pixloc
