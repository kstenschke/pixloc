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
    "\npixloc --mode \"find horizontal\" --from 0,60 --range 100 --color 188,188,188 --amount 8"
    "\npixloc --mode \"find horizontal\" --from mouse --range 100 --color 188,188,188 --amount 8"
    "\npixloc --mode \"find vertical\" --from 0,60 --range 100 --color 188,188,188 --amount 8"
    "\npixloc --mode \"find bitmask\" --from 0,60 --range 128,32 --color 188,188,188 --bitmask *__,**_,***,**_,*__"
    "\n\nsee https://github.com/kstenschke/pixloc for more detailed information\n\n";

static const char *const kModeNameFindBitmask = "find bitmask";
static const char *const kModeNameFindConsecutiveHorizontal = "find horizontal";
static const char *const kModeNameFindConsecutiveVertical = "find vertical";
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

unsigned short GetModeIdFromName(const std::string &mode);

bool IsTupelRangeMode(int mode_id);
bool IsHorizontalMode(int mode_id);
bool IsTraceMode(int mode_id);
bool IsBitmaskMode(int mode_id);

bool IsValidNumericTupel(std::string &str);
bool IsValidColor(const std::string &color);
void ValidateBitmask(const std::string &bitmask_px, int range_width, int range_height);
bool IsValidRangeForMode(int mode_id, const std::string &range);

bool ModeRequiresAmountPx(int mode_id);
bool ModeRequiresBitmask(int mode_id);
bool ModeRequiresColor(int mode_id);

void ResolveNumericTupel(const std::string &str, int &number_1, int &number_2);
void ResolveScanningRange(int mode_id, const std::string &range, int &number_1, int &number_2);
void ResolveRgbColor(const std::string &color, int &red, int &green, int &blue);

} // namespace clioptions
} // namespace pixloc

#endif
