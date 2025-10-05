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

#include "MuonAODAnalyzer.h"


MuonAODAnalyzer::MuonAODAnalyzer(const edm::ParameterSet &iConfig)
    : CSCInputTag_(iConfig.getParameter<edm::InputTag>("CSCInputTag")),
      RPCInputTag_(iConfig.getParameter<edm::InputTag>("RPCInputTag")),
      CPPFInputTag_(iConfig.getParameter<edm::InputTag>("CPPFInputTag")),
      GEMInputTag_(iConfig.getParameter<edm::InputTag>("GEMInputTag")),

      IRPCInputTag_(iConfig.getParameter<edm::InputTag>("IRPCInputTag")),
      ME0InputTag_(iConfig.getParameter<edm::InputTag>("ME0InputTag")),
      DTInputTag_(iConfig.getParameter<edm::InputTag>("DTInputTag")),

      EMTFHitTag_(iConfig.getParameter<edm::InputTag>("EMTFHitTag")),
      EMTFUnpHitTag_(iConfig.getParameter<edm::InputTag>("EMTFUnpHitTag")),

      EMTFTrackTag_(iConfig.getParameter<edm::InputTag>("EMTFTrackTag")),
      EMTFUnpTrackTag_(iConfig.getParameter<edm::InputTag>("EMTFUnpTrackTag")),

      GMTMuonTag_(iConfig.getParameter<edm::InputTag>("GMTMuonTag")),
      GMTUnpMuonTag_(iConfig.getParameter<edm::InputTag>("GMTUnpMuonTag")),

      GENPartTag_(iConfig.getParameter<edm::InputTag>("GENPartTag")),
      RecoMuonTag_(iConfig.getParameter<edm::InputTag>("RecoMuonTag")),

      CSCSegmentTag_(iConfig.getParameter<edm::InputTag>("CSCSegmentTag")),

      outFileName_(iConfig.getParameter<std::string>("outFileName")),
      verbose_(iConfig.getUntrackedParameter<int>("verbosity")),

      enablePhase2_(iConfig.getParameter<bool>("enablePhase2")),

      useCSC_(iConfig.getParameter<bool>("useCSC")),
      useRPC_(iConfig.getParameter<bool>("useRPC")),
      useCPPF_(iConfig.getParameter<bool>("useCPPF")),
      useGEM_(iConfig.getParameter<bool>("useGEM")),

      useIRPC_(iConfig.getParameter<bool>("useIRPC")),
      useME0_(iConfig.getParameter<bool>("useME0")),
      useDT_(iConfig.getParameter<bool>("useDT")),

      useEMTFHits_(iConfig.getParameter<bool>("useEMTFHits")),
      useEMTFUnpHits_(iConfig.getParameter<bool>("useEMTFUnpHits")),

      useEMTFTracks_(iConfig.getParameter<bool>("useEMTFTracks")),
      useEMTFUnpTracks_(iConfig.getParameter<bool>("useEMTFUnpTracks")),

      useGMTMuons_(iConfig.getParameter<bool>("useGMTMuons")),
      useGMTUnpMuons_(iConfig.getParameter<bool>("useGMTUnpMuons")),

      useGENParts_(iConfig.getParameter<bool>("useGENParts")),
      useRecoMuons_(iConfig.getParameter<bool>("useRecoMuons")),
      useEventInfo_(iConfig.getParameter<bool>("useEventInfo")),

      useCSCSegments_(iConfig.getParameter<bool>("useCSCSegments")),
      matchCSCSegments_(iConfig.getParameter<bool>("matchCSCSegments")),

      isReco_(iConfig.getParameter<bool>("isReco")),
      debug_(iConfig.getParameter<bool>("debug")),
      //trig matching
      isoTriggerNames_(iConfig.getParameter<std::vector<std::string>>("isoTriggerNames")),
      triggerNames_(iConfig.getParameter<std::vector<std::string>>("triggerNames")),
      theBFieldToken_(esConsumes<MagneticField, IdealMagneticFieldRecord>(edm::ESInputTag("", ""))),

      muPropagatorSetup1st_(iConfig.getParameter<edm::ParameterSet>("muProp1st"), consumesCollector()),
      muPropagatorSetup2nd_(iConfig.getParameter<edm::ParameterSet>("muProp2nd"), consumesCollector()),

      geometryTranslator_(consumesCollector())

{
    usesResource("TFileService"); // shared resources

    firstEvent_ = true;

    CSCInputToken_ = consumes<emtf::CSCTag::digi_collection>(CSCInputTag_);
    RPCInputToken_ = consumes<emtf::RPCTag::digi_collection>(RPCInputTag_);
    CPPFInputToken_ = consumes<emtf::CPPFTag::digi_collection>(CPPFInputTag_);
    GEMInputToken_ = consumes<emtf::GEMTag::digi_collection>(GEMInputTag_);

    IRPCInputToken_ = consumes<emtf::IRPCTag::digi_collection>(IRPCInputTag_);
    ME0InputToken_ = consumes<emtf::ME0Tag::digi_collection>(ME0InputTag_);
    DTInputToken_ = consumes<emtf::DTTag::digi_collection>(DTInputTag_);

    EMTFHitToken_ = consumes<l1t::EMTFHitCollection>(EMTFHitTag_);
    EMTFUnpHitToken_ = consumes<l1t::EMTFHitCollection>(EMTFUnpHitTag_);

    EMTFTrackToken_ = consumes<l1t::EMTFTrackCollection>(EMTFTrackTag_);
    EMTFUnpTrackToken_ = consumes<l1t::EMTFTrackCollection>(EMTFUnpTrackTag_);

    GMTMuonToken_ = consumes<l1t::MuonBxCollection>(GMTMuonTag_);
    GMTUnpMuonToken_ = consumes<l1t::MuonBxCollection>(GMTUnpMuonTag_);

    GENPartToken_ = consumes<reco::GenParticleCollection>(GENPartTag_);

    CSCSegmentToken_ = consumes<CSCSegmentCollection>(CSCSegmentTag_);

    cscGeomToken_ = esConsumes<CSCGeometry, MuonGeometryRecord>();

    // reco muons
    RecoMuonToken_ = consumes<reco::MuonCollection>(RecoMuonTag_);
    TriggerResultsToken_ = consumes<edm::TriggerResults>(edm::InputTag("TriggerResults", "", "HLT"));
    TriggerSummaryLabelsToken_ = consumes<trigger::TriggerEvent>(edm::InputTag("hltTriggerSummaryAOD", "", "HLT"));
    VerticesToken_ = consumes<reco::VertexCollection>(edm::InputTag("offlinePrimaryVertices"));


    triggerMatching_ = true;
    triggerMaxDeltaR_ = 0.1;
    triggerProcessLabel_ = "HLT";
}

MuonAODAnalyzer::~MuonAODAnalyzer() {
    // Do nothing
}

auto MuonAODAnalyzer::getHitRefs(const l1t::EMTFTrack &trk,
                            const l1t::EMTFHitCollection &hits) {
    using namespace l1t;

    std::vector<int32_t> hit_refs = {-1, -1, -1, -1};
    EMTFHitCollection::const_iterator conv_hits_it1 = trk.Hits().begin();
    EMTFHitCollection::const_iterator conv_hits_end1 = trk.Hits().end();

    for (; conv_hits_it1 != conv_hits_end1; ++conv_hits_it1) {
        EMTFHitCollection::const_iterator conv_hits_it2 = hits.begin();
        EMTFHitCollection::const_iterator conv_hits_end2 = hits.end();

        for (; conv_hits_it2 != conv_hits_end2; ++conv_hits_it2) {
            const EMTFHit &conv_hit_i = *conv_hits_it1;
            const EMTFHit &conv_hit_j = *conv_hits_it2;

            // See L1Trigger/L1TMuonEndCap/src/PrimitiveMatching.cc
            // All these must match: [bx_history][station][chamber][segment]
            if ((conv_hit_i.Subsystem() == conv_hit_j.Subsystem()) &&
                (conv_hit_i.PC_station() == conv_hit_j.PC_station()) &&
                (conv_hit_i.PC_chamber() == conv_hit_j.PC_chamber()) &&
                (conv_hit_i.Ring() == conv_hit_j.Ring()) && // because of ME1/1
                (conv_hit_i.Strip() == conv_hit_j.Strip()) &&
                (conv_hit_i.Wire() == conv_hit_j.Wire()) &&
                (conv_hit_i.Pattern() == conv_hit_j.Pattern()) &&
                (conv_hit_i.BX() == conv_hit_j.BX()) &&
                (conv_hit_i.Strip_low() ==
                 conv_hit_j.Strip_low()) && // For RPC clusters
                (conv_hit_i.Strip_hi() ==
                 conv_hit_j.Strip_hi()) && // For RPC clusters
                (conv_hit_i.Roll() == conv_hit_j.Roll()) &&
                (conv_hit_i.Endcap() ==
                 conv_hit_j.Endcap()) && // Needed only in the ntupler
                (conv_hit_i.Sector() ==
                 conv_hit_j.Sector()) && // Needed only in the ntupler
                true) {
                int istation = (conv_hit_i.Station() - 1);
                auto hit_ref = std::distance(hits.begin(), conv_hits_it2);
                hit_refs.at(istation) = hit_ref;
            } // end if
        }     // end loop over hits
    }         // end loop over trk.Hits()

    // Sanity check
    // for (int istation = 0; istation < 4; ++istation) {
    //   bool has_hit = trk.Mode() & (1 << (3 - istation));
    //   bool has_hit_check = (hit_refs.at(istation) != -1);

    //   if (has_hit != has_hit_check) {
    //     std::cout << "[ERROR] mode: " << trk.Mode() << " station: " <<
    //     istation << std::endl; EMTFHitCollection::const_iterator
    //     conv_hits_it1  = trk.Hits().begin();
    //     EMTFHitCollection::const_iterator conv_hits_end1 = trk.Hits().end();
    //     for (; conv_hits_it1 != conv_hits_end1; ++conv_hits_it1) {
    //       const EMTFHit& conv_hit_i = *conv_hits_it1;
    //       std::cout << ".. " << conv_hit_i.Subsystem() << " " <<
    //       conv_hit_i.PC_station() << " " << conv_hit_i.PC_chamber() << " " <<
    //       conv_hit_i.Ring() << " " << conv_hit_i.Strip() << " " <<
    //       conv_hit_i.Wire() << " " << conv_hit_i.Pattern() << " " <<
    //       conv_hit_i.BX() << " " << conv_hit_i.Endcap() << " " <<
    //       conv_hit_i.Sector() << std::endl;
    //     }
    //     EMTFHitCollection::const_iterator conv_hits_it2  = hits.begin();
    //     EMTFHitCollection::const_iterator conv_hits_end2 = hits.end();
    //     for (; conv_hits_it2 != conv_hits_end2; ++conv_hits_it2) {
    //       const EMTFHit& conv_hit_j = *conv_hits_it2;
    //       std::cout << ".... " << conv_hit_j.Subsystem() << " " <<
    //       conv_hit_j.PC_station() << " " << conv_hit_j.PC_chamber() << " " <<
    //       conv_hit_j.Ring() << " " << conv_hit_j.Strip() << " " <<
    //       conv_hit_j.Wire() << " " << conv_hit_j.Pattern() << " " <<
    //       conv_hit_j.BX() << " " << conv_hit_j.Endcap() << " " <<
    //       conv_hit_j.Sector() << std::endl;
    //     }
    //   }
    //   assert(has_hit == has_hit_check);
    // }

    return hit_refs;
}

