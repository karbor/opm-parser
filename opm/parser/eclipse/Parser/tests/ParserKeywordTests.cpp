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


#define BOOST_TEST_MODULE ParserTests
#include <boost/test/unit_test.hpp>

#include <opm/json/JsonObject.hpp>
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>
#include <opm/parser/eclipse/Parser/ParserDoubleItem.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>

#include <opm/parser/eclipse/RawDeck/RawEnums.hpp>



using namespace Opm;

BOOST_AUTO_TEST_CASE(construct_withname_nameSet) {
    ParserKeyword parserKeyword("BPR");
    BOOST_CHECK_EQUAL(parserKeyword.getName(), "BPR");
}

BOOST_AUTO_TEST_CASE(NamedInit) {
    std::string keyword("KEYWORD");

    ParserKeyword parserKeyword(keyword, (size_t) 100);
    BOOST_CHECK_EQUAL(parserKeyword.getName(), keyword);
}

BOOST_AUTO_TEST_CASE(ParserKeyword_default_SizeTypedefault) {
    std::string keyword("KEYWORD");
    ParserKeyword parserKeyword(keyword);
    BOOST_CHECK_EQUAL(parserKeyword.getSizeType() , SLASH_TERMINATED);
}


BOOST_AUTO_TEST_CASE(ParserKeyword_withSize_SizeTypeFIXED) {
    std::string keyword("KEYWORD");
    ParserKeyword parserKeyword(keyword, (size_t) 100);
    BOOST_CHECK_EQUAL(parserKeyword.getSizeType() , FIXED);
}


BOOST_AUTO_TEST_CASE(ParserKeyword_withOtherSize_SizeTypeOTHER) {
    std::string keyword("KEYWORD");
    ParserKeyword parserKeyword(keyword, "EQUILDIMS" , "NTEQUIL");
    const std::pair<std::string,std::string>& sizeKW = parserKeyword.getSizeDefinitionPair();
    BOOST_CHECK_EQUAL(OTHER_KEYWORD_IN_DECK , parserKeyword.getSizeType() );
    BOOST_CHECK_EQUAL("EQUILDIMS", sizeKW.first );
    BOOST_CHECK_EQUAL("NTEQUIL" , sizeKW.second );
}



