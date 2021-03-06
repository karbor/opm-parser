add_subdirectory(Parser/tests)
add_subdirectory(RawDeck/tests)
add_subdirectory(Deck/tests)
add_subdirectory(IntegrationTests)
add_subdirectory(EclipseState/tests)
add_subdirectory(EclipseState/Schedule/tests)
add_subdirectory(Utility/tests)
add_subdirectory(Units/tests)

add_subdirectory( Applications )

set( rawdeck_source 
RawDeck/StarToken.cpp
RawDeck/RawKeyword.cpp 
RawDeck/RawRecord.cpp )

set( unit_source
Units/UnitSystem.cpp
Units/Dimension.cpp)

set( deck_source
Deck/Deck.cpp
Deck/DeckKeyword.cpp
Deck/DeckRecord.cpp
Deck/DeckItem.cpp
Deck/DeckIntItem.cpp
Deck/DeckDoubleItem.cpp
Deck/DeckFloatItem.cpp
Deck/DeckStringItem.cpp
Deck/KeywordContainer.cpp
)

set( parser_source 
Parser/ParserEnums.cpp
Parser/ParserKeyword.cpp 
Parser/Parser.cpp 
Parser/ParserRecord.cpp
Parser/ParserItem.cpp
Parser/ParserIntItem.cpp  
Parser/ParserBoolItem.cpp  
Parser/ParserFloatItem.cpp
Parser/ParserDoubleItem.cpp
Parser/ParserStringItem.cpp  
${PROJECT_BINARY_DIR}/generated-source/DefaultKeywordList.cpp
)

set( build_parser_source 
Parser/ParserEnums.cpp
Parser/ParserKeyword.cpp 
Parser/ParserRecord.cpp
Parser/ParserItem.cpp
Parser/ParserIntItem.cpp  
Parser/ParserBoolItem.cpp  
Parser/ParserFloatItem.cpp
Parser/ParserDoubleItem.cpp
Parser/ParserStringItem.cpp  
)

set (state_source
EclipseState/EclipseState.cpp
#
EclipseState/Schedule/TimeMap.cpp 
EclipseState/Schedule/Schedule.cpp 
EclipseState/Schedule/Well.cpp
EclipseState/Schedule/WellSet.cpp
EclipseState/Schedule/Group.cpp
EclipseState/Schedule/Completion.cpp
EclipseState/Schedule/CompletionSet.cpp
EclipseState/Schedule/ScheduleEnums.cpp 
EclipseState/Schedule/GroupTreeNode.cpp
EclipseState/Schedule/GroupTree.cpp)

set (utility_source
Utility/SimpleTable.cpp
Utility/SimpleMultiRecordTable.cpp)

set( HEADER_FILES
RawDeck/RawConsts.hpp 
RawDeck/RawKeyword.hpp 
RawDeck/RawRecord.hpp 
RawDeck/StarToken.hpp
RawDeck/RawEnums.hpp
#
Deck/Deck.hpp
Deck/DeckKeyword.hpp
Deck/DeckRecord.hpp
Deck/DeckItem.hpp
Deck/DeckIntItem.hpp
Deck/DeckDoubleItem.hpp
Deck/DeckFloatItem.hpp
Deck/DeckStringItem.hpp
Deck/KeywordContainer.hpp
#
Parser/ParserEnums.hpp
Parser/ParserKeyword.hpp 
Parser/Parser.hpp 
Parser/ParserRecord.hpp
Parser/ParserItem.hpp
Parser/ParserIntItem.hpp  
Parser/ParserBoolItem.hpp  
Parser/ParserFloatItem.hpp
Parser/ParserDoubleItem.hpp
Parser/ParserStringItem.hpp
#
Units/UnitSystem.hpp
Units/Dimension.hpp
Units/ConversionFactors.hpp
#
EclipseState/EclipseState.hpp
#
EclipseState/Schedule/TimeMap.hpp
EclipseState/Schedule/Schedule.hpp
EclipseState/Schedule/Well.hpp
EclipseState/Schedule/WellSet.hpp
EclipseState/Schedule/Group.hpp
EclipseState/Schedule/DynamicState.hpp
EclipseState/Schedule/Completion.hpp
EclipseState/Schedule/CompletionSet.hpp
EclipseState/Schedule/ScheduleEnums.hpp
EclipseState/Schedule/GroupTreeNode.hpp 
EclipseState/Schedule/GroupTree.hpp 
#
Utility/WconinjeWrapper.hpp
Utility/CompdatWrapper.hpp
Utility/PvtoOuterTable.hpp
Utility/WconinjWrapper.hpp
Utility/PvtgInnerTable.hpp
Utility/PvdoTable.hpp
Utility/WgrupconWrapper.hpp
Utility/PvdcoTable.hpp
Utility/GconprodWrapper.hpp
Utility/PvdgTable.hpp
Utility/SimpleMultiRecordTable.hpp
Utility/TstepWrapper.hpp
Utility/PvtgTable.hpp
Utility/StartWrapper.hpp
Utility/SpecgridWrapper.hpp
Utility/SwofTable.hpp
Utility/PvtwTable.hpp
Utility/FullTable.hpp
Utility/WconprodWrapper.hpp
Utility/GconinjeWrapper.hpp
Utility/WelopenWrapper.hpp
Utility/SimpleTable.hpp
Utility/SgofTable.hpp
Utility/PvtoTable.hpp
Utility/PvtoInnerTable.hpp
Utility/GruptreeWrapper.hpp
Utility/WelspecsWrapper.hpp
Utility/PvtgOuterTable.hpp
Utility/EquilWrapper.hpp
Utility/EndscaleWrapper.hpp
Utility/ScalecrsWrapper.hpp
Utility/EnptvdTable.hpp
Utility/EnkrvdTable.hpp
Utility/RocktabTable.hpp
Utility/RockTable.hpp
)

add_library(buildParser ${rawdeck_source} ${build_parser_source} ${deck_source} ${unit_source})
target_link_libraries(buildParser opm-json ${Boost_LIBRARIES})

#-----------------------------------------------------------------

add_executable( createDefaultKeywordList Parser/createDefaultKeywordList.cpp )
target_link_libraries( createDefaultKeywordList buildParser opm-json ${Boost_LIBRARIES} )

add_custom_command( OUTPUT ${PROJECT_BINARY_DIR}/generated-source/DefaultKeywordList.cpp
                    COMMAND createDefaultKeywordList ${PROJECT_SOURCE_DIR}/opm/parser/share/keywords ${PROJECT_BINARY_DIR}/generated-source/DefaultKeywordList.cpp
                    DEPENDS createDefaultKeywordList )


#-----------------------------------------------------------------

add_library(Parser ${rawdeck_source} ${parser_source} ${deck_source} ${state_source} ${utility_source} ${unit_source})
target_link_libraries(Parser opm-json ${Boost_LIBRARIES})

include( ${PROJECT_SOURCE_DIR}/cmake/Modules/install_headers.cmake )   
install_headers( "${HEADER_FILES}" "${CMAKE_INSTALL_PREFIX}" )
install( TARGETS Parser DESTINATION ${CMAKE_INSTALL_LIBDIR} )