// ------------ method called for each event  ------------
void MuonAODAnalyzer::analyze(const edm::Event &iEvent,
                         const edm::EventSetup &iSetup) {
    // ___________________________________________________________________________
    // Get Handles
    getHandles(iEvent, iSetup);
    if (debug_) {  // debug
        printDebugOutput();
    }
    muPropagator1st_ = muPropagatorSetup1st_.init(iSetup);
    muPropagator2nd_ = muPropagatorSetup2nd_.init(iSetup);
    if (verbose_ > 0)
        std::cout << "******* Processing Objects *******" << std::endl;

    // ___________________________________________________________________________
    // Process objects

    // Run 3 inputs

    // CSC inputs
    if (useCSC_) {
        for (const auto &tp : CSCInputs_) {
            if (tp.subsystem() == L1TMuon::kCSC) {
                const CSCDetId &tp_detId = tp.detId<CSCDetId>();
                const CSCData &tp_data = tp.getCSCData();

                cscInput_bx->push_back(tp.getBX());
                cscInput_endcap->push_back(tp_detId.endcap());
                cscInput_station->push_back(tp_detId.station());
                cscInput_ring->push_back(tp_detId.ring());
                cscInput_sector->push_back(tp_detId.triggerSector());
                cscInput_chamber->push_back(tp_detId.chamber());
                cscInput_cscid->push_back(tp_data.cscID);
                cscInput_strip->push_back(tp.getStrip());
                cscInput_wire->push_back(tp.getWire());
                cscInput_quality->push_back(tp_data.quality);
                cscInput_pattern->push_back(tp_data.pattern);
                cscInput_bend->push_back(tp_data.bend);
            }
        } // end for loop
    }     // end if

    if (useRPC_) {
        for (const auto &tp : RPCInputs_) {
            if (tp.subsystem() == L1TMuon::kRPC) {
                const RPCDetId &tp_detId = tp.detId<RPCDetId>();
                const RPCData &tp_data = tp.getRPCData();

                rpcInput_bx->push_back(tp.getBX());
                rpcInput_region->push_back(tp_detId.region());
                rpcInput_station->push_back(tp_detId.station());
                rpcInput_ring->push_back(tp_detId.ring());
                rpcInput_sector->push_back(tp_detId.sector());
                rpcInput_subsector->push_back(tp_detId.subsector());
                rpcInput_roll->push_back(tp_detId.roll());
                rpcInput_strip->push_back(tp.getStrip());
                rpcInput_strip_high->push_back(tp_data.strip_hi);
                rpcInput_strip_low->push_back(tp_data.strip_low);
                rpcInput_time->push_back(tp_data.time);
                rpcInput_valid->push_back(tp_data.valid);
            }
        } // end for loop
    }     // end if

    if (useGEM_) {
        for (const auto &tp : GEMInputs_) {
            if (tp.subsystem() == L1TMuon::kGEM) {
                const GEMDetId &tp_detId = tp.detId<GEMDetId>();
                const GEMData &tp_data = tp.getGEMData();

                gemInput_bx->push_back(tp.getBX());
                gemInput_region->push_back(tp_detId.region());
                gemInput_station->push_back(tp_detId.station());
                gemInput_ring->push_back(tp_detId.ring());
                // gemInput_sector            ->push_back(tp_detId.sector());
                gemInput_chamber->push_back(tp_detId.chamber());
                gemInput_roll->push_back(tp_detId.roll());
                gemInput_layer->push_back(tp_detId.layer());
                gemInput_pad->push_back(tp.getStrip());
                gemInput_pad_low->push_back(tp_data.pad_low);
                gemInput_pad_high->push_back(tp_data.pad_hi);
            }
        } // end for loop
    }     // end if

    // Phase 2 inputs

    // TO DO

    // EMTF Hits
    for (const auto &hit : EMTFHits_) {
        emtfHit_endcap->push_back(hit.Endcap());
        emtfHit_station->push_back(hit.Station());
        emtfHit_ring->push_back(hit.Ring());
        emtfHit_sector->push_back(hit.PC_sector());
        emtfHit_subsector->push_back(hit.Subsector());
        emtfHit_chamber->push_back(hit.Chamber());
        emtfHit_cscid->push_back(hit.CSC_ID());
        emtfHit_bx->push_back(hit.BX());
        emtfHit_type->push_back(hit.Subsystem());
        emtfHit_neighbor->push_back(hit.Neighbor());
        //
        emtfHit_strip->push_back(hit.Strip());
        emtfHit_strip_qses->push_back((hit.Strip_quart_bit() << 1) + hit.Strip_eighth_bit());
        emtfHit_wire->push_back(hit.Wire());
        emtfHit_roll->push_back(hit.Roll());
        emtfHit_layer->push_back(hit.Layer());
        emtfHit_quality->push_back(hit.Quality());
        emtfHit_pattern->push_back(hit.Pattern());
        emtfHit_bend->push_back(hit.Bend());
        emtfHit_slope->push_back(hit.Slope());
        emtfHit_time->push_back(hit.Time());
        // emtfHit_fr         ->push_back(isFront(hit));
        emtfHit_emtf_phi->push_back(hit.Phi_fp());
        emtfHit_emtf_theta->push_back(hit.Theta_fp());
        //
        emtfHit_sim_phi->push_back(hit.Phi_sim());
        emtfHit_sim_theta->push_back(hit.Theta_sim());
        emtfHit_sim_r->push_back(hit.Rho_sim());
        emtfHit_sim_z->push_back(hit.Z_sim());
        //
        emtfHit_match_iSeg->push_back(-1);
        emtfHit_match_iSeg2->push_back(-1);
        emtfHit_match_nSegs->push_back(0);
        emtfHit_match_seg_unique->push_back(0);
    }
    (*emtfHit_size) = EMTFHits_.size();

    // EMTF Unpacked Hits
    for (const auto &hit : EMTFUnpHits_) {
        emtfUnpHit_endcap->push_back(hit.Endcap());
        emtfUnpHit_station->push_back(hit.Station());
        emtfUnpHit_ring->push_back(hit.Ring());
        // emtfUnpHit_sector->push_back(hit.PC_sector());
        emtfUnpHit_sector->push_back(hit.Sector());
        emtfUnpHit_subsector->push_back(hit.Subsector());
        emtfUnpHit_chamber->push_back(hit.Chamber());
        emtfUnpHit_cscid->push_back(hit.CSC_ID());
        emtfUnpHit_bx->push_back(hit.BX());
        emtfUnpHit_type->push_back(hit.Subsystem());
        emtfUnpHit_neighbor->push_back(hit.Neighbor());
        //
        emtfUnpHit_strip->push_back(hit.Strip());
        emtfUnpHit_strip_qses->push_back((hit.Strip_quart_bit() << 1) + hit.Strip_eighth_bit());
        emtfUnpHit_wire->push_back(hit.Wire());
        emtfUnpHit_roll->push_back(hit.Roll());
        emtfUnpHit_quality->push_back(hit.Quality());
        emtfUnpHit_pattern->push_back(hit.Pattern());
        emtfUnpHit_bend->push_back(hit.Bend());
        emtfUnpHit_time->push_back(hit.Time());
        // emtfUnpHit_fr         ->push_back(isFront(hit));      // added
        emtfUnpHit_emtf_phi->push_back(hit.Phi_fp());
        emtfUnpHit_emtf_theta->push_back(hit.Theta_fp());
        //
        emtfUnpHit_sim_phi->push_back(hit.Phi_sim());
        emtfUnpHit_sim_theta->push_back(hit.Theta_sim());
        emtfUnpHit_sim_r->push_back(hit.Rho_sim());
        emtfUnpHit_sim_z->push_back(hit.Z_sim());
    }
    (*emtfUnpHit_size) = EMTFUnpHits_.size();

    // EMTF Tracks
    for (const auto &trk : EMTFTracks_) {

        const std::vector<int32_t> &hit_refs = getHitRefs(trk, EMTFHits_);
        assert(hit_refs.size() == 4);

        const l1t::EMTFPtLUT &ptlut_data = trk.PtLUT();

        emtfTrack_pt->push_back(trk.Pt());
        emtfTrack_xml_pt->push_back(trk.Pt_XML());
        emtfTrack_pt_dxy->push_back(trk.Pt_dxy());
        emtfTrack_dxy->push_back(trk.Dxy());
        // emtfTrack_invpt_prompt ->push_back(trk.Invpt_prompt());
        // emtfTrack_invpt_displ  ->push_back(trk.Invpt_displ());
        emtfTrack_phi->push_back(trk.Phi_glob());
        emtfTrack_phi_fp->push_back(trk.Phi_fp());
        emtfTrack_theta->push_back(trk.Theta());
        emtfTrack_theta_fp->push_back(trk.Theta_fp());
        emtfTrack_eta->push_back(trk.Eta());
        emtfTrack_GMT_phi->push_back(trk.GMT_phi());
        emtfTrack_GMT_eta->push_back(trk.GMT_eta());
        emtfTrack_GMT_dxy->push_back(trk.GMT_dxy());
        emtfTrack_q->push_back(trk.Charge());
        //
        emtfTrack_mode->push_back(trk.Mode());
        emtfTrack_endcap->push_back(trk.Endcap());
        emtfTrack_sector->push_back(trk.Sector());
        emtfTrack_bx->push_back(trk.BX());
        emtfTrack_nhits->push_back(trk.Hits().size());
        // emtfTrack_straightness ->push_back(trk.Straightness());
        emtfTrack_hitref1->push_back(hit_refs.at(0));
        emtfTrack_hitref2->push_back(hit_refs.at(1));
        emtfTrack_hitref3->push_back(hit_refs.at(2));
        emtfTrack_hitref4->push_back(hit_refs.at(3));

        emtfTrack_ptLUT_address->push_back(ptlut_data.address);
        emtfTrack_ptLUT_mode->push_back(ptlut_data.mode);
        emtfTrack_ptLUT_theta->push_back(ptlut_data.theta);
        emtfTrack_ptLUT_st1_ring2->push_back(ptlut_data.st1_ring2);
        emtfTrack_ptLUT_eta->push_back(ptlut_data.eta);

        // 6 of these quantities per track
        std::vector<uint16_t> deltaPh = {
            ptlut_data.delta_ph[0], ptlut_data.delta_ph[1],
            ptlut_data.delta_ph[2], ptlut_data.delta_ph[3],
            ptlut_data.delta_ph[4], ptlut_data.delta_ph[5]};
        std::vector<uint16_t> deltaTh = {
            ptlut_data.delta_th[0], ptlut_data.delta_th[1],
            ptlut_data.delta_th[2], ptlut_data.delta_th[3],
            ptlut_data.delta_th[4], ptlut_data.delta_th[5]};
        std::vector<uint16_t> signPh = {
            ptlut_data.sign_ph[0], ptlut_data.sign_ph[1],
            ptlut_data.sign_ph[2], ptlut_data.sign_ph[3],
            ptlut_data.sign_ph[4], ptlut_data.sign_ph[5]};
        std::vector<uint16_t> signTh = {
            ptlut_data.sign_th[0], ptlut_data.sign_th[1],
            ptlut_data.sign_th[2], ptlut_data.sign_th[3],
            ptlut_data.sign_th[4], ptlut_data.sign_th[5]};
        // 4 of these quantities per track
        std::vector<uint16_t> cpattern = {
            ptlut_data.cpattern[0], ptlut_data.cpattern[1],
            ptlut_data.cpattern[2], ptlut_data.cpattern[3]};
        std::vector<uint16_t> fr = {ptlut_data.fr[0], ptlut_data.fr[1],
                                    ptlut_data.fr[2], ptlut_data.fr[3]};
        // 5 of these quantities per track
        std::vector<uint16_t> bt_vi = {ptlut_data.bt_vi[0], ptlut_data.bt_vi[1],
                                       ptlut_data.bt_vi[2], ptlut_data.bt_vi[3],
                                       ptlut_data.bt_vi[4]};
        std::vector<uint16_t> bt_hi = {ptlut_data.bt_hi[0], ptlut_data.bt_hi[1],
                                       ptlut_data.bt_hi[2], ptlut_data.bt_hi[3],
                                       ptlut_data.bt_hi[4]};
        std::vector<uint16_t> bt_ci = {ptlut_data.bt_ci[0], ptlut_data.bt_ci[1],
                                       ptlut_data.bt_ci[2], ptlut_data.bt_ci[3],
                                       ptlut_data.bt_ci[4]};
        std::vector<uint16_t> bt_si = {ptlut_data.bt_si[0], ptlut_data.bt_si[1],
                                       ptlut_data.bt_si[2], ptlut_data.bt_si[3],
                                       ptlut_data.bt_si[4]};

        emtfTrack_ptLUT_deltaPh->push_back(deltaPh);
        emtfTrack_ptLUT_deltaTh->push_back(deltaTh);
        emtfTrack_ptLUT_signPh->push_back(signPh);
        emtfTrack_ptLUT_signTh->push_back(signTh);
        emtfTrack_ptLUT_cpattern->push_back(cpattern);
        emtfTrack_ptLUT_fr->push_back(fr);
        emtfTrack_ptLUT_bt_vi->push_back(bt_vi);
        emtfTrack_ptLUT_bt_hi->push_back(bt_hi);
        emtfTrack_ptLUT_bt_ci->push_back(bt_ci);
        emtfTrack_ptLUT_bt_si->push_back(bt_si);
    }
    (*emtfTrack_size) = EMTFTracks_.size();

    // EMTF Unpacked Tracks
    for (const auto &trk : EMTFUnpTracks_) {
        const std::vector<int32_t> &hit_refs = getHitRefs(trk, EMTFUnpHits_);
        assert(hit_refs.size() == 4);

        const l1t::EMTFPtLUT &ptlut_data = trk.PtLUT();

        emtfUnpTrack_pt->push_back(trk.Pt());
        emtfUnpTrack_xml_pt->push_back(trk.Pt_XML());
        emtfUnpTrack_pt_dxy       ->push_back(trk.Pt_dxy());
        emtfUnpTrack_dxy          ->push_back(trk.Dxy());
        // emtfUnpTrack_invpt_prompt ->push_back(trk.Invpt_prompt());
        // emtfUnpTrack_invpt_displ  ->push_back(trk.Invpt_displ());
        emtfUnpTrack_phi->push_back(trk.Phi_glob());
        emtfUnpTrack_theta->push_back(trk.Theta());
        emtfUnpTrack_eta->push_back(trk.Eta());
        emtfUnpTrack_GMT_dxy->push_back(trk.GMT_dxy());
        emtfUnpTrack_q->push_back(trk.Charge());
        //
        emtfUnpTrack_mode->push_back(trk.Mode());
        emtfUnpTrack_endcap->push_back(trk.Endcap());
        emtfUnpTrack_sector->push_back(trk.Sector());
        emtfUnpTrack_bx->push_back(trk.BX());
        emtfUnpTrack_nhits->push_back(trk.Hits().size());
        emtfUnpTrack_hitref1->push_back(hit_refs.at(0));
        emtfUnpTrack_hitref2->push_back(hit_refs.at(1));
        emtfUnpTrack_hitref3->push_back(hit_refs.at(2));
        emtfUnpTrack_hitref4->push_back(hit_refs.at(3));

        emtfUnpTrack_ptLUT_address->push_back(ptlut_data.address);
        emtfUnpTrack_ptLUT_mode->push_back(ptlut_data.mode);
        emtfUnpTrack_ptLUT_theta->push_back(ptlut_data.theta);
        emtfUnpTrack_ptLUT_st1_ring2->push_back(ptlut_data.st1_ring2);
        emtfUnpTrack_ptLUT_eta->push_back(ptlut_data.eta);

        // 6 of these quantities per track
        std::vector<uint16_t> deltaPh = {
            ptlut_data.delta_ph[0], ptlut_data.delta_ph[1],
            ptlut_data.delta_ph[2], ptlut_data.delta_ph[3],
            ptlut_data.delta_ph[4], ptlut_data.delta_ph[5]};
        std::vector<uint16_t> deltaTh = {
            ptlut_data.delta_th[0], ptlut_data.delta_th[1],
            ptlut_data.delta_th[2], ptlut_data.delta_th[3],
            ptlut_data.delta_th[4], ptlut_data.delta_th[5]};
        std::vector<uint16_t> signPh = {
            ptlut_data.sign_ph[0], ptlut_data.sign_ph[1],
            ptlut_data.sign_ph[2], ptlut_data.sign_ph[3],
            ptlut_data.sign_ph[4], ptlut_data.sign_ph[5]};
        std::vector<uint16_t> signTh = {
            ptlut_data.sign_th[0], ptlut_data.sign_th[1],
            ptlut_data.sign_th[2], ptlut_data.sign_th[3],
            ptlut_data.sign_th[4], ptlut_data.sign_th[5]};
        // 4 of these quantities per track
        std::vector<uint16_t> cpattern = {
            ptlut_data.cpattern[0], ptlut_data.cpattern[1],
            ptlut_data.cpattern[2], ptlut_data.cpattern[3]};
        std::vector<uint16_t> fr = {ptlut_data.fr[0], ptlut_data.fr[1],
                                    ptlut_data.fr[2], ptlut_data.fr[3]};
        // 5 of these quantities per track
        std::vector<uint16_t> bt_vi = {ptlut_data.bt_vi[0], ptlut_data.bt_vi[1],
                                       ptlut_data.bt_vi[2], ptlut_data.bt_vi[3],
                                       ptlut_data.bt_vi[4]};
        std::vector<uint16_t> bt_hi = {ptlut_data.bt_hi[0], ptlut_data.bt_hi[1],
                                       ptlut_data.bt_hi[2], ptlut_data.bt_hi[3],
                                       ptlut_data.bt_hi[4]};
        std::vector<uint16_t> bt_ci = {ptlut_data.bt_ci[0], ptlut_data.bt_ci[1],
                                       ptlut_data.bt_ci[2], ptlut_data.bt_ci[3],
                                       ptlut_data.bt_ci[4]};
        std::vector<uint16_t> bt_si = {ptlut_data.bt_si[0], ptlut_data.bt_si[1],
                                       ptlut_data.bt_si[2], ptlut_data.bt_si[3],
                                       ptlut_data.bt_si[4]};

        emtfUnpTrack_ptLUT_deltaPh->push_back(deltaPh);
        emtfUnpTrack_ptLUT_deltaTh->push_back(deltaTh);
        emtfUnpTrack_ptLUT_signPh->push_back(signPh);
        emtfUnpTrack_ptLUT_signTh->push_back(signTh);
        emtfUnpTrack_ptLUT_cpattern->push_back(cpattern);
        emtfUnpTrack_ptLUT_fr->push_back(fr);
        emtfUnpTrack_ptLUT_bt_vi->push_back(bt_vi);
        emtfUnpTrack_ptLUT_bt_hi->push_back(bt_hi);
        emtfUnpTrack_ptLUT_bt_ci->push_back(bt_ci);
        emtfUnpTrack_ptLUT_bt_si->push_back(bt_si);
    }
    (*emtfUnpTrack_size) = EMTFUnpTracks_.size();

    // GMT Muons
    if (useGMTMuons_) {
        for (const auto &muon : *GMTMuons_) {
            gmtMuon_pt->push_back(muon.pt());
            gmtMuon_pt_dxy->push_back(muon.ptUnconstrained());
            gmtMuon_dxy->push_back(muon.hwDXY());
            gmtMuon_phi->push_back(muon.phi());
            gmtMuon_eta->push_back(muon.eta());
            gmtMuon_q->push_back(muon.charge());
            gmtMuon_qual->push_back(muon.hwQual());
        }
        (*gmtMuon_size) = GMTMuons_->size();
    }

    // GMT Unpacked Muons
    if (useGMTUnpMuons_) {
        for (const auto &muon : *GMTUnpMuons_) {
            gmtUnpMuon_pt->push_back(muon.pt());
            // gmtUnpMuon_pt_dxy    ->push_back(muon.ptUnconstrained());
            // gmtUnpMuon_dxy       ->push_back(muon.hwDXY());
            gmtUnpMuon_phi->push_back(muon.phi());
            gmtUnpMuon_eta->push_back(muon.eta());
            gmtUnpMuon_q->push_back(muon.charge());
            gmtUnpMuon_qual->push_back(muon.hwQual());
        }
        (*gmtUnpMuon_size) = GMTUnpMuons_->size();
    }

    // Gen particles
    if (useGENParts_) {
        for (const auto &part : *GENParts_) {
            if (abs(part.pdgId()) != 13)
                continue;

            int parentID = -9999;
            if (part.numberOfMothers() > 0)
                parentID =
                    dynamic_cast<const reco::GenParticle *>(part.mother(0))
                        ->pdgId();
            if (parentID == part.pdgId())
                continue;

            genPart_pt->push_back(part.pt());
            // genPart_dxy        ->push_back(part.dxy());
            genPart_eta->push_back(part.eta());
            genPart_phi->push_back(part.phi());
            genPart_q->push_back(part.charge());
            genPart_ID->push_back(part.pdgId());
            genPart_parentID->push_back(parentID);
            genPart_vx->push_back(part.vx());
            genPart_vy->push_back(part.vy());
            genPart_vz->push_back(part.vz());

            const MagneticField &theBField = iSetup.getData(theBFieldToken_);
            const MagneticField *theMagneticField = &theBField;


            GlobalPoint gp(part.vx(), part.vy(), part.vz());
            GlobalVector gv(part.px(), part.py(), part.pz());
            FreeTrajectoryState fts(gp, gv, part.charge(), theMagneticField);

            // extrapolation of track coordinates
            TrajectoryStateOnSurface stateAtMuSt1 = muPropagator1st_.extrapolate(fts);
            if (stateAtMuSt1.isValid()) {
                genPart_etaSt1->push_back(stateAtMuSt1.globalPosition().eta());
                genPart_phiSt1->push_back(stateAtMuSt1.globalPosition().phi());
            } else {
                genPart_etaSt1->push_back(-9999);
                genPart_phiSt1->push_back(-9999);
            }

            TrajectoryStateOnSurface stateAtMuSt2 = muPropagator2nd_.extrapolate(fts);
            if (stateAtMuSt2.isValid()) {
                genPart_etaSt2->push_back(stateAtMuSt2.globalPosition().eta());
                genPart_phiSt2->push_back(stateAtMuSt2.globalPosition().phi());
            } else {
                genPart_etaSt2->push_back(-9999);
                genPart_phiSt2->push_back(-9999);
            }
        }
    }

    // Reco muons
    if (useRecoMuons_) {
        for (const auto &muon : *RecoMuons_) {
            recoMuon_e->push_back(muon.energy());
            recoMuon_et->push_back(muon.et());
            recoMuon_pt->push_back(muon.pt());
            recoMuon_eta->push_back(muon.eta());
            recoMuon_phi->push_back(muon.phi());
            recoMuon_charge->push_back(muon.charge());

            if (Vertices_ != nullptr){
                if( !(muon.muonBestTrack().isNull())){
                    recoMuon_dz->push_back( muon.muonBestTrack()->dz((*Vertices_)[0].position()));
                    recoMuon_dxy->push_back( muon.muonBestTrack()->dxy((*Vertices_)[0].position()));
                }
            }

            bool isLoose = (muon.isPFMuon() && (muon.isGlobalMuon() || muon.isTrackerMuon()));
            bool goodGlob = muon.isGlobalMuon() && muon.globalTrack()->normalizedChi2() < 3 &&
                  muon.combinedQuality().chi2LocalPosition < 12 && muon.combinedQuality().trkKink < 20;
            bool isMedium = isLoose && muon.innerTrack()->validFraction() > 0.49 &&
                  muon::segmentCompatibility(muon) > (goodGlob ? 0.303 : 0.451);
            bool isTight = false;
            if (Vertices_ != nullptr){
                isTight = muon.isGlobalMuon() && muon.isPFMuon() && muon.globalTrack()->normalizedChi2() < 10. &&
                    muon.globalTrack()->hitPattern().numberOfValidMuonHits() > 0 &&
                    muon.numberOfMatchedStations() > 1 &&
                    std::abs(muon.muonBestTrack()->dxy(((*Vertices_)[0]).position())) < 0.2 &&
                    std::abs(muon.muonBestTrack()->dz(((*Vertices_)[0]).position())) < 0.5 &&
                    muon.innerTrack()->hitPattern().numberOfValidPixelHits() > 0 &&
                    muon.innerTrack()->hitPattern().trackerLayersWithMeasurement() > 5 &&
                    muon.globalTrack()->normalizedChi2() < 1;
            }
            recoMuon_isLooseMuon->push_back(isLoose);
            recoMuon_isMediumMuon->push_back(isMedium);
            recoMuon_isTightMuon->push_back(isTight);

            double iso = (muon.pfIsolationR04().sumChargedHadronPt +
                max(0.,
                    muon.pfIsolationR04().sumNeutralHadronEt + muon.pfIsolationR04().sumPhotonEt -
                        0.5 * muon.pfIsolationR04().sumPUPt)) /
                muon.pt();
            recoMuon_iso->push_back(iso);

            if (triggerMatching_) {
                double isoMatchDeltaR = 9999.;
                double matchDeltaR = 9999.;
                int hasIsoTriggered = 0;
                int hasTriggered = 0;

                int passesSingleMuonFlag = 0;

                // first check if the trigger results are valid:
                if (TriggerResults_ != nullptr) {
                    if (TriggerSummaryLabels_ != nullptr) {
                        const edm::TriggerNames& trigNames = iEvent.triggerNames(*TriggerResults_);

                        for (UInt_t iPath = 0; iPath < isoTriggerNames_.size(); ++iPath) {
                            if (passesSingleMuonFlag == 1)
                                continue;
                            std::string pathName = isoTriggerNames_.at(iPath);

                            bool passTrig = false;

                            if (trigNames.triggerIndex(pathName) < trigNames.size())
                                passTrig = TriggerResults_->accept(trigNames.triggerIndex(pathName));
                            if (passTrig)
                                passesSingleMuonFlag = 1;
                        }

                        // get trigger objects:
                        const trigger::TriggerObjectCollection triggerObjects = TriggerSummaryLabels_->getObjects();

                        matchDeltaR = match_trigger(triggerIndices_, triggerObjects, *TriggerSummaryLabels_, muon);
                        if (matchDeltaR < triggerMaxDeltaR_)
                            hasTriggered = 1;

                        isoMatchDeltaR = match_trigger(isoTriggerIndices_, triggerObjects, *TriggerSummaryLabels_, muon);

                        if (isoMatchDeltaR < triggerMaxDeltaR_)
                            hasIsoTriggered = 1;


                    }  // end if (TriggerSummaryLabels_.isValid())

                }  // end if (TriggerResults_.isValid())

                recoMuon_hlt_isomu->push_back(hasIsoTriggered);
                recoMuon_hlt_mu->push_back(hasTriggered);
                recoMuon_hlt_isoDeltaR->push_back(isoMatchDeltaR);
                recoMuon_hlt_deltaR->push_back(matchDeltaR);
                recoMuon_passesSingleMuon->push_back(passesSingleMuonFlag);
            } else {
                recoMuon_hlt_isomu->push_back(-999);
                recoMuon_hlt_mu->push_back(-999);
                recoMuon_hlt_isoDeltaR->push_back(-999);
                recoMuon_hlt_deltaR->push_back(-999);
                recoMuon_passesSingleMuon->push_back(-999);
            }

            // extrapolation of track coordinates
            TrajectoryStateOnSurface stateAtMuSt1 = muPropagator1st_.extrapolate(muon);
            if (stateAtMuSt1.isValid()) {
                recoMuon_etaSt1->push_back(stateAtMuSt1.globalPosition().eta());
                recoMuon_phiSt1->push_back(stateAtMuSt1.globalPosition().phi());
            } else {
                recoMuon_etaSt1->push_back(-9999);
                recoMuon_phiSt1->push_back(-9999);
            }

            TrajectoryStateOnSurface stateAtMuSt2 = muPropagator2nd_.extrapolate(muon);
            if (stateAtMuSt2.isValid()) {
                recoMuon_etaSt2->push_back(stateAtMuSt2.globalPosition().eta());
                recoMuon_phiSt2->push_back(stateAtMuSt2.globalPosition().phi());
            } else {
                recoMuon_etaSt2->push_back(-9999);
                recoMuon_phiSt2->push_back(-9999);
            }

        }
        (*recoMuon_size) = RecoMuons_->size();

    }


    // CSC Segments
    if (useCSCSegments_) {
        cscGeom_ = iSetup.getHandle(cscGeomToken_);
        for (const auto &cscSeg : *CSCSegments_) {
            // Set CSCDetId to get position of segment in detector
            CSCDetId id = (CSCDetId)(cscSeg).cscDetId();

            // Get CSC segment global position from CSC geometry
            // See
            // https://github.com/aminnj/CSCValidationRunning/blob/me42/RecoLocalMuon/CSCValidation/src/CSCValidation.cc#L1153
            LocalPoint locPos = (cscSeg).localPosition();
            const CSCChamber *cscChamb = cscGeom_->chamber(id);
            GlobalPoint globPos = cscChamb->toGlobal(locPos);

            // globPos.phi() has bizzare properties when you multiply it
            // directly It 'rotates' in a [-pi, pi] range instead of scaling
            float globPos_theta = ((globPos.theta() * 180. / M_PI) > 90)
                                      ? (180 - (globPos.theta() * 180. / M_PI))
                                      : (globPos.theta() * 180. / M_PI);

            // Fetch wire and strip information
            int wire_max = -1;
            int wire_min = 1000;
            int strip_max = -1;
            int strip_min = 1000;

            const std::vector<CSCRecHit2D> recHits = cscSeg.specificRecHits();

            for (int iHit = 0; iHit < cscSeg.nRecHits(); iHit++) {
                const CSCRecHit2D recHit = recHits.at(iHit);

                if (wire_max < recHit.hitWire())
                    wire_max = recHit.hitWire();

                if (wire_min > recHit.hitWire())
                    wire_min = recHit.hitWire();

                for (int i = 0; i < 3; i++) {
                    if (strip_max < recHit.channels(i))
                        strip_max = recHit.channels(i);

                    if (strip_min > recHit.channels(i))
                        strip_min = recHit.channels(i);
                }
            }

            // Check if it exists
            int nDuplicates = 0;

            for (int i = 0; i < *cscSegment_size; i++) {
                if (id.zendcap() == (*cscSegment_endcap)[i] &&
                    id.ring() == (*cscSegment_ring)[i] &&
                    id.station() == (*cscSegment_station)[i] &&
                    id.chamber() == (*cscSegment_chamber)[i] &&
                    // Total overlap in strip and wire range (one contained
                    // inside the other, in both strip and wire)
                    ((strip_min >= (*cscSegment_strip_min)[i] &&
                      strip_max <= (*cscSegment_strip_max)[i]) ||
                     (strip_min <= (*cscSegment_strip_min)[i] &&
                      strip_max >= (*cscSegment_strip_max)[i])) &&
                    ((wire_min >= (*cscSegment_wire_min)[i] &&
                      wire_max <= (*cscSegment_wire_max)[i]) ||
                     (wire_min <= (*cscSegment_wire_min)[i] &&
                      wire_max >= (*cscSegment_wire_max)[i]))) {
                    if ((cscSeg.nRecHits() > (*cscSegment_nRecHits)[i]) ||
                        (cscSeg.nRecHits() == (*cscSegment_nRecHits)[i] &&
                         cscSeg.chi2() < (*cscSegment_chi2)[i] + 0.001)) {
                        // std::cout << "LATER DUPLICATE SEGMENT APPEARS TO BE
                        // BETTER QUALITY!!!" << std::endl;

                        // Delete segment
                        cscSegment_locX->erase(cscSegment_locX->begin() + i);
                        cscSegment_locY->erase(cscSegment_locY->begin() + i);
                        cscSegment_globX->erase(cscSegment_globX->begin() + i);
                        cscSegment_globY->erase(cscSegment_globY->begin() + i);
                        cscSegment_globZ->erase(cscSegment_globZ->begin() + i);
                        cscSegment_eta->erase(cscSegment_eta->begin() + i);
                        cscSegment_phi->erase(cscSegment_phi->begin() + i);
                        cscSegment_theta->erase(cscSegment_theta->begin() + i);
                        cscSegment_chi2->erase(cscSegment_chi2->begin() + i);
                        cscSegment_time->erase(cscSegment_time->begin() + i);
                        cscSegment_dirX->erase(cscSegment_dirX->begin() + i);
                        cscSegment_dirY->erase(cscSegment_dirY->begin() + i);
                        cscSegment_dirZ->erase(cscSegment_dirZ->begin() + i);
                        cscSegment_bend_theta->erase(
                            cscSegment_bend_theta->begin() + i);
                        cscSegment_bend_phi->erase(
                            cscSegment_bend_phi->begin() + i);
                        cscSegment_endcap->erase(cscSegment_endcap->begin() +
                                                 i);
                        cscSegment_ring->erase(cscSegment_ring->begin() + i);
                        cscSegment_station->erase(cscSegment_station->begin() +
                                                  i);
                        cscSegment_chamber->erase(cscSegment_chamber->begin() +
                                                  i);
                        cscSegment_sector->erase(cscSegment_sector->begin() +
                                                 i);
                        cscSegment_CSC_ID->erase(cscSegment_CSC_ID->begin() +
                                                 i);

                        cscSegment_nRecHits->erase(
                            cscSegment_nRecHits->begin() + i);

                        cscSegment_wire_min->erase(
                            cscSegment_wire_min->begin() + i);
                        cscSegment_wire_max->erase(
                            cscSegment_wire_max->begin() + i);
                        cscSegment_strip_min->erase(
                            cscSegment_strip_min->begin() + i);
                        cscSegment_strip_max->erase(
                            cscSegment_strip_max->begin() + i);

                        cscSegment_match_iHit->erase(
                            cscSegment_match_iHit->begin() + i);
                        cscSegment_match_iHit2->erase(
                            cscSegment_match_iHit2->begin() + i);
                        cscSegment_match_nHits->erase(
                            cscSegment_match_nHits->begin() + i);
                        cscSegment_match_hit_unique->erase(
                            cscSegment_match_hit_unique->begin() + i);

                        // Update counts
                        *cscSegment_size = *cscSegment_size - 1;

                        if (abs(cscSeg.time()) < 12.5) {
                            *cscSegment_nBx0 = *cscSegment_nBx0 - 1;
                        }
                    } else {
                        // Increase duplicate count
                        nDuplicates += 1;
                    }
                }
            }

            // if (nDuplicates > 1) std::cout << "Found " << nDuplicates << "
            // duplicate segments" << std::endl;

            if (nDuplicates > 0)
                continue;

            *cscSegment_size = *cscSegment_size + 1;

            if (abs(cscSeg.time()) < 12.5) {
                *cscSegment_nBx0 = *cscSegment_nBx0 + 1;
            }

            // Write cscSegment information
            cscSegment_locX->push_back(locPos.x());
            cscSegment_locY->push_back(locPos.y());
            cscSegment_globX->push_back(globPos.x());
            cscSegment_globY->push_back(globPos.y());
            cscSegment_globZ->push_back(globPos.z());
            cscSegment_eta->push_back(globPos.eta());
            cscSegment_phi->push_back(globPos.phi() * 180. / M_PI);
            cscSegment_theta->push_back(globPos_theta);
            cscSegment_chi2->push_back(cscSeg.chi2());
            cscSegment_time->push_back(cscSeg.time());
            cscSegment_dirX->push_back(cscSeg.localDirection().x());
            cscSegment_dirY->push_back(cscSeg.localDirection().y());
            cscSegment_dirZ->push_back(cscSeg.localDirection().z());

            float bend_theta = TMath::ATan2(cscSeg.localDirection().y(),
                                            abs(cscSeg.localDirection().z()));
            float bend_phi = TMath::ATan2(cscSeg.localDirection().x(),
                                          abs(cscSeg.localDirection().z()));

            cscSegment_bend_theta->push_back(bend_theta);
            cscSegment_bend_phi->push_back(bend_phi);

            cscSegment_endcap->push_back(id.zendcap());
            cscSegment_ring->push_back(id.ring());
            cscSegment_station->push_back(id.station());
            cscSegment_chamber->push_back(id.chamber());
            cscSegment_sector->push_back(id.triggerSector());
            cscSegment_CSC_ID->push_back(id.triggerCscId());

            cscSegment_nRecHits->push_back(cscSeg.nRecHits());

            cscSegment_wire_min->push_back(wire_min);
            cscSegment_wire_max->push_back(wire_max);
            cscSegment_strip_min->push_back(strip_min);
            cscSegment_strip_max->push_back(strip_max);

            cscSegment_match_iHit->push_back(-1);
            cscSegment_match_iHit2->push_back(-1);
            cscSegment_match_nHits->push_back(0);
            cscSegment_match_hit_unique->push_back(0);
        }
    }

    if (useCSCSegments_ && matchCSCSegments_) {
        // Prepare variables for loops over segments/LCTs
        const int n1 = *cscSegment_size;
        const int n2 = *emtfHit_size;

        // std::cout << "nsegs: " << n1 << ", nLCTs: " << n2 << std::endl;

        for (int i = 0; i < n1; i++) {     // Loop over segments
            for (int j = 0; j < n2; j++) { // Loop over LCTs
                // Skip non-csc hits
                if ((*emtfHit_type)[j] != 1) {
                    continue;
                }

                // Skip anomalous huge segments
                if (((*cscSegment_strip_max)[i] - (*cscSegment_strip_min)[i]) >
                    24)
                    continue;

                // Require matching within 1 BX
                if (fabs((*cscSegment_time)[i] - (*emtfHit_bx)[j] * 25.) > 37.5)
                    continue;

                // Check to see if the segment and LCT are in the same endcap
                // (EMTFHit: endcap --> +/-1, CSCDetId: endcap --> +/-1)
                int seg_endcap = (*cscSegment_endcap)[i];
                int LCT_endcap = (*emtfHit_endcap)[j];

                if (seg_endcap != LCT_endcap)
                    continue;

                // Check to see if the segment and LCT are in the same station
                // (EMTFHit: station --> 1-4, CSCDetId: station --> 1-4)
                int seg_station = (*cscSegment_station)[i];
                int LCT_station = (*emtfHit_station)[j];

                if (seg_station != LCT_station)
                    continue;

                // Check to see if the segment and LCT are in the same Ring
                // (EMTFHit: ring --> 1-4, CSCDetId: ring --> 1-4)
                int seg_ring = (*cscSegment_ring)[i];
                int LCT_ring = (*emtfHit_ring)[j];

                if (seg_ring != LCT_ring)
                    continue;

                // Check to see if the segment and LCT are in the same Chamber
                // (EMTFHit: chamber --> 1-36, CSCDetId: chamber --> 1-36)
                int seg_chamber = (*cscSegment_chamber)[i];
                int LCT_chamber = (*emtfHit_chamber)[j];

                if (seg_chamber != LCT_chamber)
                    continue;

                // Grab segment strip and wire information
                int Seg_strip_min = (*cscSegment_strip_min)[i] *
                                    2; // Convert to units of halfstrips
                int Seg_strip_max = (*cscSegment_strip_max)[i] *
                                    2; // Convert to units of halfstrips
                int Seg_wire_min = (*cscSegment_wire_min)[i];
                int Seg_wire_max = (*cscSegment_wire_max)[i];

                float Seg_strip_avg = (Seg_strip_min + Seg_strip_max) / 2.;
                float Seg_wire_avg = (Seg_wire_min + Seg_wire_max) / 2.;

                // Now find the LCT strip and wire information for potential
                // matches

                // Shift one full strip
                int LCT_strip = (*emtfHit_strip)[j] + 2;

                // Shifted because LCT starts counting at 0
                int LCT_wire = (*emtfHit_wire)[j] + 1;

                // // Nice for printouts
                // int nRecHits = ACCESS(cscSegs.mVInt, "seg_nRecHits").at(i);
                // std::cout << "\nnumber of Rechits:" << nRecHits   <<
                // std::endl; std::cout << "Segment strip:" << Seg_strip_min <<
                // "-" << Seg_strip_max << "     LCT strip:" << LCT_strip  <<
                // std::endl; std::cout << "Segment wire:" << Seg_wire_min <<
                // "-" << Seg_wire_max << "        LCT wire:" << LCT_wire  <<
                // "\n" << std::endl;

                // Now start the meat of matching code: check to see if it is
                // within 1 strip and wire
                if (LCT_strip < Seg_strip_min - 1 ||
                    LCT_strip > Seg_strip_max + 1)
                    continue;

                if (LCT_wire < Seg_wire_min - 1 || LCT_wire > Seg_wire_max + 1)
                    continue;

                // std::cout << "We have a full match, segment index " << i << "
                // to LCT index " << j << "!" << std::endl;

                // Only set LCT to match segment, not segment to match LCT
                if ((*emtfHit_neighbor)[j] == 1) {
                    (*emtfHit_match_iSeg)[j] = i;
                    (*emtfHit_match_nSegs)[j] = (*emtfHit_match_nSegs)[j] + 1;

                    continue;
                }

                assert(((*cscSegment_match_nHits)[i] >= 1) ==
                       ((*cscSegment_match_iHit)[i] >= 0));
                assert(((*emtfHit_match_nSegs)[j] >= 1) ==
                       ((*emtfHit_match_iSeg)[j] >= 0));

                (*cscSegment_match_nHits)[i] = (*cscSegment_match_nHits)[i] + 1;
                (*emtfHit_match_nSegs)[j] = (*emtfHit_match_nSegs)[j] + 1;

                // Old matched hit index
                int l = (*cscSegment_match_iHit)[i];

                // Old matched segment index
                int k = (*emtfHit_match_iSeg)[j];

                assert(l != j);
                assert(k != i);

                // If no double matching then we save the indices
                if (l < 0)
                    // Sets i^th value in vector to j
                    (*cscSegment_match_iHit)[i] = j;

                if (k < 0)
                    // Sets j^th value in vector to i
                    (*emtfHit_match_iSeg)[j] = i;

                // Check for segments matched to two LCTs
                if (l >= 0) {
                    // std::cout << "But segment already had a match with index
                    // " << ACCESS(cscSegs.mVInt, "seg_match_iHit").at(i) <<
                    // "!!!" << std::endl;

                    // Old strip
                    int LCT_strip_l = (*emtfHit_strip)[l] + 2;

                    // Old wire
                    int LCT_wire_l = (*emtfHit_wire)[l] + 1;

                    // Match both LCTs to the segment index but the segment
                    // index only to the closer LCT
                    if (abs(LCT_strip - Seg_strip_avg) <
                        abs(LCT_strip_l - Seg_strip_avg)) {
                        // Sets the index to new LCT index
                        (*cscSegment_match_iHit)[i] = j;

                        // Sets the second index to old LCT index
                        (*cscSegment_match_iHit2)[i] = l;
                    } else if (abs(LCT_strip - Seg_strip_avg) >
                               abs(LCT_strip_l - Seg_strip_avg)) {
                        // Sets the second index to new LCT index
                        (*cscSegment_match_iHit2)[i] = j;
                    } else if (abs(LCT_wire - Seg_wire_avg) <
                               abs(LCT_wire_l - Seg_wire_avg)) {
                        // Sets the index to new LCT index
                        (*cscSegment_match_iHit)[i] = j;

                        // Sets the second index to old LCT index
                        (*cscSegment_match_iHit2)[i] = l;
                    } else if (abs(LCT_wire - Seg_wire_avg) >
                               abs(LCT_wire_l - Seg_wire_avg)) {
                        // Sets the second index to new LCT index
                        (*cscSegment_match_iHit2)[i] = j;
                    } else {
                        // std::cout << "\nBizzare case in CSC segment matching:
                        // two distinct LCTs, same distance from the segment!"
                        // << std::endl; PrintSeg(&(cscSegs.mVInt),
                        // &(cscSegs.mVFlt), i); PrintHit(&(emtfHits.mVInt), l);
                        // PrintHit(&(emtfHits.mVInt), j);
                        // std::cout << "dStrip_l = "  << LCT_strip_l << " - "
                        // << Seg_strip_avg << " = " << (LCT_strip_l -
                        // Seg_strip_avg)
                        //        << ", dStrip = "  << LCT_strip   << " - " <<
                        //        Seg_strip_avg << " = " << (LCT_strip -
                        //        Seg_strip_avg)
                        //        << ", dWire_l = " << LCT_wire_l  << " - " <<
                        //        Seg_wire_avg  << " = " << (LCT_wire_l -
                        //        Seg_wire_avg)
                        //        << ", dWire = "   << LCT_wire    << " - " <<
                        //        Seg_wire_avg  << " = "
                        //        << (LCT_wire - Seg_wire_avg) << std::endl;

                        // Sets the second index to new LCT index
                        (*cscSegment_match_iHit2)[i] = j;
                    }
                }

                // Check for LCTs matched to two segments
                if (k >= 0) {
                    // std::cout << "But LCT already had a match with index " <<
                    // ACCESS(emtfHits.mVInt, "hit_match_iSeg").at(j) << "!!!"
                    // << std::endl;

                    // Check which segment has the most rechits and keep that
                    // one
                    if ((*cscSegment_nRecHits)[i] > (*cscSegment_nRecHits)[k]) {
                        // Sets j^th value in vector to i
                        (*emtfHit_match_iSeg)[j] = i;

                        // Sets j^th value in vector to k
                        (*emtfHit_match_iSeg2)[j] = k;
                    } else if ((*cscSegment_nRecHits)[i] <
                               (*cscSegment_nRecHits)[k]) {
                        // Sets j^th value in vector to i
                        (*emtfHit_match_iSeg2)[j] = i;
                    } else if ((*cscSegment_chi2)[i] < (*cscSegment_chi2)[k]) {
                        // Sets j^th value in vector to i
                        (*emtfHit_match_iSeg)[j] = i;

                        // Sets j^th value in vector to k
                        (*emtfHit_match_iSeg2)[j] = k;
                    } else if ((*cscSegment_chi2)[i] > (*cscSegment_chi2)[k]) {
                        // Sets j^th value in vector to i
                        (*emtfHit_match_iSeg2)[j] = i;
                    } else {
                        // std::cout << "\nBizzare case in LCT matching: two
                        // distinct "
                        //              "segements with the same nRecHits and
                        //              Chi2!"
                        //           << std::endl;
                        // std::cout << "Hit index " << j << ", old segment
                        // index "
                        //           << k << ", new segment index " << i <<
                        //           std::endl;
                        // PrintHit(&(emtfHits.mVInt), j);
                        // PrintSeg(&(cscSegs.mVInt), &(cscSegs.mVFlt), k);
                        // PrintSeg(&(cscSegs.mVInt), &(cscSegs.mVFlt), i);

                        // Sets the second index to new segment index
                        (*emtfHit_match_iSeg2)[j] = i;
                    }
                }

                assert(((*cscSegment_match_nHits)[i] >= 2) ==
                       ((*cscSegment_match_iHit2)[i] >= 0));
                assert(((*emtfHit_match_nSegs)[j] >= 2) ==
                       ((*emtfHit_match_iSeg2)[j] >= 0));

                assert(((*cscSegment_match_nHits)[i] == 0) ||
                       ((*cscSegment_match_iHit2)[i] !=
                        (*cscSegment_match_iHit)[i]));
                assert(((*emtfHit_match_nSegs)[j] == 0) ||
                       ((*emtfHit_match_iSeg2)[j] != (*emtfHit_match_iSeg)[j]));

            } // End j loop (over LCTs)
        }     // End i loop (over segments)

        // Loop over segments
        for (int i = 0; i < n1; i++) {
            int j = (*cscSegment_match_iHit)[i];

            if (j > 0 && (*emtfHit_match_iSeg)[j] == i) {
                (*cscSegment_match_hit_unique)[i] = 1;
            }
        }

        // Loop over LCTs
        for (int j = 0; j < n2; j++) {
            int i = (*emtfHit_match_iSeg)[j];

            if (i > 0 && (*cscSegment_match_iHit)[i] == j) {
                (*emtfHit_match_seg_unique)[j] = 1;
            }
        }
    }

    // Event Info
    eventInfo_event = iEvent.id().event();
    eventInfo_run = iEvent.id().run();
    eventInfo_lumi = iEvent.luminosityBlock();
    eventInfo_bx = iEvent.bunchCrossing();

    // ___________________________________________________________________________
    // Fill

    fillTree();
}

