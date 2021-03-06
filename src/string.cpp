//------------------------------------------------------------------------------
/// \file  string.cpp
//------------------------------------------------------------------------------
/// \brief Implementation of general purpose functions for string processing
//------------------------------------------------------------------------------
// Copyright (c) 2010 Serge Aleynikov <saleyn@gmail.com>
// Created: 2010-05-06
//------------------------------------------------------------------------------
/*
***** BEGIN LICENSE BLOCK *****

This file is part of the utxx open-source project.

Copyright (C) 2010 Serge Aleynikov <saleyn@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

***** END LICENSE BLOCK *****
*/

#include <utxx/string.hpp>
#include <utxx/print_opts.hpp>

namespace utxx {

std::string to_bin_string(const char* buf, size_t sz,
                          bool hex, bool readable, bool eol)
{
    std::stringstream out;
    const char* begin = buf, *end = buf + sz;
    print_opts  opts  = hex
                      ? (readable ? print_opts::printable_or_hex : print_opts::hex)
                      : (readable ? print_opts::printable_or_dec : print_opts::dec);
    output(out, begin, end, opts, ",", "", "\"", "<<", ">>");
    if (eol) out << std::endl;
    return out.str();
}

bool wildcard_match(const char* a_input, const char* a_pattern)
{
    const char* ip = nullptr, *pp = nullptr;

    while (*a_input && *a_pattern != '*') {
        if ((*a_pattern != *a_input) && (*a_pattern != '?'))
            return false;
        a_pattern++;
        a_input++;
    }

    while (*a_input) {
        if (*a_pattern == '*') {
            if (!*++a_pattern)
                return true;
            pp = a_pattern;
            ip = a_input+1;
        } else if ((*a_pattern == *a_input) || (*a_pattern == '?')) {
            a_pattern++;
            a_input++;
        } else {
            a_pattern = pp;
            a_input = ip++;
        }
    }

    while (*a_pattern == '*') a_pattern++;

    return !*a_pattern;
}

} // namespace utxx
