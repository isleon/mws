/*

Copyright (C) 2010-2015 KWARC Group <kwarc.info>

This file is part of MathWebSearch.

MathWebSearch is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MathWebSearch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MathWebSearch.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef _GENERICANSWER_HPP
#define _GENERICANSWER_HPP

/**
  * @author Radu Hambasan
  * @date 30 Dec 2014
  */

namespace mws {

/**
  * @brief Generic interface used to answer queries
  *
  */
struct GenericAnswer {
    virtual ~GenericAnswer() {}
};

}  // namespace mws

#endif  // _GENERICANSWER_HPP
