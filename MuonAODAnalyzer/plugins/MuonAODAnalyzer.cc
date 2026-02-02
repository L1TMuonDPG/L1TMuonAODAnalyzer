// -*- C++ -*-
//
// Package:    MuonAODAnalyzer/MuonAODAnalyzer
// Class:      MuonAODAnalyzer
//
//
// Original Author:  Efe Yigitbasi
//         Created:  Sat, 10 Sep 2022 11:08:53 GMT
//
//

#include "L1TMuonAODAnalyzer/MuonAODAnalyzer/plugins/MuonAODAnalyzer.h"

MuonAODAnalyzer::MuonAODAnalyzer(const edm::ParameterSet &iConfig)
    : RecoMuonTag_(iConfig.getParameter<edm::InputTag>("RecoMuonTag")),
    //   TriggerResultsToken_(consumes<TriggerResults>(iConfig.getParameter<edm::InputTag>("Triggers"))),
      outFileName_(iConfig.getParameter<std::string>("outFileName")),
      verbose_(iConfig.getUntrackedParameter<int>("verbosity")),
      useRecoMuons_(iConfig.getParameter<bool>("useRecoMuons")),
      useEventInfo_(iConfig.getParameter<bool>("useEventInfo")),
      debug_(iConfig.getParameter<bool>("debug")),
      // trig matching
      isoTriggerNames_(iConfig.getParameter<std::vector<std::string>>("isoTriggerNames")),
      triggerNames_(iConfig.getParameter<std::vector<std::string>>("triggerNames")),
      theBFieldToken_(esConsumes<MagneticField, IdealMagneticFieldRecord>(edm::ESInputTag("", ""))),
      muPropagatorSetup1st_(iConfig.getParameter<edm::ParameterSet>("muProp1st"), consumesCollector()),
      muPropagatorSetup2nd_(iConfig.getParameter<edm::ParameterSet>("muProp2nd"), consumesCollector())
{
    usesResource("TFileService"); // shared resources

    firstEvent_ = true;

    // reco muons
    RecoMuonToken_ = consumes<reco::MuonCollection>(RecoMuonTag_);
    l1MuonToken_ = consumes<l1t::MuonBxCollection>(edm::InputTag("gmtStage2Digis","Muon"));
    TriggerResultsToken_ = consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("Triggers"));
    // TriggerResultsToken_ = consumes<edm::TriggerResults>(edm::InputTag("TriggerResults", "", "HLTX"));
    TriggerSummaryLabelsToken_ = consumes<trigger::TriggerEvent>(edm::InputTag("hltTriggerSummaryAOD", "", "HLT"));
    VerticesToken_ = consumes<reco::VertexCollection>(edm::InputTag("offlinePrimaryVertices"));

    triggerMatching_ = true;
    triggerMaxDeltaR_ = 0.1;
    triggerProcessLabel_ = "HLT";
}

MuonAODAnalyzer::~MuonAODAnalyzer() {
    // Do nothing
}