// Match trigger
double MuonAODAnalyzer::match_trigger(std::vector<int> &trigIndices,
                                              const trigger::TriggerObjectCollection &trigObjs,
                                              const trigger::TriggerEvent &triggerEvent,
                                              const reco::Muon &mu) {
  double matchDeltaR = 9999;

  for (size_t iTrigIndex = 0; iTrigIndex < trigIndices.size(); ++iTrigIndex) {
    int triggerIndex = trigIndices[iTrigIndex];
    const std::vector<std::string> moduleLabels(hltConfig_.moduleLabels(triggerIndex));
    // find index of the last module:
    const unsigned moduleIndex = hltConfig_.size(triggerIndex) - 2;
    // find index of HLT trigger name:
    const unsigned hltFilterIndex =
        triggerEvent.filterIndex(edm::InputTag(moduleLabels[moduleIndex], "", triggerProcessLabel_));

    if (hltFilterIndex < triggerEvent.sizeFilters()) {
      const trigger::Keys triggerKeys(triggerEvent.filterKeys(hltFilterIndex));
      const trigger::Vids triggerVids(triggerEvent.filterIds(hltFilterIndex));

      const unsigned nTriggers = triggerVids.size();
      for (size_t iTrig = 0; iTrig < nTriggers; ++iTrig) {
        // loop over all trigger objects:
        const trigger::TriggerObject trigObject = trigObjs[triggerKeys[iTrig]];

        double dRtmp = deltaR(mu, trigObject);

        if (dRtmp < matchDeltaR) {
          matchDeltaR = dRtmp;
        }

      }  // loop over different trigger objects
    }    // if trigger is in event (should apply hltFilter with used trigger...)
  }      // loop over muon candidates

  return matchDeltaR;
}

