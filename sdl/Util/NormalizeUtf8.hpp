// Copyright 2014-2015 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/** \file

    NFC, Windows-1252, corrupt utf8 handling, ascii control char removal
*/

#ifndef NORMALIZEUTF8_JG_2014_02_15_HPP
#define NORMALIZEUTF8_JG_2014_02_15_HPP
#pragma once

#include <sdl/Util/Chomp.hpp>
#include <sdl/Util/Nfc.hpp>
#include <sdl/Util/Utf8.hpp>
#include <iostream>

namespace sdl {
namespace Util {

/// FixUnicode deals in 1:1 Unicode subst and deletions; Nfc is many:many
/// (they're logically related, but differ in implementation)
struct NormalizeUtf8 : NfcOptions, FixUnicode {

  NormalizeUtf8() {}
  NormalizeUtf8(bool makeNfc) : NfcOptions(makeNfc) {}

  template <class Config>
  void configure(Config& config) {
    NfcOptions::configure(config);
    FixUnicode::configure(config);
    config.is("NormalizeUtf8");
  }

  /// normalize: also does FixUnicode
  void normalize(std::string& str) const;

  /// does all but FixUnicode. you may want this if you handle FixUnicode yourself e.g. StringToTokens
  void normalizeNfc(std::string& str, Constraints& c) const;

  void normalizeNfc(std::string& str) const;
  void normalize(std::string& str, Constraints& c) const;

  void normalize(std::string const& str, std::string& out) const;

  bool getlineNormalized(std::istream& in, std::string& utf8) const {
    if ((bool)std::getline(in, utf8)) {
      normalize(utf8);
      return true;
    } else
      return false;
  }
};


template <class StringConsumer>
inline std::size_t visitChompedLines(std::istream& in, StringConsumer const& consumer,
                                     NormalizeUtf8 const& opt = NormalizeUtf8()) {
  std::string line;
  std::size_t nlines = 0;
  while (opt.getlineNormalized(in, line)) {
    chomp(line);
    ++nlines;
    SDL_TRACE(Util.visitChompedLinesUntil, ":" << nlines << ": " << line);
    consumer(line);
  }
  return nlines;
}

/// consumer(line) for all lines from in until eof or separatorLine (which is consumed off in)
template <class StringConsumer>
inline std::size_t visitChompedLinesUntil(std::istream& in, std::string const& separatorLine,
                                          StringConsumer const& consumer,
                                          NormalizeUtf8 const& opt = NormalizeUtf8()) {
  std::string line;
  std::size_t nlines = 0;
  while (opt.getlineNormalized(in, line)) {
    chomp(line);
    if (line == separatorLine) break;
    ++nlines;
    SDL_TRACE(Util.visitChompedLinesUntil, "until " << separatorLine << ":" << nlines << ": " << line);
    consumer(line);
  }
  return nlines;
}


}}

#endif
