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

    // Reco muons
    if (useRecoMuons_ && RecoMuons_ != nullptr) {
        for (const auto &muon : *RecoMuons_) {
            muon_e->push_back(muon.energy());
            muon_et->push_back(muon.et());
            muon_pt->push_back(muon.pt());
            muon_eta->push_back(muon.eta());
            muon_phi->push_back(muon.phi());
            muon_charge->push_back(muon.charge());

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
                std::cout<<"Trigger matching started for a muon"<<std::endl;
                double isoMatchDeltaR = 9999.;
                double matchDeltaR = 9999.;
                int hasIsoTriggered = 0;
                int hasTriggered = 0;
                int passesSingleMuonFlag = 0;

                // first check if the trigger results are valid:
                if (TriggerResults_ != nullptr) {
                    std::cout<<"Trigger results are valid"<<TriggerResults_<<std::endl;
                    if (TriggerSummaryLabels_ != nullptr) {
                        const edm::TriggerNames& trigNames = iEvent.triggerNames(*TriggerResults_);
                        std::cout<<"TriggerSummaryLabels_ are valid"<<TriggerSummaryLabels_<<std::endl;
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
  std::cout<<"Trigger matching started"<<std::endl;
  for (size_t iTrigIndex = 0; iTrigIndex < trigIndices.size(); ++iTrigIndex) {
    int triggerIndex = trigIndices[iTrigIndex];
    std::cout<<"Inside for loop, triggerIndex = "<<triggerIndex<<std::endl;
    if (triggerIndex >= (int)hltConfig_.size()) continue; // Safety check
    std::cout<<"Safety check"<<std::endl;
    const std::vector<std::string> moduleLabels(hltConfig_.moduleLabels(triggerIndex));
    // find index of the last module:
    const unsigned moduleIndex = hltConfig_.size(triggerIndex) - 2;
    // find index of HLT trigger name:
    const unsigned hltFilterIndex =
        triggerEvent.filterIndex(edm::InputTag(moduleLabels[moduleIndex], "", triggerProcessLabel_));

    if (hltFilterIndex < triggerEvent.sizeFilters()) {
      const trigger::Keys triggerKeys(triggerEvent.filterKeys(hltFilterIndex));
      const trigger::Vids triggerVids(triggerEvent.filterIds(hltFilterIndex));
      std::cout<<"Passed the hltfilterindex"<<std::endl;
      const unsigned nTriggers = triggerVids.size();
      for (size_t iTrig = 0; iTrig < nTriggers; ++iTrig) {
        std::cout<<"Inside the second loop, iTrig = "<<iTrig<<"Number of Triggers ="<<nTriggers<<std::endl;
        // loop over all trigger objects:
        const trigger::TriggerObject trigObject = trigObjs[triggerKeys[iTrig]];

        double dRtmp = deltaR(mu, trigObject);
        std::cout<<"Delta R calculated: "<<dRtmp<<std::endl;
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

    // Clear flags
    (*HLT_IsoMu24) = false;
    (*HLT_Mu50_L1SingleMuShower) = false;
    (*HLT_Mu50) = false;
}

void MuonAODAnalyzer::endRun(const edm::Run &run, const edm::EventSetup &eventSetup) {
}

// define this as a plug-in
DEFINE_FWK_MODULE(MuonAODAnalyzer);