// ------------ method called once each job just before starting event loop
// ------------
void MuonAODAnalyzer::beginJob() {
    makeTree();
}

// ------------ method called once each run just before starting event loop
// ------------
void MuonAODAnalyzer::beginRun(const edm::Run &run, const edm::EventSetup &eventSetup) {
  // Prepare for trigger matching for each new run:
  // Look up triggetIndices in the HLT config for the different paths
  if (triggerMatching_) {
    bool changed = true;
    if (!hltConfig_.init(run, eventSetup, triggerProcessLabel_, changed)) {
      // if you can't initialize hlt configuration, crash!
      std::cout << "Error: didn't find process" << triggerProcessLabel_ << std::endl;
      assert(false);
    }

    bool enableWildcard = true;
    for (size_t iTrig = 0; iTrig < triggerNames_.size(); ++iTrig) {
      // prepare for regular expression (with wildcards) functionality:
      TString tNameTmp = TString(triggerNames_[iTrig]);
      TRegexp tNamePattern = TRegexp(tNameTmp, enableWildcard);
      int tIndex = -1;
      // find the trigger index:
      for (unsigned ipath = 0; ipath < hltConfig_.size(); ++ipath) {
        // use TString since it provides reg exp functionality:
        TString tmpName = TString(hltConfig_.triggerName(ipath));
        if (tmpName.Contains(tNamePattern)) {
          tIndex = int(ipath);
          triggerIndices_.push_back(tIndex);
        }
      }
      if (tIndex < 0) {  // if can't find trigger path at all, give warning:
        std::cout << "Warning: Could not find trigger" << triggerNames_[iTrig] << std::endl;
        //assert(false);
      }
    }  // end for triggerNames
    for (size_t iTrig = 0; iTrig < isoTriggerNames_.size(); ++iTrig) {
      // prepare for regular expression functionality:
      TString tNameTmp = TString(isoTriggerNames_[iTrig]);
      TRegexp tNamePattern = TRegexp(tNameTmp, enableWildcard);
      int tIndex = -1;
      // find the trigger index:
      for (unsigned ipath = 0; ipath < hltConfig_.size(); ++ipath) {
        // use TString since it provides reg exp functionality:
        TString tmpName = TString(hltConfig_.triggerName(ipath));
        if (tmpName.Contains(tNamePattern)) {
          tIndex = int(ipath);
          isoTriggerIndices_.push_back(tIndex);
        }
      }
      if (tIndex < 0) {  // if can't find trigger path at all, give warning:
        std::cout << "Warning: Could not find trigger" << isoTriggerNames_[iTrig] << std::endl;
        //assert(false);
      }
    }  // end for isoTriggerNames
  }    // end if (triggerMatching_)
}

// ------------ method called once each job just after ending the event loop
// ------------
void MuonAODAnalyzer::endJob() {
}

// ------------ method fills 'descriptions' with the allowed parameters for the
// module  ------------
void MuonAODAnalyzer::fillDescriptions(
    edm::ConfigurationDescriptions &descriptions) {
    // The following says we do not know what parameters are allowed so do no
    // validation
    // Please change this to state exactly what you do use, even if it is no
    // parameters
    edm::ParameterSetDescription desc;
    desc.setUnknown();
    descriptions.addDefault(desc);
}

