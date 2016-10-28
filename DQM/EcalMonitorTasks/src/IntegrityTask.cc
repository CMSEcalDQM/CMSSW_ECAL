#include "../interface/IntegrityTask.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Common/interface/TriggerResultsByName.h"
#include "CondFormats/DataRecord/interface/EcalTPGTowerStatusRcd.h"
#include "CondFormats/DataRecord/interface/EcalTPGStripStatusRcd.h"

namespace ecaldqm
{
  IntegrityTask::IntegrityTask() :
    DQWorkerTask()
  {
  }
/*
 // ========== TTF plots requested by ETT  ========== //
 // Lifted from TrigPrimTask:
 // Too costly to run whole TrigPrimTask sequence so include TTF plots here instead
 void
  IntegrityTask::beginRun(edm::Run const&, edm::EventSetup const& _es)
  {
    // Read-in Status records:
    // Status records stay constant over run so they are read-in only once here
    // but filled by LS in runOnRealTPs() because MEs are not yet booked at beginRun()
    _es.get<EcalTPGTowerStatusRcd>().get( TTStatusRcd );
    _es.get<EcalTPGStripStatusRcd>().get( StripStatusRcd );
  }

  void
  IntegrityTask::runOnRealTPs(EcalTrigPrimDigiCollection const& _tps)
  {
    MESet& meTTFlags4( MEs_.at("TTFlags4") );
    MESet& meTTFlagsMap( MEs_.at("TTFlagsMap") );
    for( EcalTrigPrimDigiCollection::const_iterator tpItr(_tps.begin()); tpItr != _tps.end(); ++tpItr ) { 

      EcalTrigTowerDetId ttid( tpItr->id() );

      // Fill TT Flag MEs
      float ttF( tpItr->ttFlag() );
      // Monitor occupancy of TTF=4
      // which contains info about TT auto-masking
      if ( ttF == 4. )
        meTTFlags4.fill( ttid );
      // Flag Status Map for all TT flags
      meTTFlagsMap.setBinContent( ttid, ttF );

    } // TP loop

    // Set TT/Strip Masking status in Ecal3P view
    // Status Records are read-in at beginRun() but filled here
    // Requestied by ECAL Trigger in addition to TTMaskMap plots in SM view
    MESet& meTTMaskMapAll(MEs_.at("TTMaskMapAll"));

    // Fill from TT Status Rcd
    const EcalTPGTowerStatus *TTStatus( TTStatusRcd.product() );
    const EcalTPGTowerStatusMap &TTStatusMap( TTStatus->getMap() );
    for( EcalTPGTowerStatusMap::const_iterator ttItr(TTStatusMap.begin()); ttItr != TTStatusMap.end(); ++ttItr ){
      const EcalTrigTowerDetId ttid( ttItr->first );
      if ( ttItr->second > 0 )
        meTTMaskMapAll.setBinContent( ttid,1 ); // TT is masked
    } // TTs

    // Fill from Strip Status Rcd
    const EcalTPGStripStatus *StripStatus( StripStatusRcd.product() );
    const EcalTPGStripStatusMap &StripStatusMap( StripStatus->getMap() );
    for( EcalTPGStripStatusMap::const_iterator stItr(StripStatusMap.begin()); stItr != StripStatusMap.end(); ++stItr ){
      const EcalTriggerElectronicsId stid( stItr->first );
      // Since ME has kTriggerTower binning, convert to EcalTrigTowerDetId first
      // In principle, setBinContent() could be implemented for EcalTriggerElectronicsId class as well
      const EcalTrigTowerDetId ttid( getElectronicsMap()->getTrigTowerDetId(stid.tccId(), stid.ttId()) );
      if ( stItr->second > 0 )
        meTTMaskMapAll.setBinContent( ttid,1 ); // PseudoStrip is masked
    } // PseudoStrips

  } // runOnRealTPs
 // ==================== //
*/
  void
  IntegrityTask::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
  {
    MEs_.at("ByLumi").reset();
  }

  template<typename IDCollection>
  void
  IntegrityTask::runOnDetIdCollection(IDCollection const& _ids, Collections _collection)
  {
    if(_ids.size() == 0) return;

    MESet* set(0);
    switch(_collection){
    case kEBGainErrors:
    case kEEGainErrors:
      set = &MEs_.at("Gain");
      break;
    case kEBChIdErrors:
    case kEEChIdErrors:
      set = &MEs_.at("ChId");
      break;
    case kEBGainSwitchErrors:
    case kEEGainSwitchErrors:
      set = &MEs_.at("GainSwitch");
      break;
    default:
      return;
    }

    MESet& meByLumi(MEs_.at("ByLumi"));
    MESet& meTotal(MEs_.at("Total"));
    MESet& meTrendNErrors(MEs_.at("TrendNErrors"));

    std::for_each(_ids.begin(), _ids.end(),
                  [&](typename IDCollection::value_type const& id){
                    set->fill(id);
                    int dccid(dccId(id));
                    meByLumi.fill(dccid);
                    meTotal.fill(dccid);

                    meTrendNErrors.fill(double(timestamp_.iLumi), 1.);
                  });
  }

  void
  IntegrityTask::runOnElectronicsIdCollection(EcalElectronicsIdCollection const& _ids, Collections _collection)
  {
    if(_ids.size() == 0) return;

    MESet* set(0);
    switch(_collection){
    case kTowerIdErrors:
      set = &MEs_.at("TowerId");
      break;
    case kBlockSizeErrors:
      set = &MEs_.at("BlockSize");
      break;
    default:
      return;
    }

    MESet& meByLumi(MEs_.at("ByLumi"));
    MESet& meTotal(MEs_.at("Total"));
    MESet& meTrendNErrors(MEs_.at("TrendNErrors"));

    std::for_each(_ids.begin(), _ids.end(),
                  [&](EcalElectronicsIdCollection::value_type const& id){
                    set->fill(id);
                    int dccid(id.dccId());
                    double nCrystals(0.);
                    if(dccid <= kEEmHigh + 1 || dccid >= kEEpLow + 1)
                      nCrystals = getElectronicsMap()->dccTowerConstituents(dccid, id.towerId()).size();
                    else
                      nCrystals = 25.;
                    meByLumi.fill(dccid, nCrystals);
                    meTotal.fill(dccid, nCrystals);

                    meTrendNErrors.fill(double(timestamp_.iLumi), nCrystals);
                  });
  }

  DEFINE_ECALDQM_WORKER(IntegrityTask);
}


