/*
 * Copyright (c) 2025, Rauli Laine
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <cstring>

#include "./sheet.hpp"
#include "./termbox2.h"

void handle_event(struct sheet& sheet);
void render(struct sheet& sheet);

static void
print_usage(std::ostream& output, const char* executable_name)
{
  output << std::endl
         << "Usage: "
         << executable_name
         << " [switches] [filename]"
         << std::endl
         << "  -s separator      Separator character to use. (Default `,')"
         << std::endl
         << "  --version         Print the version."
         << std::endl
         << "  --help            Display this message."
         << std::endl
         << std::endl;
}

static void
parse_args(struct sheet& sheet, int argc, char** argv)
{
  int offset = 1;

  while (offset < argc)
  {
    auto arg = argv[offset++];

    if (!*arg)
    {
      continue;
    }
    else if (*arg != '-')
    {
      sheet.filename = arg;
      break;
    }
    else if (!arg[1])
    {
      break;
    }
    else if (arg[1] == '-')
    {
      if (!std::strcmp(arg, "--help"))
      {
        print_usage(std::cout, argv[0]);
        std::exit(EXIT_SUCCESS);
      }
      else if (!std::strcmp(arg, "--version"))
      {
        std::cerr << "Levite 1.0.0" << std::endl;
        std::exit(EXIT_SUCCESS);
      } else {
        std::cerr << "Unrecognized switch: " << arg << std::endl;
        print_usage(std::cerr, argv[0]);
        std::exit(EXIT_FAILURE);
      }
    }
    for (int i = 1; arg[i]; ++i)
    {
      switch (arg[i])
      {
        case 's':
          if (offset < argc)
          {
            const auto separator = argv[offset++];

            if (std::strlen(separator) != 1)
            {
              std::cerr << "Separator must be a single character." << std::endl;
              print_usage(std::cerr, argv[0]);
              std::exit(EXIT_FAILURE);
            }
            sheet.separator = separator[0];
          } else {
            std::cerr << "Argument expected for the -s option." << std::endl;
            print_usage(std::cerr, argv[0]);
            std::exit(EXIT_FAILURE);
          }
          break;

        case 'h':
          print_usage(std::cout, argv[0]);
          std::exit(EXIT_SUCCESS);
          break;

        default:
          std::cerr << "Unrecognized switch: `" << arg[i] << "'" << std::endl;
          std::exit(EXIT_FAILURE);
          break;
      }
    }
  }

  if (offset < argc)
  {
    std::cerr << "Too many arguments given." << std::endl;
    print_usage(std::cerr, argv[0]);
    std::exit(EXIT_FAILURE);
  }
}

int
main(int argc, char** argv)
{
  struct sheet sheet;

  parse_args(sheet, argc, argv);
  if (sheet.filename)
  {
    sheet.load(*sheet.filename, sheet.separator);
  }
  tb_init();
  tb_hide_cursor();
  for (;;)
  {
    render(sheet);
    handle_event(sheet);
  }

  return EXIT_SUCCESS;
}
