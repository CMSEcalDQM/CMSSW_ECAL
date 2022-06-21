#include "../interface/TrigPrimTask.h"

#include "DQM/EcalCommon/interface/EcalDQMCommonUtils.h"
#include "FWCore/Framework/interface/EventSetup.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Common/interface/TriggerResultsByName.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include <iomanip>

namespace ecaldqm
{
  TrigPrimTask::TrigPrimTask() :
    DQWorkerTask(),
    realTps_(0),
    runOnEmul_(false),
    //     HLTCaloPath_(""),
    //     HLTMuonPath_(""),
    //     HLTCaloBit_(false),
    //     HLTMuonBit_(false),
    bxBinEdges_{1, 271, 541, 892, 1162, 1432, 1783, 2053, 2323, 2674, 2944, 3214, 3446, 3490, 3491, 3565},
    bxBin_(0.),
    towerReadouts_()
  {
  }

  void
  TrigPrimTask::setParams(edm::ParameterSet const& _params)
  {
    runOnEmul_ = _params.getUntrackedParameter<bool>("runOnEmul");
    if(!runOnEmul_){
      MEs_.erase(std::string("EtMaxEmul"));
      MEs_.erase(std::string("EmulMaxIndex"));
      MEs_.erase(std::string("MatchedIndex"));
      MEs_.erase(std::string("EtEmulError"));
      MEs_.erase(std::string("FGEmulError"));
    }
  }

  void
  TrigPrimTask::addDependencies(DependencySet& _dependencies)
  {
    if(runOnEmul_) _dependencies.push_back(Dependency(kTrigPrimEmulDigi, kEBDigi, kEEDigi, kTrigPrimDigi));
  }

 void
  TrigPrimTask::beginRun(edm::Run const&, edm::EventSetup const& _es)
  {
    // Read-in Status records:
    // Status records stay constant over run so they are read-in only once here
    // but filled by LS in runOnRealTPs() because MEs are not yet booked at beginRun()
     TTStatus = &_es.getData(TTStatusRcd_);
     StripStatus = &_es.getData(StripStatusRcd_);

  }

  void
  TrigPrimTask::beginEvent(edm::Event const& _evt, edm::EventSetup const&  _es, bool const&, bool&)
  {
    using namespace std;

    towerReadouts_.clear();

    realTps_ = 0;

    //     HLTCaloBit_ = false;
    //     HLTMuonBit_ = false;

    int* pBin(std::upper_bound(bxBinEdges_, bxBinEdges_ + nBXBins + 1, _evt.bunchCrossing()));
    bxBin_ = static_cast<int>(pBin - bxBinEdges_) - 0.5;

    const EcalTPGTowerStatusMap& towerMap = TTStatus->getMap();
    const EcalTPGStripStatusMap& stripMap = StripStatus->getMap();

    MESet& meTTMaskMap(MEs_.at("TTMaskMap"));

    for(EcalTPGTowerStatusMap::const_iterator ttItr(towerMap.begin()); ttItr != towerMap.end(); ++ttItr){

       if ((*ttItr).second > 0)
       {
         const EcalTrigTowerDetId  ttid((*ttItr).first);
         //if(ttid.subDet() == EcalBarrel)
            meTTMaskMap.fill(getEcalDQMSetupObjects(), ttid,1);
       }//masked
    }//loop on towers

    for(EcalTPGStripStatusMap::const_iterator stItr(stripMap.begin()); stItr != stripMap.end(); ++stItr){

       if ((*stItr).second > 0)
       {
         const EcalElectronicsId stid((*stItr).first);
         //if(stid.subdet() == EcalEndcap);
            meTTMaskMap.fill(getEcalDQMSetupObjects(), stid,1);
       }//masked
    }//loop on pseudo-strips

    //     if(HLTCaloPath_.size() || HLTMuonPath_.size()){
    //       edm::TriggerResultsByName results(_evt.triggerResultsByName("HLT"));
    //       if(!results.isValid()) results = _evt.triggerResultsByName("RECO");
    //       if(results.isValid()){
    //  const vector<string>& pathNames(results.triggerNames());

    //  size_t caloStar(HLTCaloPath_.find('*'));
    //  if(caloStar != string::npos){
    //    string caloSub(HLTCaloPath_.substr(0, caloStar));
    //    bool found(false);
    //    for(unsigned iP(0); iP < pathNames.size(); ++iP){
    //      if(pathNames[iP].substr(0, caloStar) == caloSub){
    //        HLTCaloPath_ = pathNames[iP];
    //        found = true;
    //        break;
    //      }
    //    }
    //    if(!found) HLTCaloPath_ = "";
    //  }

    //  size_t muonStar(HLTMuonPath_.find('*'));
    //  if(muonStar != string::npos){
    //    string muonSub(HLTMuonPath_.substr(0, muonStar));
    //    bool found(false);
    //    for(unsigned iP(0); iP < pathNames.size(); ++iP){
    //      if(pathNames[iP].substr(0, muonStar) == muonSub){
    //        HLTMuonPath_ = pathNames[iP];
    //        found = true;
    //        break;
    //      }
    //    }
    //    if(!found) HLTMuonPath_ = "";
    //  }

    //  if(HLTCaloPath_.size()){
    //    try{
    //      HLTCaloBit_ = results.accept(HLTCaloPath_);
    //    }
    //    catch(cms::Exception e){
    //      if(e.category() != "LogicError") throw e;
    //      HLTCaloPath_ = "";
    //    }
    //  }
    //  if(HLTMuonPath_.size()){
    //    try{
    //      HLTMuonBit_ = results.accept(HLTMuonPath_);
    //    }
    //    catch(cms::Exception e){
    //      if(e.category() != "LogicError") throw e;
    //      HLTMuonPath_ = "";
    //    }
    //  }
    //       }
    //     }
  }

