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
    edm::EDGetTokenT<l1t::MuonBxCollection>l1MuonToken_;
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

    //Displaced Muons
    vector<Float_t>  dispMuon_eta;
    vector<Float_t>  dispMuon_etaAtSt1;
    vector<Float_t>  dispMuon_etaAtSt2;
    vector<Float_t>  dispMuon_phi;
    vector<Float_t>  dispMuon_phiAtSt1;
    vector<Float_t>  dispMuon_phiAtSt2;
    vector<Float_t>  dispMuon_pt;
    vector<Float_t>  dispMuon_ptCorr;
    vector <int>     dispMuon_charge;
    vector<Float_t>  dispMuon_dz;
    vector<Float_t>  dispMuon_dzError;
    vector<Float_t>  dispMuon_dxy;
    vector<Float_t>  dispMuon_dxyError;
    vector<Float_t>  dispMuon_3dIP;
    vector<Float_t>  dispMuon_3dIPError;
    vector<Bool_t>  dispMuon_PassTightID;
    vector<Bool_t>  dispMuon_PassLooseID;
    vector<Bool_t> dispMuon_isSAMuon;
    vector<Bool_t> dispMuon_isGlobalMuon;
    vector<Bool_t> dispMuon_isTrackerMuon;
    vector<Bool_t> dispMuon_isPFMuon;
    vector<Bool_t> dispMuon_hasInnerTrack;

    vector<Float_t> dispMuon_vx;
    vector<Float_t> dispMuon_vy;
    vector<Float_t> dispMuon_vz;
    vector<Float_t> dispMuon_px;
    vector<Float_t> dispMuon_py;
    vector<Float_t> dispMuon_pz;

    vector<int> dispMuon_nChambers;
    vector<int> dispMuon_nChambersCSCorDT;
    vector<int> dispMuon_nMatches;
    vector<int> dispMuon_nMatchedStations;
    vector<unsigned int> dispMuon_expectedNumberOfMatchedStations;
    vector<unsigned int> dispMuon_stationMask;
    vector<int> dispMuon_nMatchedRPCLayers;
    vector<unsigned int> dispMuon_RPClayerMask;
    int dispMuon_size;

    //Cosmic Muons
    vector<Float_t>  cosmicMuon_eta;
    vector<Float_t>  cosmicMuon_etaAtSt1;
    vector<Float_t>  cosmicMuon_etaAtSt2;
    vector<Float_t>  cosmicMuon_phi;
    vector<Float_t>  cosmicMuon_phiAtSt1;
    vector<Float_t>  cosmicMuon_phiAtSt2;
    vector<Float_t>  cosmicMuon_pt;
    vector<Float_t>  cosmicMuon_ptCorr;
    vector <int>     cosmicMuon_charge;
    vector<Float_t>  cosmicMuon_dz;
    vector<Float_t>  cosmicMuon_dzError;
    vector<Float_t>  cosmicMuon_dxy;
    vector<Float_t>  cosmicMuon_dxyError;
    vector<Float_t>  cosmicMuon_3dIP;
    vector<Float_t>  cosmicMuon_3dIPError;
    vector<Bool_t>  cosmicMuon_PassTightID;
    vector<Bool_t>  cosmicMuon_PassLooseID;
    vector<Bool_t> cosmicMuon_isSAMuon;
    vector<Bool_t> cosmicMuon_isGlobalMuon;
    vector<Bool_t> cosmicMuon_isTrackerMuon;
    vector<Bool_t> cosmicMuon_isPFMuon;
    vector<Bool_t> cosmicMuon_hasInnerTrack;

    vector<Float_t> cosmicMuon_vx;
    vector<Float_t> cosmicMuon_vy;
    vector<Float_t> cosmicMuon_vz;
    vector<Float_t> cosmicMuon_px;
    vector<Float_t> cosmicMuon_py;
    vector<Float_t> cosmicMuon_pz;

    vector<int> cosmicMuon_nChambers;
    vector<int> cosmicMuon_nChambersCSCorDT;
    vector<int> cosmicMuon_nMatches;
    vector<int> cosmicMuon_nMatchedStations;
    vector<unsigned int> cosmicMuon_expectedNumberOfMatchedStations;
    vector<unsigned int> cosmicMuon_stationMask;
    vector<int> cosmicMuon_nMatchedRPCLayers;
    vector<unsigned int> cosmicMuon_RPClayerMask;
    int cosmicMuon_size;

    //Cosmic Muons 1 Leg
    vector<Float_t>  cosmicMuon1Leg_eta;
    vector<Float_t>  cosmicMuon1Leg_etaAtSt1;
    vector<Float_t>  cosmicMuon1Leg_etaAtSt2;
    vector<Float_t>  cosmicMuon1Leg_phi;
    vector<Float_t>  cosmicMuon1Leg_phiAtSt1;
    vector<Float_t>  cosmicMuon1Leg_phiAtSt2;
    vector<Float_t>  cosmicMuon1Leg_pt;
    vector<Float_t>  cosmicMuon1Leg_ptCorr;
    vector <int>     cosmicMuon1Leg_charge;
    vector<Float_t>  cosmicMuon1Leg_dz;
    vector<Float_t>  cosmicMuon1Leg_dzError;
    vector<Float_t>  cosmicMuon1Leg_dxy;
    vector<Float_t>  cosmicMuon1Leg_dxyError;
    vector<Float_t>  cosmicMuon1Leg_3dIP;
    vector<Float_t>  cosmicMuon1Leg_3dIPError;
    vector<Bool_t>  cosmicMuon1Leg_PassTightID;
    vector<Bool_t>  cosmicMuon1Leg_PassLooseID;
    vector<Bool_t> cosmicMuon1Leg_isSAMuon;
    vector<Bool_t> cosmicMuon1Leg_isGlobalMuon;
    vector<Bool_t> cosmicMuon1Leg_isTrackerMuon;
    vector<Bool_t> cosmicMuon1Leg_isPFMuon;
    vector<Bool_t> cosmicMuon1Leg_hasInnerTrack;

    vector<Float_t> cosmicMuon1Leg_vx;
    vector<Float_t> cosmicMuon1Leg_vy;
    vector<Float_t> cosmicMuon1Leg_vz;
    vector<Float_t> cosmicMuon1Leg_px;
    vector<Float_t> cosmicMuon1Leg_py;
    vector<Float_t> cosmicMuon1Leg_pz;

    vector<int> cosmicMuon1Leg_nChambers;
    vector<int> cosmicMuon1Leg_nChambersCSCorDT;
    vector<int> cosmicMuon1Leg_nMatches;
    vector<int> cosmicMuon1Leg_nMatchedStations;
    vector<unsigned int> cosmicMuon1Leg_expectedNumberOfMatchedStations;
    vector<unsigned int> cosmicMuon1Leg_stationMask;
    vector<int> cosmicMuon1Leg_nMatchedRPCLayers;
    vector<unsigned int> cosmicMuon1Leg_RPClayerMask;
    int cosmicMuon1Leg_size;

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
    

    // Trigger flags
    std::unique_ptr<bool> HLT_IsoMu24;
    std::unique_ptr<bool> HLT_Mu50_L1SingleMuShower;
    std::unique_ptr<bool> HLT_Mu50;
};