#include "DQM/EcalCommon/interface/MESetDet1D.h"

#include "DQM/EcalCommon/interface/EcalDQMCommonUtils.h"

namespace ecaldqm
{

  MESetDet1D::MESetDet1D(std::string const& _fullPath, binning::ObjectType _otype, binning::BinningType _btype, MonitorElement::Kind _kind, binning::AxisSpecs const* _yaxis/* = 0*/) :
    MESetEcal(_fullPath, _otype, _btype, _kind, 1, 0, _yaxis, 0)
  {
    switch(kind_){
    case MonitorElement::Kind::TH1F:
    case MonitorElement::Kind::TPROFILE:
    case MonitorElement::Kind::TH2F:
    case MonitorElement::Kind::TPROFILE2D:
      break;
    default:
      throw_("Unsupported MonitorElement kind");
    }
  }

  MESetDet1D::MESetDet1D(MESetDet1D const& _orig) :
    MESetEcal(_orig)
  {
  }

  MESetDet1D::~MESetDet1D()
  {
  }

  MESet*
  MESetDet1D::clone(std::string const& _path/* = ""*/) const
  {
    std::string path(path_);
    if(_path != "") path_ = _path;
    MESet* copy(new MESetDet1D(*this));
    path_ = path;
    return copy;
  }

  void
  MESetDet1D::book(DQMStore::IBooker& _ibooker, const EcalElectronicsMapping *electronicsMap)
  {
    MESetEcal::book(_ibooker, electronicsMap);

    if(btype_ == binning::kDCC){
      for(unsigned iME(0); iME < mes_.size(); iME++){
        MonitorElement* me(mes_[iME]);

        binning::ObjectType actualObject(binning::getObject(otype_, iME));
        if(actualObject == binning::kEB){
          for(int iBin(1); iBin <= me->getNbinsX(); iBin++)
            me->setBinLabel(iBin, binning::channelName(electronicsMap, iBin + kEBmLow));
        }
        else if(actualObject == binning::kEE){
          for(int iBin(1); iBin <= me->getNbinsX() / 2; iBin++){
            me->setBinLabel(iBin, binning::channelName(electronicsMap, iBin));
            me->setBinLabel(iBin + me->getNbinsX() / 2, binning::channelName(electronicsMap, iBin + 45));
          }
        }
        else if(actualObject == binning::kEEm){
          for(int iBin(1); iBin <= me->getNbinsX(); iBin++)
            me->setBinLabel(iBin, binning::channelName(electronicsMap, iBin));
        }
        else if(actualObject == binning::kEEp){
          for(int iBin(1); iBin <= me->getNbinsX(); iBin++)
            me->setBinLabel(iBin, binning::channelName(electronicsMap, iBin + 45));
        }
      }
    }
    else if(btype_ == binning::kTriggerTower){
      for(unsigned iME(0); iME < mes_.size(); iME++){
        MonitorElement* me(mes_[iME]);

        binning::ObjectType actualObject(binning::getObject(otype_, iME));
        unsigned dccid(0);
        if(actualObject == binning::kSM && (iME <= kEEmHigh || iME >= kEEpLow)) dccid = iME + 1;
        else if(actualObject == binning::kEESM) dccid = iME <= kEEmHigh ? iME + 1 : iME + 37;

        if(dccid > 0){
          std::stringstream ss;
          std::pair<unsigned, unsigned> inner(innerTCCs(iME + 1));
          std::pair<unsigned, unsigned> outer(outerTCCs(iME + 1));
          ss << "TCC" << inner.first << " TT1";
          me->setBinLabel(1, ss.str());
          ss.str("");
          ss << "TCC" << inner.second << " TT1";
          me->setBinLabel(25, ss.str());
          ss.str("");
          ss << "TCC" << outer.first << " TT1";
          me->setBinLabel(49, ss.str());
          ss.str("");
          ss << "TCC" << outer.second << " TT1";
          me->setBinLabel(65, ss.str());
          int offset(0);
          for(int iBin(4); iBin <= 80; iBin += 4){
            if(iBin == 28) offset = 24;
            else if(iBin == 52) offset = 48;
            else if(iBin == 68) offset = 64;
            ss.str("");
            ss << iBin - offset;
            me->setBinLabel(iBin, ss.str());
          }
        }
      }
    }
  }

