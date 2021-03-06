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

#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>



namespace Opm {

    Schedule::Schedule(DeckConstPtr deck) {
        if (deck->hasKeyword("SCHEDULE")) {
            initFromDeck(deck);
        } else
            throw std::invalid_argument("Deck does not contain SCHEDULE section.\n");
    }

    void Schedule::initFromDeck(DeckConstPtr deck) {
        createTimeMap(deck);
        addGroup( "FIELD", 0 );
        initRootGroupTreeNode(getTimeMap());
        iterateScheduleSection(deck);
    }

    void Schedule::initRootGroupTreeNode(TimeMapConstPtr timeMap) {
        m_rootGroupTree.reset(new DynamicState<GroupTreePtr>(timeMap, GroupTreePtr(new GroupTree())));
    }

    void Schedule::createTimeMap(DeckConstPtr deck) {
        boost::posix_time::ptime startTime(defaultStartDate);
        if (deck->hasKeyword("START")) {
            DeckKeywordConstPtr startKeyword = deck->getKeyword("START");
            startTime = TimeMap::timeFromEclipse(startKeyword->getRecord(0));
        }

        m_timeMap.reset(new TimeMap(startTime));
    }

    void Schedule::iterateScheduleSection(DeckConstPtr deck) {
        DeckKeywordConstPtr scheduleKeyword = deck->getKeyword("SCHEDULE");
        size_t deckIndex = scheduleKeyword->getDeckIndex() + 1;
        size_t currentStep = 0;
        while (deckIndex < deck->size()) {

            DeckKeywordConstPtr keyword = deck->getKeyword(deckIndex);

            if (keyword->name() == "DATES") {
                handleDATES(keyword);
                currentStep += keyword->size();
            }

            if (keyword->name() == "TSTEP") {
                handleTSTEP(keyword);
                currentStep += keyword->getRecord(0)->getItem(0)->size(); // This is a bit weird API.
            }

            if (keyword->name() == "WELSPECS") {
                handleWELSPECS(keyword, currentStep);
            }

            if (keyword->name() == "WCONHIST")
                handleWCONHIST(keyword, currentStep);

            if (keyword->name() == "WCONPROD")
                handleWCONPROD(keyword, currentStep);

            if (keyword->name() == "WCONINJE")
                handleWCONINJE(deck, keyword, currentStep);

            if (keyword->name() == "WCONINJH")
                handleWCONINJH(deck, keyword, currentStep);

            if (keyword->name() == "WGRUPCON")
                handleWGRUPCON(keyword, currentStep);

            if (keyword->name() == "COMPDAT")
                handleCOMPDAT(keyword, currentStep);

            if (keyword->name() == "WELOPEN")
                handleWELOPEN(keyword, currentStep);

            if (keyword->name() == "GRUPTREE")
                handleGRUPTREE(keyword, currentStep);

            if (keyword->name() == "GCONINJE")
                handleGCONINJE( deck, keyword , currentStep );

            if (keyword->name() == "GCONPROD")
                handleGCONPROD( keyword , currentStep );

            deckIndex++;
        }
    }

    void Schedule::handleDATES(DeckKeywordConstPtr keyword) {
        m_timeMap->addFromDATESKeyword(keyword);
    }

    void Schedule::handleTSTEP(DeckKeywordConstPtr keyword) {
        m_timeMap->addFromTSTEPKeyword(keyword);
    }

    bool Schedule::handleGroupFromWELSPECS(const std::string& groupName, GroupTreePtr newTree) const {
        bool treeUpdated = false;
        if (!newTree->getNode(groupName)) {
            treeUpdated = true;
            newTree->updateTree(groupName);
        }
        return treeUpdated;
    }