// ------------ method called for each event  ------------
void MuonAODAnalyzer::analyze(const edm::Event &iEvent,
                         const edm::EventSetup &iSetup) {
    // ___________________________________________________________________________
    // Get Handles
    getHandles(iEvent, iSetup);
    
    muPropagator1st_ = muPropagatorSetup1st_.init(iSetup);
    muPropagator2nd_ = muPropagatorSetup2nd_.init(iSetup);
    
    if (verbose_ > 0)
        std::cout << "******* Processing Objects *******" << std::endl;

    // ___________________________________________________________________________
    // Process objects

    // L1 muons
    edm::Handle<l1t::MuonBxCollection> l1muoncoll;
    iEvent.getByToken(l1MuonToken_ , l1muoncoll);
    for(int i = l1muoncoll->getFirstBX() ; i<= l1muoncoll->getLastBX() ;i++){
        for( l1t::MuonBxCollection::const_iterator l1muonit= l1muoncoll->begin(i); l1muonit != l1muoncoll->end(i) ; ++l1muonit){
        if(l1muonit->pt() < 0) continue;
        l1mu_qual.push_back( l1muonit->hwQual() );
        l1mu_charge.push_back( l1muonit->charge() );
        l1mu_pt.push_back( l1muonit->pt() );
        l1mu_pt_dxy.push_back( l1muonit->ptUnconstrained() );
        l1mu_dxy.push_back( l1muonit->hwDXY() );
        l1mu_eta.push_back( l1muonit->eta() );
        l1mu_etaAtVtx.push_back( l1muonit->etaAtVtx() );
        l1mu_phi.push_back( l1muonit->phi() );
        l1mu_phiAtVtx.push_back( l1muonit->phiAtVtx() );
        l1mu_tfIdx.push_back(l1muonit->tfMuonIndex());

        l1mu_bx.push_back( i);
        l1mu_size++;

        }
    }

    // Reco muons
    if (useRecoMuons_ && RecoMuons_ != nullptr) {
        for (const auto &muon : *RecoMuons_) {
            muon_e->push_back(muon.energy());
            muon_et->push_back(muon.et());
            muon_pt->push_back(muon.pt());
            muon_eta->push_back(muon.eta());
            muon_phi->push_back(muon.phi());
            muon_charge->push_back(muon.charge());
            muon_isSAMuon->push_back(muon.isStandAloneMuon());
            muon_isTrackerMuon->push_back(muon.isTrackerMuon());
            muon_isGlobalMuon->push_back(muon.isGlobalMuon());
            muon_isPFMuon->push_back(muon.isPFMuon());
            muon_vx->push_back(muon.vx());
            muon_vy->push_back(muon.vy());
            muon_vz->push_back(muon.vz());
            muon_px->push_back(muon.px());
            muon_py->push_back(muon.py());
            muon_pz->push_back(muon.pz());
            muon_nChambers->push_back(muon.numberOfChambers() );
            muon_nChambersCSCorDT->push_back(muon.numberOfChambersCSCorDT() );
            muon_nMatches->push_back(muon.numberOfMatches() );
            muon_nMatchedStations->push_back(muon.numberOfMatchedStations() );
            muon_expectedNumberOfMatchedStations->push_back(muon.expectedNnumberOfMatchedStations() );
            muon_stationMask->push_back(muon.stationMask() );
            muon_nMatchedRPCLayers->push_back(muon.numberOfMatchedRPCLayers() );
            muon_RPClayerMask->push_back(muon.RPClayerMask() );

            if (Vertices_ != nullptr && !Vertices_->empty()){
                if( !(muon.muonBestTrack().isNull())){
                    muon_dz->push_back( muon.muonBestTrack()->dz((*Vertices_)[0].position()));
                    muon_dxy->push_back( muon.muonBestTrack()->dxy((*Vertices_)[0].position()));
                }
            }

            bool isLoose = (muon.isPFMuon() && (muon.isGlobalMuon() || muon.isTrackerMuon()));
            bool goodGlob = muon.isGlobalMuon() && muon.globalTrack()->normalizedChi2() < 3 &&
                  muon.combinedQuality().chi2LocalPosition < 12 && muon.combinedQuality().trkKink < 20;
            bool isMedium = isLoose && muon.innerTrack()->validFraction() > 0.49 &&
                  muon::segmentCompatibility(muon) > (goodGlob ? 0.303 : 0.451);
            bool isTight = false;
            
            if (Vertices_ != nullptr && !Vertices_->empty()){
                isTight = muon.isGlobalMuon() && muon.isPFMuon() && muon.globalTrack()->normalizedChi2() < 10. &&
                    muon.globalTrack()->hitPattern().numberOfValidMuonHits() > 0 &&
                    muon.numberOfMatchedStations() > 1 &&
                    std::abs(muon.muonBestTrack()->dxy(((*Vertices_)[0]).position())) < 0.2 &&
                    std::abs(muon.muonBestTrack()->dz(((*Vertices_)[0]).position())) < 0.5 &&
                    muon.innerTrack()->hitPattern().numberOfValidPixelHits() > 0 &&
                    muon.innerTrack()->hitPattern().trackerLayersWithMeasurement() > 5 &&
                    muon.globalTrack()->normalizedChi2() < 1;
            }
            muon_isLooseMuon->push_back(isLoose);
            muon_isMediumMuon->push_back(isMedium);
            muon_isTightMuon->push_back(isTight);

            double iso = (muon.pfIsolationR04().sumChargedHadronPt +
                std::max(0.,
                    muon.pfIsolationR04().sumNeutralHadronEt + muon.pfIsolationR04().sumPhotonEt -
                        0.5 * muon.pfIsolationR04().sumPUPt)) /
                muon.pt();
            muon_iso->push_back(iso);

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

                    } 
                } 

                muon_hlt_isomu->push_back(hasIsoTriggered);
                muon_hlt_mu->push_back(hasTriggered);
                muon_hlt_isoDeltaR->push_back(isoMatchDeltaR);
                muon_hlt_deltaR->push_back(matchDeltaR);
                muon_passesSingleMuon->push_back(passesSingleMuonFlag);
            } else {
                muon_hlt_isomu->push_back(-999);
                muon_hlt_mu->push_back(-999);
                muon_hlt_isoDeltaR->push_back(-999);
                muon_hlt_deltaR->push_back(-999);
                muon_passesSingleMuon->push_back(-999);
            }

            // extrapolation of track coordinates
            TrajectoryStateOnSurface stateAtMuSt1 = muPropagator1st_.extrapolate(muon);
            if (stateAtMuSt1.isValid()) {
                muon_etaSt1->push_back(stateAtMuSt1.globalPosition().eta());
                muon_phiSt1->push_back(stateAtMuSt1.globalPosition().phi());
            } else {
                muon_etaSt1->push_back(-9999);
                muon_phiSt1->push_back(-9999);
            }

            TrajectoryStateOnSurface stateAtMuSt2 = muPropagator2nd_.extrapolate(muon);
            if (stateAtMuSt2.isValid()) {
                muon_etaSt2->push_back(stateAtMuSt2.globalPosition().eta());
                muon_phiSt2->push_back(stateAtMuSt2.globalPosition().phi());
            } else {
                muon_etaSt2->push_back(-9999);
                muon_phiSt2->push_back(-9999);
            }

        }
        (*muon_size) = RecoMuons_->size();
    }

    // Trigger Results - specific flags
    if (TriggerResults_ != nullptr) {
        const edm::TriggerNames& trigNames = iEvent.triggerNames(*TriggerResults_);
        
        std::vector<std::string> ddmTriggerNames = {
            "HLT_IsoMu24_v25",
            "HLT_Mu50_L1SingleMuShower_v11",
            "HLT_Mu50_v25",
        };

        (*HLT_IsoMu24) = false;
        (*HLT_Mu50_L1SingleMuShower) = false;
        (*HLT_Mu50) = false;

        // Note: Using 'contains' logic or checking exact names would be safer in production, 
        // but keeping the logic close to original where it seems to expect specific indices/names availability
        // or one might implement a helper to find index by name
        
        // Simple check logic based on original file structure
        for(size_t i=0; i<trigNames.size(); ++i){
             if (trigNames.triggerName(i).find("HLT_IsoMu24") != std::string::npos && TriggerResults_->accept(i)) (*HLT_IsoMu24) = true;
             if (trigNames.triggerName(i).find("HLT_Mu50_L1SingleMuShower") != std::string::npos && TriggerResults_->accept(i)) (*HLT_Mu50_L1SingleMuShower) = true;
             if (trigNames.triggerName(i).find("HLT_Mu50") != std::string::npos && TriggerResults_->accept(i)) (*HLT_Mu50) = true;
        }
    }  

    // Event Info
    if (useEventInfo_) {
        eventInfo_event = iEvent.id().event();
        eventInfo_run = iEvent.id().run();
        eventInfo_lumi = iEvent.luminosityBlock();
        eventInfo_bx = iEvent.bunchCrossing();
        
        if (Vertices_ != nullptr) {
             eventInfo_nvtx = Vertices_->size();
             // Assuming npv is number of vertices as well for simplicity, or 0 if not calculated differently
             eventInfo_npv = Vertices_->size(); 
        } else {
             eventInfo_nvtx = 0;
             eventInfo_npv = 0;
        }
    }

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
    if (triggerIndex >= (int)hltConfig_.size()) continue; // Safety check
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
    }    // if trigger is in event
  }      // loop over trigger indices

  return matchDeltaR;
}

