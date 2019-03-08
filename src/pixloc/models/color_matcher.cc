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

#include <iostream>
#include "color_matcher.h"

namespace pixloc {

// Constructor
ColorMatcher::ColorMatcher(unsigned short find_red,
                           unsigned short find_green,
                           unsigned short find_blue,
                           unsigned short tolerance) {
  // @todo test needed minimum tolerance for color values (that are a multiple of 256)
//  if (tolerance == 0) tolerance = 128;

  this->red_min = CalculateLimit(find_red, tolerance, kOperationSubtract);
  this->red_max = CalculateLimit(find_red, tolerance, kOperationAdd);
  this->green_min = CalculateLimit(find_green, tolerance, kOperationSubtract);
  this->green_max = CalculateLimit(find_green, tolerance, kOperationAdd);
  this->blue_min = CalculateLimit(find_blue, tolerance, kOperationSubtract);
  this->blue_max = CalculateLimit(find_blue, tolerance, kOperationAdd);
}

unsigned short ColorMatcher::CalculateLimit(unsigned short color_value,
                                            unsigned short tolerance,
                                            unsigned short operation) {
  if (operation==kOperationSubtract) {
    if (tolerance > color_value) return 0; 
     return color_value - tolerance;
  }

  if (color_value + tolerance > kMaximumColorValue) return static_cast<unsigned short>(kMaximumColorValue);
  return color_value + tolerance;
}

bool ColorMatcher::Matches(unsigned short red, unsigned short green, unsigned short blue) {
  return
      red >= this->red_min &&
          red <= this->red_max &&
          green >= this->green_min &&
          green <= this->green_max &&
          blue >= this->blue_min &&
          blue <= this->blue_max;
}

} // namespace pixloc