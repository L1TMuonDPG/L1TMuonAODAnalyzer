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

//l1 muons
#include "DataFormats/L1TGlobal/interface/GlobalAlgBlk.h"
#include "DataFormats/L1TGlobal/interface/GlobalExtBlk.h"
#include "DataFormats/L1TMuon/interface/RegionalMuonCand.h"
#include "DataFormats/L1Trigger/interface/Muon.h"
#include "DataFormats/L1Trigger/interface/BXVector.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"

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

#include <fstream>
#include "TLorentzVector.h"
#include "TDirectory.h"
#include <fmt/printf.h>

#include "CondFormats/DataRecord/interface/L1TUtmTriggerMenuRcd.h"
#include "CondFormats/L1TObjects/interface/L1TUtmTriggerMenu.h"

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
    const edm::InputTag DispMuonTag_;
    const edm::InputTag CosmicMuonTag_;
    const edm::InputTag CosmicMuon1LegTag_;
    const std::string outFileName_;
    int verbose_;

    bool useRecoMuons_;
    bool useEventInfo_;
    bool useDispMuons_;
    bool useCosmicMuons_;
    bool useCosmicMuons1Leg_;
    bool debug_;

    // trig matching
    std::vector<std::string> isoTriggerNames_;
    std::vector<std::string> triggerNames_;

    // Tokens
    edm::EDGetTokenT<reco::MuonCollection> RecoMuonToken_;
    edm::EDGetTokenT<l1t::MuonBxCollection>l1MuonToken_;
    edm::EDGetTokenT<edm::TriggerResults> TriggerResultsToken_;
    edm::EDGetTokenT<trigger::TriggerEvent> TriggerSummaryLabelsToken_;
    edm::EDGetTokenT<reco::VertexCollection> VerticesToken_;
    edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> theBFieldToken_;

    edm::EDGetTokenT<std::vector< reco::Muon> > DispMuonToken_;
    edm::EDGetTokenT<std::vector< reco::Muon> > CosmicMuonToken_;
    edm::EDGetTokenT<std::vector< reco::Muon> > CosmicMuon1LegToken_;

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
    const reco::MuonCollection *DispMuons_;
    const reco::MuonCollection *CosmicMuons_;
    const reco::MuonCollection *CosmicMuons1Leg_;
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
    std::unique_ptr<std::vector<float>> muon_vx;
    std::unique_ptr<std::vector<float>> muon_vy;
    std::unique_ptr<std::vector<float>> muon_vz;
    std::unique_ptr<std::vector<float>> muon_px;
    std::unique_ptr<std::vector<float>> muon_py;
    std::unique_ptr<std::vector<float>> muon_pz;
    std::unique_ptr<std::vector<bool>> muon_isSAMuon;
    std::unique_ptr<std::vector<bool>> muon_isGlobalMuon;
    std::unique_ptr<std::vector<bool>> muon_isTrackerMuon;
    std::unique_ptr<std::vector<bool>> muon_isPFMuon;
    std::unique_ptr<std::vector<int>>  muon_nChambers;
    std::unique_ptr<std::vector<int>>  muon_nChambersCSCorDT;
    std::unique_ptr<std::vector<int>>  muon_nMatches;
    std::unique_ptr<std::vector<int>>  muon_nMatchedStations;
    std::unique_ptr<std::vector<unsigned int>> muon_expectedNumberOfMatchedStations;
    std::unique_ptr<std::vector<unsigned int>> muon_stationMask;
    std::unique_ptr<std::vector<int>>  muon_nMatchedRPCLayers;
    std::unique_ptr<std::vector<unsigned int>> muon_RPClayerMask;

    //L1 muon
    vector <int> l1mu_qual;
    vector <int> l1mu_charge;
    vector <Float_t> l1mu_pt;
    vector <Float_t> l1mu_pt_dxy;
    vector <int> l1mu_dxy;
    vector <Float_t> l1mu_eta;
    vector <Float_t> l1mu_etaAtVtx;
    vector <Float_t> l1mu_phi;
    vector <Float_t> l1mu_phiAtVtx;
    vector <int> l1mu_tfIdx;
    vector <int> l1mu_bx;
    int l1mu_size;

    //Displaced Muons
    std::unique_ptr<int32_t> dispMuon_size;
    std::unique_ptr<std::vector<float>> dispMuon_e;
    std::unique_ptr<std::vector<float>> dispMuon_et;
    std::unique_ptr<std::vector<float>> dispMuon_pt;
    std::unique_ptr<std::vector<float>> dispMuon_eta;
    std::unique_ptr<std::vector<float>> dispMuon_phi;
    std::unique_ptr<std::vector<float>> dispMuon_dxy;
    std::unique_ptr<std::vector<float>> dispMuon_dz;
    std::unique_ptr<std::vector<bool>> dispMuon_isLooseMuon;
    std::unique_ptr<std::vector<bool>> dispMuon_isMediumMuon;
    std::unique_ptr<std::vector<bool>> dispMuon_isTightMuon;
    std::unique_ptr<std::vector<float>> dispMuon_iso;
    std::unique_ptr<std::vector<short>> dispMuon_hlt_isomu;
    std::unique_ptr<std::vector<short>> dispMuon_hlt_mu;
    std::unique_ptr<std::vector<float>> dispMuon_hlt_isoDeltaR;
    std::unique_ptr<std::vector<float>> dispMuon_hlt_deltaR;
    std::unique_ptr<std::vector<int>> dispMuon_passesSingleMuon;
    std::unique_ptr<std::vector<int>> dispMuon_charge;
    std::unique_ptr<std::vector<float>> dispMuon_etaSt1;
    std::unique_ptr<std::vector<float>> dispMuon_phiSt1;
    std::unique_ptr<std::vector<float>> dispMuon_etaSt2;
    std::unique_ptr<std::vector<float>> dispMuon_phiSt2;
    std::unique_ptr<std::vector<float>> dispMuon_vx;
    std::unique_ptr<std::vector<float>> dispMuon_vy;
    std::unique_ptr<std::vector<float>> dispMuon_vz;
    std::unique_ptr<std::vector<float>> dispMuon_px;
    std::unique_ptr<std::vector<float>> dispMuon_py;
    std::unique_ptr<std::vector<float>> dispMuon_pz;
    std::unique_ptr<std::vector<bool>> dispMuon_isSAMuon;
    std::unique_ptr<std::vector<bool>> dispMuon_isGlobalMuon;
    std::unique_ptr<std::vector<bool>> dispMuon_isTrackerMuon;
    std::unique_ptr<std::vector<bool>> dispMuon_isPFMuon;
    std::unique_ptr<std::vector<int>>  dispMuon_nChambers;
    std::unique_ptr<std::vector<int>>  dispMuon_nChambersCSCorDT;
    std::unique_ptr<std::vector<int>>  dispMuon_nMatches;
    std::unique_ptr<std::vector<int>>  dispMuon_nMatchedStations;
    std::unique_ptr<std::vector<unsigned int>> dispMuon_expectedNumberOfMatchedStations;
    std::unique_ptr<std::vector<unsigned int>> dispMuon_stationMask;
    std::unique_ptr<std::vector<int>>  dispMuon_nMatchedRPCLayers;
    std::unique_ptr<std::vector<unsigned int>> dispMuon_RPClayerMask;

    // Cosmic muon info
    std::unique_ptr<int32_t> cosmicMuon_size;
    std::unique_ptr<std::vector<float>> cosmicMuon_e;
    std::unique_ptr<std::vector<float>> cosmicMuon_et;
    std::unique_ptr<std::vector<float>> cosmicMuon_pt;
    std::unique_ptr<std::vector<float>> cosmicMuon_eta;
    std::unique_ptr<std::vector<float>> cosmicMuon_phi;
    std::unique_ptr<std::vector<float>> cosmicMuon_dxy;
    std::unique_ptr<std::vector<float>> cosmicMuon_dz;
    std::unique_ptr<std::vector<bool>> cosmicMuon_isLooseMuon;
    std::unique_ptr<std::vector<bool>> cosmicMuon_isMediumMuon;
    std::unique_ptr<std::vector<bool>> cosmicMuon_isTightMuon;
    std::unique_ptr<std::vector<float>> cosmicMuon_iso;
    std::unique_ptr<std::vector<short>> cosmicMuon_hlt_isomu;
    std::unique_ptr<std::vector<short>> cosmicMuon_hlt_mu;
    std::unique_ptr<std::vector<float>> cosmicMuon_hlt_isoDeltaR;
    std::unique_ptr<std::vector<float>> cosmicMuon_hlt_deltaR;
    std::unique_ptr<std::vector<int>> cosmicMuon_passesSingleMuon;
    std::unique_ptr<std::vector<int>> cosmicMuon_charge;
    std::unique_ptr<std::vector<float>> cosmicMuon_etaSt1;
    std::unique_ptr<std::vector<float>> cosmicMuon_phiSt1;
    std::unique_ptr<std::vector<float>> cosmicMuon_etaSt2;
    std::unique_ptr<std::vector<float>> cosmicMuon_phiSt2;
    std::unique_ptr<std::vector<float>> cosmicMuon_vx;
    std::unique_ptr<std::vector<float>> cosmicMuon_vy;
    std::unique_ptr<std::vector<float>> cosmicMuon_vz;
    std::unique_ptr<std::vector<float>> cosmicMuon_px;
    std::unique_ptr<std::vector<float>> cosmicMuon_py;
    std::unique_ptr<std::vector<float>> cosmicMuon_pz;
    std::unique_ptr<std::vector<bool>> cosmicMuon_isSAMuon;
    std::unique_ptr<std::vector<bool>> cosmicMuon_isGlobalMuon;
    std::unique_ptr<std::vector<bool>> cosmicMuon_isTrackerMuon;
    std::unique_ptr<std::vector<bool>> cosmicMuon_isPFMuon;
    std::unique_ptr<std::vector<int>>  cosmicMuon_nChambers;
    std::unique_ptr<std::vector<int>>  cosmicMuon_nChambersCSCorDT;
    std::unique_ptr<std::vector<int>>  cosmicMuon_nMatches;
    std::unique_ptr<std::vector<int>>  cosmicMuon_nMatchedStations;
    std::unique_ptr<std::vector<unsigned int>> cosmicMuon_expectedNumberOfMatchedStations;
    std::unique_ptr<std::vector<unsigned int>> cosmicMuon_stationMask;
    std::unique_ptr<std::vector<int>>  cosmicMuon_nMatchedRPCLayers;
    std::unique_ptr<std::vector<unsigned int>> cosmicMuon_RPClayerMask;

    // Reco muon info
    std::unique_ptr<int32_t> cosmicMuon1Leg_size;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_e;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_et;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_pt;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_eta;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_phi;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_dxy;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_dz;
    std::unique_ptr<std::vector<bool>> cosmicMuon1Leg_isLooseMuon;
    std::unique_ptr<std::vector<bool>> cosmicMuon1Leg_isMediumMuon;
    std::unique_ptr<std::vector<bool>> cosmicMuon1Leg_isTightMuon;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_iso;
    std::unique_ptr<std::vector<short>> cosmicMuon1Leg_hlt_isomu;
    std::unique_ptr<std::vector<short>> cosmicMuon1Leg_hlt_mu;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_hlt_isoDeltaR;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_hlt_deltaR;
    std::unique_ptr<std::vector<int>> cosmicMuon1Leg_passesSingleMuon;
    std::unique_ptr<std::vector<int>> cosmicMuon1Leg_charge;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_etaSt1;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_phiSt1;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_etaSt2;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_phiSt2;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_vx;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_vy;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_vz;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_px;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_py;
    std::unique_ptr<std::vector<float>> cosmicMuon1Leg_pz;
    std::unique_ptr<std::vector<bool>> cosmicMuon1Leg_isSAMuon;
    std::unique_ptr<std::vector<bool>> cosmicMuon1Leg_isGlobalMuon;
    std::unique_ptr<std::vector<bool>> cosmicMuon1Leg_isTrackerMuon;
    std::unique_ptr<std::vector<bool>> cosmicMuon1Leg_isPFMuon;
    std::unique_ptr<std::vector<int>>  cosmicMuon1Leg_nChambers;
    std::unique_ptr<std::vector<int>>  cosmicMuon1Leg_nChambersCSCorDT;
    std::unique_ptr<std::vector<int>>  cosmicMuon1Leg_nMatches;
    std::unique_ptr<std::vector<int>>  cosmicMuon1Leg_nMatchedStations;
    std::unique_ptr<std::vector<unsigned int>> cosmicMuon1Leg_expectedNumberOfMatchedStations;
    std::unique_ptr<std::vector<unsigned int>> cosmicMuon1Leg_stationMask;
    std::unique_ptr<std::vector<int>>  cosmicMuon1Leg_nMatchedRPCLayers;
    std::unique_ptr<std::vector<unsigned int>> cosmicMuon1Leg_RPClayerMask;
    

    // Trigger flags
    std::unique_ptr<bool> HLT_IsoMu24;
    std::unique_ptr<bool> HLT_Mu50_L1SingleMuShower;
    std::unique_ptr<bool> HLT_Mu50;
};