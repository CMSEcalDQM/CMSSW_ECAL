#include "../interface/IntegrityClient.h"

#include "CondFormats/EcalObjects/interface/EcalDQMStatusHelper.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DQM/EcalCommon/interface/EcalDQMCommonUtils.h"

namespace ecaldqm
{
  IntegrityClient::IntegrityClient() :
    DQWorkerClient(),
    errFractionThreshold_(0.)
  {
    qualitySummaries_.insert("Quality");
    qualitySummaries_.insert("QualitySummary");
  }

  void
  IntegrityClient::setParams(edm::ParameterSet const& _params)
  {
    errFractionThreshold_ = _params.getUntrackedParameter<double>("errFractionThreshold");
  }

  void
  IntegrityClient::producePlots(ProcessType)
  {
    uint32_t mask(1 << EcalDQMStatusHelper::CH_ID_ERROR |
                  1 << EcalDQMStatusHelper::CH_GAIN_ZERO_ERROR |
                  1 << EcalDQMStatusHelper::CH_GAIN_SWITCH_ERROR |
                  1 << EcalDQMStatusHelper::TT_ID_ERROR |
                  1 << EcalDQMStatusHelper::TT_SIZE_ERROR);

    MESet& meQuality(MEs_.at("Quality"));
    MESet& meQualitySummary(MEs_.at("QualitySummary"));

    MESet const& sOccupancy(sources_.at("Occupancy"));
    MESet const& sGain(sources_.at("Gain"));
    MESet const& sChId(sources_.at("ChId"));
    MESet const& sGainSwitch(sources_.at("GainSwitch"));
    MESet const& sTowerId(sources_.at("TowerId"));
    MESet const& sBlockSize(sources_.at("BlockSize"));

    MESet::iterator qEnd(meQuality.end(GetElectronicsMap()));
    MESet::const_iterator occItr(GetElectronicsMap(), sOccupancy);
    for(MESet::iterator qItr(meQuality.beginChannel(GetElectronicsMap())); qItr != qEnd; qItr.toNextChannel(GetElectronicsMap())){

      occItr = qItr;

      DetId id(qItr->getId());

      bool doMask(meQuality.maskMatches(id, mask, statusManager_, GetTrigTowerMap()));

      float entries(occItr->getBinContent());

      float gain(sGain.getBinContent(getEcalDQMSetupObjects(), id));
      float chid(sChId.getBinContent(getEcalDQMSetupObjects(), id));
      float gainswitch(sGainSwitch.getBinContent(getEcalDQMSetupObjects(), id));

      float towerid(sTowerId.getBinContent(getEcalDQMSetupObjects(), id));
      float blocksize(sBlockSize.getBinContent(getEcalDQMSetupObjects(), id));

      if(entries + gain + chid + gainswitch + towerid + blocksize < 1.){
        qItr->setBinContent(doMask ? kMUnknown : kUnknown);
        meQualitySummary.setBinContent(getEcalDQMSetupObjects(), id, doMask ? kMUnknown : kUnknown);
        continue;
      }

      float chErr((gain + chid + gainswitch + towerid + blocksize) / (entries + gain + chid + gainswitch + towerid + blocksize));

      if(chErr > errFractionThreshold_){
        qItr->setBinContent(doMask ? kMBad : kBad);
        meQualitySummary.setBinContent(getEcalDQMSetupObjects(), id, doMask ? kMBad : kBad);
      }
      else{
        qItr->setBinContent(doMask ? kMGood : kGood);
        meQualitySummary.setBinContent(getEcalDQMSetupObjects(), id, doMask ? kMGood : kGood);
      }
    }
/*
    // ========== TTF plots requested by ETT  ========== //
    // Lifted from TrigPrimClient:
    // Too costly to run whole TrigPrimClient sequence so include TTF plots here instead

    // Fill TTF4 v Masking ME
    // NOT an occupancy plot: only tells you if non-zero TTF4 occupancy was seen
    // without giving info about how many were seen
    MESet& meTTF4vMask(MEs_.at("TTF4vMask"));
    MESet const& sTTFlags4(sources_.at("TTFlags4"));
    MESet const& sTTMaskMapAll(sources_.at("TTMaskMapAll"));

    // Loop over all TTs
    for(unsigned iTT(0); iTT < EcalTrigTowerDetId::kSizeForDenseIndexing; iTT++) {
      EcalTrigTowerDetId ttid(EcalTrigTowerDetId::detIdFromDenseIndex(iTT));
      bool isMasked( sTTMaskMapAll.getBinContent(ttid) > 0. );
      bool hasTTF4( sTTFlags4.getBinContent(ttid) > 0. );
      if ( isMasked ) {
        if ( hasTTF4 )
          meTTF4vMask.setBinContent( ttid,12 ); // Masked, has TTF4
        else
          meTTF4vMask.setBinContent( ttid,11 ); // Masked, no TTF4
      } else {
        if ( hasTTF4 )
          meTTF4vMask.setBinContent( ttid,13 ); // not Masked, has TTF4
      }
    } // TT loop
    // ==================== //
*/
  } // producePlots()

  DEFINE_ECALDQM_WORKER(IntegrityClient);
}
