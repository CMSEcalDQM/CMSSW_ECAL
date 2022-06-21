#ifndef MESetBinningUtils_H
#define MESetBinningUtils_H

#include "DQMServices/Core/interface/DQMStore.h"
#include "Geometry/EcalMapping/interface/EcalElectronicsMapping.h"

#include <string>

class DetId;
class EcalElectronicsId;
namespace edm
{
  class ParameterSet;
  class ParameterSetDescription;
}

namespace ecaldqm
{
  namespace binning
  {
    enum ObjectType {
      kEB,
      kEE,
      kEEm,
      kEEp,
      kSM,
      kEBSM,
      kEESM,
      kSMMEM,
      kEBSMMEM,
      kEESMMEM,
      kEcal,
      kMEM,
      kEBMEM,
      kEEMEM,
      kEcal2P,
      kEcal3P,
      kEE2P,
      kMEM2P,
      kChannel,
      nObjType
    };

    enum BinningType {
      kCrystal,
      kTriggerTower,
      kSuperCrystal,
      kPseudoStrip,
      kTCC,
      kDCC,
      kProjEta,
      kProjPhi,
      kRCT,
      kUser,
      kReport,
      kTrend,
      nBinType
    };

    enum Constants {
      nPresetBinnings = kRCT + 1,

      nEBSMEta = 85,
      nEBSMPhi = 20,
      nEESMX = 40,
      nEESMXRed = 30, // for EE+-01&05&09
      nEESMXExt = 45, // for EE+-02&08
      nEESMY = 40,
      nEESMYRed = 35, // for EE+-03&07

      nEBEtaBins = 34,
      nEEEtaBins = 20,
      nPhiBins = 36
    };

    struct AxisSpecs {
      int nbins;
      float low, high;
      float* edges;
      std::string* labels;
      std::string title;
      AxisSpecs() : nbins(0), low(0.), high(0.), edges(0), labels(0), title("") {}
      AxisSpecs(AxisSpecs const& _specs) :
        nbins(_specs.nbins), low(_specs.low), high(_specs.high), edges(0), labels(0), title(_specs.title)
      {
        if(_specs.edges){
          edges = new float[nbins + 1];
          for(int i(0); i <= nbins; i++) edges[i] = _specs.edges[i];
        }
        if(_specs.labels){
          labels = new std::string[nbins];
          for(int i(0); i < nbins; i++) labels[i] = _specs.labels[i];
        }
      }
      AxisSpecs& operator=(AxisSpecs const& _rhs)
      {
        if(edges){ delete [] edges; edges = 0; }
        if(labels){ delete [] labels; labels = 0; }
        nbins = _rhs.nbins; low = _rhs.low; high = _rhs.high; title = _rhs.title;
        if(_rhs.edges){
          edges = new float[nbins + 1];
          for(int i(0); i <= nbins; i++) edges[i] = _rhs.edges[i];
        }
        if(_rhs.labels){
          labels = new std::string[nbins];
          for(int i(0); i < nbins; i++) labels[i] = _rhs.labels[i];
        }
        return *this;
      }
      ~AxisSpecs()
      {
        delete [] edges;
        delete [] labels;
      }
    };

    AxisSpecs getBinning(EcalElectronicsMapping const *, ObjectType, BinningType, bool, int, unsigned);

    int findBin1D(EcalElectronicsMapping const *, ObjectType, BinningType, DetId const &);
    int findBin1D(EcalElectronicsMapping const *, ObjectType, BinningType, EcalElectronicsId const &);
    int findBin1D(EcalElectronicsMapping const *, ObjectType, BinningType, int);

    int findBin2D(EcalElectronicsMapping const *, ObjectType, BinningType, DetId const &);
    int findBin2D(EcalElectronicsMapping const *, ObjectType, BinningType, EcalElectronicsId const &);
    int findBin2D(EcalElectronicsMapping const *, ObjectType, BinningType, int);

    unsigned findPlotIndex(EcalElectronicsMapping const *, ObjectType, DetId const &);
    unsigned findPlotIndex(EcalElectronicsMapping const *, ObjectType, EcalElectronicsId const &);
    unsigned findPlotIndex(EcalElectronicsMapping const *, ObjectType, int, BinningType _btype = kDCC);

    ObjectType getObject(ObjectType, unsigned);

    unsigned getNObjects(ObjectType);

    bool isValidIdBin(EcalElectronicsMapping const *, ObjectType, BinningType, unsigned, int);

    std::string channelName(EcalElectronicsMapping const *, uint32_t, BinningType _btype = kDCC);

    uint32_t idFromName(std::string const&);
    uint32_t idFromBin(ObjectType, BinningType, unsigned, int);

    AxisSpecs formAxis(edm::ParameterSet const&);
    void fillAxisDescriptions(edm::ParameterSetDescription&);

    ObjectType translateObjectType(std::string const&);
    BinningType translateBinningType(std::string const&);
    dqm::legacy::MonitorElement::Kind translateKind(std::string const&);

    /* Functions used only internally within binning namespace */

    // used for SM binnings
    int xlow_(int);
    int ylow_(int);

    AxisSpecs getBinningEB_(BinningType, bool, int);
    AxisSpecs getBinningEE_(BinningType, bool, int, int);
    AxisSpecs getBinningSM_(BinningType, bool, unsigned, int, EcalElectronicsMapping const *);
    AxisSpecs getBinningSMMEM_(BinningType, bool, unsigned, int);
    AxisSpecs getBinningEcal_(BinningType, bool, int);
    AxisSpecs getBinningMEM_(BinningType, bool, int, int);

    int findBinCrystal_(EcalElectronicsMapping const *, ObjectType, DetId const &, int = -1);
    int findBinCrystal_(EcalElectronicsMapping const *, ObjectType, EcalElectronicsId const &);
    int findBinTriggerTower_(EcalElectronicsMapping const *, ObjectType, DetId const &);
    int findBinPseudoStrip_(EcalElectronicsMapping const *, ObjectType, DetId const &);
    int findBinRCT_(ObjectType, DetId const &);
    int findBinSuperCrystal_(EcalElectronicsMapping const *, ObjectType, DetId const &, int = -1);
    int findBinSuperCrystal_(EcalElectronicsMapping const *, ObjectType, EcalElectronicsId const &);
  }
}

#endif
