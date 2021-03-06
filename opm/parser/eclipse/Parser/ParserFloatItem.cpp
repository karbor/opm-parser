/*
 Copyright 2013 Statoil ASA.

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

#include <opm/json/JsonObject.hpp>

#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserFloatItem.hpp>
#include <opm/parser/eclipse/Parser/ParserEnums.hpp>

#include <opm/parser/eclipse/RawDeck/RawRecord.hpp>
#include <opm/parser/eclipse/RawDeck/StarToken.hpp>
#include <opm/parser/eclipse/Deck/DeckFloatItem.hpp>

namespace Opm
{

    ParserFloatItem::ParserFloatItem(const std::string& itemName,
            ParserItemSizeEnum sizeType) :
            ParserItem(itemName, sizeType)
    {
        m_default = defaultFloat();
    }

    ParserFloatItem::ParserFloatItem(const std::string& itemName) : ParserItem(itemName)
    {
        m_default = defaultFloat();
    }


    ParserFloatItem::ParserFloatItem(const std::string& itemName, float defaultValue) : ParserItem(itemName)
    {
        setDefault( defaultValue );
    }


    ParserFloatItem::ParserFloatItem(const std::string& itemName, ParserItemSizeEnum sizeType, float defaultValue) : ParserItem(itemName, sizeType)
    {
        setDefault( defaultValue );
    }


    float ParserFloatItem::getDefault() const {
        return m_default;
    }


    void ParserFloatItem::setDefault(float defaultValue) {
        m_default = defaultValue;
        m_defaultSet = true;
    }



    ParserFloatItem::ParserFloatItem(const Json::JsonObject& jsonConfig) :
            ParserItem(jsonConfig)
    {
        if (jsonConfig.has_item("default"))
            setDefault( jsonConfig.get_double("default"));
        else
            m_default = defaultFloat();
    }


    bool ParserFloatItem::equal(const ParserItem& other) const
    {
        // cast to a pointer to avoid bad_cast exception
        const ParserFloatItem* rhs = dynamic_cast<const ParserFloatItem*>(&other);
        if (rhs && ParserItem::equal(other) && (getDefault() == rhs->getDefault())) {
            return equalDimensions( other );
        }
        else
            return false;
    }


    bool ParserFloatItem::equalDimensions(const ParserItem& other) const {
        bool equal=false;
        if (other.numDimensions() == numDimensions()) {
            equal = true;
            for (size_t idim=0; idim < numDimensions(); idim++) {
                if (other.getDimension(idim) != getDimension(idim))
                    equal = false;
            }
        }
        return equal;
    }

    void ParserFloatItem::push_backDimension(const std::string& dimension) {
        if ((sizeType() == SINGLE) && (m_dimensions.size() > 0))
            throw std::invalid_argument("Internal error - can not add more than one dimension to a Item os size 1");

        m_dimensions.push_back( dimension );
    }

    bool ParserFloatItem::hasDimension() const {
        return (m_dimensions.size() > 0);
    }

    size_t ParserFloatItem::numDimensions() const {
        return m_dimensions.size();
    }

    const std::string& ParserFloatItem::getDimension(size_t index) const {
        if (index < m_dimensions.size())
            return m_dimensions[index];
        else
            throw std::invalid_argument("Invalid index ");
    }


    /// Scans the rawRecords data according to the ParserItems definition.
    /// returns a DeckItem object.
    /// NOTE: data are popped from the rawRecords deque!
    DeckItemPtr ParserFloatItem::scan(RawRecordPtr rawRecord) const {
        DeckFloatItemPtr deckItem(new DeckFloatItem(name()));
        float defaultValue = m_default;

        if (sizeType() == ALL) {  // This can probably not be combined with a default value ....
            // The '*' should be interpreted as a multiplication sign
            while (rawRecord->size() > 0) {
                std::string token = rawRecord->pop_front();
                if (tokenContainsStar( token )) {
                    StarToken<float> st(token);
                    float value = defaultValue;    // This probably does never apply
                    if (st.hasValue())
                        value = st.value();
                    deckItem->push_backMultiple( value , st.multiplier() );
                } else {
                    float value = readValueToken<float>(token);
                    deckItem->push_back(value);
                }
            }
        } else {
            // The '*' should be interpreted as a default indicator
            if (rawRecord->size() > 0) {
                std::string token = rawRecord->pop_front();
                if (tokenContainsStar( token )) {
                    StarToken<float> st(token);

                    if (st.hasValue()) { // Probably never true
                        deckItem->push_back( st.value() );
                        std::string stringValue = boost::lexical_cast<std::string>(st.value());
                        for (size_t i=1; i < st.multiplier(); i++)
                            rawRecord->push_front( stringValue );
                    } else {
                        deckItem->push_backDefault( defaultValue );
                        for (size_t i=1; i < st.multiplier(); i++)
                            rawRecord->push_front( "*" );
                    }
                } else {
                    float value = readValueToken<float>(token);
                    deckItem->push_back(value);
                }
            } else
                deckItem->push_backDefault( defaultValue );
        }
        return deckItem;
    }

  void ParserFloatItem::inlineNew(std::ostream& os) const {
        os << "new ParserFloatItem(" << "\"" << name() << "\"" << "," << ParserItemSizeEnum2String( sizeType() );
        if (m_defaultSet)
            os << "," << getDefault();
        os << ")";
    }

}

