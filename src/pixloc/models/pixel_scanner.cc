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

// Constructor
PixelScanner::PixelScanner(Display *display,
                           unsigned short x_start, unsigned short y_start,
                           unsigned short range_x, unsigned short range_y,
                           unsigned short find_red, unsigned short find_green, unsigned short find_blue,
                           unsigned short tolerance) {
  this->display = display;
  this->color = new XColor();

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
int PixelScanner::ScanUniaxial(unsigned short amount_find, unsigned short step_size, bool trace) {
  auto *color = new XColor;

  unsigned short step_size_x, step_size_y;
  InitUniaxialStepSize(step_size, step_size_x, step_size_y);

  unsigned short amount_found = 0;
  for (unsigned short y = 0; y < range_y; y += step_size_y) {
    for (unsigned short x = 0; x < range_x; x += step_size_x) {
      color->pixel = XGetPixel(image, x, y);
      XQueryColor(display, DefaultColormap(display, DefaultScreen(display)), color);

      if (trace)
        std::cout << (color->red/256) << "," << (color->green/256) << "," << (color->blue/256) << "\n";
      else if (color_matcher->Matches(color->red, color->green, color->blue)) {
        if (step_size==1) {
          // Found matching pixel while scanning with frequency of 1 pixel
          ++amount_found;
          if (amount_found==amount_find) {
            XFree(image);
            return range_y==1 ? x : y;
          }
        } else {
          // Found 1 matching pixel while interval scanning, now scan directly neighbouring pixels
          signed short starting_value = GetStartingValueOfHomochromaticSetAtCoordinate(x, y, amount_find);
          if (starting_value > -1) {
            XFree(image);
            return starting_value;
          }
        }
      } else amount_found = 0;
    }
  }
  XFree(image);
  return -1;
}

signed short PixelScanner::GetStartingValueOfHomochromaticSetAtCoordinate(
    unsigned short x_start,
    unsigned short y_start,
    unsigned short amount_find
) {
  auto *color = new XColor;

  unsigned short topmost_matching_y = y_start;
  unsigned short leftmost_matching_x = x_start;

  unsigned short amount_found = 0;
  if (range_y > range_x) {
    // Scan vertical
    for (unsigned short offset_y = 1; offset_y < amount_found; ++offset_y) {
      // 1. Scan from starting y up, until y == 0 or sought amount was found or a not-matching pixel reached
      if (y_start - offset_y < 0) break;
      color->pixel = XGetPixel(image, x_start, y_start - offset_y);
      XQueryColor(display, DefaultColormap(display, DefaultScreen(display)), color);
      if (color_matcher->Matches(color->red, color->green, color->blue)) {
        topmost_matching_y = y_start - offset_y;
        ++amount_found;
        if (amount_found==amount_find) {
          XFree(image);
          return topmost_matching_y;
        }
      } else break;
    }
    for (unsigned short offset_y = 1; offset_y < amount_found; ++offset_y) {
      // 2. Scan from starting y down, until y >= range or sought amount was found or a not-matching pixel reached
      if (y_start + offset_y > range_y) break;
      color->pixel = XGetPixel(image, x_start, y_start + offset_y);
      XQueryColor(display, DefaultColormap(display, DefaultScreen(display)), color);
      if (color_matcher->Matches(color->red, color->green, color->blue)) {
        ++amount_found;
        if (amount_found==amount_find) {
          XFree(image);
          return topmost_matching_y;
        }
      } else break;
    }
  } else {
    // Scan horizontal
    for (unsigned short offset_x = 1; offset_x < amount_find; ++offset_x) {
      // 1. Scan from starting x to the left, until x == 0 or sought amount was found or a not-matching pixel reached
      if (x_start - offset_x < 0) break;
      color->pixel = XGetPixel(image, x_start - offset_x, y_start);
      XQueryColor(display, DefaultColormap(display, DefaultScreen(display)), color);
      if (color_matcher->Matches(color->red, color->green, color->blue)) {
        leftmost_matching_x = x_start - offset_x;
        ++amount_found;
        if (amount_found==amount_find) {
          XFree(image);
          return leftmost_matching_x;
        }
      } else break;
    }
    for (unsigned short offset_x = 1; offset_x < amount_found; ++offset_x) {
      // 2. Scan from starting x to the right, until x >= range or sought amount was found or a not-matching pixel reached
      if (x_start + offset_x > range_x) break;
      color->pixel = XGetPixel(image, x_start + offset_x, y_start);
      XQueryColor(display, DefaultColormap(display, DefaultScreen(display)), color);
      if (color_matcher->Matches(color->red, color->green, color->blue)) {
        ++amount_found;
        if (amount_found==amount_find) {
          XFree(image);
          return leftmost_matching_x;
        }
      } else break;
    }
  }

  return -1;
}

void PixelScanner::InitUniaxialStepSize(unsigned short step_size,
                                        unsigned short &step_size_x,
                                        unsigned short &step_size_y) const {
  if (range_x > 1) {
    step_size_y = 1;
    step_size_x = step_size;
  } else {
    step_size_x = 1;
    step_size_y = step_size;
  }
}

void PixelScanner::TraceMainColor() {
  auto *color = new XColor;

  std::vector<std::string> colors;

  for (unsigned short y = 0; y < range_y; ++y) {
    for (unsigned short x = 0; x < range_x; ++x) {
      color->pixel = XGetPixel(image, x, y);
      XQueryColor(display, DefaultColormap(display, DefaultScreen(display)), color);
      char rgb[12];
      sprintf(rgb, "%d,%d,%d", color->red/256, color->green/256, color->blue/256);
      colors.emplace_back(rgb);
    }
  }
  XFree(image);

  std::cout << helper::strings::FindMostCommon(colors);
}

void PixelScanner::TraceBitmask() {
  for (unsigned short y = 0; y < range_y; ++y) {
    std::cout << this->GetBitmaskLineFromImage(y) << (y < range_y - 1 ? "," : "") << "\n";
  }

  XFree(image);
}

std::string PixelScanner::GetBitmaskLineFromImage(unsigned short y) {
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

  unsigned short haystack_width = this->range_x - this->x_start;
  unsigned short amount_haystack_lines = this->range_y - this->y_start;
  std::vector<std::string> haystack_lines(amount_haystack_lines);
  // index to enable lazy-loading: next line that has to be grabbed
  unsigned short index_haystack_line_empty = 0;
  auto amount_needle_lines = static_cast<unsigned short>(needle_lines.size());
  auto needle_width = static_cast<unsigned short>(needle_lines[0].length());
  unsigned short last_possible_offset_per_haystack_line = haystack_width - needle_width;
  auto last_possible_occurrence_haystack_line =
      static_cast<unsigned short>(amount_haystack_lines - amount_needle_lines + 1);
  signed long offset_needle;
  unsigned short index_haystack_line;
  unsigned short index_needle_line;
  auto index_last_needle_line = static_cast<unsigned short>(needle_lines.size() - 1);

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
                      haystack_line);
    // Find 1st line of sought bitmask within current line of screenshot
    offset_needle = static_cast<signed short>(haystack_line.find(needle_lines[0], 0));

    while (true) {
      // Look at all occurrences of 1st line of needle
      if (offset_needle==std::string::npos ||
          offset_needle > last_possible_offset_per_haystack_line ||
          index_haystack_line > last_possible_occurrence_haystack_line)
        break;

      if (amount_needle_lines==1) {
        // Bitmask has only 1 line: found it
        XFree(image);
        return FormatCoordinate(offset_needle, index_haystack_line);
      }

      // Iterate down the lines of needle
      for (index_needle_line = 1; index_needle_line < amount_needle_lines; ++index_needle_line) {
        // Check whether following haystack line contains resp. needle line at rel. offset
        needle_line = needle_lines[index_needle_line].c_str();

        index_haystack_line_compare = index_haystack_line + index_needle_line;
        if (index_haystack_line > last_possible_occurrence_haystack_line)
          break;

        FetchHaystackLine(haystack_lines,
                          index_haystack_line_empty,
                          index_haystack_line_compare,
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
                                     std::string &haystack_line) {
  if (index_empty_haystack_line > index_haystack_line) {
    haystack_line = haystack_lines.at(index_haystack_line);
  } else {
    haystack_line = this->GetBitmaskLineFromImage(index_haystack_line);
    haystack_lines.at(index_haystack_line) = haystack_line;
    index_empty_haystack_line = static_cast<unsigned short>(index_haystack_line + 1);
  }
}

std::string PixelScanner::FormatCoordinate(signed long offset_needle, unsigned short index_haystack_line) const {
  return "x=" + std::to_string(x_start + offset_needle - 1) +
      "; y=" + std::to_string(y_start + index_haystack_line - 1) + ";\n";
}

} // namespace pixloc
