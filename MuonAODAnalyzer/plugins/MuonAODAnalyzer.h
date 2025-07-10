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
#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include "TLorentzVector.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TTree.h"
#include "TMath.h"
#include <fmt/printf.h>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CondFormats/DataRecord/interface/L1TUtmTriggerMenuRcd.h"
#include "CondFormats/L1TObjects/interface/L1TUtmTriggerMenu.h"


// Data formats
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/L1Trigger/interface/Muon.h"
#include "DataFormats/L1Trigger/interface/BXVector.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/L1TGlobal/interface/GlobalAlgBlk.h"
#include "DataFormats/L1TGlobal/interface/GlobalExtBlk.h"
#include "DataFormats/L1TMuon/interface/RegionalMuonCand.h"

// Information about stations
#include "DataFormats/MuonReco/interface/Muon.h"

// muon track extrapolation
#include "MuonAnalysis/MuonAssociators/interface/PropagateToMuonSetup.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateOnSurface.h"


//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.
using namespace edm;
// using namespace std;
using namespace reco;


class MuonAODAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
  public:
    explicit MuonAODAnalyzer(const edm::ParameterSet&);
    ~MuonAODAnalyzer() override;

  private:
    void beginJob() override;
    void analyze(const edm::Event&, const edm::EventSetup&) override;
    void endJob() override;
    // void beginRun(const edm::Run&, const edm::EventSetup&);
    // void endRun(const edm::Run&, const edm::EventSetup&);
    virtual void InitandClearStuff();

    // void fillTree();
    // void makeTree();


    // ----------member data ---------------------------
    edm::EDGetTokenT<std::vector< reco::Muon> > muonToken_;
    edm::EDGetTokenT<l1t::MuonBxCollection>l1MuonToken_;
    edm::EDGetTokenT<BXVector<l1t::RegionalMuonCand>> l1BMTFRegionalMuonCandToken_;
    edm::EDGetTokenT<std::vector<Vertex> > verticesToken_;
    edm::EDGetTokenT<edm::TriggerResults> trgresultsToken_;
    edm::EDGetTokenT<GlobalExtBlkBxCollection> UnprefirableEventToken_;
    edm::EDGetTokenT<BXVector<GlobalAlgBlk>> l1GtToken_;

    edm::EDGetTokenT<std::vector< reco::Muon> > dispMuonToken_;
    edm::EDGetTokenT<std::vector< reco::Muon> > CosmicMuonToken_;
    edm::EDGetTokenT<std::vector< reco::Muon> > CosmicMuon1LegToken_;

    Float_t MuonPtCut_;
    Bool_t SaveTree_, IsMC_, Debug_;

    const PropagateToMuonSetup muPropagatorSetup1st_;
    const PropagateToMuonSetup muPropagatorSetup2nd_;

    PropagateToMuon muPropagator1st_;
    PropagateToMuon muPropagator2nd_;

    TTree* outputTree;

    unsigned long _eventNb;
    unsigned long _runNb;
    unsigned long _lumiBlock;
    unsigned long _bx;

    //Nb of primary vertices
    int _n_PV;
    // Float_t _LV_x,_LV_y,_LV_z;
    // Float_t _LV_errx,_LV_erry,_LV_errz;
    // Float_t _PUV1_x,_PUV1_y,_PUV1_z;
    int trueNVtx;

    GlobalAlgBlk const *results_;
    unsigned long long cache_id_;

    //MINIAOD original MET filters decisions
    bool Flag_goodVertices;
    bool Flag_globalTightHalo2016Filter;
    bool Flag_globalSuperTightHalo2016Filter;
    bool Flag_BadPFMuonFilter;
    bool Flag_BadPFMuonDzFilter;

    //Muons
    vector<Float_t>  muon_eta;
    vector<Float_t>  muon_etaAtSt1;
    vector<Float_t>  muon_etaAtSt2;
    vector<Float_t>  muon_phi;
    vector<Float_t>  muon_phiAtSt1;
    vector<Float_t>  muon_phiAtSt2;
    vector<Float_t>  muon_pt;
    vector<Float_t>  muon_ptCorr;
    vector <int>     muon_charge;
    vector<Float_t>  muon_dz;
    vector<Float_t>  muon_dzError;
    vector<Float_t>  muon_dxy;
    vector<Float_t>  muon_dxyError;
    vector<Float_t>  muon_3dIP;
    vector<Float_t>  muon_3dIPError;
    vector<Bool_t>  muon_PassTightID;
    vector<Bool_t>  muon_PassLooseID;
    vector<Bool_t> muon_isSAMuon;
    vector<Bool_t> muon_isGlobalMuon;
    vector<Bool_t> muon_isTrackerMuon;
    vector<Bool_t> muon_isPFMuon;
    vector<Bool_t> muon_hasInnerTrack;

    vector<Float_t> muon_vx;
    vector<Float_t> muon_vy;
    vector<Float_t> muon_vz;
    vector<Float_t> muon_px;
    vector<Float_t> muon_py;
    vector<Float_t> muon_pz;

    vector<int> muon_nChambers;
    vector<int> muon_nChambersCSCorDT;
    vector<int> muon_nMatches;
    vector<int> muon_nMatchedStations;
    vector<unsigned int> muon_expectedNumberOfMatchedStations;
    vector<unsigned int> muon_stationMask;
    vector<int> muon_nMatchedRPCLayers;
    vector<unsigned int> muon_RPClayerMask;
    int muon_size;

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
    
    // BMTF RegionalMuonCand muons
    vector <Float_t> BMTFMu_processor;
    vector <Float_t> BMTFMu_hwPt;
    vector <Float_t> BMTFMu_hwQual;
    vector <Float_t> BMTFMu_hwSign;
    vector <Float_t> BMTFMu_hwSignValid;
    vector <Float_t> BMTFMu_hwEta;
    vector <Float_t> BMTFMu_hwPhi;

    //Triggers
    bool HLT_IsoMu27;
    bool HLT_IsoMu24;
  
    bool Flag_IsUnprefirable;
    bool passL1_Final_bxmin1;
    bool passL1_Final_bxmin2;

    //
    // constants, enums and typedefs
    //

    const int  N_METFilters=18;
    enum METFilterIndex{
      idx_Flag_goodVertices,
      idx_Flag_globalTightHalo2016Filter,
      idx_Flag_globalSuperTightHalo2016Filter,
      idx_Flag_HBHENoiseFilter,
      idx_Flag_HBHENoiseIsoFilter,
      idx_Flag_EcalDeadCellTriggerPrimitiveFilter,
      idx_Flag_BadPFMuonFilter,
      idx_Flag_BadPFMuonDzFilter,
      idx_Flag_hfNoisyHitsFilter,
      idx_Flag_BadChargedCandidateFilter,
      idx_Flag_eeBadScFilter,
      idx_Flag_ecalBadCalibFilter,
      idx_Flag_ecalLaserCorrFilter,
      idx_Flag_EcalDeadCellBoundaryEnergyFilter,
      idx_PassecalBadCalibFilter_Update,
      idx_PassecalLaserCorrFilter_Update,
      idx_PassEcalDeadCellBoundaryEnergyFilter_Update,
      idx_PassBadChargedCandidateFilter_Update
    };


    //
    // static data member definitions
    //

};