    void Schedule::handleWELSPECS(DeckKeywordConstPtr keyword, size_t currentStep) {
        bool needNewTree = false;
        GroupTreePtr newTree = m_rootGroupTree->get(currentStep)->deepCopy();

        for (size_t recordNr = 0; recordNr < keyword->size(); recordNr++) {
            DeckRecordConstPtr record = keyword->getRecord(recordNr);
            const std::string& wellName = record->getItem("WELL")->getString(0);
            const std::string& groupName = record->getItem("GROUP")->getString(0);

            if (!hasGroup(groupName)) {
                addGroup(groupName , currentStep);
            }

            if (!hasWell(wellName)) {
                addWell(wellName, record, currentStep);
            }

            WellConstPtr currentWell = getWell(wellName);
            checkWELSPECSConsistency(currentWell, record);

            addWellToGroup( getGroup(groupName) , getWell(wellName) , currentStep);
            bool treeChanged = handleGroupFromWELSPECS(groupName, newTree);
            needNewTree = needNewTree || treeChanged;
        }
        if (needNewTree) {
            m_rootGroupTree->add(currentStep, newTree);
        }
    }

    void Schedule::checkWELSPECSConsistency(WellConstPtr well, DeckRecordConstPtr record) const {
        if (well->getHeadI() != record->getItem("HEAD_I")->getInt(0) - 1) {
            throw std::invalid_argument("Unable process WELSPECS for well " + well->name() + ", HEAD_I deviates from existing value");
        }
        if (well->getHeadJ() != record->getItem("HEAD_J")->getInt(0) - 1) {
            throw std::invalid_argument("Unable process WELSPECS for well " + well->name() + ", HEAD_J deviates from existing value");
        }
        if (well->getRefDepth() != record->getItem("REF_DEPTH")->getSIDouble(0)) {
            throw std::invalid_argument("Unable process WELSPECS for well " + well->name() + ", REF_DEPTH deviates from existing value");
        }
    }

    

    void Schedule::handleWCONProducer(DeckKeywordConstPtr keyword, size_t currentStep, bool isPredictionMode) {
        for (size_t recordNr = 0; recordNr < keyword->size(); recordNr++) {
            DeckRecordConstPtr record = keyword->getRecord(recordNr);
            const std::string& wellName = record->getItem("WELL")->getString(0);
            WellPtr well                          = getWell(wellName);
            double orat                           = record->getItem("ORAT")->getSIDouble(0);
            double wrat                           = record->getItem("WRAT")->getSIDouble(0);
            double grat                           = record->getItem("GRAT")->getSIDouble(0);
            WellProducer::ControlModeEnum control = WellProducer::ControlModeFromString( record->getItem("CMODE")->getString(0));
            WellCommon::StatusEnum status         = WellCommon::StatusFromString( record->getItem("STATUS")->getString(0));

            well->setProducerControlMode( currentStep , control );
            well->setStatus( currentStep , status );
            well->setOilRate(currentStep, orat);
            well->setWaterRate(currentStep, wrat);
            well->setGasRate(currentStep, grat);
            well->setInPredictionMode(currentStep, isPredictionMode);
            {
                double liquidRate = 0;
                double resVRate = 0;
                double BHPLimit = 0;  
                double THPLimit = 0;

                if (isPredictionMode) {
                    liquidRate = record->getItem("LRAT")->getSIDouble(0);
                    BHPLimit = record->getItem("BHP")->getSIDouble(0);
                    THPLimit = record->getItem("THP")->getSIDouble(0);
                    resVRate = record->getItem("RESV")->getSIDouble(0);
                }
                
                well->setLiquidRate( currentStep , liquidRate );
                well->setResVRate( currentStep , resVRate );
                well->setBHPLimit(currentStep, BHPLimit , true);
                well->setTHPLimit(currentStep, THPLimit , true);

                if (isPredictionMode) {
                    if (record->getItem("LRAT")->defaultApplied())
                        well->dropProductionControl( currentStep , WellProducer::LRAT );
                    
                    if (record->getItem("RESV")->defaultApplied())
                        well->dropProductionControl( currentStep , WellProducer::RESV );
                    
                    if (record->getItem("BHP")->defaultApplied())
                        well->dropProductionControl( currentStep , WellProducer::BHP );
                    
                    if (record->getItem("THP")->defaultApplied())
                        well->dropProductionControl( currentStep , WellProducer::THP );
                } else {
                    well->dropProductionControl( currentStep , WellProducer::LRAT );
                    well->dropProductionControl( currentStep , WellProducer::RESV );
                    well->dropProductionControl( currentStep , WellProducer::BHP );
                    well->dropProductionControl( currentStep , WellProducer::THP );
                }
            }
            
            if (record->getItem("ORAT")->defaultApplied())
                well->dropProductionControl( currentStep , WellProducer::ORAT );
            
            if (record->getItem("GRAT")->defaultApplied()) {
                well->dropProductionControl( currentStep , WellProducer::GRAT );
            }

            if (record->getItem("WRAT")->defaultApplied())
                well->dropProductionControl( currentStep , WellProducer::WRAT );
        }
    }