// ------------ method called once each job just before starting event loop
void MuonAODAnalyzer::beginJob() {
    makeTree();
}

// ------------ method called once each run just before starting event loop
void MuonAODAnalyzer::beginRun(const edm::Run &run, const edm::EventSetup &eventSetup) {
  // Prepare for trigger matching for each new run
  if (triggerMatching_) {
    bool changed = true;
    if (!hltConfig_.init(run, eventSetup, triggerProcessLabel_, changed)) {
      std::cout << "Error: didn't find process" << triggerProcessLabel_ << std::endl;
      return; 
    }

    bool enableWildcard = true;
    triggerIndices_.clear();
    isoTriggerIndices_.clear();

    for (size_t iTrig = 0; iTrig < triggerNames_.size(); ++iTrig) {
      TString tNameTmp = TString(triggerNames_[iTrig]);
      TRegexp tNamePattern = TRegexp(tNameTmp, enableWildcard);
      int tIndex = -1;
      for (unsigned ipath = 0; ipath < hltConfig_.size(); ++ipath) {
        TString tmpName = TString(hltConfig_.triggerName(ipath));
        if (tmpName.Contains(tNamePattern)) {
          tIndex = int(ipath);
          triggerIndices_.push_back(tIndex);
        }
      }
    } 

    for (size_t iTrig = 0; iTrig < isoTriggerNames_.size(); ++iTrig) {
      TString tNameTmp = TString(isoTriggerNames_[iTrig]);
      TRegexp tNamePattern = TRegexp(tNameTmp, enableWildcard);
      int tIndex = -1;
      for (unsigned ipath = 0; ipath < hltConfig_.size(); ++ipath) {
        TString tmpName = TString(hltConfig_.triggerName(ipath));
        if (tmpName.Contains(tNamePattern)) {
          tIndex = int(ipath);
          isoTriggerIndices_.push_back(tIndex);
        }
      }
    } 
  }
}

