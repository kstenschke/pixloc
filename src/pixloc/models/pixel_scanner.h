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

#ifndef CLASS_PIXLOC_SCANNER
#define CLASS_PIXLOC_SCANNER

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>

#include "pixloc/models/color_matcher.h"

namespace pixloc {
class PixelScanner {

 public:
  // Constructor
  PixelScanner(Display *display,
               unsigned short x_start, unsigned short y_start,
               unsigned short range_x, unsigned short range_y,
               unsigned short find_red, unsigned short find_green, unsigned short find_blue,
               unsigned short tolerance);

  // Scan pixels on x or y axis, trace or find
  int ScanUniaxial(unsigned short amount_find, unsigned short step_size, bool trace);

  void TraceMainColor();

  void TraceBitmask();

  std::string FindBitmask(const std::string &bitmask);

 private:
  Display *display;
  XImage *image;
  XColor *color;

  unsigned short x_start;
  unsigned short y_start;
  unsigned short range_x;
  unsigned short range_y;

  ColorMatcher *color_matcher;

  void InitUniaxialStepSize(unsigned short step_size, unsigned short &step_size_x, unsigned short &step_size_y) const;

  signed short GetStartingValueOfHomochromaticSetAtCoordinate(unsigned short x,
                                                              unsigned short y,
                                                              unsigned short amount_find);

  std::string GetBitmaskLineFromImage(unsigned short y);

  // Get line from bitmask haystack. this is lazy-loaded: initialize it via GetBitmaskLineFromImage if not yet
  void FetchHaystackLine(std::vector<std::string> &haystack_lines,
                         unsigned short &index_empty_haystack_line,
                         unsigned short index_haystack_line,
                         std::string &haystack_line);

  std::string FormatCoordinate(signed long offset_needle, unsigned short index_haystack_line) const;
}; // class Scanner
} // namespace pixloc

#endif