    void Schedule::handleWCONHIST(DeckKeywordConstPtr keyword, size_t currentStep) {
        handleWCONProducer(keyword, currentStep, false);
    }

    void Schedule::handleWCONPROD(DeckKeywordConstPtr keyword, size_t currentStep) {
        handleWCONProducer(keyword, currentStep, true);
    }

    void Schedule::handleWCONINJE(DeckConstPtr deck, DeckKeywordConstPtr keyword, size_t currentStep) {
        for (size_t recordNr = 0; recordNr < keyword->size(); recordNr++) {
            DeckRecordConstPtr record = keyword->getRecord(recordNr);
            const std::string& wellName = record->getItem("WELL")->getString(0);
            WellPtr well = getWell(wellName);

            // calculate the injection rates. These are context
            // dependent, so we have to jump through some hoops
            // here...
            WellInjector::TypeEnum injectorType = WellInjector::TypeFromString( record->getItem("TYPE")->getString(0) );
            double surfaceInjectionRate = record->getItem("RATE")->getRawDouble(0);
            double reservoirInjectionRate = record->getItem("RESV")->getRawDouble(0);
            surfaceInjectionRate = convertInjectionRateToSI(surfaceInjectionRate, injectorType, *deck->getActiveUnitSystem());
            reservoirInjectionRate = convertInjectionRateToSI(reservoirInjectionRate, injectorType, *deck->getActiveUnitSystem());

            double BHPLimit                           = record->getItem("BHP")->getSIDouble(0);
            double THPLimit                           = record->getItem("THP")->getSIDouble(0);
            WellCommon::StatusEnum status             = WellCommon::StatusFromString( record->getItem("STATUS")->getString(0));
            WellInjector::ControlModeEnum controlMode = WellInjector::ControlModeFromString( record->getItem("CMODE")->getString(0));
       
            well->setStatus( currentStep , status );
            well->setSurfaceInjectionRate( currentStep , surfaceInjectionRate );
            well->setReservoirInjectionRate( currentStep , reservoirInjectionRate );
            well->setBHPLimit(currentStep, BHPLimit , false);
            well->setTHPLimit(currentStep, THPLimit , false);
            well->setInjectorControlMode(currentStep , controlMode );
            well->setInjectorType( currentStep , injectorType );
            well->setInPredictionMode(currentStep, true);
            
            if (record->getItem("RATE")->defaultApplied())
                well->dropInjectionControl( currentStep , WellInjector::RATE );

            if (record->getItem("RESV")->defaultApplied())
                well->dropInjectionControl( currentStep , WellInjector::RESV );
            
            if (record->getItem("THP")->defaultApplied())
                well->dropInjectionControl( currentStep , WellInjector::THP );

            if (record->getItem("BHP")->defaultApplied())
                well->dropInjectionControl( currentStep , WellInjector::BHP );
            
        }
    }