// ------------ method called once each job just after ending the event loop
void MuonAODAnalyzer::endJob() {
}

// ------------ method fills 'descriptions' 
void MuonAODAnalyzer::fillDescriptions(edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.setUnknown();
    descriptions.addDefault(desc);
}

void MuonAODAnalyzer::getHandles(const edm::Event &iEvent,
                            const edm::EventSetup &iSetup) {

    if (verbose_ > 0)
        std::cout << "******* Getting Handles *******" << std::endl;

    // reco muons
    auto muon_handle = make_handle(RecoMuons_);
    auto Vertices_handle = make_handle(Vertices_);
    auto TriggerResults_handle = make_handle(TriggerResults_);
    auto TriggerSummaryLabels_handle = make_handle(TriggerSummaryLabels_);

    if (useRecoMuons_) {
        if (!RecoMuonToken_.isUninitialized()) {
            iEvent.getByToken(RecoMuonToken_, muon_handle);
        }
        if (!muon_handle.isValid()) {
            if (firstEvent_)
                edm::LogError("NtupleMaker") << "Cannot get the product: " << RecoMuonTag_;
            RecoMuons_ = nullptr;
        } else {
            RecoMuons_ = muon_handle.product();
        }
        
        if (!VerticesToken_.isUninitialized()) {
            iEvent.getByToken(VerticesToken_, Vertices_handle);
        }
        if (!Vertices_handle.isValid()) {
            Vertices_ = nullptr;
        } else {
            Vertices_ = Vertices_handle.product();
        }
    } else {
        RecoMuons_ = nullptr;
        Vertices_ = nullptr;
    }
    
    // Trigger handles
    if (!TriggerResultsToken_.isUninitialized()) {
        iEvent.getByToken(TriggerResultsToken_, TriggerResults_handle);
    }
    if (!TriggerResults_handle.isValid()) {
        TriggerResults_ = nullptr;
    } else {
        TriggerResults_ = TriggerResults_handle.product();
    }
    
    if (!TriggerSummaryLabelsToken_.isUninitialized()) {
        iEvent.getByToken(TriggerSummaryLabelsToken_, TriggerSummaryLabels_handle);
    }
    if (!TriggerSummaryLabels_handle.isValid()) {
        TriggerSummaryLabels_ = nullptr;
    } else {
        TriggerSummaryLabels_ = TriggerSummaryLabels_handle.product();
    }

    firstEvent_ = false;
}