BOOST_AUTO_TEST_CASE(ParserKeyword_wildCardName) {
    BOOST_CHECK_EQUAL( true  , ParserKeyword::wildCardName("SUM*"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::wildCardName("SUM*X"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::wildCardName("sUM*"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::wildCardName("5UM*"));
    BOOST_CHECK_EQUAL( true , ParserKeyword::wildCardName("U5M*"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::wildCardName("ABCDEFGH*"));
}


BOOST_AUTO_TEST_CASE(ParserKeyword_validName) {
    BOOST_CHECK_EQUAL( true , ParserKeyword::validName("SUMMARY"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validName("MixeCase"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validName("NAMETOOLONG"));
    BOOST_CHECK_EQUAL( true , ParserKeyword::validName("STRING88"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validName("88STRING"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validName("KEY.EXT"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validName("STRING~"));

    BOOST_CHECK_EQUAL( true  , ParserKeyword::validName("TVDP*"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validName("*"));
}


BOOST_AUTO_TEST_CASE(ParserKeywordMathces) {
    ParserKeyword parserKeyword("TVDP*" , (size_t) 1);
    BOOST_CHECK_EQUAL( true , parserKeyword.matches("TVDP"));
    BOOST_CHECK_EQUAL( true , parserKeyword.matches("TVDPX"));
    BOOST_CHECK_EQUAL( true , parserKeyword.matches("TVDPXY"));
    BOOST_CHECK_EQUAL( false , parserKeyword.matches("TVD"));
    BOOST_CHECK_EQUAL( false , parserKeyword.matches("ATVDP"));
}



BOOST_AUTO_TEST_CASE(AddDataKeyword_correctlyConfigured) {
    ParserKeyword parserKeyword("PORO" , (size_t) 1);
    ParserIntItemConstPtr item = ParserIntItemConstPtr(new ParserIntItem( "ACTNUM" , ALL , 0 ));
    BOOST_CHECK_EQUAL( false , parserKeyword.isDataKeyword() );
    parserKeyword.addDataItem( item );
    BOOST_CHECK_EQUAL( true , parserKeyword.isDataKeyword() );
    
    BOOST_CHECK_EQUAL(true , parserKeyword.hasFixedSize( ));
    BOOST_CHECK_EQUAL(1U , parserKeyword.getFixedSize() );
    BOOST_CHECK_EQUAL(1U , parserKeyword.numItems() );
}


BOOST_AUTO_TEST_CASE(WrongConstructor_addDataItem_throws) {
    ParserKeyword parserKeyword("PORO");
    ParserIntItemConstPtr dataItem = ParserIntItemConstPtr(new ParserIntItem( "ACTNUM" , ALL , 0 ));
    BOOST_CHECK_THROW( parserKeyword.addDataItem( dataItem ) , std::invalid_argument);
}




BOOST_AUTO_TEST_CASE(MixingDataAndItems_throws1) {
    ParserKeyword parserKeyword("PORO" , (size_t) 1);
    ParserIntItemConstPtr dataItem = ParserIntItemConstPtr(new ParserIntItem( "ACTNUM" , ALL , 0 ));
    ParserIntItemConstPtr item     = ParserIntItemConstPtr(new ParserIntItem( "XXX" , ALL , 0 ));
    parserKeyword.addDataItem( dataItem );
    BOOST_CHECK_THROW( parserKeyword.addItem( item ) , std::invalid_argument);
    BOOST_CHECK_THROW( parserKeyword.addItem( dataItem ) , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(MixingDataAndItems_throws2) {
    ParserKeyword parserKeyword("PORO" , (size_t) 1);
    ParserIntItemConstPtr dataItem = ParserIntItemConstPtr(new ParserIntItem( "ACTNUM" , ALL , 0 ));
    ParserIntItemConstPtr item     = ParserIntItemConstPtr(new ParserIntItem( "XXX" , ALL , 0 ));
    parserKeyword.addItem( item );
    BOOST_CHECK_THROW( parserKeyword.addDataItem( dataItem ) , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(DefaultConstructur_setDescription_canReadBack) {
    ParserKeyword parserKeyword("BPR");
    std::string description("This is the description");
    parserKeyword.setDescription(description);
    BOOST_CHECK_EQUAL( description, parserKeyword.getDescription());
}


/*****************************************************************/
/* json */

BOOST_AUTO_TEST_CASE(ConstructFromJsonObject) {
    Json::JsonObject jsonObject("{\"name\": \"XXX\" , \"size\" : 0}");
    ParserKeyword parserKeyword(jsonObject);
    BOOST_CHECK_EQUAL("XXX" , parserKeyword.getName());
    BOOST_CHECK_EQUAL( true , parserKeyword.hasFixedSize() );
}


BOOST_AUTO_TEST_CASE(ConstructFromJsonObjectWithActionInvalidThrows) {
    Json::JsonObject jsonObject("{\"name\": \"XXX\" , \"size\" : 0, \"action\" : \"WhatEver?\"}");
    BOOST_CHECK_THROW(ParserKeyword parserKeyword(jsonObject) , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(ConstructFromJsonObjectWithAction) {
    Json::JsonObject jsonObject("{\"name\": \"XXX\" , \"size\" : 0, \"action\" : \"IGNORE\"}");
    ParserKeyword parserKeyword(jsonObject);
    BOOST_CHECK_EQUAL( IGNORE , parserKeyword.getAction() );
}


BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_withSize) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"size\" : 100 , \"items\" :[{\"name\":\"ItemX\" , \"size_type\":\"SINGLE\" , \"value_type\" : \"DOUBLE\"}]}");

    ParserKeyword parserKeyword(jsonObject);
    BOOST_CHECK_EQUAL("BPR" , parserKeyword.getName());
    BOOST_CHECK_EQUAL( true , parserKeyword.hasFixedSize() );
    BOOST_CHECK_EQUAL( 100U , parserKeyword.getFixedSize() );

    BOOST_CHECK_EQUAL( 1U , parserKeyword.numItems());
}


BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_missingItemThrows) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"size\" : 100}");
    BOOST_CHECK_THROW( ParserKeyword parserKeyword(jsonObject) , std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_missingItemActionIgnoreOK) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"size\" : 100, \"action\" : \"IGNORE\"}");
    BOOST_CHECK_NO_THROW( ParserKeyword parserKeyword(jsonObject));
}


BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_nosize_notItems_OK) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\"}");
    ParserKeyword parserKeyword(jsonObject);
    BOOST_CHECK_EQUAL( true , parserKeyword.hasFixedSize() );
    BOOST_CHECK_EQUAL( 0U , parserKeyword.getFixedSize());
}



BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_withSizeOther) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"size\" : {\"keyword\" : \"Bjarne\" , \"item\": \"BjarneIgjen\"},  \"items\" :[{\"name\":\"ItemX\" ,  \"value_type\" : \"DOUBLE\"}]}");
    ParserKeyword parserKeyword(jsonObject);
    const std::pair<std::string,std::string>& sizeKW = parserKeyword.getSizeDefinitionPair();
    BOOST_CHECK_EQUAL("BPR" , parserKeyword.getName());
    BOOST_CHECK_EQUAL( false , parserKeyword.hasFixedSize() );
    BOOST_CHECK_EQUAL(OTHER_KEYWORD_IN_DECK , parserKeyword.getSizeType());
    BOOST_CHECK_EQUAL("Bjarne", sizeKW.first );
    BOOST_CHECK_EQUAL("BjarneIgjen" , sizeKW.second );
}


BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_missingName_throws) {
    Json::JsonObject jsonObject("{\"nameXX\": \"BPR\", \"size\" : 100}");
    BOOST_CHECK_THROW(ParserKeyword parserKeyword(jsonObject) , std::invalid_argument);
}

/*
  "items": [{"name" : "I" , "size_type" : "SINGLE" , "value_type" : "int"}]
*/
BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_invalidItems_throws) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"size\" : 100 , \"items\" : 100}");
    BOOST_CHECK_THROW(ParserKeyword parserKeyword(jsonObject) , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_ItemMissingName_throws) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"size\" : 100 , \"items\" : [{\"nameX\" : \"I\"  , \"value_type\" : \"INT\"}]}");
    BOOST_CHECK_THROW(ParserKeyword parserKeyword(jsonObject) , std::invalid_argument);
}



BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_ItemMissingValueType_throws) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"size\" : 100 , \"items\" : [{\"name\" : \"I\" , \"size_type\" : \"SINGLE\" , \"Xvalue_type\" : \"INT\"}]}");
    BOOST_CHECK_THROW(ParserKeyword parserKeyword(jsonObject) , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_ItemInvalidEnum_throws) {
    Json::JsonObject jsonObject1("{\"name\": \"BPR\", \"size\" : 100 , \"items\" : [{\"name\" : \"I\" , \"size_type\" : \"XSINGLE\" , \"value_type\" : \"INT\"}]}");
    Json::JsonObject jsonObject2("{\"name\": \"BPR\", \"size\" : 100 , \"items\" : [{\"name\" : \"I\" , \"size_type\" : \"SINGLE\" , \"value_type\" : \"INTX\"}]}");
    
    BOOST_CHECK_THROW(ParserKeyword parserKeyword(jsonObject1) , std::invalid_argument);
    BOOST_CHECK_THROW(ParserKeyword parserKeyword(jsonObject2) , std::invalid_argument);
}



BOOST_AUTO_TEST_CASE(ConstructFromJsonObjectItemsOK) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"size\" : 100 , \"items\" : [{\"name\" : \"I\", \"value_type\" : \"INT\"}]}");
    ParserKeyword parserKeyword(jsonObject);
    ParserRecordConstPtr record = parserKeyword.getRecord();
    ParserItemConstPtr item = record->get( 0 );
    BOOST_CHECK_EQUAL( 1U , record->size( ) );
    BOOST_CHECK_EQUAL( "I" , item->name( ) );
    BOOST_CHECK_EQUAL( SINGLE , item->sizeType()); 
}


BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_sizeFromOther) {
    Json::JsonObject jsonConfig("{\"name\": \"EQUILX\", \"size\" : {\"keyword\":\"EQLDIMS\" , \"item\" : \"NTEQUL\"},  \"items\" :[{\"name\":\"ItemX\" ,\"value_type\" : \"DOUBLE\"}]}");
    BOOST_CHECK_NO_THROW( ParserKeyword parserKeyword(jsonConfig) );
}


BOOST_AUTO_TEST_CASE(Default_NotData) {
    ParserKeyword parserKeyword("BPR");
    BOOST_CHECK_EQUAL( false , parserKeyword.isDataKeyword());
}


BOOST_AUTO_TEST_CASE(AddDataKeywordFromJson_correctlyConfigured) {
    Json::JsonObject jsonConfig("{\"name\": \"ACTNUM\", \"data\" : {\"value_type\": \"INT\" , \"default\" : 100}}");
    ParserKeyword parserKeyword(jsonConfig);
    ParserRecordConstPtr parserRecord = parserKeyword.getRecord();
    ParserItemConstPtr item = parserRecord->get(0);


    BOOST_CHECK_EQUAL( true , parserKeyword.isDataKeyword());
    BOOST_CHECK_EQUAL(true , parserKeyword.hasFixedSize( ));
    BOOST_CHECK_EQUAL(1U , parserKeyword.getFixedSize() );
    BOOST_CHECK_EQUAL(1U , parserKeyword.numItems() );

    BOOST_CHECK_EQUAL( item->name() , parserKeyword.getName());
    BOOST_CHECK_EQUAL( ALL , item->sizeType() );
}


BOOST_AUTO_TEST_CASE(AddkeywordFromJson_numTables_incoorect_throw) {
    Json::JsonObject jsonConfig("{\"name\": \"PVTG\", \"num_tables\" : 100}");
    BOOST_CHECK_THROW(ParserKeyword parserKeyword(jsonConfig) , std::invalid_argument);
}




BOOST_AUTO_TEST_CASE(AddkeywordFromJson_isTableCollection) {
    Json::JsonObject jsonConfig("{\"name\": \"PVTG\", \"num_tables\" : {\"keyword\": \"TABDIMS\" , \"item\" : \"NTPVT\"} , \"items\" : [{\"name\" : \"data\", \"value_type\" : \"DOUBLE\"}]}");
    ParserKeyword parserKeyword(jsonConfig);
    ParserRecordConstPtr parserRecord = parserKeyword.getRecord();


    BOOST_CHECK_EQUAL( true , parserKeyword.isTableCollection() );
    BOOST_CHECK_EQUAL( false , parserKeyword.isDataKeyword());
    BOOST_CHECK_EQUAL( false , parserKeyword.hasFixedSize( ));
}

BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_InvalidSize_throws) {
    Json::JsonObject jsonObject1("{\"name\": \"BPR\", \"size\" : \"string\" , \"items\" : [{\"name\" : \"I\" , \"size_type\" : \"SINGLE\" , \"value_type\" : \"INT\"}]}");
    Json::JsonObject jsonObject2("{\"name\": \"BPR\", \"size\" : [1,2,3]    , \"items\" : [{\"name\" : \"I\" , \"size_type\" : \"SINGLE\" , \"value_type\" : \"INT\"}]}");
    
    BOOST_CHECK_THROW(ParserKeyword parserKeyword(jsonObject1) , std::invalid_argument);
    BOOST_CHECK_THROW(ParserKeyword parserKeyword(jsonObject2) , std::invalid_argument);

}

BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_SizeUNKNOWN_OK) {
    Json::JsonObject jsonObject1("{\"name\": \"BPR\", \"size\" : \"UNKNOWN\" , \"items\" : [{\"name\" : \"I\" , \"size_type\" : \"SINGLE\" , \"value_type\" : \"INT\"}]}");
    ParserKeyword parserKeyword(jsonObject1);
    
    BOOST_CHECK_EQUAL( UNKNOWN , parserKeyword.getSizeType() );
}


BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_WithDescription_DescriptionPropertyShouldBePopulated) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"description\" : \"Description\"}");
    ParserKeyword parserKeyword(jsonObject);

    BOOST_CHECK_EQUAL( "Description", parserKeyword.getDescription() );
}


BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_WithoutDescription_DescriptionPropertyShouldBeEmpty) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\"}");
    ParserKeyword parserKeyword(jsonObject);

    BOOST_CHECK_EQUAL( "", parserKeyword.getDescription() );
}


/* </Json> */
/*****************************************************************/

BOOST_AUTO_TEST_CASE(constructor_nametoolongwithfixedsize_exceptionthrown) {
    std::string keyword("KEYWORDTOOLONG");
    BOOST_CHECK_THROW(ParserKeyword parserKeyword(keyword, (size_t) 100), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(constructor_nametoolong_exceptionthrown) {
    std::string keyword("KEYWORDTOOLONG");
    BOOST_CHECK_THROW(ParserKeyword parserKeyword(keyword), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(MixedCase) {
    std::string keyword("KeyWord");

    BOOST_CHECK_THROW(ParserKeyword parserKeyword(keyword, (size_t) 100), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(getFixedSize_sizeObjectHasFixedSize_sizeReturned) {
    ParserKeywordPtr parserKeyword(new ParserKeyword("JA", (size_t) 3));
    BOOST_CHECK_EQUAL(3U, parserKeyword->getFixedSize());

}

BOOST_AUTO_TEST_CASE(getFixedSize_sizeObjectDoesNotHaveFixedSizeObjectSet_ExceptionThrown) {
    ParserKeywordPtr parserKeyword(new ParserKeyword("JA"));
    BOOST_CHECK_THROW(parserKeyword->getFixedSize(), std::logic_error);
}


BOOST_AUTO_TEST_CASE(hasFixedSize_hasFixedSizeObject_returnstrue) {
    ParserKeywordPtr parserKeyword(new ParserKeyword("JA", (size_t) 2));
    BOOST_CHECK(parserKeyword->hasFixedSize());
}

BOOST_AUTO_TEST_CASE(hasFixedSize_sizeObjectDoesNotHaveFixedSize_returnsfalse) {
    ParserKeywordPtr parserKeyword(new ParserKeyword("JA"));
    BOOST_CHECK(!parserKeyword->hasFixedSize());
}

/******/
/* Tables: */

BOOST_AUTO_TEST_CASE(DefaultIsNot_TableKeyword) {
    ParserKeywordPtr parserKeyword(new ParserKeyword("JA"));
    BOOST_CHECK(!parserKeyword->isTableCollection());
}

BOOST_AUTO_TEST_CASE(ConstructorIsTableCollection) {
    ParserKeywordPtr parserKeyword(new ParserKeyword("JA" , "TABDIMS" , "NTPVT" , INTERNALIZE , true));
    const std::pair<std::string,std::string>& sizeKW = parserKeyword->getSizeDefinitionPair();
    BOOST_CHECK(parserKeyword->isTableCollection());
    BOOST_CHECK(!parserKeyword->hasFixedSize());

    BOOST_CHECK_EQUAL( parserKeyword->getSizeType() , OTHER_KEYWORD_IN_DECK);
    BOOST_CHECK_EQUAL("TABDIMS", sizeKW.first );
    BOOST_CHECK_EQUAL("NTPVT" , sizeKW.second );
}



BOOST_AUTO_TEST_CASE(ParseEmptyRecord) {
    ParserKeywordPtr tabdimsKeyword( new ParserKeyword("TEST" , 1));
    ParserIntItemConstPtr item(new ParserIntItem(std::string("ITEM") , ALL));
    RawKeywordPtr rawkeyword(new RawKeyword( tabdimsKeyword->getName() , "FILE" , 10U , 1));

    
    BOOST_CHECK_EQUAL( Raw::FIXED , rawkeyword->getSizeType());
    rawkeyword->addRawRecordString("/");
    tabdimsKeyword->addItem(item);

    DeckKeywordConstPtr deckKeyword = tabdimsKeyword->parse( rawkeyword );
    BOOST_REQUIRE_EQUAL( 1U , deckKeyword->size());

    DeckRecordConstPtr deckRecord = deckKeyword->getRecord(0);
    BOOST_REQUIRE_EQUAL( 1U , deckRecord->size());

    DeckItemConstPtr deckItem = deckRecord->getItem(0);
    BOOST_CHECK_EQUAL(0U , deckItem->size());
}


/*****************************************************************/
/* Action value */

BOOST_AUTO_TEST_CASE(DefaultActionISINTERNALIZE) {
    ParserKeywordPtr parserKeyword(new ParserKeyword("JA"));
    BOOST_CHECK_EQUAL(INTERNALIZE , parserKeyword->getAction());
}


BOOST_AUTO_TEST_CASE(CreateWithAction) {
    ParserKeywordPtr parserKeyword(new ParserKeyword("JA" , UNKNOWN , IGNORE));
    BOOST_CHECK_EQUAL(IGNORE , parserKeyword->getAction());
}


/*****************************************************************/
/* DImension */

BOOST_AUTO_TEST_CASE(ParseKeywordHasDimensionCorrect) {
    ParserKeywordPtr parserKeyword(new ParserKeyword("JA"));
    ParserIntItemConstPtr itemI(new ParserIntItem("I", SINGLE));
    ParserDoubleItemPtr item2(new ParserDoubleItem("ID", SINGLE));
    
    BOOST_CHECK_EQUAL( false , parserKeyword->hasDimension());
    
    parserKeyword->addItem( itemI );
    parserKeyword->addItem( item2 );
    BOOST_CHECK_EQUAL( false , parserKeyword->hasDimension());
    BOOST_CHECK_EQUAL( 0U , itemI->numDimensions() );

    item2->push_backDimension("Length*Length/Time");
    BOOST_CHECK_EQUAL( true , parserKeyword->hasDimension());
    BOOST_CHECK_EQUAL( 1U , item2->numDimensions() );
}


BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_withDimension) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"size\" : 100 , \"items\" :[{\"name\":\"ItemX\" , \"size_type\":\"SINGLE\" , \"value_type\" : \"DOUBLE\" , \"dimension\" : \"Length*Length/Time\"}]}");
    ParserKeyword parserKeyword(jsonObject);
    ParserRecordConstPtr record = parserKeyword.getRecord();
    ParserItemConstPtr item = record->get("ItemX");      

    BOOST_CHECK_EQUAL("BPR" , parserKeyword.getName());
    BOOST_CHECK_EQUAL( true , parserKeyword.hasFixedSize() );
    BOOST_CHECK_EQUAL( 100U , parserKeyword.getFixedSize() );

    BOOST_CHECK_EQUAL( 1U , parserKeyword.numItems());
    BOOST_CHECK( parserKeyword.hasDimension() );
    BOOST_CHECK( item->hasDimension() );
    BOOST_CHECK_EQUAL( 1U , item->numDimensions() );
}



BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_withDimensionList) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"size\" : 100 , \"items\" :[{\"name\":\"ItemX\" , \"size_type\":\"ALL\" , \"value_type\" : \"DOUBLE\" , \"dimension\" : [\"Length*Length/Time\" , \"Time\", \"1\"]}]}");
    ParserKeyword parserKeyword(jsonObject);
    ParserRecordConstPtr record = parserKeyword.getRecord();
    ParserItemConstPtr item = record->get("ItemX");      

    BOOST_CHECK_EQUAL("BPR" , parserKeyword.getName());
    BOOST_CHECK_EQUAL( true , parserKeyword.hasFixedSize() );
    BOOST_CHECK_EQUAL( 100U , parserKeyword.getFixedSize() );

    BOOST_CHECK_EQUAL( 1U , parserKeyword.numItems());
    BOOST_CHECK( parserKeyword.hasDimension() );
    BOOST_CHECK( item->hasDimension() );
    BOOST_CHECK_EQUAL( 3U , item->numDimensions() );
}
