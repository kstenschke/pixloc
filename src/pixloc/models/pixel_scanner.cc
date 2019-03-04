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
#include "pixloc/helper/strings.h"

namespace pixloc {

/*
 * Constructor
 */
PixelScanner::PixelScanner(Display *display,
                         int x_start, int y_start,
                         unsigned int range_x, unsigned int range_y,
                         int find_red, int find_green, int find_blue,
                         int tolerance) {
  this->display = display;

  this->x_start = x_start;
  this->y_start = y_start;
  this->range_x = range_x;
  this->range_y = range_y;

  if (tolerance==0) {
    this->color_matcher = new ColorMatcherPrecise(find_red, find_green, find_blue);
  } else {
    this->color_matcher = new ColorMatcherRange(find_red, find_green, find_blue);
    this->color_matcher->SetTolerance(tolerance);
  }

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
int PixelScanner::ScanUniaxial(int amount_find, bool trace) {
  auto *color = new XColor;

  int red, green, blue;
  int amount_found = 0;

  for (int y = 0; y < range_y; y++) {
    for (int x = 0; x < range_x; x++) {
      color->pixel = XGetPixel(image, x, y);
      XQueryColor(display, DefaultColormap(display, DefaultScreen(display)), color);

      red = color->red/256;
      green = color->green/256;
      blue = color->blue/256;

      if (trace) std::cout << red << "," << green << "," << blue << "\n";

      if (color_matcher->Matches(red, green, blue)) {
        amount_found++;
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

void PixelScanner::TraceBitmask() {
  auto *color = new XColor;
  for (int y = 0; y < range_y; y++) {
    std::cout << this->GetBitmaskLineFromImage(display, color, y) << (y < range_y - 1 ? "," : "") << "\n";
  }

  XFree(image);
}

std::string PixelScanner::GetBitmaskLineFromImage(Display *display, XColor *color, int y) {
  std::string bitmask_haystack;
  for (int x = 0; x < this->range_x; x++) {
    color->pixel = XGetPixel(this->image, x, y);
    XQueryColor(display, DefaultColormap(display, DefaultScreen(display)), color);

    bitmask_haystack += this->color_matcher->Matches(
        color->red/256, color->green/256, color->blue/256)
                        ? '*'
                        : '_';
  }

  return bitmask_haystack;
}

// Find coordinate of bitmask sought-after
std::string PixelScanner::FindBitmask(const std::string &bitmask_needle) {
  std::vector<std::string> needle_lines = helper::strings::Explode(bitmask_needle, ',');

  unsigned long amount_haystack_lines = this->range_y - this->y_start;
  std::vector<std::string> haystack_lines(amount_haystack_lines);
  // index to enable lazy-loading: next line that has to be grabbed
  long index_haystack_line_empty = 0;

  unsigned long haystack_width = this->range_x - this->x_start;

  auto amount_needle_lines = needle_lines.size();
  auto needle_width = needle_lines[0].length();

  unsigned long last_possible_offset_per_haystack_line = haystack_width - needle_width;
  unsigned long last_possible_occurrence_haystack_line = amount_haystack_lines - amount_needle_lines + 1;
  unsigned long offset_needle;

  unsigned long index_haystack_line;
  unsigned long index_needle_line;
  auto index_last_needle_line = needle_lines.size() - 1;

  auto *color = new XColor;

  std::string haystack_line_compare;
  std::string needle_compare;

  // Iterate over lines of haystack
  std::string haystack_line;
  for (index_haystack_line = 0; index_haystack_line < last_possible_occurrence_haystack_line; index_haystack_line++) {
    FetchHaystackLine(haystack_lines,
                      index_haystack_line_empty,
                      index_haystack_line,
                      color,
                      haystack_line);
    offset_needle = haystack_line.find(needle_lines[0], 0);

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
      for (index_needle_line = 1; index_needle_line < amount_needle_lines; index_needle_line++) {
        // Check whether haystack line contains resp. needle line at rel. offset
        const char *needle_line = needle_lines[index_needle_line].c_str();

        unsigned long index_haystack_line_compare = index_haystack_line + index_needle_line;
        if (index_haystack_line > last_possible_occurrence_haystack_line)
          break;

        FetchHaystackLine(haystack_lines,
                          index_haystack_line_empty,
                          index_haystack_line_compare,
                          color,
                          haystack_line_compare);
        needle_compare = haystack_line_compare.substr(offset_needle, needle_width);

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
      offset_needle = haystack_line.find(needle_lines[0], offset_needle + 1);
    }
  }

  return "-1";
}

// Get line from bitmask haystack. this is lazy-loaded: initialize it via GetBitmaskLineFromImage if not yet
void PixelScanner::FetchHaystackLine(std::vector<std::string> &haystack_lines,
                                long &index_empy_haystack_line,
                                unsigned long index_haystack_line,
                                XColor *color,
                                std::string &haystack_line) {
  if (index_empy_haystack_line > index_haystack_line) {
    haystack_line = haystack_lines.at(index_haystack_line);
  } else {
    haystack_line = this->GetBitmaskLineFromImage(display, color, static_cast<int>(index_haystack_line));
    haystack_lines.at(index_haystack_line) = haystack_line;
    index_empy_haystack_line = index_haystack_line + 1;
  }
}

std::string PixelScanner::FormatCoordinate(unsigned long offset_needle, unsigned long index_haystack_line) const {
  return "cx=" + std::to_string(x_start + offset_needle - 1) +
      "; cy=" + std::to_string(y_start + index_haystack_line - 1) + ";\n";
}

} // namespace pixloc