    void Schedule::handleWCONINJH(DeckConstPtr deck, DeckKeywordConstPtr keyword, size_t currentStep) {
        for (size_t recordNr = 0; recordNr < keyword->size(); recordNr++) {
            DeckRecordConstPtr record = keyword->getRecord(recordNr);
            const std::string& wellName = record->getItem("WELL")->getString(0);
            WellPtr well = getWell(wellName);

            // convert injection rates to SI
            WellInjector::TypeEnum wellType = WellInjector::TypeFromString( record->getItem("TYPE")->getString(0));
            double injectionRate = record->getItem("RATE")->getRawDouble(0);
            injectionRate = convertInjectionRateToSI(injectionRate, wellType, *deck->getActiveUnitSystem());

            WellCommon::StatusEnum status = WellCommon::StatusFromString( record->getItem("STATUS")->getString(0));

            well->setStatus( currentStep , status );
            well->setSurfaceInjectionRate( currentStep , injectionRate );
            well->setInPredictionMode(currentStep, false );
        }
    }

    void Schedule::handleWELOPEN(DeckKeywordConstPtr keyword, size_t currentStep) {
        for (size_t recordNr = 0; recordNr < keyword->size(); recordNr++) {
            DeckRecordConstPtr record = keyword->getRecord(recordNr);
            const std::string& wellName = record->getItem("WELL")->getString(0);
            WellPtr well = getWell(wellName);

            for (size_t i=2; i<7; i++) {
                if (record->getItem(i)->getInt(0) > 0 ) {
                    throw std::logic_error("Error processing WELOPEN keyword, specifying specific connections is not supported yet.");
                }
            }
            WellCommon::StatusEnum status = WellCommon::StatusFromString( record->getItem("STATUS")->getString(0));
            well->setStatus(currentStep, status);
        }
    }


    void Schedule::handleGCONINJE(DeckConstPtr deck, DeckKeywordConstPtr keyword, size_t currentStep) {
        for (size_t recordNr = 0; recordNr < keyword->size(); recordNr++) {
            DeckRecordConstPtr record = keyword->getRecord(recordNr);
            const std::string& groupName = record->getItem("GROUP")->getString(0);
            GroupPtr group = getGroup(groupName);

            {
                Phase::PhaseEnum phase = Phase::PhaseEnumFromString( record->getItem("PHASE")->getString(0) );
                group->setInjectionPhase( currentStep , phase );
            }
            {
                GroupInjection::ControlEnum controlMode = GroupInjection::ControlEnumFromString( record->getItem("CONTROL_MODE")->getString(0) );
                group->setInjectionControlMode( currentStep , controlMode );
            }

            Phase::PhaseEnum wellPhase = Phase::PhaseEnumFromString( record->getItem("PHASE")->getString(0));

            // calculate SI injection rates for the group
            double surfaceInjectionRate = record->getItem("SURFACE_TARGET")->getRawDouble(0);
            surfaceInjectionRate = convertInjectionRateToSI(surfaceInjectionRate, wellPhase, *deck->getActiveUnitSystem());
            double reservoirInjectionRate = record->getItem("RESV_TARGET")->getRawDouble(0);
            reservoirInjectionRate = convertInjectionRateToSI(reservoirInjectionRate, wellPhase, *deck->getActiveUnitSystem());

            group->setSurfaceMaxRate( currentStep , surfaceInjectionRate);
            group->setReservoirMaxRate( currentStep , reservoirInjectionRate);
            group->setTargetReinjectFraction( currentStep , record->getItem("REINJ_TARGET")->getSIDouble(0));
            group->setTargetVoidReplacementFraction( currentStep , record->getItem("VOIDAGE_TARGET")->getSIDouble(0));

            group->setProductionGroup(currentStep, false);
        }
    }


    void Schedule::handleGCONPROD(DeckKeywordConstPtr keyword, size_t currentStep) {
        for (size_t recordNr = 0; recordNr < keyword->size(); recordNr++) {
            DeckRecordConstPtr record = keyword->getRecord(recordNr);
            const std::string& groupName = record->getItem("GROUP")->getString(0);
            GroupPtr group = getGroup(groupName);
            {
                GroupProduction::ControlEnum controlMode = GroupProduction::ControlEnumFromString( record->getItem("CONTROL_MODE")->getString(0) );
                group->setProductionControlMode( currentStep , controlMode );
            }
            group->setOilTargetRate( currentStep , record->getItem("OIL_TARGET")->getSIDouble(0));
            group->setGasTargetRate( currentStep , record->getItem("GAS_TARGET")->getSIDouble(0));
            group->setWaterTargetRate( currentStep , record->getItem("WATER_TARGET")->getSIDouble(0));
            group->setLiquidTargetRate( currentStep , record->getItem("LIQUID_TARGET")->getSIDouble(0));
            {
                GroupProductionExceedLimit::ActionEnum exceedAction = GroupProductionExceedLimit::ActionEnumFromString(record->getItem("EXCEED_PROC")->getString(0) );
                group->setProductionExceedLimitAction( currentStep , exceedAction );
            }
            
            group->setProductionGroup(currentStep, true);
        }
    }