  template<typename DigiCollection>
  void
  TrigPrimTask::runOnDigis(DigiCollection const& _digis)
  {
    for(typename DigiCollection::const_iterator digiItr(_digis.begin()); digiItr != _digis.end(); ++digiItr){
      EcalTrigTowerDetId ttid(GetTrigTowerMap()->towerOf(digiItr->id()));
      towerReadouts_[ttid.rawId()]++;
    }
  }
   void TrigPrimTask::setTokens(edm::ConsumesCollector& _collector) { 
    TTStatusRcd_ = _collector.esConsumes<edm::Transition::BeginRun>();
    StripStatusRcd_ = _collector.esConsumes<edm::Transition::BeginRun>();
  }
 
  void
  TrigPrimTask::runOnRealTPs(EcalTrigPrimDigiCollection const& _tps)
  {
    MESet& meEtVsBx(MEs_.at("EtVsBx"));
    MESet& meEtReal(MEs_.at("EtReal"));
    MESet& meEtRealMap(MEs_.at("EtRealMap"));
    MESet& meEtSummary(MEs_.at("EtSummary"));
    MESet& meLowIntMap(MEs_.at("LowIntMap"));
    MESet& meMedIntMap(MEs_.at("MedIntMap"));
    MESet& meHighIntMap(MEs_.at("HighIntMap"));
    MESet& meTTFlags(MEs_.at("TTFlags"));
    MESet& meTTFlags4( MEs_.at("TTFlags4") );
    MESet& meTTFMismatch(MEs_.at("TTFMismatch"));
    MESet& meOccVsBx(MEs_.at("OccVsBx"));

    realTps_ = &_tps;

    double nTP[] = {0., 0., 0.};

    for(EcalTrigPrimDigiCollection::const_iterator tpItr(_tps.begin()); tpItr != _tps.end(); ++tpItr){
      EcalTrigTowerDetId ttid(tpItr->id());
      float et(tpItr->compressedEt());

      if(et > 0.){
        if(ttid.subDet() == EcalBarrel)
          nTP[0] += 1.;
        else if(ttid.zside() < 0)
          nTP[1] += 1.;
        else
          nTP[2] += 2.;
        meEtVsBx.fill(getEcalDQMSetupObjects(), ttid, bxBin_, et);
      }

      meEtReal.fill(getEcalDQMSetupObjects(), ttid, et);
      meEtRealMap.fill(getEcalDQMSetupObjects(), ttid, et);
      meEtSummary.fill(getEcalDQMSetupObjects(), ttid, et);

      int interest(tpItr->ttFlag() & 0x3);

      switch(interest){
      case 0:
        meLowIntMap.fill(getEcalDQMSetupObjects(), ttid);
        break;
      case 1:
        meMedIntMap.fill(getEcalDQMSetupObjects(), ttid);
        break;
      case 3:
        meHighIntMap.fill(getEcalDQMSetupObjects(), ttid);
        break;
      default:
        break;
      }

      // Fill TT Flag MEs
      float ttF( tpItr->ttFlag() );
      meTTFlags.fill(getEcalDQMSetupObjects(),  ttid, ttF );
      // Monitor occupancy of TTF=4
      // which contains info about TT auto-masking
      if ( ttF == 4. )
        meTTFlags4.fill(getEcalDQMSetupObjects(),  ttid );

      if((interest == 1 || interest == 3) && towerReadouts_[ttid.rawId()] != GetTrigTowerMap()->constituentsOf(ttid).size())
        meTTFMismatch.fill(getEcalDQMSetupObjects(), ttid);
    }

    meOccVsBx.fill(getEcalDQMSetupObjects(),  EcalBarrel, bxBin_, nTP[0]);
    meOccVsBx.fill(getEcalDQMSetupObjects(), -EcalEndcap, bxBin_, nTP[1]);
    meOccVsBx.fill(getEcalDQMSetupObjects(),  EcalEndcap, bxBin_, nTP[2]);

    // Set TT/Strip Masking status in Ecal3P view
    // Status Records are read-in at beginRun() but filled here
    // Requestied by ECAL Trigger in addition to TTMaskMap plots in SM view
    MESet& meTTMaskMapAll(MEs_.at("TTMaskMapAll"));

    // Fill from TT Status Rcd
    const EcalTPGTowerStatusMap& TTStatusMap(TTStatus->getMap());
    for( EcalTPGTowerStatusMap::const_iterator ttItr(TTStatusMap.begin()); ttItr != TTStatusMap.end(); ++ttItr ){
      const EcalTrigTowerDetId ttid( ttItr->first );
      if ( ttItr->second > 0 )
        meTTMaskMapAll.setBinContent(getEcalDQMSetupObjects(), ttid,1 ); // TT is masked
    } // TTs

    // Fill from Strip Status Rcd
    const EcalTPGStripStatusMap& StripStatusMap(StripStatus->getMap());
    for( EcalTPGStripStatusMap::const_iterator stItr(StripStatusMap.begin()); stItr != StripStatusMap.end(); ++stItr ){
      const EcalTriggerElectronicsId stid( stItr->first );
      // Since ME has kTriggerTower binning, convert to EcalTrigTowerDetId first
      // In principle, setBinContent() could be implemented for EcalTriggerElectronicsId class as well
      const EcalTrigTowerDetId ttid( GetElectronicsMap()->getTrigTowerDetId(stid.tccId(), stid.ttId()) );
      if ( stItr->second > 0 )
        meTTMaskMapAll.setBinContent(getEcalDQMSetupObjects(), ttid,1 ); // PseudoStrip is masked
    } // PseudoStrips

  } // TrigPrimTask::runOnRealTPs()