void MuonAODAnalyzer::getHandles(const edm::Event &iEvent,
                            const edm::EventSetup &iSetup) {

    if (verbose_ > 0)
        std::cout << "******* Getting Handles *******" << std::endl;

    geometryTranslator_.checkAndUpdateGeometry(iSetup);

    auto tp_geom = &(geometryTranslator_);

    // Run 3 inputs
    if (useCSC_) {
        collector_.extractPrimitives(emtf::CSCTag(), tp_geom, iEvent,
                                     CSCInputToken_, CSCInputs_);
    }

    if (useRPC_) {
        collector_.extractPrimitives(emtf::RPCTag(), tp_geom, iEvent,
                                     RPCInputToken_, RPCInputs_);
    }

    if (useGEM_) {
        collector_.extractPrimitives(emtf::GEMTag(), tp_geom, iEvent,
                                     GEMInputToken_, GEMInputs_);
    }

    // Phase 2 inputs
    if (useIRPC_) {
        collector_.extractPrimitives(emtf::IRPCTag(), tp_geom, iEvent,
                                     IRPCInputToken_, IRPCInputs_);
    }

    if (useME0_) {
        collector_.extractPrimitives(emtf::ME0Tag(), tp_geom, iEvent,
                                     ME0InputToken_, ME0Inputs_);
    }

    if (useDT_) {
        // collector_.extractPrimitives(emtf::DTTag(), tp_geom, iEvent,
        // DTInputToken_, DTInputs_);
    }

    auto CSCSegments_handle = make_handle(CSCSegments_);

    if (useCSCSegments_ && isReco_) {
        if (!CSCSegmentToken_.isUninitialized()) {
            iEvent.getByToken(CSCSegmentToken_, CSCSegments_handle);
        }
        if (!CSCSegments_handle.isValid()) {
            if (firstEvent_)
                edm::LogError("NtupleMaker")
                    << "Cannot get the product: " << CSCSegmentTag_;
            CSCSegments_ = nullptr;
        } else {
            CSCSegments_ = CSCSegments_handle.product();
        }

    } else {
        CSCSegments_ = nullptr;
    }

    // EMTF hits and tracks
    auto EMTFHits_handle = make_handle(EMTFHits_);
    auto EMTFUnpHits_handle = make_handle(EMTFUnpHits_);

    auto EMTFTracks_handle = make_handle(EMTFTracks_);
    auto EMTFUnpTracks_handle = make_handle(EMTFUnpTracks_);

    if (useEMTFHits_) {
        if (!EMTFHitToken_.isUninitialized()) {
            iEvent.getByToken(EMTFHitToken_, EMTFHits_handle);
        }
        if (!EMTFHits_handle.isValid()) {
            if (firstEvent_)
                edm::LogError("NtupleMaker")
                    << "Cannot get the product: " << EMTFHitTag_;
        } else {
            EMTFHits_.clear();
            for (const auto &hit : (*EMTFHits_handle)) {
                // if (!(-2 <= hit.BX() && hit.BX() <= 2))  continue;  // only
                // BX=[-2,+2]
                EMTFHits_.push_back(hit);
            }
        }
    }

    if (useEMTFUnpHits_) {
        if (!EMTFUnpHitToken_.isUninitialized()) {
            iEvent.getByToken(EMTFUnpHitToken_, EMTFUnpHits_handle);
        }
        if (!EMTFUnpHits_handle.isValid()) {
            if (firstEvent_)
                edm::LogError("NtupleMaker")
                    << "Cannot get the product: " << EMTFUnpHitTag_;
        } else {
            EMTFUnpHits_.clear();
            for (const auto &hit : (*EMTFUnpHits_handle)) {
                // if (!(-2 <= hit.BX() && hit.BX() <= 2))  continue;  // only
                // BX=[-2,+2]
                EMTFUnpHits_.push_back(hit);
            }
        }
    }

    if (useEMTFTracks_) {
        if (!EMTFTrackToken_.isUninitialized()) {
            iEvent.getByToken(EMTFTrackToken_, EMTFTracks_handle);
        }
        if (!EMTFTracks_handle.isValid()) {
            if (firstEvent_)
                edm::LogError("NtupleMaker")
                    << "Cannot get the product: " << EMTFTrackTag_;
        } else {
            EMTFTracks_.clear();
            for (const auto &track : (*EMTFTracks_handle)) {
                // if (trk.BX() != 0)      continue;  // only BX=0
                EMTFTracks_.push_back(track);
            }
        }
    }

    if (useEMTFUnpTracks_) {
        if (!EMTFUnpTrackToken_.isUninitialized()) {
            iEvent.getByToken(EMTFUnpTrackToken_, EMTFUnpTracks_handle);
        }
        if (!EMTFUnpTracks_handle.isValid()) {
            if (firstEvent_)
                edm::LogError("NtupleMaker")
                    << "Cannot get the product: " << EMTFUnpTrackTag_;
        } else {
            EMTFUnpTracks_.clear();
            for (const auto &track : (*EMTFUnpTracks_handle)) {
                // if (trk.BX() != 0)      continue;  // only BX=0
                EMTFUnpTracks_.push_back(track);
            }
        }
    }

    // GMT muons and GEN particles
    auto GMTMuons_handle = make_handle(GMTMuons_);
    auto GMTUnpMuons_handle = make_handle(GMTUnpMuons_);

    auto GENParts_handle = make_handle(GENParts_);

    if (useGMTMuons_) {
        if (!GMTMuonToken_.isUninitialized()) {
            iEvent.getByToken(GMTMuonToken_, GMTMuons_handle);
        }
        if (!GMTMuons_handle.isValid()) {
            if (firstEvent_)
                edm::LogError("NtupleMaker")
                    << "Cannot get the product: " << GMTMuonTag_;
            GMTMuons_ = nullptr;
        } else {
            GMTMuons_ = GMTMuons_handle.product();
        }
    } else {
        GMTMuons_ = nullptr;
    }

    if (useGMTUnpMuons_) {
        if (!GMTUnpMuonToken_.isUninitialized()) {
            iEvent.getByToken(GMTUnpMuonToken_, GMTUnpMuons_handle);
        }
        if (!GMTUnpMuons_handle.isValid()) {
            if (firstEvent_)
                edm::LogError("NtupleMaker")
                    << "Cannot get the product: " << GMTUnpMuonTag_;
            GMTUnpMuons_ = nullptr;
        } else {
            GMTUnpMuons_ = GMTUnpMuons_handle.product();
        }
    } else {
        GMTUnpMuons_ = nullptr;
    }
    if (!iEvent.isRealData() && useGENParts_) {
        if (!GENPartToken_.isUninitialized()) {
            iEvent.getByToken(GENPartToken_, GENParts_handle);
        }
        if (!GENParts_handle.isValid()) {
            if (firstEvent_)
                edm::LogError("NtupleMaker")
                    << "Cannot get the product: " << GENPartTag_;
            GENParts_ = nullptr;
        } else {
            GENParts_ = GENParts_handle.product();
        }
    } else {
        GENParts_ = nullptr;
    }

    // reco muons
    auto RecoMuon_handle = make_handle(RecoMuons_);
    auto Vertices_handle = make_handle(Vertices_);
    auto TriggerResults_handle = make_handle(TriggerResults_);
    auto TriggerSummaryLabels_handle = make_handle(TriggerSummaryLabels_);

    if (useRecoMuons_) {
        if (!RecoMuonToken_.isUninitialized()) {
            iEvent.getByToken(RecoMuonToken_, RecoMuon_handle);
        }
        if (!RecoMuon_handle.isValid()) {
            if (firstEvent_)
                edm::LogError("NtupleMaker")
                    << "Cannot get the product: " << RecoMuonTag_;
            RecoMuons_ = nullptr;
        } else {
            RecoMuons_ = RecoMuon_handle.product();
        }
        if (!TriggerResultsToken_.isUninitialized()) {
            iEvent.getByToken(TriggerResultsToken_, TriggerResults_handle);
        }
        if (!TriggerResults_handle.isValid()) {
            if (firstEvent_)
                edm::LogError("NtupleMaker")
                    << "Cannot get the product: TriggerResults";
            TriggerResults_ = nullptr;
        } else {
            TriggerResults_ = TriggerResults_handle.product();
        }
        if (!TriggerSummaryLabelsToken_.isUninitialized()) {
            iEvent.getByToken(TriggerSummaryLabelsToken_, TriggerSummaryLabels_handle);
        }
        if (!TriggerSummaryLabels_handle.isValid()) {
            if (firstEvent_)
                edm::LogError("NtupleMaker")
                    << "Cannot get the product: hltTriggerSummaryAOD";
            TriggerSummaryLabels_ = nullptr;
        } else {
            TriggerSummaryLabels_ = TriggerSummaryLabels_handle.product();
        }
        if (!VerticesToken_.isUninitialized()) {
            iEvent.getByToken(VerticesToken_, Vertices_handle);
        }
        if (!Vertices_handle.isValid()) {
            if (firstEvent_)
                edm::LogError("NtupleMaker")
                    << "Cannot get the product: hltTriggerSummaryAOD";
            Vertices_ = nullptr;
        } else {
            Vertices_ = Vertices_handle.product();
        }
    } else {
        RecoMuons_ = nullptr;
        TriggerResults_ = nullptr;
        TriggerSummaryLabels_ = nullptr;
        Vertices_ = nullptr;
    }
}

