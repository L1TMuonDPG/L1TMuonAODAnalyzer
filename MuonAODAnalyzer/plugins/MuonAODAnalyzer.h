// -*- C++ -*-
//
// Package:    MuonAODAnalyzer/MuonAODAnalyzer
// Class:      MuonAODAnalyzer
//
/**\class MuonAODAnalyzer MuonAODAnalyzer.cc MuonAODAnalyzer/MuonAODAnalyzer/plugins/MuonAODAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Efe Yigitbasi
//         Created:  Sat, 10 Sep 2022 11:08:53 GMT
//
//

// system include files
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// ROOT includes
#include "TFile.h"
#include "TMath.h"
#include "TString.h"
#include "TTree.h"
#include "TRegexp.h"

// CMSSW includes
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

// reco muons
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/MuonReco/interface/MuonEnergy.h"
#include "DataFormats/MuonReco/interface/MuonTime.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

// trigger info
#include "DataFormats/Math/interface/deltaR.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/HLTReco/interface/TriggerObject.h"
#include "FWCore/Common/interface/TriggerNames.h"

// vertex
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

// track extrapolation
#include "MuonAnalysis/MuonAssociators/interface/PropagateToMuonSetup.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateOnSurface.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "MagneticField/Engine/interface/MagneticField.h"

class MuonAODAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources, edm::one::WatchRuns> {
  public:
    explicit MuonAODAnalyzer(const edm::ParameterSet &);
    ~MuonAODAnalyzer();

    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

  private:
    void beginJob() override;
    void beginRun(const edm::Run &, const edm::EventSetup &) override;
    void analyze(const edm::Event &, const edm::EventSetup &) override;
    void endJob() override;
    void endRun(const edm::Run &, const edm::EventSetup &) override;

    // Aux functions
    void getHandles(const edm::Event &iEvent, const edm::EventSetup &iSetup);
    double match_trigger(std::vector<int> &trigIndices,
                    const trigger::TriggerObjectCollection &trigObjs,
                    const trigger::TriggerEvent &triggerEvent,
                    const reco::Muon &mu);
    void fillTree();
    void makeTree();
    
    template <typename T> edm::Handle<T> make_handle(T *t) {
        return edm::Handle<T>();
    }

    // ---------- Member data ---------------------------

    const edm::InputTag RecoMuonTag_;
    const std::string outFileName_;
    int verbose_;

    bool useRecoMuons_;
    bool useEventInfo_;
    bool debug_;

    // trig matching
    std::vector<std::string> isoTriggerNames_;
    std::vector<std::string> triggerNames_;

    // Tokens
    edm::EDGetTokenT<reco::MuonCollection> RecoMuonToken_;
    edm::EDGetTokenT<edm::TriggerResults> TriggerResultsToken_;
    edm::EDGetTokenT<trigger::TriggerEvent> TriggerSummaryLabelsToken_;
    edm::EDGetTokenT<reco::VertexCollection> VerticesToken_;
    edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> theBFieldToken_;

    double triggerMaxDeltaR_;
    bool triggerMatching_;
    std::string triggerProcessLabel_;
    std::vector<int> isoTriggerIndices_;
    std::vector<int> triggerIndices_;
    HLTConfigProvider hltConfig_;

    const PropagateToMuonSetup muPropagatorSetup1st_;
    const PropagateToMuonSetup muPropagatorSetup2nd_;

    PropagateToMuon muPropagator1st_;
    PropagateToMuon muPropagator2nd_;

    const reco::MuonCollection *RecoMuons_;
    const edm::TriggerResults *TriggerResults_;
    const trigger::TriggerEvent *TriggerSummaryLabels_;
    const reco::VertexCollection *Vertices_;

    // TTree
    TTree *tree;
    bool firstEvent_;

    // Output collections

    // Event info
    unsigned long eventInfo_event;
    unsigned long eventInfo_run;
    unsigned long eventInfo_lumi;
    unsigned long eventInfo_bx;
    int eventInfo_npv;
    int eventInfo_nvtx;

    // Reco muon info
    std::unique_ptr<int32_t> muon_size;
    std::unique_ptr<std::vector<float>> muon_e;
    std::unique_ptr<std::vector<float>> muon_et;
    std::unique_ptr<std::vector<float>> muon_pt;
    std::unique_ptr<std::vector<float>> muon_eta;
    std::unique_ptr<std::vector<float>> muon_phi;
    std::unique_ptr<std::vector<float>> muon_dxy;
    std::unique_ptr<std::vector<float>> muon_dz;
    std::unique_ptr<std::vector<bool>> muon_isLooseMuon;
    std::unique_ptr<std::vector<bool>> muon_isMediumMuon;
    std::unique_ptr<std::vector<bool>> muon_isTightMuon;
    std::unique_ptr<std::vector<float>> muon_iso;
    std::unique_ptr<std::vector<short>> muon_hlt_isomu;
    std::unique_ptr<std::vector<short>> muon_hlt_mu;
    std::unique_ptr<std::vector<float>> muon_hlt_isoDeltaR;
    std::unique_ptr<std::vector<float>> muon_hlt_deltaR;
    std::unique_ptr<std::vector<int>> muon_passesSingleMuon;
    std::unique_ptr<std::vector<int>> muon_charge;
    std::unique_ptr<std::vector<float>> muon_etaSt1;
    std::unique_ptr<std::vector<float>> muon_phiSt1;
    std::unique_ptr<std::vector<float>> muon_etaSt2;
    std::unique_ptr<std::vector<float>> muon_phiSt2;

    // Trigger flags
    std::unique_ptr<bool> HLT_IsoMu24;
    std::unique_ptr<bool> HLT_Mu50_L1SingleMuShower;
    std::unique_ptr<bool> HLT_Mu50;
};