void MuonAODAnalyzer::makeTree() {
    if (verbose_ > 0)
        std::cout << "******* Making Output Tree *******" << std::endl;

    edm::Service<TFileService> fs;
    tree = fs->make<TTree>("tree", "tree");

    // Reco muon info pointers
    muon_size = std::make_unique<int32_t>(0);
    muon_e = std::make_unique<std::vector<float>>();
    muon_et = std::make_unique<std::vector<float>>();
    muon_pt = std::make_unique<std::vector<float>>();
    muon_eta = std::make_unique<std::vector<float>>();
    muon_phi = std::make_unique<std::vector<float>>();
    muon_dxy = std::make_unique<std::vector<float>>();
    muon_dz = std::make_unique<std::vector<float>>();
    muon_isLooseMuon = std::make_unique<std::vector<bool>>();
    muon_isMediumMuon = std::make_unique<std::vector<bool>>();
    muon_isTightMuon = std::make_unique<std::vector<bool>>();
    muon_iso = std::make_unique<std::vector<float>>();
    muon_hlt_isomu = std::make_unique<std::vector<short>>();
    muon_hlt_mu = std::make_unique<std::vector<short>>();
    muon_hlt_isoDeltaR = std::make_unique<std::vector<float>>();
    muon_hlt_deltaR = std::make_unique<std::vector<float>>();
    muon_passesSingleMuon = std::make_unique<std::vector<int>>();
    muon_charge = std::make_unique<std::vector<int>>();
    muon_etaSt1 = std::make_unique<std::vector<float>>();
    muon_phiSt1 = std::make_unique<std::vector<float>>();
    muon_etaSt2 = std::make_unique<std::vector<float>>();
    muon_phiSt2 = std::make_unique<std::vector<float>>();
    muon_vx = std::make_unique<std::vector<float>>();
    muon_vy = std::make_unique<std::vector<float>>();
    muon_vz = std::make_unique<std::vector<float>>();
    muon_px = std::make_unique<std::vector<float>>();
    muon_py = std::make_unique<std::vector<float>>();
    muon_pz = std::make_unique<std::vector<float>>();
    muon_isSAMuon = std::make_unique<std::vector<bool>>();
    muon_isGlobalMuon = std::make_unique<std::vector<bool>>();
    muon_isTrackerMuon = std::make_unique<std::vector<bool>>();
    muon_isPFMuon = std::make_unique<std::vector<bool>>();
    muon_nChambers = std::make_unique<std::vector<int>>();
    muon_nChambersCSCorDT = std::make_unique<std::vector<int>>();
    muon_nMatches = std::make_unique<std::vector<int>>();
    muon_nMatchedStations = std::make_unique<std::vector<int>>();
    muon_expectedNumberOfMatchedStations = std::make_unique<std::vector<unsigned int>>();
    muon_stationMask = std::make_unique<std::vector<unsigned int>>();
    muon_nMatchedRPCLayers = std::make_unique<std::vector<int>>();
    muon_RPClayerMask = std::make_unique<std::vector<unsigned int>>();


    // Trigger flags pointers
    HLT_IsoMu24 = std::make_unique<bool>();
    HLT_Mu50_L1SingleMuShower = std::make_unique<bool>();
    HLT_Mu50 = std::make_unique<bool>();

    // Event info
    if (useEventInfo_) {
        tree->Branch("eventInfo_event", &(eventInfo_event));
        tree->Branch("eventInfo_run", &(eventInfo_run));
        tree->Branch("eventInfo_lumi", &(eventInfo_lumi));
        tree->Branch("eventInfo_npv", &(eventInfo_npv));
        tree->Branch("eventInfo_nvtx", &(eventInfo_nvtx));
        tree->Branch("eventInfo_bx", &(eventInfo_bx));
    }

    tree->Branch("l1mu_qual",&l1mu_qual);
    tree->Branch("l1mu_charge",&l1mu_charge);
    tree->Branch("l1mu_pt",&l1mu_pt);
    tree->Branch("l1mu_pt_dxy",&l1mu_pt_dxy);
    tree->Branch("l1mu_dxy",&l1mu_dxy);
    tree->Branch("l1mu_eta",&l1mu_eta);
    tree->Branch("l1mu_etaAtVtx",&l1mu_etaAtVtx);
    tree->Branch("l1mu_phi",&l1mu_phi);
    tree->Branch("l1mu_phiAtVtx",&l1mu_phiAtVtx);
    tree->Branch("l1mu_tfIdx",&l1mu_tfIdx);
    tree->Branch("l1mu_bx",&l1mu_bx);
    tree->Branch("l1mu_size", &l1mu_size, "l1mu_size/I");

    // Reco muons
    if (useRecoMuons_) {
        tree->Branch("muon_size", &(*muon_size));
        tree->Branch("muon_e", &(*muon_e));
        tree->Branch("muon_et", &(*muon_et));
        tree->Branch("muon_pt", &(*muon_pt));
        tree->Branch("muon_eta", &(*muon_eta));
        tree->Branch("muon_phi", &(*muon_phi));
        tree->Branch("muon_dxy", &(*muon_dxy));
        tree->Branch("muon_dz", &(*muon_dz));
        tree->Branch("muon_isLooseMuon", &(*muon_isLooseMuon));
        tree->Branch("muon_isMediumMuon", &(*muon_isMediumMuon));
        tree->Branch("muon_isTightMuon", &(*muon_isTightMuon));
        tree->Branch("muon_iso", &(*muon_iso));
        tree->Branch("muon_hlt_isomu", &(*muon_hlt_isomu));
        tree->Branch("muon_hlt_mu", &(*muon_hlt_mu));
        tree->Branch("muon_hlt_isoDeltaR", &(*muon_hlt_isoDeltaR));
        tree->Branch("muon_hlt_deltaR", &(*muon_hlt_deltaR));
        tree->Branch("muon_passesSingleMuon", &(*muon_passesSingleMuon));
        tree->Branch("muon_charge", &(*muon_charge));
        tree->Branch("muon_etaSt1", &(*muon_etaSt1));
        tree->Branch("muon_phiSt1", &(*muon_phiSt1));
        tree->Branch("muon_etaSt2", &(*muon_etaSt2));
        tree->Branch("muon_phiSt2", &(*muon_phiSt2));
        tree->Branch("muon_vx", &(*muon_vx));
        tree->Branch("muon_vy", &(*muon_vy));
        tree->Branch("muon_vz", &(*muon_vz));
        tree->Branch("muon_px", &(*muon_px));
        tree->Branch("muon_py", &(*muon_py));
        tree->Branch("muon_pz", &(*muon_pz));
        tree->Branch("muon_isSAMuon", &(*muon_isSAMuon));
        tree->Branch("muon_isTrackerMuon", &(*muon_isTrackerMuon));
        tree->Branch("muon_isGlobalMuon", &(*muon_isGlobalMuon));
        tree->Branch("muon_isPFMuon", &(*muon_isPFMuon));
        tree->Branch("muon_nChambers", &(*muon_nChambers));
        tree->Branch("muon_nChambersCSCorDT", &(*muon_nChambersCSCorDT));
        tree->Branch("muon_nMatches", &(*muon_nMatches));
        tree->Branch("muon_nMatchedStations", &(*muon_nMatchedStations));
        tree->Branch("muon_expectedNumberOfMatchedStations", &(*muon_expectedNumberOfMatchedStations));
        tree->Branch("muon_stationMask", &(*muon_stationMask));
        tree->Branch("muon_nMatchedRPCLayers", &(*muon_nMatchedRPCLayers));
        tree->Branch("muon_RPClayerMask", &(*muon_RPClayerMask));
    }

    tree->Branch("HLT_IsoMu24", &(*HLT_IsoMu24));
    tree->Branch("HLT_Mu50_L1SingleMuShower", &(*HLT_Mu50_L1SingleMuShower));
    tree->Branch("HLT_Mu50", &(*HLT_Mu50));
}

