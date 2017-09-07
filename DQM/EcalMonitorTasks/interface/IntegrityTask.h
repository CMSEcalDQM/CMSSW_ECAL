#ifndef IntegrityTask_H
#define IntegrityTask_H

#include "DQWorkerTask.h"

#include "DQM/EcalCommon/interface/EcalDQMCommonUtils.h"

#include "DataFormats/DetId/interface/DetIdCollection.h"
#include "DataFormats/EcalDetId/interface/EcalDetIdCollections.h"

#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/EcalDigi/interface/EcalTriggerPrimitiveDigi.h"
#include "CondFormats/EcalObjects/interface/EcalTPGTowerStatus.h"
#include "CondFormats/EcalObjects/interface/EcalTPGStripStatus.h"

namespace ecaldqm
{

  class IntegrityTask : public DQWorkerTask {
  public:
    IntegrityTask();
    ~IntegrityTask() {}

    void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

    bool analyze(void const*, Collections) override;

    template<class C> void runOnDetIdCollection(C const&, Collections);
    void runOnElectronicsIdCollection(EcalElectronicsIdCollection const&, Collections);

    // TTF:
    //void beginRun(edm::Run const&, edm::EventSetup const&) override;
    //void runOnRealTPs(EcalTrigPrimDigiCollection const&);
  //};

  // TTF:
  //private:
    //edm::ESHandle<EcalTPGTowerStatus> TTStatusRcd;
    //edm::ESHandle<EcalTPGStripStatus> StripStatusRcd;
  };

  inline bool IntegrityTask::analyze(void const* _p, Collections _collection){
    switch(_collection){
    case kEBGainErrors:
    case kEBChIdErrors:
    case kEBGainSwitchErrors:
      if(_p) runOnDetIdCollection(*static_cast<EBDetIdCollection const*>(_p), _collection);
      return true;
    case kEEGainErrors:
    case kEEChIdErrors:
    case kEEGainSwitchErrors:
      if(_p) runOnDetIdCollection(*static_cast<EEDetIdCollection const*>(_p), _collection);
      return true;
      break;
    case kTowerIdErrors:
    case kBlockSizeErrors:
      if(_p) runOnElectronicsIdCollection(*static_cast<EcalElectronicsIdCollection const*>(_p), _collection);
      return true;
      break;
    // TTF:
//    case kTrigPrimDigi:
//      if(_p) runOnRealTPs(*static_cast<EcalTrigPrimDigiCollection const*>(_p));
//      return true;
//      break;
    default:
      break;
    }

    return false;
  }

}

#endif

