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

#ifndef CLASS_PIXLOC_COLOR_MATCHER_RANGE
#define CLASS_PIXLOC_COLOR_MATCHER_RANGE

#include "color_matcher.h"

namespace pixloc {
/**
 * Color matcher (RGB range)
 */
class ColorMatcher {
 public:
  // Maximum possible value of red, green or blue, = 256*256
  static const int kMaximumColorValue = 65536;

  static const int kOperationAdd = 1;
  static const int kOperationSubtract = 2;

  // Constructor
  ColorMatcher(unsigned short find_red, unsigned short find_green, unsigned short find_blue, unsigned short tolerance);

  unsigned short CalculateLimit(unsigned short color_value, unsigned short tolerance, unsigned short operation);

  bool Matches(unsigned short red, unsigned short green, unsigned short blue);

 private:
  unsigned short red_min;
  unsigned short red_max;
  unsigned short green_min;
  unsigned short green_max;
  unsigned short blue_min;
  unsigned short blue_max;
};
} // namespace pixloc

#endif //PIXLOC_COLOR_MATCHER_RGB_RANGE