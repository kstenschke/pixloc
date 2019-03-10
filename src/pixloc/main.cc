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

  // Use clara CLI options parser
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
  auto clara_result = parser.parse(Args(argc, reinterpret_cast<const char *const *>(argv)));
  if (!clara_result) {
    std::cerr << "Error in command line: " << clara_result.errorMessage() << std::endl;
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

  // Resolve options
  Display *display;

  unsigned short mode_id, amount_px = 1, color_tolerance = 0;

  int from_x = -1, from_y = -1,
      range_x = -1, range_y = -1,
      red = -1, green = -1, blue = -1;

  bool is_bitmask_mode, is_trace_mode;

  try {
    display = XOpenDisplay(nullptr);
    if (!display) throw "Failed to open default display.\n";

    mode_id = pixloc::clioptions::GetModeIdFromName(mode);
    is_trace_mode = pixloc::clioptions::IsTraceMode(mode_id);

    bool use_mouse_for_from = strcmp(from.c_str(), "mouse")==0;
    if (use_mouse_for_from || mode_id==pixloc::clioptions::kModeIdTraceMouse) {
      // Get current mouse position
      XEvent event{};
      XQueryPointer(display, RootWindow(display, DefaultScreen(display)),
                    &event.xbutton.root, &event.xbutton.window,
                    &event.xbutton.x_root, &event.xbutton.y_root,
                    &from_x, &from_y,
                    &event.xbutton.state);
      if (is_trace_mode) {
        printf("x=%d; y=%d;\n", from_x, from_y);
        if (mode_id==pixloc::clioptions::kModeIdTraceMouse) return 0;
      }
    }

    if (!use_mouse_for_from) pixloc::clioptions::ResolveNumericTupel(from, from_x, from_y);
    if ((from_x != -1 && from_y != -1)) pixloc::clioptions::ResolveScanningRange(mode_id, range, range_x, range_y);

    // @TODO validate from_x + range_x and from_y + range_y against available display dimension

    if (pixloc::clioptions::ModeRequiresAmountPx(mode_id) &&
        (amount_px = static_cast<unsigned short>(helper::strings::ToInt(amount, 0)))==0)
      throw "Valid amount of pixels to find is required.";

    is_bitmask_mode = pixloc::clioptions::IsBitmaskMode(mode_id);
    if (pixloc::clioptions::ModeRequiresBitmask(mode_id))
      pixloc::clioptions::ValidateBitmask(bitmask, range_x, range_y);

    if (pixloc::clioptions::ModeRequiresColor(mode_id)) pixloc::clioptions::ResolveRgbColor(color, red, green, blue);

    if (!tolerance.empty()) {
      if (!helper::strings::IsNumeric(tolerance)) throw "Invalid color tolerance value given.";
      color_tolerance = static_cast<unsigned short>(helper::strings::ToInt(tolerance, 0));
    }
  } catch (char const *exception) {
    std::cerr << "Error: " << exception << "\nFor help run: pixloc -h\n\n";
    return -1;
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
  } else if (is_bitmask_mode) {
    if (is_trace_mode) scanner->TraceBitmask();
    else std::cout << scanner->FindBitmask(bitmask);
  } else {
    int location = scanner->ScanUniaxial(amount_px, is_trace_mode);
    if (!is_trace_mode) std::cout << (range_y < 2 ? "x:" : "y:" ) << location << ";";
  }

  return 0;
}