  void MESetDet1D::fill(
      EcalDQMSetupObjects const edso, DetId const &_id, double _wy /* = 1.*/, double _w /* = 1.*/, double) {
    if(!active_) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));

    if(kind_ == MonitorElement::Kind::TH2F || kind_ == MonitorElement::Kind::TPROFILE2D)
      fill_(iME, xbin, _wy, _w);
    else
      fill_(iME, xbin, _wy);
  }

  void MESetDet1D::fill(
      EcalDQMSetupObjects const edso, EcalElectronicsId const &_id, double _wy /* = 1.*/, double _w /* = 1.*/, double) {
    if(!active_) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));

    if(kind_ == MonitorElement::Kind::TH2F || kind_ == MonitorElement::Kind::TPROFILE2D)
      fill_(iME, xbin, _wy, _w);
    else
      fill_(iME, xbin, _wy);
  }

  void MESetDet1D::fill(
      EcalDQMSetupObjects const edso, int _dcctccid, double _wy /* = 1.*/, double _w /* = 1.*/, double) {
    if(!active_) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _dcctccid, btype_));
    checkME_(iME);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _dcctccid));

    if(kind_ == MonitorElement::Kind::TH2F || kind_ == MonitorElement::Kind::TPROFILE2D)
      fill_(iME, xbin, _wy, _w);
    else
      fill_(iME, xbin, _wy);
  }

  void MESetDet1D::setBinContent(EcalDQMSetupObjects const edso, DetId const &_id, double _content) {
    if(!active_) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));

    if(kind_ == MonitorElement::Kind::TH2F || kind_ == MonitorElement::Kind::TPROFILE2D){
      int nbinsY(me->getTH1()->GetNbinsY());
      for(int iY(1); iY <= nbinsY; iY++)
        me->setBinContent(xbin, iY, _content);
    }
    else
      me->setBinContent(xbin, _content);
  }

  void MESetDet1D::setBinContent(EcalDQMSetupObjects const edso, EcalElectronicsId const &_id, double _content) {
    if(!active_) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));

    if(kind_ == MonitorElement::Kind::TH2F || kind_ == MonitorElement::Kind::TPROFILE2D){
      int nbinsY(me->getTH1()->GetNbinsY());
      for(int iY(1); iY <= nbinsY; iY++)
        me->setBinContent(xbin, iY, _content);
    }
    else
      me->setBinContent(xbin, _content);
  }

  void MESetDet1D::setBinContent(EcalDQMSetupObjects const edso, int _dcctccid, double _content) {
    if(!active_) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _dcctccid, btype_));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _dcctccid));

    if(kind_ == MonitorElement::Kind::TH2F || kind_ == MonitorElement::Kind::TPROFILE2D){
      int nbinsY(me->getTH1()->GetNbinsY());
      for(int iY(1); iY <= nbinsY; iY++)
        me->setBinContent(xbin, iY, _content);
    }
    else
      me->setBinContent(xbin, _content);
  }

  void MESetDet1D::setBinContent(EcalDQMSetupObjects const edso, DetId const &_id, int _ybin, double _content) {
    if(!active_) return;
    if(kind_ != MonitorElement::Kind::TH2F && kind_ != MonitorElement::Kind::TPROFILE2D) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));
    me->setBinContent(xbin, _ybin, _content);
  }

  void MESetDet1D::setBinContent(EcalDQMSetupObjects const edso,
                                 EcalElectronicsId const &_id,
                                 int _ybin,
                                 double _content) {
    if(!active_) return;
    if(kind_ != MonitorElement::Kind::TH2F && kind_ != MonitorElement::Kind::TPROFILE2D) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));
    me->setBinContent(xbin, _ybin, _content);
  }

  void MESetDet1D::setBinContent(EcalDQMSetupObjects const edso, int _dcctccid, int _ybin, double _content) {
    if(!active_) return;
    if(kind_ != MonitorElement::Kind::TH2F && kind_ != MonitorElement::Kind::TPROFILE2D) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _dcctccid));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _dcctccid));
    me->setBinContent(xbin, _ybin, _content);
  }

  void MESetDet1D::setBinError(EcalDQMSetupObjects const edso, DetId const &_id, double _error) {
    if(!active_) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));

    if(kind_ == MonitorElement::Kind::TH2F || kind_ == MonitorElement::Kind::TPROFILE2D){
      int nbinsY(me->getTH1()->GetNbinsY());
      for(int iY(1); iY <= nbinsY; iY++)
        me->setBinError(xbin, iY, _error);
    }
    else
      me->setBinError(xbin, _error);
  }

  void MESetDet1D::setBinError(EcalDQMSetupObjects const edso, EcalElectronicsId const &_id, double _error) {
    if(!active_) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));

    if(kind_ == MonitorElement::Kind::TH2F || kind_ == MonitorElement::Kind::TPROFILE2D){
      int nbinsY(me->getTH1()->GetNbinsY());
      for(int iY(1); iY <= nbinsY; iY++)
        me->setBinError(xbin, iY, _error);
    }
    else
      me->setBinError(xbin, _error);
  }

  void MESetDet1D::setBinError(EcalDQMSetupObjects const edso, int _dcctccid, double _error) {
    if(!active_) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _dcctccid, btype_));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _dcctccid));

    if(kind_ == MonitorElement::Kind::TH2F || kind_ == MonitorElement::Kind::TPROFILE2D){
      int nbinsY(me->getTH1()->GetNbinsY());
      for(int iY(1); iY <= nbinsY; iY++)
        me->setBinError(xbin, iY, _error);
    }
    else
      me->setBinError(xbin, _error);
  }

  void MESetDet1D::setBinError(EcalDQMSetupObjects const edso, DetId const &_id, int _ybin, double _error) {
    if(!active_) return;
    if(kind_ != MonitorElement::Kind::TH2F && kind_ != MonitorElement::Kind::TPROFILE2D) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));
    me->setBinError(xbin, _ybin, _error);
  }

  void MESetDet1D::setBinError(EcalDQMSetupObjects const edso, EcalElectronicsId const &_id, int _ybin, double _error) {
    if(!active_) return;
    if(kind_ != MonitorElement::Kind::TH2F && kind_ != MonitorElement::Kind::TPROFILE2D) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));
    me->setBinError(xbin, _ybin, _error);
  }

  void MESetDet1D::setBinError(EcalDQMSetupObjects const edso, int _dcctccid, int _ybin, double _error) {
    if(!active_) return;
    if(kind_ != MonitorElement::Kind::TH2F && kind_ != MonitorElement::Kind::TPROFILE2D) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _dcctccid));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _dcctccid));
    me->setBinError(xbin, _ybin, _error);
  }

  void MESetDet1D::setBinEntries(EcalDQMSetupObjects const edso, DetId const &_id, double _entries) {
    if(!active_) return;
    if(kind_ != MonitorElement::Kind::TPROFILE && kind_ != MonitorElement::Kind::TPROFILE2D) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));

    if(kind_ == MonitorElement::Kind::TPROFILE2D){
      int nbinsX(me->getTH1()->GetNbinsX());
      int nbinsY(me->getTH1()->GetNbinsY());
      for(int iY(1); iY <= nbinsY; iY++)
        me->setBinEntries((nbinsX + 2) * iY + xbin, _entries);
    }
    else
      me->setBinEntries(xbin, _entries);
  }

  void MESetDet1D::setBinEntries(EcalDQMSetupObjects const edso, EcalElectronicsId const &_id, double _entries) {
    if(!active_) return;
    if(kind_ != MonitorElement::Kind::TPROFILE && kind_ != MonitorElement::Kind::TPROFILE2D) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));

    if(kind_ == MonitorElement::Kind::TPROFILE2D){
      int nbinsX(me->getTH1()->GetNbinsX());
      int nbinsY(me->getTH1()->GetNbinsY());
      for(int iY(1); iY <= nbinsY; iY++)
        me->setBinEntries((nbinsX + 2) * iY + xbin, _entries);
    }
    else
      me->setBinEntries(xbin, _entries);
  }

  void MESetDet1D::setBinEntries(EcalDQMSetupObjects const edso, int _dcctccid, double _entries) {
    if(!active_) return;
    if(kind_ != MonitorElement::Kind::TPROFILE && kind_ != MonitorElement::Kind::TPROFILE2D) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _dcctccid, btype_));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _dcctccid));

    if(kind_ == MonitorElement::Kind::TPROFILE2D){
      int nbinsX(me->getTH1()->GetNbinsX());
      int nbinsY(me->getTH1()->GetNbinsY());
      for(int iY(1); iY <= nbinsY; iY++)
        me->setBinEntries((nbinsX + 2) * iY + xbin, _entries);
    }
    else
      me->setBinEntries(xbin, _entries);
  }

  void MESetDet1D::setBinEntries(EcalDQMSetupObjects const edso, DetId const &_id, int _ybin, double _entries) {
    if(!active_) return;
    if(kind_ != MonitorElement::Kind::TPROFILE2D) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));
    int nbinsX(me->getTH1()->GetNbinsX());
    me->setBinEntries((nbinsX + 2) * _ybin + xbin, _entries);
  }

  void MESetDet1D::setBinEntries(EcalDQMSetupObjects const edso,
                                 EcalElectronicsId const &_id,
                                 int _ybin,
                                 double _entries) {
    if(!active_) return;
    if(kind_ != MonitorElement::Kind::TPROFILE2D) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));
    int nbinsX(me->getTH1()->GetNbinsX());
    me->setBinEntries((nbinsX + 2) * _ybin + xbin, _entries);
  }

  void MESetDet1D::setBinEntries(EcalDQMSetupObjects const edso, int _dcctccid, int _ybin, double _entries) {
    if(!active_) return;
    if(kind_ != MonitorElement::Kind::TPROFILE2D) return;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _dcctccid));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _dcctccid));
    int nbinsX(me->getTH1()->GetNbinsX());
    me->setBinEntries((nbinsX + 2) * _ybin + xbin, _entries);
  }

  double MESetDet1D::getBinContent(EcalDQMSetupObjects const edso, DetId const &_id, int _ybin /* = 0*/) const {
    if(!active_) return 0.;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));
    int nbinsX(me->getTH1()->GetNbinsX());

    return me->getBinContent((nbinsX + 2) * _ybin + xbin);
  }

  double MESetDet1D::getBinContent(EcalDQMSetupObjects const edso,
                                   EcalElectronicsId const &_id,
                                   int _ybin /* = 0*/) const {
    if(!active_) return 0.;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));
    int nbinsX(me->getTH1()->GetNbinsX());

    return me->getBinContent((nbinsX + 2) * _ybin + xbin);
  }

  double MESetDet1D::getBinContent(EcalDQMSetupObjects const edso, int _dcctccid, int _ybin /* = 0*/) const {
    if(!active_) return 0.;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _dcctccid, btype_));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _dcctccid));
    int nbinsX(me->getTH1()->GetNbinsX());

    return me->getBinContent((nbinsX + 2) * _ybin + xbin);
  }

  double MESetDet1D::getBinError(EcalDQMSetupObjects const edso, DetId const &_id, int _ybin /* = 0*/) const {
    if(!active_) return 0.;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));
    int nbinsX(me->getTH1()->GetNbinsX());

    return me->getBinError((nbinsX + 2) * _ybin + xbin);
  }

  double MESetDet1D::getBinError(EcalDQMSetupObjects const edso,
                                 EcalElectronicsId const &_id,
                                 int _ybin /* = 0*/) const {
    if(!active_) return 0.;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));
    int nbinsX(me->getTH1()->GetNbinsX());

    return me->getBinError((nbinsX + 2) * _ybin + xbin);
  }

  double MESetDet1D::getBinError(EcalDQMSetupObjects const edso, int _dcctccid, int _ybin /* = 0*/) const {
    if(!active_) return 0.;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _dcctccid, btype_));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _dcctccid));
    int nbinsX(me->getTH1()->GetNbinsX());

    return me->getBinError((nbinsX + 2) * _ybin + xbin);
  }

  double MESetDet1D::getBinEntries(EcalDQMSetupObjects const edso, DetId const &_id, int _ybin /* = 0*/) const {
    if(!active_) return 0.;
    if(kind_ != MonitorElement::Kind::TPROFILE && kind_ != MonitorElement::Kind::TPROFILE2D) return 0.;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));
    int nbinsX(me->getTH1()->GetNbinsX());

    return me->getBinEntries((nbinsX + 2) * _ybin + xbin);
  }

  double MESetDet1D::getBinEntries(EcalDQMSetupObjects const edso,
                                   EcalElectronicsId const &_id,
                                   int _ybin /* = 0*/) const {
    if(!active_) return 0.;
    if(kind_ != MonitorElement::Kind::TPROFILE && kind_ != MonitorElement::Kind::TPROFILE2D) return 0.;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));
    int nbinsX(me->getTH1()->GetNbinsX());

    return me->getBinEntries((nbinsX + 2) * _ybin + xbin);
  }

  double MESetDet1D::getBinEntries(EcalDQMSetupObjects const edso, int _dcctccid, int _ybin /* = 0*/) const {
    if(!active_) return 0.;
    if(kind_ != MonitorElement::Kind::TPROFILE && kind_ != MonitorElement::Kind::TPROFILE2D) return 0.;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _dcctccid, btype_));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _dcctccid));
    int nbinsX(me->getTH1()->GetNbinsX());

    return me->getBinEntries((nbinsX + 2) * _ybin + xbin);
  }

  int MESetDet1D::findBin(EcalDQMSetupObjects const edso, DetId const &_id) const {
    if(!active_) return -1;
    if(kind_ == MonitorElement::Kind::TPROFILE || kind_ == MonitorElement::Kind::TPROFILE2D) return -1;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    return binning::findBin1D(edso.electronicsMap, obj, btype_, _id);
  }

  int MESetDet1D::findBin(EcalDQMSetupObjects const edso, EcalElectronicsId const &_id) const {
    if(!active_) return -1;
    if(kind_ == MonitorElement::Kind::TPROFILE || kind_ == MonitorElement::Kind::TPROFILE2D) return -1;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    return binning::findBin1D(edso.electronicsMap, obj, btype_, _id);
  }

  int MESetDet1D::findBin(EcalDQMSetupObjects const edso, int _dcctccid) const {
    if(!active_) return -1;
    if(kind_ == MonitorElement::Kind::TPROFILE || kind_ == MonitorElement::Kind::TPROFILE2D) return -1;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _dcctccid));
    checkME_(iME);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    return binning::findBin1D(edso.electronicsMap, obj, btype_, _dcctccid);
  }

  int MESetDet1D::findBin(EcalDQMSetupObjects const edso, DetId const &_id, double _y, double) const {
    if(!active_) return -1;
    if(kind_ != MonitorElement::Kind::TPROFILE && kind_ != MonitorElement::Kind::TPROFILE2D) return -1;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));
    int nbinsX(me->getTH1()->GetNbinsX());
    return xbin + (nbinsX + 2) * me->getTH1()->GetYaxis()->FindBin(_y);
  }

  int MESetDet1D::findBin(EcalDQMSetupObjects const edso, EcalElectronicsId const &_id, double _y, double) const {
    if(!active_) return -1;
    if(kind_ != MonitorElement::Kind::TPROFILE && kind_ != MonitorElement::Kind::TPROFILE2D) return -1;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _id));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _id));
    int nbinsX(me->getTH1()->GetNbinsX());
    return xbin + (nbinsX + 2) * me->getTH1()->GetYaxis()->FindBin(_y);
  }

  int MESetDet1D::findBin(EcalDQMSetupObjects const edso, int _dcctccid, double _y, double) const {
    if(!active_) return -1;
    if(kind_ != MonitorElement::Kind::TPROFILE && kind_ != MonitorElement::Kind::TPROFILE2D) return -1;

    unsigned iME(binning::findPlotIndex(edso.electronicsMap, otype_, _dcctccid));
    checkME_(iME);

    MonitorElement* me(mes_[iME]);

    binning::ObjectType obj(binning::getObject(otype_, iME));
    int xbin(binning::findBin1D(edso.electronicsMap, obj, btype_, _dcctccid));
    int nbinsX(me->getTH1()->GetNbinsX());
    return xbin + (nbinsX + 2) * me->getTH1()->GetYaxis()->FindBin(_y);
  }

  void MESetDet1D::reset(EcalElectronicsMapping const *,
                         double _content /* = 0.*/,
                         double _err /* = 0.*/,
                         double _entries /* = 0.*/) {
    unsigned nME(binning::getNObjects(otype_));

    bool isProfile(kind_ == MonitorElement::Kind::TPROFILE || kind_ == MonitorElement::Kind::TPROFILE2D);
    bool is2D(kind_ == MonitorElement::Kind::TH2F || kind_ == MonitorElement::Kind::TPROFILE2D);

    for(unsigned iME(0); iME < nME; iME++) {
      MonitorElement* me(mes_[iME]);

      int nbinsX(me->getTH1()->GetNbinsX());
      int nbinsY(me->getTH1()->GetNbinsY());
      for(int ix(1); ix <= nbinsX; ix++){
        for(int iy(1); iy <= nbinsY; iy++){
          int bin(is2D ? (nbinsX + 2) * iy + ix : ix);
          me->setBinContent(bin, _content);
          me->setBinError(bin, _err);
          if(isProfile) me->setBinEntries(bin, _entries);
        }
      }
    }
  }
}