void MuonAODAnalyzer::makeTree() {

    if (verbose_ > 0)
        std::cout << "******* Making Output Tree *******" << std::endl;

    // TFileService
    edm::Service<TFileService> fs;
    tree = fs->make<TTree>("tree", "tree");

    // Create pointers
    // CSC Inputs
    cscInput_endcap = std::make_unique<std::vector<int16_t>>();
    cscInput_station = std::make_unique<std::vector<int16_t>>();
    cscInput_ring = std::make_unique<std::vector<int16_t>>();
    cscInput_sector = std::make_unique<std::vector<int16_t>>();
    cscInput_chamber = std::make_unique<std::vector<int16_t>>();
    cscInput_cscid = std::make_unique<std::vector<int16_t>>();
    cscInput_bx = std::make_unique<std::vector<int16_t>>();
    cscInput_strip = std::make_unique<std::vector<int16_t>>();
    cscInput_wire = std::make_unique<std::vector<int16_t>>();
    cscInput_quality = std::make_unique<std::vector<int16_t>>();
    cscInput_pattern = std::make_unique<std::vector<int16_t>>();
    cscInput_bend = std::make_unique<std::vector<int16_t>>();

    // RPC Inputs
    rpcInput_region = std::make_unique<std::vector<int16_t>>();
    rpcInput_station = std::make_unique<std::vector<int16_t>>();
    rpcInput_ring = std::make_unique<std::vector<int16_t>>();
    rpcInput_sector = std::make_unique<std::vector<int16_t>>();
    rpcInput_subsector = std::make_unique<std::vector<int16_t>>();
    rpcInput_roll = std::make_unique<std::vector<int16_t>>();
    rpcInput_bx = std::make_unique<std::vector<int16_t>>();
    rpcInput_strip = std::make_unique<std::vector<int16_t>>();
    rpcInput_strip_high = std::make_unique<std::vector<int16_t>>();
    rpcInput_strip_low = std::make_unique<std::vector<int16_t>>();
    rpcInput_time = std::make_unique<std::vector<int16_t>>();
    rpcInput_valid = std::make_unique<std::vector<int16_t>>();

    // GEM Inputs
    gemInput_region = std::make_unique<std::vector<int16_t>>();
    gemInput_station = std::make_unique<std::vector<int16_t>>();
    gemInput_ring = std::make_unique<std::vector<int16_t>>();
    gemInput_sector = std::make_unique<std::vector<int16_t>>();
    gemInput_chamber = std::make_unique<std::vector<int16_t>>();
    gemInput_roll = std::make_unique<std::vector<int16_t>>();
    gemInput_bx = std::make_unique<std::vector<int16_t>>();
    gemInput_layer = std::make_unique<std::vector<int16_t>>();
    gemInput_pad = std::make_unique<std::vector<int16_t>>();
    gemInput_pad_low = std::make_unique<std::vector<int16_t>>();
    gemInput_pad_high = std::make_unique<std::vector<int16_t>>();

    // ME0 Inputs
    me0Input_region = std::make_unique<std::vector<int16_t>>();
    me0Input_station = std::make_unique<std::vector<int16_t>>();
    me0Input_ring = std::make_unique<std::vector<int16_t>>();
    me0Input_sector = std::make_unique<std::vector<int16_t>>();
    me0Input_chamber = std::make_unique<std::vector<int16_t>>();
    me0Input_roll = std::make_unique<std::vector<int16_t>>();
    me0Input_bx = std::make_unique<std::vector<int16_t>>();
    me0Input_layer = std::make_unique<std::vector<int16_t>>();
    me0Input_phiposition = std::make_unique<std::vector<int16_t>>();
    me0Input_deltaphi = std::make_unique<std::vector<int16_t>>();
    me0Input_quality = std::make_unique<std::vector<int16_t>>();
    me0Input_bend = std::make_unique<std::vector<int16_t>>();
    me0Input_partition = std::make_unique<std::vector<int16_t>>();

    // DT Inputs
    dtInput_wheel = std::make_unique<std::vector<int16_t>>();
    dtInput_station = std::make_unique<std::vector<int16_t>>();
    dtInput_btigroup = std::make_unique<std::vector<int16_t>>();
    dtInput_bx = std::make_unique<std::vector<int16_t>>();
    dtInput_strip = std::make_unique<std::vector<int16_t>>();
    dtInput_wire = std::make_unique<std::vector<int16_t>>();
    dtInput_quality = std::make_unique<std::vector<int16_t>>();
    dtInput_bend = std::make_unique<std::vector<int16_t>>();

    // EMTF Hits
    emtfHit_endcap = std::make_unique<std::vector<int16_t>>();
    emtfHit_station = std::make_unique<std::vector<int16_t>>();
    emtfHit_ring = std::make_unique<std::vector<int16_t>>();
    emtfHit_sector = std::make_unique<std::vector<int16_t>>();
    emtfHit_subsector = std::make_unique<std::vector<int16_t>>();
    emtfHit_chamber = std::make_unique<std::vector<int16_t>>();
    emtfHit_cscid = std::make_unique<std::vector<int16_t>>();
    emtfHit_bx = std::make_unique<std::vector<int16_t>>();
    emtfHit_type = std::make_unique<
        std::vector<int16_t>>(); // subsystem: DT=0,CSC=1,RPC=2,GEM=3,ME0=4
    emtfHit_neighbor = std::make_unique<std::vector<int16_t>>();
    //
    emtfHit_strip = std::make_unique<std::vector<int16_t>>();
    emtfHit_strip_qses = std::make_unique<std::vector<int16_t>>();
    emtfHit_wire = std::make_unique<std::vector<int16_t>>();
    emtfHit_roll = std::make_unique<std::vector<int16_t>>();
    emtfHit_layer = std::make_unique<std::vector<int16_t>>();
    emtfHit_quality = std::make_unique<std::vector<int16_t>>();
    emtfHit_pattern = std::make_unique<std::vector<int16_t>>();
    emtfHit_bend = std::make_unique<std::vector<int16_t>>();
    emtfHit_slope = std::make_unique<std::vector<int16_t>>();
    emtfHit_time = std::make_unique<std::vector<int16_t>>();
    emtfHit_fr = std::make_unique<std::vector<int16_t>>();
    emtfHit_emtf_phi = std::make_unique<std::vector<int32_t>>(); // integer unit
    emtfHit_emtf_theta =
        std::make_unique<std::vector<int32_t>>(); // integer unit
    //
    emtfHit_sim_phi = std::make_unique<std::vector<float>>();
    emtfHit_sim_theta = std::make_unique<std::vector<float>>();
    emtfHit_sim_r = std::make_unique<std::vector<float>>();
    emtfHit_sim_z = std::make_unique<std::vector<float>>();
    // emtfHit_sim_tp1            = std::make_unique<std::vector<int32_t> >();
    // emtfHit_sim_tp2            = std::make_unique<std::vector<int32_t> >();
    //
    emtfHit_match_iSeg = std::make_unique<std::vector<int32_t>>();
    emtfHit_match_iSeg2 = std::make_unique<std::vector<int32_t>>();
    emtfHit_match_nSegs = std::make_unique<std::vector<int32_t>>();
    emtfHit_match_seg_unique = std::make_unique<std::vector<int16_t>>();

    emtfHit_size = std::make_unique<int32_t>(0);

    // EMTF Unpacked Hits
    emtfUnpHit_endcap = std::make_unique<std::vector<int16_t>>();
    emtfUnpHit_station = std::make_unique<std::vector<int16_t>>();
    emtfUnpHit_ring = std::make_unique<std::vector<int16_t>>();
    emtfUnpHit_sector = std::make_unique<std::vector<int16_t>>();
    emtfUnpHit_subsector = std::make_unique<std::vector<int16_t>>();
    emtfUnpHit_chamber = std::make_unique<std::vector<int16_t>>();
    emtfUnpHit_cscid = std::make_unique<std::vector<int16_t>>();
    emtfUnpHit_bx = std::make_unique<std::vector<int16_t>>();
    emtfUnpHit_type = std::make_unique<
        std::vector<int16_t>>(); // subsystem: DT=0,CSC=1,RPC=2,GEM=3,ME0=4
    emtfUnpHit_neighbor = std::make_unique<std::vector<int16_t>>();
    //
    emtfUnpHit_strip = std::make_unique<std::vector<int16_t>>();
    emtfUnpHit_strip_qses = std::make_unique<std::vector<int16_t>>();
    emtfUnpHit_wire = std::make_unique<std::vector<int16_t>>();
    emtfUnpHit_roll = std::make_unique<std::vector<int16_t>>();
    emtfUnpHit_quality = std::make_unique<std::vector<int16_t>>();
    emtfUnpHit_pattern = std::make_unique<std::vector<int16_t>>();
    emtfUnpHit_bend = std::make_unique<std::vector<int16_t>>();
    emtfUnpHit_time = std::make_unique<std::vector<int16_t>>();
    emtfUnpHit_fr = std::make_unique<std::vector<int16_t>>();
    emtfUnpHit_emtf_phi =
        std::make_unique<std::vector<int32_t>>(); // integer unit
    emtfUnpHit_emtf_theta =
        std::make_unique<std::vector<int32_t>>(); // integer unit
    //
    emtfUnpHit_sim_phi = std::make_unique<std::vector<float>>();
    emtfUnpHit_sim_theta = std::make_unique<std::vector<float>>();
    emtfUnpHit_sim_r = std::make_unique<std::vector<float>>();
    emtfUnpHit_sim_z = std::make_unique<std::vector<float>>();
    // emtfUnpHit_sim_tp1         = std::make_unique<std::vector<int32_t> >();
    // emtfUnpHit_sim_tp2         = std::make_unique<std::vector<int32_t> >();
    emtfUnpHit_size = std::make_unique<int32_t>(0);

    // EMTF Tracks
    emtfTrack_pt = std::make_unique<std::vector<float>>();
    emtfTrack_xml_pt = std::make_unique<std::vector<float>>();
    emtfTrack_pt_dxy = std::make_unique<std::vector<float>>();
    emtfTrack_dxy = std::make_unique<std::vector<float>>();
    emtfTrack_invpt_prompt = std::make_unique<std::vector<float>>();
    emtfTrack_invpt_displ = std::make_unique<std::vector<float>>();
    emtfTrack_phi = std::make_unique<std::vector<float>>();        // in degrees
    emtfTrack_phi_fp = std::make_unique<std::vector<int32_t>>();   // in degrees
    emtfTrack_theta = std::make_unique<std::vector<float>>();      // in degrees
    emtfTrack_theta_fp = std::make_unique<std::vector<int32_t>>(); // in degrees
    emtfTrack_eta = std::make_unique<std::vector<float>>();
    emtfTrack_GMT_phi = std::make_unique<std::vector<int32_t>>();
    emtfTrack_GMT_eta = std::make_unique<std::vector<int32_t>>();
    emtfTrack_GMT_dxy = std::make_unique<std::vector<int32_t>>();
    emtfTrack_q = std::make_unique<std::vector<int16_t>>(); // charge
    //
    emtfTrack_mode = std::make_unique<std::vector<int16_t>>();
    emtfTrack_endcap = std::make_unique<std::vector<int16_t>>();
    emtfTrack_sector = std::make_unique<std::vector<int16_t>>();
    emtfTrack_bx = std::make_unique<std::vector<int16_t>>();
    emtfTrack_nhits = std::make_unique<std::vector<int16_t>>();
    emtfTrack_hitref1 = std::make_unique<std::vector<int32_t>>();
    emtfTrack_hitref2 = std::make_unique<std::vector<int32_t>>();
    emtfTrack_hitref3 = std::make_unique<std::vector<int32_t>>();
    emtfTrack_hitref4 = std::make_unique<std::vector<int32_t>>();
    emtfTrack_size = std::make_unique<int32_t>(0);

    emtfTrack_straightness = std::make_unique<std::vector<int16_t>>();

    // PtLUT information
    emtfTrack_ptLUT_address = std::make_unique<std::vector<uint64_t>>();
    emtfTrack_ptLUT_mode = std::make_unique<std::vector<uint16_t>>();
    emtfTrack_ptLUT_theta = std::make_unique<std::vector<uint16_t>>();
    emtfTrack_ptLUT_st1_ring2 = std::make_unique<std::vector<uint16_t>>();
    emtfTrack_ptLUT_eta = std::make_unique<std::vector<uint16_t>>();
    emtfTrack_ptLUT_deltaPh =
        std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfTrack_ptLUT_deltaTh =
        std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfTrack_ptLUT_signPh =
        std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfTrack_ptLUT_signTh =
        std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfTrack_ptLUT_cpattern =
        std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfTrack_ptLUT_fr = std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfTrack_ptLUT_bt_vi =
        std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfTrack_ptLUT_bt_hi =
        std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfTrack_ptLUT_bt_ci =
        std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfTrack_ptLUT_bt_si =
        std::make_unique<std::vector<std::vector<uint16_t>>>();

    // EMTF Unpacked Tracks
    emtfUnpTrack_pt = std::make_unique<std::vector<float>>();
    emtfUnpTrack_xml_pt = std::make_unique<std::vector<float>>();
    emtfUnpTrack_pt_dxy = std::make_unique<std::vector<float>>();
    emtfUnpTrack_dxy = std::make_unique<std::vector<float>>();
    emtfUnpTrack_invpt_prompt = std::make_unique<std::vector<float>>();
    emtfUnpTrack_invpt_displ = std::make_unique<std::vector<float>>();
    emtfUnpTrack_phi = std::make_unique<std::vector<float>>();   // in degrees
    emtfUnpTrack_theta = std::make_unique<std::vector<float>>(); // in degrees
    emtfUnpTrack_eta = std::make_unique<std::vector<float>>();
    emtfUnpTrack_GMT_dxy = std::make_unique<std::vector<int32_t>>();
    emtfUnpTrack_q = std::make_unique<std::vector<int16_t>>(); // charge
    //
    emtfUnpTrack_mode = std::make_unique<std::vector<int16_t>>();
    emtfUnpTrack_endcap = std::make_unique<std::vector<int16_t>>();
    emtfUnpTrack_sector = std::make_unique<std::vector<int16_t>>();
    emtfUnpTrack_bx = std::make_unique<std::vector<int16_t>>();
    emtfUnpTrack_nhits = std::make_unique<std::vector<int16_t>>();
    emtfUnpTrack_hitref1 = std::make_unique<std::vector<int32_t>>();
    emtfUnpTrack_hitref2 = std::make_unique<std::vector<int32_t>>();
    emtfUnpTrack_hitref3 = std::make_unique<std::vector<int32_t>>();
    emtfUnpTrack_hitref4 = std::make_unique<std::vector<int32_t>>();
    emtfUnpTrack_size = std::make_unique<int32_t>(0);
    // PtLUT information
    emtfUnpTrack_ptLUT_address = std::make_unique<std::vector<uint64_t>>();
    emtfUnpTrack_ptLUT_mode = std::make_unique<std::vector<uint16_t>>();
    emtfUnpTrack_ptLUT_theta = std::make_unique<std::vector<uint16_t>>();
    emtfUnpTrack_ptLUT_st1_ring2 = std::make_unique<std::vector<uint16_t>>();
    emtfUnpTrack_ptLUT_eta = std::make_unique<std::vector<uint16_t>>();
    emtfUnpTrack_ptLUT_deltaPh =
        std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfUnpTrack_ptLUT_deltaTh =
        std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfUnpTrack_ptLUT_signPh =
        std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfUnpTrack_ptLUT_signTh =
        std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfUnpTrack_ptLUT_cpattern =
        std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfUnpTrack_ptLUT_fr =
        std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfUnpTrack_ptLUT_bt_vi =
        std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfUnpTrack_ptLUT_bt_hi =
        std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfUnpTrack_ptLUT_bt_ci =
        std::make_unique<std::vector<std::vector<uint16_t>>>();
    emtfUnpTrack_ptLUT_bt_si =
        std::make_unique<std::vector<std::vector<uint16_t>>>();

    // GMT Muons
    gmtMuon_pt = std::make_unique<std::vector<float>>();
    gmtMuon_pt_dxy = std::make_unique<std::vector<float>>();
    gmtMuon_dxy = std::make_unique<std::vector<int16_t>>();
    gmtMuon_phi = std::make_unique<std::vector<float>>(); // in degrees
    gmtMuon_eta = std::make_unique<std::vector<float>>();
    gmtMuon_q = std::make_unique<std::vector<int16_t>>(); // charge
    gmtMuon_qual = std::make_unique<std::vector<int16_t>>();
    gmtMuon_size = std::make_unique<int32_t>(0);

    // GMT Unpacked Muons
    gmtUnpMuon_pt = std::make_unique<std::vector<float>>();
    gmtUnpMuon_pt_dxy = std::make_unique<std::vector<float>>();
    gmtUnpMuon_dxy = std::make_unique<std::vector<int16_t>>();
    gmtUnpMuon_phi = std::make_unique<std::vector<float>>(); // in degrees
    gmtUnpMuon_eta = std::make_unique<std::vector<float>>();
    gmtUnpMuon_q = std::make_unique<std::vector<int16_t>>(); // charge
    gmtUnpMuon_qual = std::make_unique<std::vector<int16_t>>();
    gmtUnpMuon_size = std::make_unique<int32_t>(0);

    // GEN particles
    genPart_pt = std::make_unique<std::vector<float>>();
    genPart_dxy = std::make_unique<std::vector<float>>();
    genPart_eta = std::make_unique<std::vector<float>>();
    genPart_phi = std::make_unique<std::vector<float>>();
    genPart_q = std::make_unique<std::vector<int16_t>>(); // charge
    genPart_ID = std::make_unique<std::vector<int16_t>>();
    genPart_parentID = std::make_unique<std::vector<int32_t>>();
    genPart_vx = std::make_unique<std::vector<float>>();
    genPart_vy = std::make_unique<std::vector<float>>();
    genPart_vz = std::make_unique<std::vector<float>>();
    genPart_etaSt1 = std::make_unique<std::vector<float>>();
    genPart_phiSt1 = std::make_unique<std::vector<float>>();
    genPart_etaSt2 = std::make_unique<std::vector<float>>();
    genPart_phiSt2 = std::make_unique<std::vector<float>>();

    // CSC Segment info
    cscSegment_locX = std::make_unique<std::vector<float>>();
    cscSegment_locY = std::make_unique<std::vector<float>>();
    cscSegment_globX = std::make_unique<std::vector<float>>();
    cscSegment_globY = std::make_unique<std::vector<float>>();
    cscSegment_globZ = std::make_unique<std::vector<float>>();
    cscSegment_eta = std::make_unique<std::vector<float>>();
    cscSegment_phi = std::make_unique<std::vector<float>>();
    cscSegment_theta = std::make_unique<std::vector<float>>();
    cscSegment_chi2 = std::make_unique<std::vector<float>>();
    cscSegment_time = std::make_unique<std::vector<float>>();
    cscSegment_dirX = std::make_unique<std::vector<float>>();
    cscSegment_dirY = std::make_unique<std::vector<float>>();
    cscSegment_dirZ = std::make_unique<std::vector<float>>();
    cscSegment_bend_theta = std::make_unique<std::vector<float>>();
    cscSegment_bend_phi = std::make_unique<std::vector<float>>();

    cscSegment_endcap = std::make_unique<std::vector<int32_t>>();
    cscSegment_ring = std::make_unique<std::vector<int32_t>>();
    cscSegment_station = std::make_unique<std::vector<int32_t>>();
    cscSegment_chamber = std::make_unique<std::vector<int32_t>>();
    cscSegment_sector = std::make_unique<std::vector<int32_t>>();
    cscSegment_CSC_ID = std::make_unique<std::vector<int32_t>>();

    cscSegment_nRecHits = std::make_unique<std::vector<int32_t>>();

    cscSegment_wire_min = std::make_unique<std::vector<int32_t>>();
    cscSegment_wire_max = std::make_unique<std::vector<int32_t>>();
    cscSegment_strip_min = std::make_unique<std::vector<int32_t>>();
    cscSegment_strip_max = std::make_unique<std::vector<int32_t>>();

    cscSegment_match_iHit = std::make_unique<std::vector<int32_t>>();
    cscSegment_match_iHit2 = std::make_unique<std::vector<int32_t>>();
    cscSegment_match_nHits = std::make_unique<std::vector<int32_t>>();
    cscSegment_match_hit_unique = std::make_unique<std::vector<int16_t>>();

    cscSegment_size = std::make_unique<int32_t>(0);
    cscSegment_nBx0 = std::make_unique<int32_t>(0);

    // Reco muon info
    recoMuon_size = std::make_unique<int32_t>(0);
    recoMuon_e = std::make_unique<std::vector<float>>();
    recoMuon_et = std::make_unique<std::vector<float>>();
    recoMuon_pt = std::make_unique<std::vector<float>>();
    recoMuon_eta = std::make_unique<std::vector<float>>();
    recoMuon_phi = std::make_unique<std::vector<float>>();
    recoMuon_dxy = std::make_unique<std::vector<float>>();
    recoMuon_dz = std::make_unique<std::vector<float>>();
    recoMuon_isLooseMuon = std::make_unique<std::vector<bool>>();
    recoMuon_isMediumMuon = std::make_unique<std::vector<bool>>();
    recoMuon_isTightMuon = std::make_unique<std::vector<bool>>();
    recoMuon_iso = std::make_unique<std::vector<float>>();
    recoMuon_hlt_isomu = std::make_unique<std::vector<short>>();
    recoMuon_hlt_mu = std::make_unique<std::vector<short>>();
    recoMuon_hlt_isoDeltaR = std::make_unique<std::vector<float>>();
    recoMuon_hlt_deltaR = std::make_unique<std::vector<float>>();
    recoMuon_passesSingleMuon = std::make_unique<std::vector<int>>();
    recoMuon_charge = std::make_unique<std::vector<int>>();
    recoMuon_etaSt1 = std::make_unique<std::vector<float>>();
    recoMuon_phiSt1 = std::make_unique<std::vector<float>>();
    recoMuon_etaSt2 = std::make_unique<std::vector<float>>();
    recoMuon_phiSt2 = std::make_unique<std::vector<float>>();

    // Set branches

    // CSC Inputs
    if (useCSC_) {
        tree->Branch("cscInput_endcap", &(*cscInput_endcap));
        tree->Branch("cscInput_station", &(*cscInput_station));
        tree->Branch("cscInput_ring", &(*cscInput_ring));
        tree->Branch("cscInput_sector", &(*cscInput_sector));
        tree->Branch("cscInput_chamber", &(*cscInput_chamber));
        tree->Branch("cscInput_cscid", &(*cscInput_cscid));
        tree->Branch("cscInput_bx", &(*cscInput_bx));
        tree->Branch("cscInput_strip", &(*cscInput_strip));
        tree->Branch("cscInput_wire", &(*cscInput_wire));
        tree->Branch("cscInput_quality", &(*cscInput_quality));
        tree->Branch("cscInput_pattern", &(*cscInput_pattern));
        tree->Branch("cscInput_bend", &(*cscInput_bend));
    }

    // RPC Inputs
    if (useRPC_) {
        tree->Branch("rpcInput_region", &(*rpcInput_region));
        tree->Branch("rpcInput_station", &(*rpcInput_station));
        tree->Branch("rpcInput_ring", &(*rpcInput_ring));
        tree->Branch("rpcInput_sector", &(*rpcInput_sector));
        tree->Branch("rpcInput_subsector", &(*rpcInput_subsector));
        tree->Branch("rpcInput_roll", &(*rpcInput_roll));
        tree->Branch("rpcInput_bx", &(*rpcInput_bx));
        tree->Branch("rpcInput_strip", &(*rpcInput_strip));
        tree->Branch("rpcInput_strip_high", &(*rpcInput_strip_high));
        tree->Branch("rpcInput_strip_low", &(*rpcInput_strip_low));
        tree->Branch("rpcInput_time", &(*rpcInput_time));
        tree->Branch("rpcInput_valid", &(*rpcInput_valid));
    }

    // GEM Inputs
    if (useGEM_) {
        tree->Branch("gemInput_region", &(*gemInput_region));
        tree->Branch("gemInput_station", &(*gemInput_station));
        tree->Branch("gemInput_ring", &(*gemInput_ring));
        tree->Branch("gemInput_sector", &(*gemInput_sector));
        tree->Branch("gemInput_chamber", &(*gemInput_chamber));
        tree->Branch("gemInput_roll", &(*gemInput_roll));
        tree->Branch("gemInput_bx", &(*gemInput_bx));
        tree->Branch("gemInput_layer", &(*gemInput_layer));
        tree->Branch("gemInput_pad", &(*gemInput_pad));
        tree->Branch("gemInput_pad_high", &(*gemInput_pad_high));
        tree->Branch("gemInput_pad_low", &(*gemInput_pad_low));
    }

    // Phase 2 inputs
    if (enablePhase2_) {
        // ME0 Inputs
        tree->Branch("me0Input_region", &(*me0Input_region));
        tree->Branch("me0Input_station", &(*me0Input_station));
        tree->Branch("me0Input_ring", &(*me0Input_ring));
        tree->Branch("me0Input_sector", &(*me0Input_sector));
        tree->Branch("me0Input_chamber", &(*me0Input_chamber));
        tree->Branch("me0Input_roll", &(*me0Input_roll));
        tree->Branch("me0Input_bx", &(*me0Input_bx));
        tree->Branch("me0Input_layer", &(*me0Input_layer));
        tree->Branch("me0Input_phiposition", &(*me0Input_phiposition));
        tree->Branch("me0Input_deltaphi", &(*me0Input_deltaphi));
        tree->Branch("me0Input_quality", &(*me0Input_quality));
        tree->Branch("me0Input_bend", &(*me0Input_bend));
        tree->Branch("me0Input_partition", &(*me0Input_partition));

        // DT Inputs
        tree->Branch("dtInput_wheel", &(*dtInput_wheel));
        tree->Branch("dtInput_station", &(*dtInput_station));
        tree->Branch("dtInput_btigroup", &(*dtInput_btigroup));
        tree->Branch("dtInput_bx", &(*dtInput_bx));
        tree->Branch("dtInput_strip", &(*dtInput_strip));
        tree->Branch("dtInput_wire", &(*dtInput_wire));
        tree->Branch("dtInput_quality", &(*dtInput_quality));
        tree->Branch("dtInput_bend", &(*dtInput_bend));
    }

    // EMTF Hits
    if (useEMTFHits_) {
        tree->Branch("emtfHit_endcap", &(*emtfHit_endcap));
        tree->Branch("emtfHit_station", &(*emtfHit_station));
        tree->Branch("emtfHit_ring", &(*emtfHit_ring));
        tree->Branch("emtfHit_sector", &(*emtfHit_sector));
        tree->Branch("emtfHit_subsector", &(*emtfHit_subsector));
        tree->Branch("emtfHit_chamber", &(*emtfHit_chamber));
        tree->Branch("emtfHit_cscid", &(*emtfHit_cscid));
        tree->Branch("emtfHit_bx", &(*emtfHit_bx));
        tree->Branch("emtfHit_type", &(*emtfHit_type));
        tree->Branch("emtfHit_neighbor", &(*emtfHit_neighbor));
        //
        tree->Branch("emtfHit_strip", &(*emtfHit_strip));
        tree->Branch("emtfHit_strip_qses", &(*emtfHit_strip_qses));
        tree->Branch("emtfHit_wire", &(*emtfHit_wire));
        tree->Branch("emtfHit_roll", &(*emtfHit_roll));
        tree->Branch("emtfHit_layer", &(*emtfHit_layer));
        tree->Branch("emtfHit_quality", &(*emtfHit_quality));
        tree->Branch("emtfHit_pattern", &(*emtfHit_pattern));
        tree->Branch("emtfHit_bend", &(*emtfHit_bend));
        tree->Branch("emtfHit_slope", &(*emtfHit_slope));
        tree->Branch("emtfHit_time", &(*emtfHit_time));
        tree->Branch("emtfHit_fr", &(*emtfHit_fr));
        tree->Branch("emtfHit_emtf_phi", &(*emtfHit_emtf_phi));
        tree->Branch("emtfHit_emtf_theta", &(*emtfHit_emtf_theta));
        //
        tree->Branch("emtfHit_sim_phi", &(*emtfHit_sim_phi));
        tree->Branch("emtfHit_sim_theta", &(*emtfHit_sim_theta));
        tree->Branch("emtfHit_sim_r", &(*emtfHit_sim_r));
        tree->Branch("emtfHit_sim_z", &(*emtfHit_sim_z));
        // tree->Branch("emtfHit_sim_tp1"   , &(*emtfHit_sim_tp1   ));
        // tree->Branch("emtfHit_sim_tp2"   , &(*emtfHit_sim_tp2   ));
        //
        tree->Branch("emtfHit_match_iSeg", &(*emtfHit_match_iSeg));
        tree->Branch("emtfHit_match_iSeg2", &(*emtfHit_match_iSeg2));
        tree->Branch("emtfHit_match_nSegs", &(*emtfHit_match_nSegs));
        tree->Branch("emtfHit_match_seg_unique", &(*emtfHit_match_seg_unique));

        tree->Branch("emtfHit_size", &(*emtfHit_size));
    }

    // EMTF Unpacked Hits
    if (useEMTFUnpHits_) {
        tree->Branch("emtfUnpHit_endcap", &(*emtfUnpHit_endcap));
        tree->Branch("emtfUnpHit_station", &(*emtfUnpHit_station));
        tree->Branch("emtfUnpHit_ring", &(*emtfUnpHit_ring));
        tree->Branch("emtfUnpHit_sector", &(*emtfUnpHit_sector));
        tree->Branch("emtfUnpHit_subsector", &(*emtfUnpHit_subsector));
        tree->Branch("emtfUnpHit_chamber", &(*emtfUnpHit_chamber));
        tree->Branch("emtfUnpHit_cscid", &(*emtfUnpHit_cscid));
        tree->Branch("emtfUnpHit_bx", &(*emtfUnpHit_bx));
        tree->Branch("emtfUnpHit_type", &(*emtfUnpHit_type));
        tree->Branch("emtfUnpHit_neighbor", &(*emtfUnpHit_neighbor));
        //
        tree->Branch("emtfUnpHit_strip", &(*emtfUnpHit_strip));
        tree->Branch("emtfUnpHit_strip_qses", &(*emtfUnpHit_strip_qses));
        tree->Branch("emtfUnpHit_wire", &(*emtfUnpHit_wire));
        tree->Branch("emtfUnpHit_roll", &(*emtfUnpHit_roll));
        tree->Branch("emtfUnpHit_quality", &(*emtfUnpHit_quality));
        tree->Branch("emtfUnpHit_pattern", &(*emtfUnpHit_pattern));
        tree->Branch("emtfUnpHit_bend", &(*emtfUnpHit_bend));
        tree->Branch("emtfUnpHit_time", &(*emtfUnpHit_time));
        tree->Branch("emtfUnpHit_fr", &(*emtfUnpHit_fr));
        tree->Branch("emtfUnpHit_emtf_phi", &(*emtfUnpHit_emtf_phi));
        tree->Branch("emtfUnpHit_emtf_theta", &(*emtfUnpHit_emtf_theta));
        //
        tree->Branch("emtfUnpHit_sim_phi", &(*emtfUnpHit_sim_phi));
        tree->Branch("emtfUnpHit_sim_theta", &(*emtfUnpHit_sim_theta));
        tree->Branch("emtfUnpHit_sim_r", &(*emtfUnpHit_sim_r));
        tree->Branch("emtfUnpHit_sim_z", &(*emtfUnpHit_sim_z));
        // tree->Branch("emtfUnpHit_sim_tp1"   , &(*emtfUnpHit_sim_tp1   ));
        // tree->Branch("emtfUnpHit_sim_tp2"   , &(*emtfUnpHit_sim_tp2   ));
        tree->Branch("emtfUnpHit_size", &(*emtfUnpHit_size));
    }

    // EMTF Tracks
    if (useEMTFTracks_) {
        tree->Branch("emtfTrack_pt", &(*emtfTrack_pt));
        tree->Branch("emtfTrack_xml_pt", &(*emtfTrack_xml_pt));
        tree->Branch("emtfTrack_pt_dxy", &(*emtfTrack_pt_dxy));
        tree->Branch("emtfTrack_dxy", &(*emtfTrack_dxy));
        tree->Branch("emtfTrack_invpt_prompt", &(*emtfTrack_invpt_prompt));
        tree->Branch("emtfTrack_invpt_displ", &(*emtfTrack_invpt_displ));
        tree->Branch("emtfTrack_phi", &(*emtfTrack_phi));
        tree->Branch("emtfTrack_phi_fp", &(*emtfTrack_phi_fp));
        tree->Branch("emtfTrack_theta", &(*emtfTrack_theta));
        tree->Branch("emtfTrack_theta_fp", &(*emtfTrack_theta_fp));
        tree->Branch("emtfTrack_eta", &(*emtfTrack_eta));
        tree->Branch("emtfTrack_GMT_phi", &(*emtfTrack_GMT_phi));
        tree->Branch("emtfTrack_GMT_eta", &(*emtfTrack_GMT_eta));
        tree->Branch("emtfTrack_GMT_dxy", &(*emtfTrack_GMT_dxy));
        tree->Branch("emtfTrack_q", &(*emtfTrack_q));
        //
        tree->Branch("emtfTrack_mode", &(*emtfTrack_mode));
        tree->Branch("emtfTrack_endcap", &(*emtfTrack_endcap));
        tree->Branch("emtfTrack_sector", &(*emtfTrack_sector));
        tree->Branch("emtfTrack_bx", &(*emtfTrack_bx));
        tree->Branch("emtfTrack_nhits", &(*emtfTrack_nhits));
        tree->Branch("emtfTrack_hitref1", &(*emtfTrack_hitref1));
        tree->Branch("emtfTrack_hitref2", &(*emtfTrack_hitref2));
        tree->Branch("emtfTrack_hitref3", &(*emtfTrack_hitref3));
        tree->Branch("emtfTrack_hitref4", &(*emtfTrack_hitref4));
        tree->Branch("emtfTrack_size", &(*emtfTrack_size));

        tree->Branch("emtfTrack_straightness", &(*emtfTrack_straightness));
        // PtLUT information
        tree->Branch("emtfTrack_ptLUT_address", &(*emtfTrack_ptLUT_address));
        tree->Branch("emtfTrack_ptLUT_mode", &(*emtfTrack_ptLUT_mode));
        tree->Branch("emtfTrack_ptLUT_theta", &(*emtfTrack_ptLUT_theta));
        tree->Branch("emtfTrack_ptLUT_st1_ring2",
                     &(*emtfTrack_ptLUT_st1_ring2));
        tree->Branch("emtfTrack_ptLUT_eta", &(*emtfTrack_ptLUT_eta));
        tree->Branch("emtfTrack_ptLUT_deltaPh", &(*emtfTrack_ptLUT_deltaPh));
        tree->Branch("emtfTrack_ptLUT_deltaTh", &(*emtfTrack_ptLUT_deltaTh));
        tree->Branch("emtfTrack_ptLUT_signPh", &(*emtfTrack_ptLUT_signPh));
        tree->Branch("emtfTrack_ptLUT_signTh", &(*emtfTrack_ptLUT_signTh));
        tree->Branch("emtfTrack_ptLUT_cpattern", &(*emtfTrack_ptLUT_cpattern));
        tree->Branch("emtfTrack_ptLUT_fr", &(*emtfTrack_ptLUT_fr));
        tree->Branch("emtfTrack_ptLUT_bt_vi", &(*emtfTrack_ptLUT_bt_vi));
        tree->Branch("emtfTrack_ptLUT_bt_hi", &(*emtfTrack_ptLUT_bt_hi));
        tree->Branch("emtfTrack_ptLUT_bt_ci", &(*emtfTrack_ptLUT_bt_ci));
        tree->Branch("emtfTrack_ptLUT_bt_si", &(*emtfTrack_ptLUT_bt_si));
    }

    // EMTF Unpacked Tracks
    if (useEMTFUnpTracks_) {
        tree->Branch("emtfUnpTrack_pt", &(*emtfUnpTrack_pt));
        tree->Branch("emtfUnpTrack_xml_pt", &(*emtfUnpTrack_xml_pt));
        tree->Branch("emtfUnpTrack_pt_dxy", &(*emtfUnpTrack_pt_dxy));
        tree->Branch("emtfUnpTrack_dxy", &(*emtfUnpTrack_dxy));
        tree->Branch("emtfUnpTrack_invpt_prompt",
                     &(*emtfUnpTrack_invpt_prompt));
        tree->Branch("emtfUnpTrack_invpt_displ", &(*emtfUnpTrack_invpt_displ));
        tree->Branch("emtfUnpTrack_phi", &(*emtfUnpTrack_phi));
        tree->Branch("emtfUnpTrack_theta", &(*emtfUnpTrack_theta));
        tree->Branch("emtfUnpTrack_eta", &(*emtfUnpTrack_eta));
        tree->Branch("emtfUnpTrack_GMT_dxy", &(*emtfUnpTrack_GMT_dxy));
        tree->Branch("emtfUnpTrack_q", &(*emtfUnpTrack_q));
        //
        tree->Branch("emtfUnpTrack_mode", &(*emtfUnpTrack_mode));
        tree->Branch("emtfUnpTrack_endcap", &(*emtfUnpTrack_endcap));
        tree->Branch("emtfUnpTrack_sector", &(*emtfUnpTrack_sector));
        tree->Branch("emtfUnpTrack_bx", &(*emtfUnpTrack_bx));
        tree->Branch("emtfUnpTrack_nhits", &(*emtfUnpTrack_nhits));
        tree->Branch("emtfUnpTrack_hitref1", &(*emtfUnpTrack_hitref1));
        tree->Branch("emtfUnpTrack_hitref2", &(*emtfUnpTrack_hitref2));
        tree->Branch("emtfUnpTrack_hitref3", &(*emtfUnpTrack_hitref3));
        tree->Branch("emtfUnpTrack_hitref4", &(*emtfUnpTrack_hitref4));
        tree->Branch("emtfUnpTrack_size", &(*emtfUnpTrack_size));
        // PtLUT information
        tree->Branch("emtfUnpTrack_ptLUT_address",
                     &(*emtfUnpTrack_ptLUT_address));
        tree->Branch("emtfUnpTrack_ptLUT_mode", &(*emtfUnpTrack_ptLUT_mode));
        tree->Branch("emtfUnpTrack_ptLUT_theta", &(*emtfUnpTrack_ptLUT_theta));
        tree->Branch("emtfUnpTrack_ptLUT_st1_ring2",
                     &(*emtfUnpTrack_ptLUT_st1_ring2));
        tree->Branch("emtfUnpTrack_ptLUT_eta", &(*emtfUnpTrack_ptLUT_eta));
        tree->Branch("emtfUnpTrack_ptLUT_deltaPh",
                     &(*emtfUnpTrack_ptLUT_deltaPh));
        tree->Branch("emtfUnpTrack_ptLUT_deltaTh",
                     &(*emtfUnpTrack_ptLUT_deltaTh));
        tree->Branch("emtfUnpTrack_ptLUT_signPh",
                     &(*emtfUnpTrack_ptLUT_signPh));
        tree->Branch("emtfUnpTrack_ptLUT_signTh",
                     &(*emtfUnpTrack_ptLUT_signTh));
        tree->Branch("emtfUnpTrack_ptLUT_cpattern",
                     &(*emtfUnpTrack_ptLUT_cpattern));
        tree->Branch("emtfUnpTrack_ptLUT_fr", &(*emtfUnpTrack_ptLUT_fr));
        tree->Branch("emtfUnpTrack_ptLUT_bt_vi", &(*emtfUnpTrack_ptLUT_bt_vi));
        tree->Branch("emtfUnpTrack_ptLUT_bt_hi", &(*emtfUnpTrack_ptLUT_bt_hi));
        tree->Branch("emtfUnpTrack_ptLUT_bt_ci", &(*emtfUnpTrack_ptLUT_bt_ci));
        tree->Branch("emtfUnpTrack_ptLUT_bt_si", &(*emtfUnpTrack_ptLUT_bt_si));
    }

    // GMT muons
    if (useGMTMuons_) {
        tree->Branch("gmtMuon_pt", &(*gmtMuon_pt));
        tree->Branch("gmtMuon_pt_dxy", &(*gmtMuon_pt_dxy));
        tree->Branch("gmtMuon_dxy", &(*gmtMuon_dxy));
        tree->Branch("gmtMuon_phi", &(*gmtMuon_phi));
        tree->Branch("gmtMuon_eta", &(*gmtMuon_eta));
        tree->Branch("gmtMuon_q", &(*gmtMuon_q));
        tree->Branch("gmtMuon_qual", &(*gmtMuon_qual));
        tree->Branch("gmtMuon_size", &(*gmtMuon_size));
    }

    // GMT Unpacked muons
    if (useGMTUnpMuons_) {
        tree->Branch("gmtUnpMuon_pt", &(*gmtUnpMuon_pt));
        tree->Branch("gmtUnpMuon_pt_dxy", &(*gmtUnpMuon_pt_dxy));
        tree->Branch("gmtUnpMuon_dxy", &(*gmtUnpMuon_dxy));
        tree->Branch("gmtUnpMuon_phi", &(*gmtUnpMuon_phi));
        tree->Branch("gmtUnpMuon_eta", &(*gmtUnpMuon_eta));
        tree->Branch("gmtUnpMuon_q", &(*gmtUnpMuon_q));
        tree->Branch("gmtUnpMuon_qual", &(*gmtUnpMuon_qual));
        tree->Branch("gmtUnpMuon_size", &(*gmtUnpMuon_size));
    }

    // GEN particles
    if (useGENParts_) {
        tree->Branch("genPart_pt", &(*genPart_pt));
        tree->Branch("genPart_dxy", &(*genPart_dxy));
        tree->Branch("genPart_phi", &(*genPart_phi));
        tree->Branch("genPart_eta", &(*genPart_eta));
        tree->Branch("genPart_q", &(*genPart_q));
        tree->Branch("genPart_ID", &(*genPart_ID));
        tree->Branch("genPart_parentID", &(*genPart_parentID));
        tree->Branch("genPart_vx", &(*genPart_vx));
        tree->Branch("genPart_vy", &(*genPart_vy));
        tree->Branch("genPart_vz", &(*genPart_vz));
        tree->Branch("genPart_etaSt1", &(*genPart_etaSt1));
        tree->Branch("genPart_phiSt1", &(*genPart_phiSt1));
        tree->Branch("genPart_etaSt2", &(*genPart_etaSt2));
        tree->Branch("genPart_phiSt2", &(*genPart_phiSt2));
    }

    // Event info
    if (useEventInfo_) {
        tree->Branch("eventInfo_event", &(eventInfo_event));
        tree->Branch("eventInfo_run", &(eventInfo_run));
        tree->Branch("eventInfo_lumi", &(eventInfo_lumi));
        tree->Branch("eventInfo_npv", &(eventInfo_npv));
        tree->Branch("eventInfo_nvtx", &(eventInfo_nvtx));
    }

    // Use CSC segments
    if (useCSCSegments_) {
        tree->Branch("cscSegment_locX", &(*cscSegment_locX));
        tree->Branch("cscSegment_locY", &(*cscSegment_locY));
        tree->Branch("cscSegment_globX", &(*cscSegment_globX));
        tree->Branch("cscSegment_globY", &(*cscSegment_globY));
        tree->Branch("cscSegment_globZ", &(*cscSegment_globZ));
        tree->Branch("cscSegment_eta", &(*cscSegment_eta));
        tree->Branch("cscSegment_phi", &(*cscSegment_phi));
        tree->Branch("cscSegment_theta", &(*cscSegment_theta));
        tree->Branch("cscSegment_chi2", &(*cscSegment_chi2));
        tree->Branch("cscSegment_time", &(*cscSegment_time));
        tree->Branch("cscSegment_dirX", &(*cscSegment_dirX));
        tree->Branch("cscSegment_dirY", &(*cscSegment_dirY));
        tree->Branch("cscSegment_dirZ", &(*cscSegment_dirZ));
        tree->Branch("cscSegment_bend_theta", &(*cscSegment_bend_theta));
        tree->Branch("cscSegment_bend_phi", &(*cscSegment_bend_phi));

        tree->Branch("cscSegment_endcap", &(*cscSegment_endcap));
        tree->Branch("cscSegment_ring", &(*cscSegment_ring));
        tree->Branch("cscSegment_station", &(*cscSegment_station));
        tree->Branch("cscSegment_chamber", &(*cscSegment_chamber));
        tree->Branch("cscSegment_sector", &(*cscSegment_sector));
        tree->Branch("cscSegment_CSC_ID", &(*cscSegment_CSC_ID));

        tree->Branch("cscSegment_nRecHits", &(*cscSegment_nRecHits));

        tree->Branch("cscSegment_wire_min", &(*cscSegment_wire_min));
        tree->Branch("cscSegment_wire_max", &(*cscSegment_wire_max));
        tree->Branch("cscSegment_strip_min", &(*cscSegment_strip_min));
        tree->Branch("cscSegment_strip_max", &(*cscSegment_strip_max));

        tree->Branch("cscSegment_match_iHit", &(*cscSegment_match_iHit));
        tree->Branch("cscSegment_match_iHit2", &(*cscSegment_match_iHit2));
        tree->Branch("cscSegment_match_nHits", &(*cscSegment_match_nHits));
        tree->Branch("cscSegment_match_hit_unique", &(*cscSegment_match_hit_unique));

        tree->Branch("cscSegment_size", &(*cscSegment_size));
        tree->Branch("cscSegment_nBx0", &(*cscSegment_nBx0));
    }

    // Reco muons
    if (useRecoMuons_) {
        tree->Branch("recoMuon_size", &(*recoMuon_size));
        tree->Branch("recoMuon_e", &(*recoMuon_e));
        tree->Branch("recoMuon_et", &(*recoMuon_et));
        tree->Branch("recoMuon_pt", &(*recoMuon_pt));
        tree->Branch("recoMuon_eta", &(*recoMuon_eta));
        tree->Branch("recoMuon_phi", &(*recoMuon_phi));
        tree->Branch("recoMuon_dxy", &(*recoMuon_dxy));
        tree->Branch("recoMuon_dz", &(*recoMuon_dz));
        tree->Branch("recoMuon_isLooseMuon", &(*recoMuon_isLooseMuon));
        tree->Branch("recoMuon_isMediumMuon", &(*recoMuon_isMediumMuon));
        tree->Branch("recoMuon_isTightMuon", &(*recoMuon_isTightMuon));
        tree->Branch("recoMuon_iso", &(*recoMuon_iso));
        tree->Branch("recoMuon_hlt_isomu", &(*recoMuon_hlt_isomu));
        tree->Branch("recoMuon_hlt_mu", &(*recoMuon_hlt_mu));
        tree->Branch("recoMuon_hlt_isoDeltaR", &(*recoMuon_hlt_isoDeltaR));
        tree->Branch("recoMuon_hlt_deltaR", &(*recoMuon_hlt_deltaR));
        tree->Branch("recoMuon_passesSingleMuon", &(*recoMuon_passesSingleMuon));
        tree->Branch("recoMuon_charge", &(*recoMuon_charge));
        tree->Branch("recoMuon_etaSt1", &(*recoMuon_etaSt1));
        tree->Branch("recoMuon_phiSt1", &(*recoMuon_phiSt1));
        tree->Branch("recoMuon_etaSt2", &(*recoMuon_etaSt2));
        tree->Branch("recoMuon_phiSt2", &(*recoMuon_phiSt2));
    }
}

