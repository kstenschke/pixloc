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

#include <X11/Xlib.h>
#include <cstdio>
#include <cstring>
#include <iostream>

#include "config.h"
#include "external/clara.hpp"
#include "pixloc/helper/helper_strings.h"
#include "cli_options.h"
#include "pixloc/models/pixel_scanner.h"

using namespace clara;

int print_error_and_usage_examples(const Parser &parser, const char *message);

/**
 * @param argc Amount of arguments received
 * @param argv Array of arguments received, argv[0] is name and path of executable
 */
int main(int argc, char **argv) {
  std::string mode;
  std::string from;
  std::string range;
  std::string color;
  std::string amount;
  std::string bitmask;
  std::string tolerance;

  bool show_help = false;

  // Use clara cli options parser
  auto parser =
      Opt(mode, "mode")["-m"]["--mode"]("see usage examples for available modes").required() |
          Opt(from, "from")["-f"]["--from"]("starting coordinate").required() |
          Opt(range, "range")["-r"]["--range"]("amount of pixels to be scanned").required() |
          Opt(color, "color")["-c"]["--color"]("rgb color value to find").optional() |
          Opt(amount, "amount")["-a"]["--amount"]("amount of consecutive pixels of given color to find").optional() |
          Opt(bitmask,
              "bitmask")["-b"]["--bitmask"]("pixel mask to find (* = given color, _ = other colors)").optional() |
          Opt(tolerance, "tolerance")["-t"]["--tolerance"]("optional: color tolerance").optional() |
          Help(show_help);
  auto result = parser.parse(Args(argc, reinterpret_cast<const char *const *>(argv)));
  if (!result) {
    std::cerr << "Error in command line: " << result.errorMessage() << std::endl;
    return 1;
  }

  if (show_help) {
    std::cout << "pixloc version " <<
              Pixloc_VERSION_MAJOR << "." << Pixloc_VERSION_MINOR << "\n"
                  "Copyright (c) 2019 Kay Stenschke\n\n";
    parser.writeToStream(std::cout);
    std::cout << pixloc::clioptions::kUsageExamples;
    return 0;
  }

  Display *display = XOpenDisplay(nullptr);
  if (!display) {
    std::cerr << "Error: Failed to open default display.\n";
    return -1;
  }

  // Resolve options
  int mode_id = -1,
      from_x = -1, from_y = -1,
      mouse_x = -1, mouse_y = -1,
      amount_px = 1,
      range_x = -1, range_y = -1,
      red = -1, green = -1, blue = -1, color_tolerance = 0;

  if (mode.empty() || (mode_id = pixloc::clioptions::get_mode_id_from_name(mode))==-1)
    return print_error_and_usage_examples(parser, "Valid mode is required.");

  bool is_trace_mode = pixloc::clioptions::is_trace_mode(mode_id);

  bool use_mouse_for_from = strcmp(from.c_str(), "mouse")==0;
  if (use_mouse_for_from || mode_id==pixloc::clioptions::kModeIdTraceMouse) {
    // Get current mouse position
    XEvent event{};
    XQueryPointer(display, RootWindow(display, DefaultScreen(display)),
                  &event.xbutton.root, &event.xbutton.window,
                  &event.xbutton.x_root, &event.xbutton.y_root,
                  &event.xbutton.x, &event.xbutton.y,
                  &event.xbutton.state);
    mouse_x = event.xbutton.x;
    mouse_y = event.xbutton.y;

    if (is_trace_mode) {
      printf("x=%d; y=%d;\n", event.xbutton.x, event.xbutton.y);
      if (mode_id==pixloc::clioptions::kModeIdTraceMouse) return 0;
    }
  }

  if (use_mouse_for_from) {
    from_x = mouse_x;
    from_y = mouse_y;
  } else if (!pixloc::clioptions::resolve_numeric_tupel(from, from_x, from_y))
    return print_error_and_usage_examples(parser, "Valid from coordinate is required.");

  if ((from_x==-1 || from_y==-1)
      || !pixloc::clioptions::resolve_scanning_range(mode_id, range, range_x, range_y))
    return print_error_and_usage_examples(parser, "Valid scanning range is required.");
  // @TODO validate from_x + range_x and from_y + range_y against available display dimension

  if (pixloc::clioptions::mode_requires_amount_px(mode_id) && (amount_px = helper::strings::ToInt(amount, 0))==0)
    return print_error_and_usage_examples(parser, "Valid amount of pixels to find is required.");

  bool is_bitmask_mode = pixloc::clioptions::is_bitmask_mode(mode_id);
  if (pixloc::clioptions::mode_requires_bitmask(mode_id) && !pixloc::clioptions::is_valid_bitmask(bitmask, range_x, range_y))
    return print_error_and_usage_examples(parser, "Valid bitmask to find is required.");

  if (pixloc::clioptions::mode_requires_color(mode_id) && !pixloc::clioptions::resolve_rgb_color(color, red, green, blue))
    return print_error_and_usage_examples(parser,
                                          is_bitmask_mode
                                          ? "Valid color to filter bitmask to find is required."
                                          : "Valid color to find is required.");

  if (!tolerance.empty()) {
    if (!helper::strings::IsNumeric(tolerance)) return print_error_and_usage_examples(parser, "Invalid color tolerance value given.");
    color_tolerance = helper::strings::ToInt(tolerance, 0);
  }

  // Scan pixels
  auto *scanner = new pixloc::PixelScanner(
      display,
      static_cast<unsigned short>(from_x), static_cast<unsigned short>(from_y),
      static_cast<unsigned short>(range_x), static_cast<unsigned short>(range_y),
      static_cast<unsigned short>(red * 256),
      static_cast<unsigned short>(green * 256),
      static_cast<unsigned short>(blue * 256),
      static_cast<unsigned short>(color_tolerance * 256));

  if (mode_id == pixloc::clioptions::kModeIdTraceMainColor) {
    scanner->TraceMainColor();
    return 0;
  }

  if (is_bitmask_mode) {
    if (is_trace_mode) {
      scanner->TraceBitmask();
      return 0;
    }

    std::cout << scanner->FindBitmask(bitmask);
    return 0;
  }

  int location = scanner->ScanUniaxial(amount_px, is_trace_mode);

  if (!is_trace_mode) std::cout << (range_y < 2 ? "x:" : "y:" ) << location << ";";
  return 0;
}

int print_error_and_usage_examples(const Parser &parser, const char *message) {
  std::cerr << "Error: " << message << "\n\n";
  parser.writeToStream(std::cerr);
  std::cerr << pixloc::clioptions::kUsageExamples;

  return -1;
}