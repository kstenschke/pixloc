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

#include "pixel_scanner.h"
#include "pixloc/helper/helper_strings.h"

namespace pixloc {

/*
 * Constructor
 */
PixelScanner::PixelScanner(Display *display,
                           unsigned short x_start, unsigned short y_start,
                           unsigned short range_x, unsigned short range_y,
                           unsigned short find_red, unsigned short find_green, unsigned short find_blue,
                           unsigned short tolerance) {
  this->display = display;

  this->x_start = x_start;
  this->y_start = y_start;
  this->range_x = range_x;
  this->range_y = range_y;

  this->color_matcher = new ColorMatcher(find_red, find_green, find_blue, tolerance);

  this->image = XGetImage(display,
                          RootWindow(display, DefaultScreen(display)),
                          x_start, y_start,
                          range_x, range_y,
                          AllPlanes,
                          XYPixmap);
};

// Scan (or trace) given line or column on screenshot image
// Return x or y position where given RGB occurs in given amount of consecutive pixels,
// Or return -1 if not found
int PixelScanner::ScanUniaxial(unsigned short amount_find, bool trace) {
  auto *color = new XColor;

  unsigned short red, green, blue;
  unsigned short amount_found = 0;

  for (unsigned short y = 0; y < range_y; ++y) {
    for (unsigned short x = 0; x < range_x; ++x) {
      color->pixel = XGetPixel(image, x, y);
      XQueryColor(display, DefaultColormap(display, DefaultScreen(display)), color);

      red = color->red;
      green = color->green;
      blue = color->blue;

      if (trace) std::cout << (red / 256) << "," << (green / 256) << "," << (blue / 256) << "\n";

      if (color_matcher->Matches(red, green, blue)) {
        ++amount_found;
        if (amount_found==amount_find) {
          XFree(image);
          return range_y==1 ? x : y;
        }
      } else amount_found = 0;
    }
  }
  XFree(image);
  return -1;
}

void PixelScanner::TraceMainColor() {
  auto *color = new XColor;
  int red, green, blue;

  std::vector<std::string> colors;

  for (unsigned short y = 0; y < range_y; ++y) {
    for (unsigned short x = 0; x < range_x; ++x) {
      color->pixel = XGetPixel(image, x, y);
      XQueryColor(display, DefaultColormap(display, DefaultScreen(display)), color);

      red = color->red / 256;
      green = color->green / 256;
      blue = color->blue / 256;

      char rgb[12];
      sprintf(rgb, "%d,%d,%d", red, green, blue);
      colors.emplace_back(rgb);
    }
  }
  XFree(image);

  std::cout << helper::strings::FindMostCommon(colors);
}

void PixelScanner::TraceBitmask() {
  auto *color = new XColor;
  for (unsigned short y = 0; y < range_y; ++y) {
    std::cout << this->GetBitmaskLineFromImage(color, y) << (y < range_y - 1 ? "," : "") << "\n";
  }

  XFree(image);
}

std::string PixelScanner::GetBitmaskLineFromImage(XColor *color, unsigned short y) {
  std::string bitmask_haystack;
  for (unsigned short x = 0; x < this->range_x; ++x) {
    color->pixel = XGetPixel(this->image, x, y);
    XQueryColor(display, DefaultColormap(display, DefaultScreen(display)), color);
    bitmask_haystack += this->color_matcher->Matches(color->red, color->green, color->blue) ? '*' : '_';
  }

  return bitmask_haystack;
}

// Find coordinate of bitmask sought-after
std::string PixelScanner::FindBitmask(const std::string &bitmask_needle) {
  std::vector<std::string> needle_lines = helper::strings::Explode(bitmask_needle, ',');

  unsigned short amount_haystack_lines = this->range_y - this->y_start;
  std::vector<std::string> haystack_lines(amount_haystack_lines);
  // index to enable lazy-loading: next line that has to be grabbed
  unsigned short index_haystack_line_empty = 0;

  unsigned short haystack_width = this->range_x - this->x_start;

  auto amount_needle_lines = static_cast<unsigned short>(needle_lines.size());
  auto needle_width = static_cast<unsigned short>(needle_lines[0].length());

  unsigned short last_possible_offset_per_haystack_line = haystack_width - needle_width;
  auto last_possible_occurrence_haystack_line =
      static_cast<unsigned short>(amount_haystack_lines - amount_needle_lines + 1);

  signed long offset_needle;

  unsigned short index_haystack_line;
  unsigned short index_needle_line;
  auto index_last_needle_line = static_cast<unsigned short>(needle_lines.size() - 1);

  auto *color = new XColor;

  std::string haystack_line_compare;
  std::string needle_compare;

  const char *needle_line;
  unsigned short index_haystack_line_compare;

  // Iterate over lines of haystack
  std::string haystack_line;
  for (index_haystack_line = 0; index_haystack_line < last_possible_occurrence_haystack_line; ++index_haystack_line) {
    FetchHaystackLine(haystack_lines,
                      index_haystack_line_empty,
                      index_haystack_line,
                      color,
                      haystack_line);
    offset_needle = static_cast<signed short>(haystack_line.find(needle_lines[0], 0));

    while (true) {
      // Look at all occurrences of 1st line of needle
      if (offset_needle==std::string::npos ||
          offset_needle > last_possible_offset_per_haystack_line ||
          index_haystack_line > last_possible_occurrence_haystack_line)
        break;

      if (amount_needle_lines==1) {
        XFree(image);
        return FormatCoordinate(offset_needle, index_haystack_line);
      }

      // Iterate down the lines of needle
      for (index_needle_line = 1; index_needle_line < amount_needle_lines; ++index_needle_line) {
        // Check whether haystack line contains resp. needle line at rel. offset
        needle_line = needle_lines[index_needle_line].c_str();

        index_haystack_line_compare = index_haystack_line + index_needle_line;
        if (index_haystack_line > last_possible_occurrence_haystack_line)
          break;

        FetchHaystackLine(haystack_lines,
                          index_haystack_line_empty,
                          index_haystack_line_compare,
                          color,
                          haystack_line_compare);
        needle_compare = haystack_line_compare.substr(static_cast<unsigned long>(offset_needle), needle_width);

        if (strcmp(needle_compare.c_str(), needle_line)!=0)
          break;
        else if (index_needle_line==index_last_needle_line) {
          // All lines of needle were found within haystack, return absolute coordinate
          XFree(image);
          return FormatCoordinate(offset_needle, index_haystack_line);
        }
        // Needle line found but there are more, check following haystack line
      }

      // Not all lines of needle were found at this offset in lines under occurrence of 1st needle line

      // Reached end of haystack line? Continue scanning in next haystack line
      if (offset_needle >= last_possible_offset_per_haystack_line) break;

      // Init next iteration: check lines under next occurrence of 1st needle line in current haystack line
      offset_needle = static_cast<unsigned short>(
          haystack_line.find(needle_lines[0], static_cast<unsigned long>(offset_needle + 1)));
    }
  }

  return "x=-1; y=-1;";
}

// Get line from bitmask haystack. this is lazy-loaded: initialize it via GetBitmaskLineFromImage if not yet
void PixelScanner::FetchHaystackLine(std::vector<std::string> &haystack_lines,
                                     unsigned short &index_empty_haystack_line,
                                     unsigned short index_haystack_line,
                                     XColor *color,
                                     std::string &haystack_line) {
  if (index_empty_haystack_line > index_haystack_line) {
    haystack_line = haystack_lines.at(index_haystack_line);
  } else {
    haystack_line = this->GetBitmaskLineFromImage(color, index_haystack_line);
    haystack_lines.at(index_haystack_line) = haystack_line;
    index_empty_haystack_line = static_cast<unsigned short>(index_haystack_line + 1);
  }
}

std::string PixelScanner::FormatCoordinate(signed long offset_needle, unsigned short index_haystack_line) const {
  return "cx=" + std::to_string(x_start + offset_needle - 1) +
      "; cy=" + std::to_string(y_start + index_haystack_line - 1) + ";\n";
}

} // namespace pixloc