void MuonAODAnalyzer::fillTree() {

    if (verbose_ > 0)
        std::cout << "******* Filling Output Tree and Clearing Objects *******"
                  << std::endl;

    tree->Fill();

    // Clear objects
    // CSC Inputs
    cscInput_endcap->clear();
    cscInput_station->clear();
    cscInput_ring->clear();
    cscInput_sector->clear();
    cscInput_chamber->clear();
    cscInput_cscid->clear();
    cscInput_bx->clear();
    cscInput_strip->clear();
    cscInput_wire->clear();
    cscInput_quality->clear();
    cscInput_pattern->clear();
    cscInput_bend->clear();

    // RPC Inputs
    rpcInput_region->clear();
    rpcInput_station->clear();
    rpcInput_ring->clear();
    rpcInput_sector->clear();
    rpcInput_subsector->clear();
    rpcInput_roll->clear();
    rpcInput_bx->clear();
    rpcInput_strip->clear();
    rpcInput_strip_high->clear();
    rpcInput_strip_low->clear();
    rpcInput_time->clear();
    rpcInput_valid->clear();

    // GEM Inputs
    gemInput_region->clear();
    gemInput_station->clear();
    gemInput_ring->clear();
    gemInput_sector->clear();
    gemInput_chamber->clear();
    gemInput_roll->clear();
    gemInput_bx->clear();
    gemInput_layer->clear();
    gemInput_pad->clear();
    gemInput_pad_low->clear();
    gemInput_pad_high->clear();

    // ME0 Inputs
    me0Input_region->clear();
    me0Input_station->clear();
    me0Input_ring->clear();
    me0Input_sector->clear();
    me0Input_chamber->clear();
    me0Input_roll->clear();
    me0Input_bx->clear();
    me0Input_layer->clear();
    me0Input_phiposition->clear();
    me0Input_deltaphi->clear();
    me0Input_quality->clear();
    me0Input_bend->clear();
    me0Input_partition->clear();

    // DT Inputs
    dtInput_wheel->clear();
    dtInput_station->clear();
    dtInput_btigroup->clear();
    dtInput_bx->clear();
    dtInput_strip->clear();
    dtInput_wire->clear();
    dtInput_quality->clear();
    dtInput_bend->clear();

    // EMTF Hits
    emtfHit_endcap->clear();
    emtfHit_station->clear();
    emtfHit_ring->clear();
    emtfHit_sector->clear();
    emtfHit_subsector->clear();
    emtfHit_chamber->clear();
    emtfHit_cscid->clear();
    emtfHit_bx->clear();
    emtfHit_type->clear();
    emtfHit_neighbor->clear();
    //
    emtfHit_strip->clear();
    emtfHit_strip_qses->clear();
    emtfHit_wire->clear();
    emtfHit_roll->clear();
    emtfHit_layer->clear();
    emtfHit_quality->clear();
    emtfHit_pattern->clear();
    emtfHit_bend->clear();
    emtfHit_slope->clear();
    emtfHit_time->clear();
    emtfHit_fr->clear();
    emtfHit_emtf_phi->clear();
    emtfHit_emtf_theta->clear();
    //
    emtfHit_sim_phi->clear();
    emtfHit_sim_theta->clear();
    emtfHit_sim_r->clear();
    emtfHit_sim_z->clear();
    // emtfHit_sim_tp1            ->clear();
    // emtfHit_sim_tp2            ->clear();
    //
    emtfHit_match_iSeg->clear();
    emtfHit_match_iSeg2->clear();
    emtfHit_match_nSegs->clear();
    emtfHit_match_seg_unique->clear();

    (*emtfHit_size) = 0;

    // EMTF Unpacked Hits
    emtfUnpHit_endcap->clear();
    emtfUnpHit_station->clear();
    emtfUnpHit_ring->clear();
    emtfUnpHit_sector->clear();
    emtfUnpHit_subsector->clear();
    emtfUnpHit_chamber->clear();
    emtfUnpHit_cscid->clear();
    emtfUnpHit_bx->clear();
    emtfUnpHit_type->clear();
    emtfUnpHit_neighbor->clear();
    //
    emtfUnpHit_strip->clear();
    emtfUnpHit_strip_qses->clear();
    emtfUnpHit_wire->clear();
    emtfUnpHit_roll->clear();
    emtfUnpHit_quality->clear();
    emtfUnpHit_pattern->clear();
    emtfUnpHit_bend->clear();
    emtfUnpHit_time->clear();
    emtfUnpHit_fr->clear();
    emtfUnpHit_emtf_phi->clear();
    emtfUnpHit_emtf_theta->clear();
    //
    emtfUnpHit_sim_phi->clear();
    emtfUnpHit_sim_theta->clear();
    emtfUnpHit_sim_r->clear();
    emtfUnpHit_sim_z->clear();
    // emtfUnpHit_sim_tp1         ->clear();
    // emtfUnpHit_sim_tp2         ->clear();
    (*emtfUnpHit_size) = 0;

    // EMTF Tracks
    emtfTrack_pt->clear();
    emtfTrack_xml_pt->clear();
    emtfTrack_pt_dxy->clear();
    emtfTrack_dxy->clear();
    emtfTrack_invpt_prompt->clear();
    emtfTrack_invpt_displ->clear();
    emtfTrack_phi->clear();
    emtfTrack_phi_fp->clear();
    emtfTrack_theta->clear();
    emtfTrack_theta_fp->clear();
    emtfTrack_eta->clear();
    emtfTrack_GMT_phi->clear();
    emtfTrack_GMT_eta->clear();
    emtfTrack_GMT_dxy->clear();
    emtfTrack_q->clear();
    //
    emtfTrack_mode->clear();
    emtfTrack_endcap->clear();
    emtfTrack_sector->clear();
    emtfTrack_bx->clear();
    emtfTrack_nhits->clear();
    emtfTrack_hitref1->clear();
    emtfTrack_hitref2->clear();
    emtfTrack_hitref3->clear();
    emtfTrack_hitref4->clear();
    (*emtfTrack_size) = 0;

    emtfTrack_straightness->clear();
    // PtLUT information
    emtfTrack_ptLUT_address->clear();
    emtfTrack_ptLUT_mode->clear();
    emtfTrack_ptLUT_theta->clear();
    emtfTrack_ptLUT_st1_ring2->clear();
    emtfTrack_ptLUT_eta->clear();
    emtfTrack_ptLUT_deltaPh->clear();
    emtfTrack_ptLUT_deltaTh->clear();
    emtfTrack_ptLUT_signPh->clear();
    emtfTrack_ptLUT_signTh->clear();
    emtfTrack_ptLUT_cpattern->clear();
    emtfTrack_ptLUT_fr->clear();
    emtfTrack_ptLUT_bt_vi->clear();
    emtfTrack_ptLUT_bt_hi->clear();
    emtfTrack_ptLUT_bt_ci->clear();
    emtfTrack_ptLUT_bt_si->clear();

    // EMTF Unpacked Tracks
    emtfUnpTrack_pt->clear();
    emtfUnpTrack_xml_pt->clear();
    emtfUnpTrack_pt_dxy->clear();
    emtfUnpTrack_dxy->clear();
    emtfUnpTrack_invpt_prompt->clear();
    emtfUnpTrack_invpt_displ->clear();
    emtfUnpTrack_phi->clear();
    emtfUnpTrack_theta->clear();
    emtfUnpTrack_eta->clear();
    emtfUnpTrack_GMT_dxy->clear();
    emtfUnpTrack_q->clear();
    //
    emtfUnpTrack_mode->clear();
    emtfUnpTrack_endcap->clear();
    emtfUnpTrack_sector->clear();
    emtfUnpTrack_bx->clear();
    emtfUnpTrack_nhits->clear();
    emtfUnpTrack_hitref1->clear();
    emtfUnpTrack_hitref2->clear();
    emtfUnpTrack_hitref3->clear();
    emtfUnpTrack_hitref4->clear();
    (*emtfUnpTrack_size) = 0;
    // PtLUT information
    emtfUnpTrack_ptLUT_address->clear();
    emtfUnpTrack_ptLUT_mode->clear();
    emtfUnpTrack_ptLUT_theta->clear();
    emtfUnpTrack_ptLUT_st1_ring2->clear();
    emtfUnpTrack_ptLUT_eta->clear();
    emtfUnpTrack_ptLUT_deltaPh->clear();
    emtfUnpTrack_ptLUT_deltaTh->clear();
    emtfUnpTrack_ptLUT_signPh->clear();
    emtfUnpTrack_ptLUT_signTh->clear();
    emtfUnpTrack_ptLUT_cpattern->clear();
    emtfUnpTrack_ptLUT_fr->clear();
    emtfUnpTrack_ptLUT_bt_vi->clear();
    emtfUnpTrack_ptLUT_bt_hi->clear();
    emtfUnpTrack_ptLUT_bt_ci->clear();
    emtfUnpTrack_ptLUT_bt_si->clear();

    // GMT Muons
    gmtMuon_pt->clear();
    gmtMuon_pt_dxy->clear();
    gmtMuon_dxy->clear();
    gmtMuon_phi->clear();
    gmtMuon_eta->clear();
    gmtMuon_q->clear();
    gmtMuon_qual->clear();
    (*gmtMuon_size) = 0;

    // GMT Unpacked Muons
    gmtUnpMuon_pt->clear();
    gmtUnpMuon_pt_dxy->clear();
    gmtUnpMuon_dxy->clear();
    gmtUnpMuon_phi->clear();
    gmtUnpMuon_eta->clear();
    gmtUnpMuon_q->clear();
    gmtUnpMuon_qual->clear();
    (*gmtUnpMuon_size) = 0;

    // GEN particles
    genPart_pt->clear();
    genPart_dxy->clear();
    genPart_eta->clear();
    genPart_phi->clear();
    genPart_q->clear();
    genPart_ID->clear();
    genPart_parentID->clear();
    genPart_vx->clear();
    genPart_vy->clear();
    genPart_vz->clear();
    genPart_etaSt1->clear();
    genPart_phiSt1->clear();
    genPart_etaSt2->clear();
    genPart_phiSt2->clear();

    // Event info
    eventInfo_event = 0;
    eventInfo_run = 0;
    eventInfo_lumi = 0;
    eventInfo_npv = 0;
    eventInfo_nvtx = 0;

    // CSC Segment info
    cscSegment_locX->clear();
    cscSegment_locY->clear();
    cscSegment_globX->clear();
    cscSegment_globY->clear();
    cscSegment_globZ->clear();
    cscSegment_eta->clear();
    cscSegment_phi->clear();
    cscSegment_theta->clear();
    cscSegment_chi2->clear();
    cscSegment_time->clear();
    cscSegment_dirX->clear();
    cscSegment_dirY->clear();
    cscSegment_dirZ->clear();
    cscSegment_bend_theta->clear();
    cscSegment_bend_phi->clear();

    cscSegment_endcap->clear();
    cscSegment_ring->clear();
    cscSegment_station->clear();
    cscSegment_chamber->clear();
    cscSegment_sector->clear();
    cscSegment_CSC_ID->clear();

    cscSegment_nRecHits->clear();

    cscSegment_wire_min->clear();
    cscSegment_wire_max->clear();
    cscSegment_strip_min->clear();
    cscSegment_strip_max->clear();

    cscSegment_match_iHit->clear();
    cscSegment_match_iHit2->clear();
    cscSegment_match_nHits->clear();
    cscSegment_match_hit_unique->clear();

    (*cscSegment_size) = 0;
    (*cscSegment_nBx0) = 0;

    (*recoMuon_size) = 0;
    recoMuon_e->clear();
    recoMuon_et->clear();
    recoMuon_pt->clear();
    recoMuon_eta->clear();
    recoMuon_phi->clear();
    recoMuon_dxy->clear();
    recoMuon_dz->clear();
    recoMuon_isLooseMuon->clear();
    recoMuon_isMediumMuon->clear();
    recoMuon_isTightMuon->clear();
    recoMuon_iso->clear();
    recoMuon_hlt_isomu->clear();
    recoMuon_hlt_mu->clear();
    recoMuon_hlt_isoDeltaR->clear();
    recoMuon_hlt_deltaR->clear();
    recoMuon_passesSingleMuon->clear();
    recoMuon_charge->clear();
    recoMuon_etaSt1->clear();
    recoMuon_phiSt1->clear();
    recoMuon_etaSt2->clear();
    recoMuon_phiSt2->clear();
}

