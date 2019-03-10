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

unsigned short GetModeIdFromName(const std::string &mode) {
  if (mode.empty()) throw "No mode given.";

  if (strcmp(mode.c_str(), kModeNameFindBitmask)==0) return kModeIdFindBitmask;
  if (strcmp(mode.c_str(), kModeNameFindConsecutiveHorizontal)==0) return kModeIdFindConsecutiveHorizontal;
  if (strcmp(mode.c_str(), kModeNameFindConsecutiveVertical)==0) return kModeIdFindConsecutiveVertical;
  if (strcmp(mode.c_str(), kModeNameTraceBitmask)==0) return kModeIdTraceBitmask;
  if (strcmp(mode.c_str(), kModeNameTraceHorizontal)==0) return kModeIdTraceHorizontal;
  if (strcmp(mode.c_str(), kModeNameTraceMainColor)==0) return kModeIdTraceMainColor;
  if (strcmp(mode.c_str(), kModeNameTraceMouse)==0) return kModeIdTraceMouse;
  if (strcmp(mode.c_str(), kModeNameTraceVertical)==0) return kModeIdTraceVertical;

  throw "Valid mode is required.";
}

bool IsTupelRangeMode(int mode_id) {
  return mode_id==kModeIdTraceBitmask || mode_id==kModeIdFindBitmask || mode_id==kModeIdTraceMainColor;
}

bool IsHorizontalMode(int mode_id) {
  return mode_id==kModeIdTraceHorizontal || mode_id==kModeIdFindConsecutiveHorizontal;
}

bool IsTraceMode(int mode_id) {
  return mode_id==kModeIdTraceHorizontal ||
         mode_id==kModeIdTraceVertical ||
         mode_id==kModeIdTraceBitmask ||
         mode_id==kModeIdTraceMainColor ||
         mode_id==kModeIdTraceMouse;
}

bool IsBitmaskMode(int mode_id) {
  return mode_id==kModeIdTraceBitmask || mode_id==kModeIdFindBitmask;
}

bool ModeRequiresAmountPx(int mode_id) {
  return mode_id==kModeIdFindConsecutiveHorizontal || mode_id==kModeIdFindConsecutiveVertical;
}

bool ModeRequiresBitmask(int mode_id) {
  return mode_id==kModeIdFindBitmask;
}

bool ModeRequiresColor(int mode_id) {
  return mode_id==kModeIdFindBitmask ||
         mode_id==kModeIdFindConsecutiveHorizontal ||
         mode_id==kModeIdFindConsecutiveVertical ||
         mode_id==kModeIdTraceBitmask;
}

bool IsValidRangeForMode(int mode_id, const std::string &range) {
  return IsTupelRangeMode(mode_id)
         ? IsValidNumericTupel(const_cast<std::string &>(range))
         : helper::strings::IsNumeric(range);
}

bool IsValidColor(const std::string &color) {
  return !color.empty() && std::regex_match(color, std::regex("[1-9]?[0-9]?[0-9],[1-9]?[0-9]?[0-9],[1-9]?[0-9]?[0-9]"));
}

void ValidateBitmask(const std::string &bitmask_px, int range_width, int range_height) {
  if (bitmask_px.empty()) throw "Bitmask is empty";
  if (bitmask_px.length() > range_width * range_height + range_height)
    throw "Bitmask dimension must be smaller than scanning range.";
  if (!std::regex_match(bitmask_px, std::regex("[\\*_,]+"))) throw "Valid bitmask to find is required.";
}

bool IsValidNumericTupel(std::string &str) {
  return !str.empty() && std::regex_match(str, std::regex("[1-9][0-9]*,[1-9][0-9]*"));
}

void ResolveNumericTupel(const std::string &str, int &number_1, int &number_2) {
  if (str.empty() || !IsValidNumericTupel(const_cast<std::string &>(str))) throw "Valid from coordinate is required.";

  std::vector<std::string> fromCoordinate = helper::strings::Explode(str, ',');

  number_1 = helper::strings::ToInt(fromCoordinate.at(0), 0);
  number_2 = helper::strings::ToInt(fromCoordinate.at(1), 0);


  if (!(number_1 > 0 && number_2 > 0)) throw "Valid from coordinate is required.";
}

void ResolveScanningRange(int mode_id, const std::string &range, int &range_x, int &range_y) {
  if (!IsValidRangeForMode(mode_id, range)) throw "Valid scanning range is required.";

  if (IsTupelRangeMode(mode_id)) {
    ResolveNumericTupel(range, range_x, range_y);
    if (range_x < 0 && range_y < 0) throw "Scanning range must start >= 0,0.";
  }
  if (IsHorizontalMode(mode_id)) {
    range_y = 1;
    range_x = helper::strings::ToInt(range, -1);

    if (range_x == 1) throw "Valid scanning range is required.";
  }

  range_x = 1;
  range_y = helper::strings::ToInt(range, -1);

  if (range_y == -1) throw "Valid scanning range is required.";
}

void ResolveRgbColor(const std::string &color, int &red, int &green, int &blue) {
  if (!IsValidColor(color)) throw "Valid color is required.";

  std::vector<std::string> rgb = helper::strings::Explode(color, ',');

  red = helper::strings::ToInt(rgb.at(0), -1);
  green = helper::strings::ToInt(rgb.at(1), -1);
  blue = helper::strings::ToInt(rgb.at(2), -1);

  if (red == -1 || green == -1 || blue == -1) throw "Valid color is required.";
}
} // namespace cli
} // namespace pixloc