    void Schedule::handleCOMPDAT(DeckKeywordConstPtr keyword , size_t currentStep) {
        std::map<std::string , std::vector< CompletionConstPtr> > completionMapList = Completion::completionsFromCOMPDATKeyword( keyword );
        std::map<std::string , std::vector< CompletionConstPtr> >::iterator iter;
        
        for( iter= completionMapList.begin(); iter != completionMapList.end(); iter++) {
            const std::string wellName = iter->first;
            WellPtr well = getWell(wellName);
            well->addCompletions(currentStep, iter->second);
        }
    }

    void Schedule::handleWGRUPCON(DeckKeywordConstPtr keyword, size_t currentStep) {
        for (size_t recordNr = 0; recordNr < keyword->size(); recordNr++) {
            DeckRecordConstPtr record = keyword->getRecord(recordNr);
            const std::string& wellName = record->getItem("WELL")->getString(0);
            WellPtr well = getWell(wellName);

            bool availableForGroupControl = convertEclipseStringToBool(record->getItem("GROUP_CONTROLLED")->getString(0));
            well->setAvailableForGroupControl(currentStep, availableForGroupControl);

            well->setGuideRate(currentStep, record->getItem("GUIDE_RATE")->getRawDouble(0));

            if (record->getItem("PHASE")->defaultApplied()) {
                well->setGuideRatePhase(currentStep, GuideRate::UNDEFINED);
            }
            else {
                std::string guideRatePhase = record->getItem("PHASE")->getString(0);
                well->setGuideRatePhase(currentStep, GuideRate::GuideRatePhaseEnumFromString(guideRatePhase));
            }

            well->setGuideRateScalingFactor(currentStep, record->getItem("SCALING_FACTOR")->getRawDouble(0));
        }
    }

    void Schedule::handleGRUPTREE(DeckKeywordConstPtr keyword, size_t currentStep) {
        GroupTreePtr currentTree = m_rootGroupTree->get(currentStep);
        GroupTreePtr newTree = currentTree->deepCopy();
        for (size_t recordNr = 0; recordNr < keyword->size(); recordNr++) {
            DeckRecordConstPtr record = keyword->getRecord(recordNr);
            const std::string& childName = record->getItem("CHILD_GROUP")->getString(0);
            const std::string& parentName = record->getItem("PARENT_GROUP")->getString(0);
            newTree->updateTree(childName, parentName);
        }
        m_rootGroupTree->add(currentStep, newTree);
    }

    TimeMapConstPtr Schedule::getTimeMap() const {
        return m_timeMap;
    }

    GroupTreePtr Schedule::getGroupTree(size_t timeStep) const {
        return m_rootGroupTree->get(timeStep);
    }

    void Schedule::addWell(const std::string& wellName, DeckRecordConstPtr record, size_t timeStep) {

        // We change from eclipse's 1 - n, to a 0 - n-1 solution
        int headI = record->getItem("HEAD_I")->getInt(0) - 1;
        int headJ = record->getItem("HEAD_J")->getInt(0) - 1;
        double refDepth = record->getItem("REF_DEPTH")->getSIDouble(0);
        WellPtr well(new Well(wellName, headI, headJ, refDepth, m_timeMap , timeStep));
        m_wells[ wellName ] = well;
    }

    size_t Schedule::numWells() const {
        return m_wells.size();
    }