void MuonAODAnalyzer::printDebugOutput() {

    // if(useEMTFUnpHits_ && useEMTFUnpTracks_){
    if(useEMTFUnpTracks_){
        std::cout << "******* Printing Out EMTF Unpacked Hits and Tracks *******" << std::endl;
        auto out_hits = EMTFUnpHits_;

        // Calculate neighbor hits
        for (int endcap_ = -1; endcap_ < 2; endcap_ += 2){
            for (int sector_ = 1; sector_ <= 6; sector_++){
                for (const auto &hit : EMTFUnpHits_) {
                    bool isNeighbor = false;
                    int tp_endcap = hit.Endcap();
                    int tp_sector = hit.PC_sector();
                    int tp_station = (hit.PC_station() == 0 && hit.Subsector() == 1) ? 1 : hit.PC_station();
                    int tp_ring = hit.Ring();
                    int tp_subsector = hit.Subsector();
                    int tp_csc_ID = hit.PC_chamber() + 1;
                    auto get_neighbor = [](int sector) { return (sector == 1) ? 6 : sector - 1; };
                    auto hit_neighbor = hit;

                    if (hit.Subsystem() == L1TMuon::kCSC or hit.Subsystem() == L1TMuon::kGEM){
                        if ((endcap_ == tp_endcap) && (get_neighbor(sector_) == tp_sector)) {
                            if (tp_station == 1) {
                                if ((tp_subsector == 2) && (tp_csc_ID == 3 || tp_csc_ID == 6 || tp_csc_ID == 9))
                                isNeighbor = true;

                            } else {
                                if (tp_csc_ID == 3 || tp_csc_ID == 9)
                                isNeighbor = true;
                            }
                        }
                        if (isNeighbor){
                            hit_neighbor.set_sector(sector_);
                            hit_neighbor.set_pc_sector(sector_);
                            hit_neighbor.set_station(5);
                            hit_neighbor.set_pc_station(5);
                            hit_neighbor.set_neighbor(1);
                            hit_neighbor.set_sector_idx((endcap_ == 1) ? sector_ - 1 : sector_ + 5);
                        }
                    } else if (hit.Subsystem() == L1TMuon::kRPC){
                        auto get_neighbor_subsector = [](int tp_station, int tp_ring) {
                            const bool is_irpc = (tp_station == 3 || tp_station == 4) && (tp_ring == 1);
                            if (is_irpc) {
                                // 20 degree chamber
                                return 1;
                            } else {
                                // 10 degree chamber
                                return 2;
                            }
                        };
                        isNeighbor = ((endcap_ == tp_endcap) && (sector_ == tp_sector) &&
                                (tp_subsector == get_neighbor_subsector(tp_station, tp_ring)));

                        if (isNeighbor){
                            hit_neighbor.set_sector(sector_);
                            hit_neighbor.set_pc_sector(sector_);
                            hit_neighbor.set_neighbor(1);
                            hit_neighbor.set_sector_idx((endcap_ == 1) ? sector_ - 1 : sector_ + 5);
                        }

                    }
                    if (isNeighbor)
                        out_hits.push_back(hit_neighbor);
                }
            }
        }

        emtf::dump_fw_raw_input(out_hits, EMTFUnpTracks_);

    }

    // if(useEMTFHits_ && useEMTFTracks_){
    if(useEMTFTracks_){
        std::cout << "******* Printing Out EMTF Re-Emulated Hits and Tracks *******" << std::endl;
        emtf::dump_fw_raw_input(EMTFHits_, EMTFTracks_);
    }

}

// -------------------------------------------------------
// define this as a plug-in
DEFINE_FWK_MODULE(MuonAODAnalyzer);