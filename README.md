# levite

[Spreadsheet application] with a bizarre twist: Formulas have been replaced
with an [Forth] like [RPN] programming language and it has a nice terminal
user interface that [Vi] users should feel comfortable with.

[Spreadsheet application]: https://en.wikipedia.org/wiki/Spreadsheet
[Forth]: https://en.wikipedia.org/wiki/Forth_(programming_language)
[RPN]: https://en.wikipedia.org/wiki/Reverse_Polish_notation

## Screenshot

![Screenshot](https://raw.github.com/RauliL/levite/main/screenshot.png)

## Features

- Uses [GNU MPFR] for number crunching.
- Supports measurement units.
- Recognizes dates, times, months and days of week.
- All formulas are actually tiny [Laskin] programs.
- UI inspired by [VisiCalc] with [Vi] like keybindings.
- Loads and saves [CSV] data.

[GNU MPFR]: https://en.wikipedia.org/wiki/GNU_MPFR
[Laskin]: https://github.com/RauliL/laskin
[VisiCalc]: https://en.wikipedia.org/wiki/VisiCalc
[Vi]: https://en.wikipedia.org/wiki/Vi_(text_editor)
[CSV]: https://en.wikipedia.org/wiki/Comma-separated_values

## How to compile

You need [CMake] and C++17 capable compiler to do this. Does not work on
Windows yet, sorry.

```bash
$ git clone https://github.com/RauliL/levite.git
$ cd levite
$ mkdir build
$ cd build
$ cmake ..
$ make
```

[CMake]: https://www.cmake.org
