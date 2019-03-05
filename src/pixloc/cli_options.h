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

#ifndef CLASS_PIXLOC_CLI
#define CLASS_PIXLOC_CLI

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstdio>
#include <cstring>
#include <iostream>

namespace pixloc {
namespace clioptions {

static const char *const kUsageExamples = "\nusage examples:"
    "\npixloc --mode \"trace horizontal\" --from 0,60 --range 100"
    "\npixloc --mode \"trace vertical\" --from 0,60 --range 100"
    "\npixloc --mode \"trace bitmask\" --from 0,60 --range 64,64 --color 188,188,188"
    "\npixloc --mode \"trace main color\" --from 0,60 --range 64,64"
    "\npixloc --mode \"trace mouse\""
    "\npixloc --mode \"find color horizontal\" --from 0,60 --range 100 --color 188,188,188 --amount 8"
    "\npixloc --mode \"find color horizontal\" --from mouse --range 100 --color 188,188,188 --amount 8"
    "\npixloc --mode \"find color vertical\" --from 0,60 --range 100 --color 188,188,188 --amount 8"
    "\npixloc --mode \"find bitmask\" --from 0,60 --range 128,32 --color 188,188,188 --bitmask *__,**_,***,**_,*__"
    "\n\nsee https://github.com/kstenschke/pixloc for more detailed information\n\n";

static const char *const kModeNameFindBitmask = "find bitmask";
static const char *const kModeNameFindConsecutiveHorizontal = "find color horizontal";
static const char *const kModeNameFindConsecutiveVertical = "find color vertical";
static const char *const kModeNameTraceBitmask = "trace bitmask";
static const char *const kModeNameTraceHorizontal = "trace horizontal";
static const char *const kModeNameTraceMainColor = "trace main color";
static const char *const kModeNameTraceMouse = "trace mouse";
static const char *const kModeNameTraceVertical = "trace vertical";

static const int kModeIdFindBitmask = 1;
static const int kModeIdFindConsecutiveHorizontal = 2;
static const int kModeIdFindConsecutiveVertical = 3;
static const int kModeIdTraceBitmask = 4;
static const int kModeIdTraceHorizontal = 5;
static const int kModeIdTraceMainColor = 6;
static const int kModeIdTraceMouse = 7;
static const int kModeIdTraceVertical = 8;

int get_mode_id_from_name(const std::string &mode);

bool is_tupel_range_mode(int mode_id);
bool is_horizontal_mode(int mode_id);
bool is_trace_mode(int mode_id);
bool is_bitmask_mode(int mode_id);

bool is_valid_numeric_tupel(std::string &str);
bool is_valid_color(const std::string &color);
bool is_valid_bitmask(const std::string &bitmask_px, int min_width, int min_height);
bool is_valid_range_for_mode(int mode_id, const std::string &range);

bool mode_requires_amount_px(int mode_id);
bool mode_requires_bitmask(int mode_id);
bool mode_requires_color(int mode_id);

bool resolve_numeric_tupel(const std::string &str, int &number_1, int &number_2);
bool resolve_scanning_range(int mode_id, const std::string &range, int &number_1, int &number_2);
bool resolve_rgb_color(const std::string &color, int &red, int &green, int &blue);

} // namespace clioptions
} // namespace pixloc

#endif