    bool Schedule::hasWell(const std::string& wellName) const {
        return m_wells.find(wellName) != m_wells.end();
    }

    WellPtr Schedule::getWell(const std::string& wellName) const {
        if (hasWell(wellName)) {
            return m_wells.at(wellName);
        } else
            throw std::invalid_argument("Well: " + wellName + " does not exist");
    }

    std::vector<WellConstPtr> Schedule::getWells() const {
        return getWells(m_timeMap->size()-1);
    }

    std::vector<WellConstPtr> Schedule::getWells(size_t timeStep) const {
        if (timeStep >= m_timeMap->size()) {
            throw std::invalid_argument("Timestep to large");
        }

        std::vector<WellConstPtr> wells;
        for (auto iter = m_wells.begin(); iter != m_wells.end(); ++iter) {
            WellConstPtr well = (*iter).second;
            if (well->hasBeenDefined(timeStep)) {
                wells.push_back(well);
            }
        }
        return wells;
    }

    void Schedule::addGroup(const std::string& groupName, size_t timeStep) {
        if (!m_timeMap) {
            throw std::invalid_argument("TimeMap is null, can't add group named: " + groupName);
        }
        GroupPtr group(new Group(groupName, m_timeMap , timeStep));
        m_groups[ groupName ] = group;
    }

    size_t Schedule::numGroups() const {
        return m_groups.size();
    }

    bool Schedule::hasGroup(const std::string& groupName) const {
        return m_groups.find(groupName) != m_groups.end();
    }

    GroupPtr Schedule::getGroup(const std::string& groupName) const {
        if (hasGroup(groupName)) {
            return m_groups.at(groupName);
        } else
            throw std::invalid_argument("Group: " + groupName + " does not exist");
    }

    void Schedule::addWellToGroup( GroupPtr newGroup , WellPtr well , size_t timeStep) {
        const std::string currentGroupName = well->getGroupName(timeStep);
        if (currentGroupName != "") {
            GroupPtr currentGroup = getGroup( currentGroupName );
            currentGroup->delWell( timeStep , well->name());
        }
        well->setGroupName(timeStep , newGroup->name());
        newGroup->addWell(timeStep , well);
    }


    double Schedule::convertInjectionRateToSI(double rawRate, WellInjector::TypeEnum wellType, const Opm::UnitSystem &unitSystem) const {
        switch (wellType) {
        case WellInjector::MULTI:
            // multi-phase controlled injectors are a really funny
            // construct in Eclipse: the quantity controlled for is
            // not physically meaningful, i.e. Eclipse adds up
            // MCFT/day and STB/day.
            throw std::logic_error("There is no generic way to handle multi-phase injectors at this level!");

        case WellInjector::OIL:
        case WellInjector::WATER:
            return rawRate * unitSystem.parse("LiquidVolume/Time")->getSIScaling();

        case WellInjector::GAS:
            return rawRate * unitSystem.parse("GasVolume/Time")->getSIScaling();

        default:
            throw std::logic_error("Unknown injector type");
        }
    }

    double Schedule::convertInjectionRateToSI(double rawRate, Phase::PhaseEnum wellPhase, const Opm::UnitSystem &unitSystem) const {
        switch (wellPhase) {
        case Phase::OIL:
        case Phase::WATER:
            return rawRate * unitSystem.parse("LiquidVolume/Time")->getSIScaling();

        case Phase::GAS:
            return rawRate * unitSystem.parse("GasVolume/Time")->getSIScaling();

        default:
            throw std::logic_error("Unknown injection phase");
        }
    }
    
    bool Schedule::convertEclipseStringToBool(const std::string& eclipseString) {
        std::string lowerTrimmed = boost::algorithm::to_lower_copy(eclipseString);
        boost::algorithm::trim(lowerTrimmed);

        if (lowerTrimmed == "y" || lowerTrimmed == "yes") {
            return true;
        }
        else if (lowerTrimmed == "n" || lowerTrimmed == "no") {
            return false;
        }
        else throw std::invalid_argument("String " + eclipseString + " not recognized as a boolean-convertible string.");
    }
}