void MuonAODAnalyzer::fillTree() {
    if (verbose_ > 0)
        std::cout << "******* Filling Output Tree and Clearing Objects *******" << std::endl;

    tree->Fill();

    // Event info
    eventInfo_event = 0;
    eventInfo_run = 0;
    eventInfo_lumi = 0;
    eventInfo_npv = 0;
    eventInfo_nvtx = 0;
    eventInfo_bx = 0;

    // Clear Reco Muons
    (*muon_size) = 0;
    muon_e->clear();
    muon_et->clear();
    muon_pt->clear();
    muon_eta->clear();
    muon_phi->clear();
    muon_dxy->clear();
    muon_dz->clear();
    muon_isLooseMuon->clear();
    muon_isMediumMuon->clear();
    muon_isTightMuon->clear();
    muon_iso->clear();
    muon_hlt_isomu->clear();
    muon_hlt_mu->clear();
    muon_hlt_isoDeltaR->clear();
    muon_hlt_deltaR->clear();
    muon_passesSingleMuon->clear();
    muon_charge->clear();
    muon_etaSt1->clear();
    muon_phiSt1->clear();
    muon_etaSt2->clear();
    muon_phiSt2->clear();
    muon_isSAMuon->clear() ;
    muon_isTrackerMuon->clear();
    muon_isGlobalMuon->clear();
    muon_isPFMuon->clear();
    muon_nChambers->clear();
    muon_nChambersCSCorDT->clear();
    muon_nMatches->clear();
    muon_nMatchedStations->clear();
    muon_expectedNumberOfMatchedStations->clear();
    muon_stationMask->clear();
    muon_nMatchedRPCLayers->clear();
    muon_RPClayerMask->clear();
    
    muon_vx->clear();
    muon_vy->clear();
    muon_vz->clear();
    muon_px->clear();
    muon_py->clear();
    muon_pz->clear();

    l1mu_qual.clear();
    l1mu_charge.clear();
    l1mu_pt.clear();
    l1mu_pt_dxy.clear();
    l1mu_dxy.clear();
    l1mu_eta.clear();
    l1mu_etaAtVtx.clear();
    l1mu_phi.clear();
    l1mu_phiAtVtx.clear();
    l1mu_tfIdx.clear();
    l1mu_bx.clear();
    l1mu_size = 0;

    // Clear flags
    (*HLT_IsoMu24) = false;
    (*HLT_Mu50_L1SingleMuShower) = false;
    (*HLT_Mu50) = false;
}

void MuonAODAnalyzer::endRun(const edm::Run &run, const edm::EventSetup &eventSetup) {
}

// define this as a plug-in
DEFINE_FWK_MODULE(MuonAODAnalyzer);