  void
  TrigPrimTask::runOnEmulTPs(EcalTrigPrimDigiCollection const& _tps)
  {
    MESet& meEtMaxEmul(MEs_.at("EtMaxEmul"));
    MESet& meEmulMaxIndex(MEs_.at("EmulMaxIndex"));
    MESet& meMatchedIndex(MEs_.at("MatchedIndex"));
    MESet& meEtEmulError(MEs_.at("EtEmulError"));
    MESet& meFGEmulError(MEs_.at("FGEmulError"));

    for(EcalTrigPrimDigiCollection::const_iterator tpItr(_tps.begin()); tpItr != _tps.end(); ++tpItr){
      EcalTrigTowerDetId ttid(tpItr->id());

      int et(tpItr->compressedEt());

      float maxEt(0.);
      int iMax(0);
      for(int iDigi(0); iDigi < 5; iDigi++){
        float sampleEt((*tpItr)[iDigi].compressedEt());

        if(sampleEt > maxEt){
          maxEt = sampleEt;
          iMax = iDigi + 1;
        }
      }

      meEtMaxEmul.fill(getEcalDQMSetupObjects(), ttid, maxEt);
      if(maxEt > 0.)
        meEmulMaxIndex.fill(getEcalDQMSetupObjects(), ttid, iMax);

      bool match(true);
      bool matchFG(true);

      EcalTrigPrimDigiCollection::const_iterator realItr(realTps_->find(ttid));
      if(realItr != realTps_->end()){

        int realEt(realItr->compressedEt());

        if(realEt > 0){

          int interest(realItr->ttFlag() & 0x3);
          if((interest == 1 || interest == 3) && towerReadouts_[ttid.rawId()] == GetTrigTowerMap()->constituentsOf(ttid).size()){

            if(et != realEt) match = false;
            if(tpItr->fineGrain() != realItr->fineGrain()) matchFG = false;

            std::vector<int> matchedIndex(0);
            for(int iDigi(0); iDigi < 5; iDigi++){
              if((*tpItr)[iDigi].compressedEt() == realEt)
                matchedIndex.push_back(iDigi + 1);
            }

            if(!matchedIndex.size()) matchedIndex.push_back(0);
            for(std::vector<int>::iterator matchItr(matchedIndex.begin()); matchItr != matchedIndex.end(); ++matchItr){
              meMatchedIndex.fill(getEcalDQMSetupObjects(), ttid, *matchItr + 0.5);

              // timing information is only within emulated TPs (real TPs have one time sample)
              //      if(HLTCaloBit_) MEs_[kTimingCalo].fill(ttid, float(*matchItr));
              //      if(HLTMuonBit_) MEs_[kTimingMuon].fill(ttid, float(*matchItr));
            }
          }

        }
      }
      else{
        match = false;
        matchFG = false;
      }

      if(!match) meEtEmulError.fill(getEcalDQMSetupObjects(), ttid);
      if(!matchFG) meFGEmulError.fill(getEcalDQMSetupObjects(), ttid);
    }
  }

  DEFINE_ECALDQM_WORKER(TrigPrimTask);
}
