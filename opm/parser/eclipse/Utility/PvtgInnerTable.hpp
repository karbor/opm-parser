// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*
  Copyright (C) 2014 by Andreas Lauser

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef OPM_PARSER_PVTG_INNER_TABLE_HPP
#define	OPM_PARSER_PVTG_INNER_TABLE_HPP

#include "SimpleTable.hpp"

namespace Opm {
    class PvtgInnerTable : protected SimpleTable {
        typedef SimpleTable ParentType;

    public:
        /*!
         * \brief Read the per record table of the PVTG keyword and
         *        provide some convenience methods for it.
         *
         * The first value of the record (-> Rv) is skipped.
         */
        PvtgInnerTable(Opm::DeckKeywordConstPtr keyword, int recordIdx = 0)
            : SimpleTable(keyword,
                          std::vector<std::string>{"P", "BG", "MUG"},
                          recordIdx, /*firstEntityOffset=*/1)
        {}

        int numRows() const
        { return ParentType::numRows(); };

        int numColumns() const
        { return ParentType::numColumns(); };

        const std::vector<double> &getPressureColumn() const
        { return ParentType::getColumn(0); }

        const std::vector<double> &getGasFormationFactorColumn() const
        { return ParentType::getColumn(1); }

        const std::vector<double> &getGasViscosityColumn() const
        { return ParentType::getColumn(2); }
    };
}

#endif	// OPM_PARSER_SIMPLE_TABLE_HPP

