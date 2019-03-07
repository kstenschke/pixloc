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

#include <cstring>
#include <string>
#include <codecvt>
#include <locale>
#include <vector>
#include <sstream>
#include <iostream>
#include <map>

#include "helper_strings.h"

namespace helper {
namespace strings {

/**
 * Get amount of sub string occurrences
 */
int sub_str_count(const char *str, const char *sub) {
  auto length = static_cast<int>(strlen(sub));
  if (length == 0) return 0;

  int count = 0;
  for (str = strstr(str, sub); str; str = strstr(str + length, sub)) ++count;

  return count;
}

/**
 * Split given string by given character delimiter into vector of strings
 */
std::vector<std::string> Explode(std::string const &str, char delimiter) {
  std::vector<std::string> result;
  std::istringstream iss(str);

  for (std::string token; std::getline(iss, token, delimiter);) result.push_back(std::move(token));

  return result;
}

bool IsNumeric(const std::string &str, bool allow_negative) {
  if (str.empty()) return false;

  std::string::const_iterator it = str.begin();
  int index = 0;
  while (it!=str.end() && (std::isdigit(*it) || (allow_negative && index==0 && str[0]=='-'))) {
    ++it;
    ++index;
  }

  return it==str.end() && (!allow_negative || (str[0]!='-' || str.length() > 1));
}

int ToInt(std::string str, int defaultValue) {
  return strings::IsNumeric(str)
         ? std::stoi(str)
         : defaultValue;
}

std::string FindMostCommon(const std::vector<std::string> &vec) {
  std::map<std::string,unsigned long> str_map;
  for (const auto &str : vec)
    ++str_map[str];

  typedef decltype(std::pair<std::string,unsigned long>()) pair_type;

  auto comp = [](const pair_type &pair1, const pair_type &pair2) -> bool {
    return pair1.second < pair2.second; };
  return std::max_element(str_map.cbegin(), str_map.cend(), comp)->first;
}

} // namespace strings
} // namespace helper
