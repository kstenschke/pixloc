# pixloc

pixloc is a Linux commandline tool for live-analysis and recognition of pixels currently displayed using the X Window System.

pixloc is implemented in C++ and depends only on X11.


## Table of contents

* [Why?](#why)
* [Usage](#Usage)
  * [Options](#options)
    * [Tolerance Option: Matching within a color range](#tolerance-option-matching-within-a-color-range)
    * [Mouse Option: Using a dynamic coordinate while authoring](#mouse-option-using-a-dynamic-coordinate-while-authoring)
  * [Modes](#modes)
* [Usage examples](#usage-examples)
  * [Find a set of consecutive homochromatic pixels](#find-a-set-of-consecutive-homochromatic-pixels)
  * [Find a 1-bit pixel bitmask within a specified screen area](#find-a-1-bit-pixel-bitmask-within-a-specified-screen-area)
  * [Trick: Defining variables from found bitmask coordinate](#trick-defining-variables-from-found-bitmask-coordinate)
  * [Color tracing](#color-tracing)
  * [Bitmask tracing](#bitmask-tracing)
* [Building from source](#building-from-source)
* [Code Convention](#code-convention)
* [Third party references](#third-party-references)
* [Author and license](#author-and-license)


## Why?

pixloc's pixel recognition allows to easily and fast locate specific elements that are currently displayed on the screen.
Other programs can than be used to analyse and/or trigger user interaction (e.g. mouse clicks) onto the GUI
of applications that otherwise lack an API for such interaction.


## Usage

Usage is: 

```bash
pixloc options
```

### Options

| Option         | Description                                            | Value                                      |
|----------------|--------------------------------------------------------|--------------------------------------------|
|-m, --mode      | Mode of tracing or locating pixels by color            | See details under [Modes](#modes)          |
|-f, --from      | Starting coordinate                                    | X or y value or x,y coordinate. Or "mouse" |
|-r, --range     | Amount of pixels to be scanned                         | Number                                     |
|-c, --color     | RGB color value to find                                | Red,green,blue (decimal) values            |
|-a, --amount    | Amount of consecutive pixels of given color to find    | Number                                     |
|-b, --bitmask   | Pixel mask (* = given color, _ = other colors) to find | Bitmask, * = given color, _ = other colors |
|-t, --tolerance | Optional: Color tolerance amount                       | Number                                     |
|-s, --step      | Optional: Interval step size for non-bitmask modes     | Number                                     |
|-?, -h, --help  | Display usage information                              | -                                          |


#### Tolerance Option: Matching within a color range

With the optional *tolerance* argument, matching pixels are detected within a color range.
By default, a zero-tolerance is used.

Example: Looking for pixels of RGB color 150,10,200 with a tolerance of 5, will match pixels within the following RGB value range:

* Red: Between 145 and 155
* Green: Between 5 and 15
* Blue: Between 195 and 205

#### Mouse Option: Using a dynamic coordinate while authoring

At the time of writing and testing scripts that use pixloc, it is helpful to have pixloc find out the current 
mouse position. Instead of supplying a coordinate, the *from* option accepts *"mouse"* as its value,
the *from* coordinate is than initialized from the current mouse position, trace-modes also output the coordinate
of the mouse position used. This way, finding out the optimal coordinates for pixloc to scan from, can be tested and 
tweaked very conveniently from the commandline.


### Modes

| Mode               | Description                                                                                 |
|--------------------|---------------------------------------------------------------------------------------------|
| "find horizontal"  | Locates given amount of consecutive pixels of given color, to the right of given coordinate |
| "find vertical"    | Locates given amount of consecutive pixels of given color, under given coordinate           |
| "find bitmask"     | Locates given 1-bit bitmask within given screen rectangle, filtered by given color          |
| "trace horizontal" | Traces pixel colors from given coordinate to the right                                      |
| "trace vertical"   | Traces pixel colors from given coordinate down                                              |
| "trace bitmask"    | Traces 1-bit bitmask, generated from pixels of given color vs. other colors                 |
| "trace main color" | Traces the most prominent pixel color in the given screen rectangle                         |
| "trace mouse"      | Traces the coordinate of the current mouse position                                         |


## Usage examples

### Find a set of consecutive homochromatic pixels

Useful for finding the highlighted component in a list of elements, e.g. a selected tab, list item, etc.

```bash
pixloc --mode "find horizontal" --from 1,60 --range 100 --color 188,188,188 --amount 8
```

or 

```bash
pixloc --mode "find vertical" --from 1,60 --range 100 --color 188,188,188 --amount 8
```

Scans pixels starting from the specified coordinate, iterating on the x or y axis to the right or down, and outputs the 
offset (x or y value) of the first found occurrence of a set of consecutive pixels of the specified color. 
If no such consecutive homochromatic set of pixels is found, the output is 
```x=-1;``` / ```y=-1;```.


#### Faster uniaxial scanning with (optional) *step* argument

```bash
pixloc -m "find vertical" -f 1,1 -r 100 -c 188,188,188 -a 4 -s 12
```

Scans pixels starting from 1,1 down, iterating in steps of 12 pixels.
From each found matching pixel, pixloc than scans the directly neighbouring pixels up and down from that coordinate,
checking for a homochromatic set of the given color, spanning the given amount of pixels.
The topmost y value of the color (or color range) sought after, is being output. 


### Find a 1-bit pixel bitmask within a specified screen area

Useful for locating more visually complex elements, e.g. an icon, label or similar.

```bash
pixloc --mode "find bitmask" --from 1,60 --range 128,32 --color 188,188,188 --bitmask *__,**_,***,**_,*__
```

Scans the pixels within the given rectangle (in the example from 1,60 to 129,92), 
recognizing all pixels with RGB color value 188,188,188 as foreground pixels (= *), all other colors as 
background (= _) and outputs the coordinate where the given bitmask (in the above example of a 3x5 pixel dimension)
is found. The coordinate is output like ``x=320; y=210``, to make it easily [evaluable in shell scripts](#trick-defining-variables-from-found-bitmask-coordinate).
If the given pixel mask is not found, the output is ``x=-1; y=-1``.

The optional color tolerance option makes it easier to locate bitmasks including antialias pixels, whose colors can vary:

```bash
pixloc -m "find bitmask" -f 1,60 -r 128,32 -c 188,188,188 -b *__,**_,***,**_,*__ -t 50
```


### Trick: Defining variables from found bitmask coordinate 

A found coordinate is output like for example:
   
```bash
x=100; y=200;
```

This can be evaluated when used within shell scripts, like follows:

```bash
eval $(pixloc -m "find bitmask" -f 930,166 -r 12,800 -c 49,135,219 -b *______*,________,***__***)
 ```
 
So that the variables *x* and *y* will be defined from the coordinate that is output by pixloc. 


### Color tracing

```bash
pixloc --mode "trace horizontal" --from 1,60 --range 100
```

or

```bash
pixloc --mode "trace vertical" --from 1,60 --range 100
```

Outputs the RGB color values of pixels starting from the specified coordinate,
iterating horizontally to the right or vertically down.


#### Detecting most prominent color

```bash
pixloc --mode "trace main color" --from 10,10 --range 16,16
```

Outputs the RGB value of the most prominent color in the screen rectangle from 10,10 to 26,26.


#### Using current mouse position as starting coordinate to scan from

```bash
pixloc --mode "trace vertical" --from mouse --range 100
```

Outputs the current mouse position, followed by the RGB color values of pixels starting 
from the current mouse position, iterating vertically down.


### Bitmask tracing

```bash
pixloc --mode "trace bitmask" --from 1,60 --range 8,8 --color 188,188,188
```

Outputs a 1-bit color bitmask of the pixels in a given range, starting from a given coordinate.
All pixels having the given RGB color (or if providing a tolerance value: are within that color range)
are represented by * characters, all other color values by _ characters, rows are separated by commas, followed by newlines.

Tracing an area that displays an "o" character, would result in an output like:

```bash
________,
________,
__***___,
_*___*__,
_*___*__,
__***___,
________,
________
```


## Building from source

```bash
cmake CMakeLists.txt; make
```

## Code Convention

The source code of pixloc follows the Google C++ Style Guide,
see: https://google.github.io/styleguide/cppguide.html


## Bug Reporting and Feature Requests

If you find a bug or have an enhancement request, please file an issue on the github repository.


## Third party references

pixloc uses [Clara](https://github.com/catchorg/Clara) (licensed under the [Boost Software License 1.0.](https://www.boost.org/users/license.html)) for parsing command line options.


## Author and License

Written by Kay Stenschke.
pixloc is licensed under the [New BSD License](http://opensource.org/licenses/BSD-3-Clause)

> Copyright (c), Kay Stenschke
> All rights reserved.
>
> Redistribution and use in source and binary forms, with or without modification,
> are permitted provided that the following conditions are met:
>
> 1. Redistributions of source code must retain the above copyright notice, this
>    list of conditions and the following disclaimer.
>
> 2. Redistributions in binary form must reproduce the above copyright notice,
>    this list of conditions and the following disclaimer in the documentation
>    and/or other materials provided with the distribution.
>
> 3. Neither the name of the copyright holder nor the names of its contributors
>    may be used to endorse or promote products derived from this software without
>    specific prior written permission.
>
> THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
> ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
> WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
> DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
> ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
> (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
>  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
> ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
> (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
> SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
