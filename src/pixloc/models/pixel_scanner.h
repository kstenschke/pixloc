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

#include "pixloc/models/color_matcher_abstract.h"
#include "pixloc/models/color_matcher_precise.h"
#include "pixloc/models/color_matcher_range.h"

namespace pixloc {
class PixelScanner {

 public:

  // Constructor
  PixelScanner(Display *display,
          int x_start, int y_start,
          unsigned int range_x, unsigned int range_y,
          int find_red, int find_green, int find_blue,
          int tolerance);

  // Scan pixels on x or y axis, trace or find
  int ScanUniaxial(int amount_find, bool trace);

  void TraceBitmask();

  std::string FindBitmask(const std::string &bitmask);

 private:

  Display *display;
  XImage *image;

  int x_start;
  int y_start;
  unsigned int range_x;
  unsigned int range_y;

  ColorMatcherAbstract *color_matcher = new pixloc::ColorMatcherAbstract(0, 0, 0);

  std::string GetBitmaskLineFromImage(Display *display, XColor *color, int y);

  // Get line from bitmask haystack. this is lazy-loaded: initialize it via GetBitmaskLineFromImage if not yet
  void FetchHaystackLine(std::vector<std::string> &haystack_lines,
                         long &index_empy_haystack_line,
                         unsigned long index_haystack_line,
                         XColor *color,
                         std::string &haystack_line);

  std::string FormatCoordinate(unsigned long offset_needle, unsigned long index_haystack_line) const;
}; // class Scanner
} // namespace pixloc

#endif