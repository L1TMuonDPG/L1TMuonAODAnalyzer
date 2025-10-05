// -*- C++ -*-
//
// Package:    EMTFTools/MuonAODAnalyzer
// Class:      MuonAODAnalyzer
//
/**\class MuonAODAnalyzer MuonAODAnalyzer.cc EMTFTools/MuonAODAnalyzer/plugins/MuonAODAnalyzer.cc

 Description: Creates flat ntuples to be used for EMTF studies.

*/
//
// Original Author:  Efe Yigitbasi
//         Created:  Tue, 01 Sep 2020 10:52:51 GMT
//
//

// system include files
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

// ROOT includes
#include "TFile.h"
#include "TMath.h"
#include "TString.h"
#include "TTree.h"

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

#include "DataFormats/CSCRecHit/interface/CSCSegmentCollection.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "Geometry/CSCGeometry/interface/CSCChamber.h"
#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"
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
#include "TString.h"
#include "TRegexp.h"
#include <utility>
// vertex
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
// track extrapolation
#include "MuonAnalysis/MuonAssociators/interface/PropagateToMuonSetup.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateOnSurface.h"
#include "CondFormats/AlignmentRecord/interface/TrackerSurfaceDeformationRcd.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "TrackingTools/TrajectoryState/interface/FreeTrajectoryState.h"
#include "MagneticField/Engine/interface/MagneticField.h"


#include "L1Trigger/L1TMuon/interface/GeometryTranslator.h"
#include "L1Trigger/L1TMuonEndCap/interface/Common.h"
#include "L1Trigger/L1TMuonEndCap/interface/EMTFSubsystemCollector.h"
#include "L1Trigger/L1TMuonEndCap/interface/TrackTools.h"
#include "L1Trigger/L1TMuonEndCap/interface/DebugTools.h"

#include "DataFormats/L1Trigger/interface/Muon.h"
// #include "FWCore/Framework/interface/ConsumesCollector.h"

//
// ----------------------------------------------------
//

class MuonAODAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
  public:
    explicit MuonAODAnalyzer(const edm::ParameterSet &);
    ~MuonAODAnalyzer();

    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

  private:
    void beginJob() override;
    void beginRun(const edm::Run &, const edm::EventSetup &);
    auto getHitRefs(const l1t::EMTFTrack &trk,
                    const l1t::EMTFHitCollection &hits);
    void analyze(const edm::Event &, const edm::EventSetup &) override;
    void endJob() override;

    // Aux functions
    void getHandles(const edm::Event &iEvent, const edm::EventSetup &iSetup);
    double match_trigger(std::vector<int> &trigIndices,
                    const trigger::TriggerObjectCollection &trigObjs,
                    const trigger::TriggerEvent &triggerEvent,
                    const reco::Muon &mu);
    void fillTree();
    void makeTree();
    void printDebugOutput();

    template <typename T> edm::Handle<T> make_handle(T &t) {
        return edm::Handle<T>();
    }

    template <typename T> edm::Handle<T> make_handle(T *t) {
        return edm::Handle<T>();
    }

    // ---------- Member data ---------------------------

    // Input parameters

    // Run 3 TPs
    const edm::InputTag CSCInputTag_;
    const edm::InputTag RPCInputTag_;
    const edm::InputTag CPPFInputTag_;
    const edm::InputTag GEMInputTag_;

    // Phase 2 TPs
    const edm::InputTag IRPCInputTag_;
    const edm::InputTag ME0InputTag_;
    const edm::InputTag DTInputTag_;

    // L1T collections
    const edm::InputTag EMTFHitTag_;
    const edm::InputTag EMTFUnpHitTag_;

    const edm::InputTag EMTFTrackTag_;
    const edm::InputTag EMTFUnpTrackTag_;

    const edm::InputTag GMTMuonTag_;
    const edm::InputTag GMTUnpMuonTag_;

    const edm::InputTag GENPartTag_;
    const edm::InputTag RecoMuonTag_;

    // Reco CSC segments
    const edm::InputTag CSCSegmentTag_;

    const std::string outFileName_;
    int verbose_;

    bool enablePhase2_;

    bool useCSC_;
    bool useRPC_;
    bool useCPPF_;
    bool useGEM_;

    bool useIRPC_;
    bool useME0_;
    bool useDT_;

    bool useEMTFHits_;
    bool useEMTFUnpHits_;

    bool useEMTFTracks_;
    bool useEMTFUnpTracks_;

    bool useGMTMuons_;
    bool useGMTUnpMuons_;

    bool useGENParts_;
    bool useRecoMuons_;
    bool useEventInfo_;

    bool useCSCSegments_;
    bool matchCSCSegments_;

    bool isReco_;

    bool debug_;
    // trig matching
    std::vector<std::string> isoTriggerNames_;
    std::vector<std::string> triggerNames_;
    // Run 3 TPs
    edm::EDGetTokenT<emtf::CSCTag::digi_collection> CSCInputToken_;
    edm::EDGetTokenT<emtf::RPCTag::digi_collection> RPCInputToken_;
    edm::EDGetTokenT<emtf::CPPFTag::digi_collection> CPPFInputToken_;
    edm::EDGetTokenT<emtf::GEMTag::digi_collection> GEMInputToken_;

    // Phase 2 TPs
    edm::EDGetTokenT<emtf::IRPCTag::digi_collection> IRPCInputToken_;
    edm::EDGetTokenT<emtf::ME0Tag::digi_collection> ME0InputToken_;
    edm::EDGetTokenT<emtf::DTTag::digi_collection> DTInputToken_;

    // L1T collections
    edm::EDGetTokenT<l1t::EMTFHitCollection> EMTFHitToken_;
    edm::EDGetTokenT<l1t::EMTFHitCollection> EMTFUnpHitToken_;

    edm::EDGetTokenT<l1t::EMTFTrackCollection> EMTFTrackToken_;
    edm::EDGetTokenT<l1t::EMTFTrackCollection> EMTFUnpTrackToken_;

    edm::EDGetTokenT<l1t::MuonBxCollection> GMTMuonToken_;
    edm::EDGetTokenT<l1t::MuonBxCollection> GMTUnpMuonToken_;

    edm::EDGetTokenT<reco::GenParticleCollection> GENPartToken_;

    edm::EDGetTokenT<CSCSegmentCollection> CSCSegmentToken_;

    edm::ESGetToken<CSCGeometry, MuonGeometryRecord> cscGeomToken_;
    edm::ESHandle<CSCGeometry> cscGeom_;

    // reco muon related collections
    edm::EDGetTokenT<reco::MuonCollection> RecoMuonToken_;
    edm::EDGetTokenT<edm::TriggerResults> TriggerResultsToken_;
    edm::EDGetTokenT<trigger::TriggerEvent> TriggerSummaryLabelsToken_;
    edm::EDGetTokenT<reco::VertexCollection> VerticesToken_;
    edm::Handle<edm::TriggerResults> IsoTriggerToken_;
    edm::Handle<std::vector<std::string>> IsoTriggerNamesToken_;
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

    GeometryTranslator geometryTranslator_;

    EMTFSubsystemCollector collector_;

    TriggerPrimitiveCollection CSCInputs_;
    TriggerPrimitiveCollection RPCInputs_;
    TriggerPrimitiveCollection CPPFInputs_;
    TriggerPrimitiveCollection GEMInputs_;

    TriggerPrimitiveCollection IRPCInputs_;
    TriggerPrimitiveCollection ME0Inputs_;
    TriggerPrimitiveCollection DTInputs_;

    l1t::EMTFHitCollection EMTFHits_;
    l1t::EMTFHitCollection EMTFUnpHits_;

    l1t::EMTFTrackCollection EMTFTracks_;
    l1t::EMTFTrackCollection EMTFUnpTracks_;

    const l1t::MuonBxCollection *GMTMuons_;
    const l1t::MuonBxCollection *GMTUnpMuons_;

    const reco::GenParticleCollection *GENParts_;

    const CSCSegmentCollection *CSCSegments_;

    const reco::MuonCollection *RecoMuons_;
    const edm::TriggerResults *TriggerResults_;
    const trigger::TriggerEvent *TriggerSummaryLabels_;
    const reco::VertexCollection *Vertices_;

    // TTree
    TTree *tree;

    bool firstEvent_;

    // Output collections

    // CSC Inputs
    std::unique_ptr<std::vector<int16_t>> cscInput_endcap;
    std::unique_ptr<std::vector<int16_t>> cscInput_station;
    std::unique_ptr<std::vector<int16_t>> cscInput_ring;
    std::unique_ptr<std::vector<int16_t>> cscInput_sector;
    std::unique_ptr<std::vector<int16_t>> cscInput_chamber;
    std::unique_ptr<std::vector<int16_t>> cscInput_cscid;
    std::unique_ptr<std::vector<int16_t>> cscInput_bx;
    std::unique_ptr<std::vector<int16_t>> cscInput_strip;
    std::unique_ptr<std::vector<int16_t>> cscInput_wire;
    std::unique_ptr<std::vector<int16_t>> cscInput_quality;
    std::unique_ptr<std::vector<int16_t>> cscInput_pattern;
    std::unique_ptr<std::vector<int16_t>> cscInput_bend;

    // RPC Inputs
    std::unique_ptr<std::vector<int16_t>> rpcInput_region;
    std::unique_ptr<std::vector<int16_t>> rpcInput_station;
    std::unique_ptr<std::vector<int16_t>> rpcInput_ring;
    std::unique_ptr<std::vector<int16_t>> rpcInput_sector;
    std::unique_ptr<std::vector<int16_t>> rpcInput_subsector;
    std::unique_ptr<std::vector<int16_t>> rpcInput_roll;
    std::unique_ptr<std::vector<int16_t>> rpcInput_bx;
    std::unique_ptr<std::vector<int16_t>> rpcInput_strip;
    std::unique_ptr<std::vector<int16_t>> rpcInput_strip_high;
    std::unique_ptr<std::vector<int16_t>> rpcInput_strip_low;
    std::unique_ptr<std::vector<int16_t>> rpcInput_time;
    std::unique_ptr<std::vector<int16_t>> rpcInput_valid;

    // GEM Inputs
    std::unique_ptr<std::vector<int16_t>> gemInput_region;
    std::unique_ptr<std::vector<int16_t>> gemInput_station;
    std::unique_ptr<std::vector<int16_t>> gemInput_ring;
    std::unique_ptr<std::vector<int16_t>> gemInput_sector;
    std::unique_ptr<std::vector<int16_t>> gemInput_chamber;
    std::unique_ptr<std::vector<int16_t>> gemInput_roll;
    std::unique_ptr<std::vector<int16_t>> gemInput_bx;
    std::unique_ptr<std::vector<int16_t>> gemInput_layer;
    std::unique_ptr<std::vector<int16_t>> gemInput_pad;
    std::unique_ptr<std::vector<int16_t>> gemInput_pad_low;
    std::unique_ptr<std::vector<int16_t>> gemInput_pad_high;

    // ME0 Inputs
    std::unique_ptr<std::vector<int16_t>> me0Input_region;
    std::unique_ptr<std::vector<int16_t>> me0Input_station;
    std::unique_ptr<std::vector<int16_t>> me0Input_ring;
    std::unique_ptr<std::vector<int16_t>> me0Input_sector;
    std::unique_ptr<std::vector<int16_t>> me0Input_chamber;
    std::unique_ptr<std::vector<int16_t>> me0Input_roll;
    std::unique_ptr<std::vector<int16_t>> me0Input_bx;
    std::unique_ptr<std::vector<int16_t>> me0Input_layer;
    std::unique_ptr<std::vector<int16_t>> me0Input_phiposition;
    std::unique_ptr<std::vector<int16_t>> me0Input_deltaphi;
    std::unique_ptr<std::vector<int16_t>> me0Input_quality;
    std::unique_ptr<std::vector<int16_t>> me0Input_bend;
    std::unique_ptr<std::vector<int16_t>> me0Input_partition;

    // DT Inputs
    std::unique_ptr<std::vector<int16_t>> dtInput_wheel;
    std::unique_ptr<std::vector<int16_t>> dtInput_station;
    std::unique_ptr<std::vector<int16_t>> dtInput_btigroup;
    std::unique_ptr<std::vector<int16_t>> dtInput_bx;
    std::unique_ptr<std::vector<int16_t>> dtInput_strip;
    std::unique_ptr<std::vector<int16_t>> dtInput_wire;
    std::unique_ptr<std::vector<int16_t>> dtInput_quality;
    std::unique_ptr<std::vector<int16_t>> dtInput_bend;

    // EMTF Hits
    std::unique_ptr<std::vector<int16_t>> emtfHit_endcap;
    std::unique_ptr<std::vector<int16_t>> emtfHit_station;
    std::unique_ptr<std::vector<int16_t>> emtfHit_ring;
    std::unique_ptr<std::vector<int16_t>> emtfHit_sector;
    std::unique_ptr<std::vector<int16_t>> emtfHit_subsector;
    std::unique_ptr<std::vector<int16_t>> emtfHit_chamber;
    std::unique_ptr<std::vector<int16_t>> emtfHit_cscid;
    std::unique_ptr<std::vector<int16_t>> emtfHit_bx;
    std::unique_ptr<std::vector<int16_t>>
        emtfHit_type; // subsystem: DT=0,CSC=1,RPC=2,GEM=3,ME0=4
    std::unique_ptr<std::vector<int16_t>> emtfHit_neighbor;
    //
    std::unique_ptr<std::vector<int16_t>> emtfHit_strip;
    std::unique_ptr<std::vector<int16_t>> emtfHit_strip_qses;
    std::unique_ptr<std::vector<int16_t>> emtfHit_wire;
    std::unique_ptr<std::vector<int16_t>> emtfHit_roll;
    std::unique_ptr<std::vector<int16_t>> emtfHit_layer;
    std::unique_ptr<std::vector<int16_t>> emtfHit_quality;
    std::unique_ptr<std::vector<int16_t>> emtfHit_pattern;
    std::unique_ptr<std::vector<int16_t>> emtfHit_bend;
    std::unique_ptr<std::vector<int16_t>> emtfHit_slope;
    std::unique_ptr<std::vector<int16_t>> emtfHit_time;
    std::unique_ptr<std::vector<int16_t>> emtfHit_fr;
    std::unique_ptr<std::vector<int32_t>> emtfHit_emtf_phi;   // integer unit
    std::unique_ptr<std::vector<int32_t>> emtfHit_emtf_theta; // integer unit
    //
    std::unique_ptr<std::vector<float>> emtfHit_sim_phi;   // in degrees
    std::unique_ptr<std::vector<float>> emtfHit_sim_theta; // in degrees
    std::unique_ptr<std::vector<float>> emtfHit_sim_r;     // in cm
    std::unique_ptr<std::vector<float>> emtfHit_sim_z;     // in cm
    // std::unique_ptr<std::vector<int32_t> >  emtfHit_sim_tp1;
    // std::unique_ptr<std::vector<int32_t> >  emtfHit_sim_tp2;
    std::unique_ptr<std::vector<int32_t>> emtfHit_match_iSeg;
    std::unique_ptr<std::vector<int32_t>> emtfHit_match_iSeg2;
    std::unique_ptr<std::vector<int32_t>> emtfHit_match_nSegs;
    std::unique_ptr<std::vector<int16_t>> emtfHit_match_seg_unique;
    std::unique_ptr<int32_t> emtfHit_size;

    // EMTF Unpacked Hits
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_endcap;
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_station;
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_ring;
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_sector;
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_subsector;
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_chamber;
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_cscid;
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_bx;
    std::unique_ptr<std::vector<int16_t>>
        emtfUnpHit_type; // subsystem: DT=0,CSC=1,RPC=2,GEM=3,ME0=4
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_neighbor;
    //
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_strip;
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_strip_qses;
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_wire;
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_roll;
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_quality;
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_pattern;
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_bend;
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_time;
    std::unique_ptr<std::vector<int16_t>> emtfUnpHit_fr;
    std::unique_ptr<std::vector<int32_t>> emtfUnpHit_emtf_phi;   // integer unit
    std::unique_ptr<std::vector<int32_t>> emtfUnpHit_emtf_theta; // integer unit
    //
    std::unique_ptr<std::vector<float>> emtfUnpHit_sim_phi;   // in degrees
    std::unique_ptr<std::vector<float>> emtfUnpHit_sim_theta; // in degrees
    std::unique_ptr<std::vector<float>> emtfUnpHit_sim_r;     // in cm
    std::unique_ptr<std::vector<float>> emtfUnpHit_sim_z;     // in cm
    // std::unique_ptr<std::vector<int32_t> >  emtfUnpHit_sim_tp1;
    // std::unique_ptr<std::vector<int32_t> >  emtfUnpHit_sim_tp2;
    std::unique_ptr<int32_t> emtfUnpHit_size;

    // EMTF Tracks
    std::unique_ptr<std::vector<float>> emtfTrack_pt;
    std::unique_ptr<std::vector<float>> emtfTrack_xml_pt;
    std::unique_ptr<std::vector<float>> emtfTrack_pt_dxy;
    std::unique_ptr<std::vector<float>> emtfTrack_dxy;
    std::unique_ptr<std::vector<float>> emtfTrack_invpt_prompt;
    std::unique_ptr<std::vector<float>> emtfTrack_invpt_displ;
    std::unique_ptr<std::vector<float>> emtfTrack_phi;        // in degrees
    std::unique_ptr<std::vector<int32_t>> emtfTrack_phi_fp;   // in degrees
    std::unique_ptr<std::vector<float>> emtfTrack_theta;      // in degrees
    std::unique_ptr<std::vector<int32_t>> emtfTrack_theta_fp; // in degrees
    std::unique_ptr<std::vector<float>> emtfTrack_eta;
    std::unique_ptr<std::vector<int32_t>> emtfTrack_GMT_phi;
    std::unique_ptr<std::vector<int32_t>> emtfTrack_GMT_eta;
    std::unique_ptr<std::vector<int32_t>> emtfTrack_GMT_dxy;    
    std::unique_ptr<std::vector<int16_t>> emtfTrack_q; // charge
    //
    std::unique_ptr<std::vector<int16_t>> emtfTrack_mode;
    std::unique_ptr<std::vector<int16_t>> emtfTrack_endcap;
    std::unique_ptr<std::vector<int16_t>> emtfTrack_sector;
    std::unique_ptr<std::vector<int16_t>> emtfTrack_bx;
    std::unique_ptr<std::vector<int16_t>> emtfTrack_nhits;
    std::unique_ptr<std::vector<int32_t>> emtfTrack_hitref1;
    std::unique_ptr<std::vector<int32_t>> emtfTrack_hitref2;
    std::unique_ptr<std::vector<int32_t>> emtfTrack_hitref3;
    std::unique_ptr<std::vector<int32_t>> emtfTrack_hitref4;
    std::unique_ptr<int32_t> emtfTrack_size;

    std::unique_ptr<std::vector<int16_t>> emtfTrack_straightness;

    // PtLUT information
    std::unique_ptr<std::vector<uint64_t>> emtfTrack_ptLUT_address;
    std::unique_ptr<std::vector<uint16_t>> emtfTrack_ptLUT_mode;
    std::unique_ptr<std::vector<uint16_t>> emtfTrack_ptLUT_theta;
    std::unique_ptr<std::vector<uint16_t>> emtfTrack_ptLUT_st1_ring2;
    std::unique_ptr<std::vector<uint16_t>> emtfTrack_ptLUT_eta;
    std::unique_ptr<std::vector<std::vector<uint16_t>>> emtfTrack_ptLUT_deltaPh;
    std::unique_ptr<std::vector<std::vector<uint16_t>>> emtfTrack_ptLUT_deltaTh;
    std::unique_ptr<std::vector<std::vector<uint16_t>>> emtfTrack_ptLUT_signPh;
    std::unique_ptr<std::vector<std::vector<uint16_t>>> emtfTrack_ptLUT_signTh;
    std::unique_ptr<std::vector<std::vector<uint16_t>>>
        emtfTrack_ptLUT_cpattern;
    std::unique_ptr<std::vector<std::vector<uint16_t>>> emtfTrack_ptLUT_fr;
    std::unique_ptr<std::vector<std::vector<uint16_t>>> emtfTrack_ptLUT_bt_vi;
    std::unique_ptr<std::vector<std::vector<uint16_t>>> emtfTrack_ptLUT_bt_hi;
    std::unique_ptr<std::vector<std::vector<uint16_t>>> emtfTrack_ptLUT_bt_ci;
    std::unique_ptr<std::vector<std::vector<uint16_t>>> emtfTrack_ptLUT_bt_si;

    // EMTF Unpacked Tracks
    std::unique_ptr<std::vector<float>> emtfUnpTrack_pt;
    std::unique_ptr<std::vector<float>> emtfUnpTrack_xml_pt;
    std::unique_ptr<std::vector<float>> emtfUnpTrack_pt_dxy;
    std::unique_ptr<std::vector<float>> emtfUnpTrack_dxy;
    std::unique_ptr<std::vector<float>> emtfUnpTrack_invpt_prompt;
    std::unique_ptr<std::vector<float>> emtfUnpTrack_invpt_displ;
    std::unique_ptr<std::vector<float>> emtfUnpTrack_phi;   // in degrees
    std::unique_ptr<std::vector<float>> emtfUnpTrack_theta; // in degrees
    std::unique_ptr<std::vector<float>> emtfUnpTrack_eta;
    std::unique_ptr<std::vector<int32_t>> emtfUnpTrack_GMT_dxy;
    std::unique_ptr<std::vector<int16_t>> emtfUnpTrack_q; // charge
    //
    std::unique_ptr<std::vector<int16_t>> emtfUnpTrack_mode;
    std::unique_ptr<std::vector<int16_t>> emtfUnpTrack_endcap;
    std::unique_ptr<std::vector<int16_t>> emtfUnpTrack_sector;
    std::unique_ptr<std::vector<int16_t>> emtfUnpTrack_bx;
    std::unique_ptr<std::vector<int16_t>> emtfUnpTrack_nhits;
    std::unique_ptr<std::vector<int32_t>> emtfUnpTrack_hitref1;
    std::unique_ptr<std::vector<int32_t>> emtfUnpTrack_hitref2;
    std::unique_ptr<std::vector<int32_t>> emtfUnpTrack_hitref3;
    std::unique_ptr<std::vector<int32_t>> emtfUnpTrack_hitref4;
    std::unique_ptr<int32_t> emtfUnpTrack_size;
    // PtLUT information
    std::unique_ptr<std::vector<uint64_t>> emtfUnpTrack_ptLUT_address;
    std::unique_ptr<std::vector<uint16_t>> emtfUnpTrack_ptLUT_mode;
    std::unique_ptr<std::vector<uint16_t>> emtfUnpTrack_ptLUT_theta;
    std::unique_ptr<std::vector<uint16_t>> emtfUnpTrack_ptLUT_st1_ring2;
    std::unique_ptr<std::vector<uint16_t>> emtfUnpTrack_ptLUT_eta;
    std::unique_ptr<std::vector<std::vector<uint16_t>>>
        emtfUnpTrack_ptLUT_deltaPh;
    std::unique_ptr<std::vector<std::vector<uint16_t>>>
        emtfUnpTrack_ptLUT_deltaTh;
    std::unique_ptr<std::vector<std::vector<uint16_t>>>
        emtfUnpTrack_ptLUT_signPh;
    std::unique_ptr<std::vector<std::vector<uint16_t>>>
        emtfUnpTrack_ptLUT_signTh;
    std::unique_ptr<std::vector<std::vector<uint16_t>>>
        emtfUnpTrack_ptLUT_cpattern;
    std::unique_ptr<std::vector<std::vector<uint16_t>>> emtfUnpTrack_ptLUT_fr;
    std::unique_ptr<std::vector<std::vector<uint16_t>>>
        emtfUnpTrack_ptLUT_bt_vi;
    std::unique_ptr<std::vector<std::vector<uint16_t>>>
        emtfUnpTrack_ptLUT_bt_hi;
    std::unique_ptr<std::vector<std::vector<uint16_t>>>
        emtfUnpTrack_ptLUT_bt_ci;
    std::unique_ptr<std::vector<std::vector<uint16_t>>>
        emtfUnpTrack_ptLUT_bt_si;

    // GMT Muons
    std::unique_ptr<std::vector<float>> gmtMuon_pt;
    std::unique_ptr<std::vector<float>> gmtMuon_pt_dxy;
    std::unique_ptr<std::vector<int16_t>> gmtMuon_dxy;
    std::unique_ptr<std::vector<float>> gmtMuon_phi; // in degrees
    std::unique_ptr<std::vector<float>> gmtMuon_eta;
    std::unique_ptr<std::vector<int16_t>> gmtMuon_q; // charge
    std::unique_ptr<std::vector<int16_t>> gmtMuon_qual;
    std::unique_ptr<int32_t> gmtMuon_size;

    // GMT Unpacked Muons
    std::unique_ptr<std::vector<float>> gmtUnpMuon_pt;
    std::unique_ptr<std::vector<float>> gmtUnpMuon_pt_dxy;
    std::unique_ptr<std::vector<int16_t>> gmtUnpMuon_dxy;
    std::unique_ptr<std::vector<float>> gmtUnpMuon_phi; // in degrees
    std::unique_ptr<std::vector<float>> gmtUnpMuon_eta;
    std::unique_ptr<std::vector<int16_t>> gmtUnpMuon_q; // charge
    std::unique_ptr<std::vector<int16_t>> gmtUnpMuon_qual;
    std::unique_ptr<int32_t> gmtUnpMuon_size;

    // GEN particles
    std::unique_ptr<std::vector<float>> genPart_pt;
    std::unique_ptr<std::vector<float>> genPart_dxy;
    std::unique_ptr<std::vector<float>> genPart_eta;
    std::unique_ptr<std::vector<float>> genPart_phi;
    std::unique_ptr<std::vector<int16_t>> genPart_q; // charge
    std::unique_ptr<std::vector<int16_t>> genPart_ID;
    std::unique_ptr<std::vector<int32_t>> genPart_parentID;
    std::unique_ptr<std::vector<float>> genPart_vx;
    std::unique_ptr<std::vector<float>> genPart_vy;
    std::unique_ptr<std::vector<float>> genPart_vz;
    std::unique_ptr<std::vector<float>> genPart_etaSt1;
    std::unique_ptr<std::vector<float>> genPart_phiSt1;
    std::unique_ptr<std::vector<float>> genPart_etaSt2;
    std::unique_ptr<std::vector<float>> genPart_phiSt2;

    // Event info
    unsigned long eventInfo_event;
    unsigned long eventInfo_run;
    unsigned long eventInfo_lumi;
    unsigned long eventInfo_bx;

    //Nb of primary vertices
    int eventInfo_npv;
    int eventInfo_nvtx;

    // CSC Segment info
    std::unique_ptr<std::vector<float>> cscSegment_locX;
    std::unique_ptr<std::vector<float>> cscSegment_locY;
    std::unique_ptr<std::vector<float>> cscSegment_globX;
    std::unique_ptr<std::vector<float>> cscSegment_globY;
    std::unique_ptr<std::vector<float>> cscSegment_globZ;
    std::unique_ptr<std::vector<float>> cscSegment_eta;
    std::unique_ptr<std::vector<float>> cscSegment_phi;
    std::unique_ptr<std::vector<float>> cscSegment_theta;
    std::unique_ptr<std::vector<float>> cscSegment_chi2;
    std::unique_ptr<std::vector<float>> cscSegment_time;
    std::unique_ptr<std::vector<float>> cscSegment_dirX;
    std::unique_ptr<std::vector<float>> cscSegment_dirY;
    std::unique_ptr<std::vector<float>> cscSegment_dirZ;
    std::unique_ptr<std::vector<float>> cscSegment_bend_theta;
    std::unique_ptr<std::vector<float>> cscSegment_bend_phi;
    std::unique_ptr<std::vector<int32_t>> cscSegment_endcap;
    std::unique_ptr<std::vector<int32_t>> cscSegment_ring;
    std::unique_ptr<std::vector<int32_t>> cscSegment_station;
    std::unique_ptr<std::vector<int32_t>> cscSegment_chamber;
    std::unique_ptr<std::vector<int32_t>> cscSegment_sector;
    std::unique_ptr<std::vector<int32_t>> cscSegment_CSC_ID;
    std::unique_ptr<std::vector<int32_t>> cscSegment_nRecHits;
    std::unique_ptr<std::vector<int32_t>> cscSegment_wire_min;
    std::unique_ptr<std::vector<int32_t>> cscSegment_wire_max;
    std::unique_ptr<std::vector<int32_t>> cscSegment_strip_min;
    std::unique_ptr<std::vector<int32_t>> cscSegment_strip_max;
    std::unique_ptr<std::vector<int32_t>> cscSegment_match_iHit;
    std::unique_ptr<std::vector<int32_t>> cscSegment_match_iHit2;
    std::unique_ptr<std::vector<int32_t>> cscSegment_match_nHits;
    std::unique_ptr<std::vector<int16_t>> cscSegment_match_hit_unique;
    std::unique_ptr<int32_t> cscSegment_size;
    std::unique_ptr<int32_t> cscSegment_nBx0;

    // Reco muon info
    std::unique_ptr<int32_t> recoMuon_size;
    std::unique_ptr<std::vector<float>> recoMuon_e;
    std::unique_ptr<std::vector<float>> recoMuon_et;
    std::unique_ptr<std::vector<float>> recoMuon_pt;
    std::unique_ptr<std::vector<float>> recoMuon_eta;
    std::unique_ptr<std::vector<float>> recoMuon_phi;
    std::unique_ptr<std::vector<float>> recoMuon_dxy;
    std::unique_ptr<std::vector<float>> recoMuon_dz;
    std::unique_ptr<std::vector<bool>> recoMuon_isLooseMuon;
    std::unique_ptr<std::vector<bool>> recoMuon_isMediumMuon;
    std::unique_ptr<std::vector<bool>> recoMuon_isTightMuon;
    std::unique_ptr<std::vector<float>> recoMuon_iso;
    std::unique_ptr<std::vector<short>> recoMuon_hlt_isomu;
    std::unique_ptr<std::vector<short>> recoMuon_hlt_mu;
    std::unique_ptr<std::vector<float>> recoMuon_hlt_isoDeltaR;
    std::unique_ptr<std::vector<float>> recoMuon_hlt_deltaR;
    std::unique_ptr<std::vector<int>> recoMuon_passesSingleMuon;
    std::unique_ptr<std::vector<int>> recoMuon_charge;
    std::unique_ptr<std::vector<float>> recoMuon_etaSt1;
    std::unique_ptr<std::vector<float>> recoMuon_phiSt1;
    std::unique_ptr<std::vector<float>> recoMuon_etaSt2;
    std::unique_ptr<std::vector<float>> recoMuon_phiSt2;
};