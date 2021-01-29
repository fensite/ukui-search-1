/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 */
/************************************
 * file enc : ascii
 * author   : wuyanyi09@gmail.com
 ************************************/

#ifndef LIMONP_ARGV_FUNCTS_H
#define LIMONP_ARGV_FUNCTS_H

#include <set>
#include <sstream>
#include "StringUtil.hpp"

namespace limonp {

using namespace std;

class ArgvContext {
 public :
  ArgvContext(int argc, const char* const * argv) {
    for(int i = 0; i < argc; i++) {
      if(StartsWith(argv[i], "-")) {
        if(i + 1 < argc && !StartsWith(argv[i + 1], "-")) {
          mpss_[argv[i]] = argv[i+1];
          i++;
        } else {
          sset_.insert(argv[i]);
        }
      } else {
        args_.push_back(argv[i]);
      }
    }
  }
  ~ArgvContext() {
  }

  friend ostream& operator << (ostream& os, const ArgvContext& args);
  string operator [](size_t i) const {
    if(i < args_.size()) {
      return args_[i];
    }
    return "";
  }
  string operator [](const string& key) const {
    map<string, string>::const_iterator it = mpss_.find(key);
    if(it != mpss_.end()) {
      return it->second;
    }
    return "";
  }

  bool HasKey(const string& key) const {
    if(mpss_.find(key) != mpss_.end() || sset_.find(key) != sset_.end()) {
      return true;
    }
    return false;
  }

 private:
  vector<string> args_;
  map<string, string> mpss_;
  set<string> sset_;
}; // class ArgvContext

inline ostream& operator << (ostream& os, const ArgvContext& args) {
  return os<<args.args_<<args.mpss_<<args.sset_;
}

} // namespace limonp

#endif
