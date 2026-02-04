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
    DispMuonTag_(iConfig.getParameter<edm::InputTag>("DispMuonTag")),
    CosmicMuonTag_(iConfig.getParameter<edm::InputTag>("CosmicMuonTag")),
    CosmicMuon1LegTag_(iConfig.getParameter<edm::InputTag>("CosmicMuon1LegTag")),
    //   TriggerResultsToken_(consumes<TriggerResults>(iConfig.getParameter<edm::InputTag>("Triggers"))),
      outFileName_(iConfig.getParameter<std::string>("outFileName")),
      verbose_(iConfig.getUntrackedParameter<int>("verbosity")),
      useRecoMuons_(iConfig.getParameter<bool>("useRecoMuons")),
      useEventInfo_(iConfig.getParameter<bool>("useEventInfo")),
      useDispMuons_(iConfig.getParameter<bool>("useDispMuons")),
      useCosmicMuons_(iConfig.getParameter<bool>("useCosmicMuons")),
      useCosmicMuons1Leg_(iConfig.getParameter<bool>("useCosmicMuons1Leg")),
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

    DispMuonToken_ = consumes<reco::MuonCollection>(DispMuonTag_);
    CosmicMuonToken_ = consumes<reco::MuonCollection>(CosmicMuonTag_);
    CosmicMuon1LegToken_ = consumes<reco::MuonCollection>(CosmicMuon1LegTag_);

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

    // Disp muons
    if (useDispMuons_ && DispMuons_ != nullptr) {
        for (const auto &muon : *DispMuons_) {
            dispMuon_e->push_back(muon.energy());
            dispMuon_et->push_back(muon.et());
            dispMuon_pt->push_back(muon.pt());
            dispMuon_eta->push_back(muon.eta());
            dispMuon_phi->push_back(muon.phi());
            dispMuon_charge->push_back(muon.charge());
            dispMuon_isSAMuon->push_back(muon.isStandAloneMuon());
            dispMuon_isTrackerMuon->push_back(muon.isTrackerMuon());
            dispMuon_isGlobalMuon->push_back(muon.isGlobalMuon());
            dispMuon_isPFMuon->push_back(muon.isPFMuon());
            dispMuon_vx->push_back(muon.vx());
            dispMuon_vy->push_back(muon.vy());
            dispMuon_vz->push_back(muon.vz());
            dispMuon_px->push_back(muon.px());
            dispMuon_py->push_back(muon.py());
            dispMuon_pz->push_back(muon.pz());
            dispMuon_nChambers->push_back(muon.numberOfChambers() );
            dispMuon_nChambersCSCorDT->push_back(muon.numberOfChambersCSCorDT() );
            dispMuon_nMatches->push_back(muon.numberOfMatches() );
            dispMuon_nMatchedStations->push_back(muon.numberOfMatchedStations() );
            dispMuon_expectedNumberOfMatchedStations->push_back(muon.expectedNnumberOfMatchedStations() );
            dispMuon_stationMask->push_back(muon.stationMask() );
            dispMuon_nMatchedRPCLayers->push_back(muon.numberOfMatchedRPCLayers() );
            dispMuon_RPClayerMask->push_back(muon.RPClayerMask() );

            if (Vertices_ != nullptr && !Vertices_->empty()){
                if( !(muon.muonBestTrack().isNull())){
                    dispMuon_dz->push_back( muon.muonBestTrack()->dz((*Vertices_)[0].position()));
                    dispMuon_dxy->push_back( muon.muonBestTrack()->dxy((*Vertices_)[0].position()));
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
            dispMuon_isLooseMuon->push_back(isLoose);
            dispMuon_isMediumMuon->push_back(isMedium);
            dispMuon_isTightMuon->push_back(isTight);

            double iso = (muon.pfIsolationR04().sumChargedHadronPt +
                std::max(0.,
                    muon.pfIsolationR04().sumNeutralHadronEt + muon.pfIsolationR04().sumPhotonEt -
                        0.5 * muon.pfIsolationR04().sumPUPt)) /
                muon.pt();
            dispMuon_iso->push_back(iso);

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

                dispMuon_hlt_isomu->push_back(hasIsoTriggered);
                dispMuon_hlt_mu->push_back(hasTriggered);
                dispMuon_hlt_isoDeltaR->push_back(isoMatchDeltaR);
                dispMuon_hlt_deltaR->push_back(matchDeltaR);
                dispMuon_passesSingleMuon->push_back(passesSingleMuonFlag);
            } else {
                dispMuon_hlt_isomu->push_back(-999);
                dispMuon_hlt_mu->push_back(-999);
                dispMuon_hlt_isoDeltaR->push_back(-999);
                dispMuon_hlt_deltaR->push_back(-999);
                dispMuon_passesSingleMuon->push_back(-999);
            }

            // extrapolation of track coordinates
            TrajectoryStateOnSurface stateAtMuSt1 = muPropagator1st_.extrapolate(muon);
            if (stateAtMuSt1.isValid()) {
                dispMuon_etaSt1->push_back(stateAtMuSt1.globalPosition().eta());
                dispMuon_phiSt1->push_back(stateAtMuSt1.globalPosition().phi());
            } else {
                dispMuon_etaSt1->push_back(-9999);
                dispMuon_phiSt1->push_back(-9999);
            }

            TrajectoryStateOnSurface stateAtMuSt2 = muPropagator2nd_.extrapolate(muon);
            if (stateAtMuSt2.isValid()) {
                dispMuon_etaSt2->push_back(stateAtMuSt2.globalPosition().eta());
                dispMuon_phiSt2->push_back(stateAtMuSt2.globalPosition().phi());
            } else {
                dispMuon_etaSt2->push_back(-9999);
                dispMuon_phiSt2->push_back(-9999);
            }

        }
        (*dispMuon_size) = DispMuons_->size();
    }

    // Cosmic muons
    if (useCosmicMuons_ && CosmicMuons_ != nullptr) {
        for (const auto &muon : *CosmicMuons_) {
            cosmicMuon_e->push_back(muon.energy());
            cosmicMuon_et->push_back(muon.et());
            cosmicMuon_pt->push_back(muon.pt());
            cosmicMuon_eta->push_back(muon.eta());
            cosmicMuon_phi->push_back(muon.phi());
            cosmicMuon_charge->push_back(muon.charge());
            cosmicMuon_isSAMuon->push_back(muon.isStandAloneMuon());
            cosmicMuon_isTrackerMuon->push_back(muon.isTrackerMuon());
            cosmicMuon_isGlobalMuon->push_back(muon.isGlobalMuon());
            cosmicMuon_isPFMuon->push_back(muon.isPFMuon());
            cosmicMuon_vx->push_back(muon.vx());
            cosmicMuon_vy->push_back(muon.vy());
            cosmicMuon_vz->push_back(muon.vz());
            cosmicMuon_px->push_back(muon.px());
            cosmicMuon_py->push_back(muon.py());
            cosmicMuon_pz->push_back(muon.pz());
            cosmicMuon_nChambers->push_back(muon.numberOfChambers() );
            cosmicMuon_nChambersCSCorDT->push_back(muon.numberOfChambersCSCorDT() );
            cosmicMuon_nMatches->push_back(muon.numberOfMatches() );
            cosmicMuon_nMatchedStations->push_back(muon.numberOfMatchedStations() );
            cosmicMuon_expectedNumberOfMatchedStations->push_back(muon.expectedNnumberOfMatchedStations() );
            cosmicMuon_stationMask->push_back(muon.stationMask() );
            cosmicMuon_nMatchedRPCLayers->push_back(muon.numberOfMatchedRPCLayers() );
            cosmicMuon_RPClayerMask->push_back(muon.RPClayerMask() );

            if (Vertices_ != nullptr && !Vertices_->empty()){
                if( !(muon.muonBestTrack().isNull())){
                    cosmicMuon_dz->push_back( muon.muonBestTrack()->dz((*Vertices_)[0].position()));
                    cosmicMuon_dxy->push_back( muon.muonBestTrack()->dxy((*Vertices_)[0].position()));
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
            cosmicMuon_isLooseMuon->push_back(isLoose);
            cosmicMuon_isMediumMuon->push_back(isMedium);
            cosmicMuon_isTightMuon->push_back(isTight);

            double iso = (muon.pfIsolationR04().sumChargedHadronPt +
                std::max(0.,
                    muon.pfIsolationR04().sumNeutralHadronEt + muon.pfIsolationR04().sumPhotonEt -
                        0.5 * muon.pfIsolationR04().sumPUPt)) /
                muon.pt();
            cosmicMuon_iso->push_back(iso);

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

                cosmicMuon_hlt_isomu->push_back(hasIsoTriggered);
                cosmicMuon_hlt_mu->push_back(hasTriggered);
                cosmicMuon_hlt_isoDeltaR->push_back(isoMatchDeltaR);
                cosmicMuon_hlt_deltaR->push_back(matchDeltaR);
                cosmicMuon_passesSingleMuon->push_back(passesSingleMuonFlag);
            } else {
                cosmicMuon_hlt_isomu->push_back(-999);
                cosmicMuon_hlt_mu->push_back(-999);
                cosmicMuon_hlt_isoDeltaR->push_back(-999);
                cosmicMuon_hlt_deltaR->push_back(-999);
                cosmicMuon_passesSingleMuon->push_back(-999);
            }

            // extrapolation of track coordinates
            TrajectoryStateOnSurface stateAtMuSt1 = muPropagator1st_.extrapolate(muon);
            if (stateAtMuSt1.isValid()) {
                cosmicMuon_etaSt1->push_back(stateAtMuSt1.globalPosition().eta());
                cosmicMuon_phiSt1->push_back(stateAtMuSt1.globalPosition().phi());
            } else {
                cosmicMuon_etaSt1->push_back(-9999);
                cosmicMuon_phiSt1->push_back(-9999);
            }

            TrajectoryStateOnSurface stateAtMuSt2 = muPropagator2nd_.extrapolate(muon);
            if (stateAtMuSt2.isValid()) {
                cosmicMuon_etaSt2->push_back(stateAtMuSt2.globalPosition().eta());
                cosmicMuon_phiSt2->push_back(stateAtMuSt2.globalPosition().phi());
            } else {
                cosmicMuon_etaSt2->push_back(-9999);
                cosmicMuon_phiSt2->push_back(-9999);
            }

        }
        (*cosmicMuon_size) = CosmicMuons_->size();
    }

    // Cosmic 1 Leg muons
    if (useCosmicMuons1Leg_ && CosmicMuons1Leg_ != nullptr) {
        for (const auto &muon : *CosmicMuons1Leg_) {
            cosmicMuon1Leg_e->push_back(muon.energy());
            cosmicMuon1Leg_et->push_back(muon.et());
            cosmicMuon1Leg_pt->push_back(muon.pt());
            cosmicMuon1Leg_eta->push_back(muon.eta());
            cosmicMuon1Leg_phi->push_back(muon.phi());
            cosmicMuon1Leg_charge->push_back(muon.charge());
            cosmicMuon1Leg_isSAMuon->push_back(muon.isStandAloneMuon());
            cosmicMuon1Leg_isTrackerMuon->push_back(muon.isTrackerMuon());
            cosmicMuon1Leg_isGlobalMuon->push_back(muon.isGlobalMuon());
            cosmicMuon1Leg_isPFMuon->push_back(muon.isPFMuon());
            cosmicMuon1Leg_vx->push_back(muon.vx());
            cosmicMuon1Leg_vy->push_back(muon.vy());
            cosmicMuon1Leg_vz->push_back(muon.vz());
            cosmicMuon1Leg_px->push_back(muon.px());
            cosmicMuon1Leg_py->push_back(muon.py());
            cosmicMuon1Leg_pz->push_back(muon.pz());
            cosmicMuon1Leg_nChambers->push_back(muon.numberOfChambers() );
            cosmicMuon1Leg_nChambersCSCorDT->push_back(muon.numberOfChambersCSCorDT() );
            cosmicMuon1Leg_nMatches->push_back(muon.numberOfMatches() );
            cosmicMuon1Leg_nMatchedStations->push_back(muon.numberOfMatchedStations() );
            cosmicMuon1Leg_expectedNumberOfMatchedStations->push_back(muon.expectedNnumberOfMatchedStations() );
            cosmicMuon1Leg_stationMask->push_back(muon.stationMask() );
            cosmicMuon1Leg_nMatchedRPCLayers->push_back(muon.numberOfMatchedRPCLayers() );
            cosmicMuon1Leg_RPClayerMask->push_back(muon.RPClayerMask() );

            if (Vertices_ != nullptr && !Vertices_->empty()){
                if( !(muon.muonBestTrack().isNull())){
                    cosmicMuon1Leg_dz->push_back( muon.muonBestTrack()->dz((*Vertices_)[0].position()));
                    cosmicMuon1Leg_dxy->push_back( muon.muonBestTrack()->dxy((*Vertices_)[0].position()));
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
            cosmicMuon1Leg_isLooseMuon->push_back(isLoose);
            cosmicMuon1Leg_isMediumMuon->push_back(isMedium);
            cosmicMuon1Leg_isTightMuon->push_back(isTight);

            double iso = (muon.pfIsolationR04().sumChargedHadronPt +
                std::max(0.,
                    muon.pfIsolationR04().sumNeutralHadronEt + muon.pfIsolationR04().sumPhotonEt -
                        0.5 * muon.pfIsolationR04().sumPUPt)) /
                muon.pt();
            cosmicMuon1Leg_iso->push_back(iso);

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

                cosmicMuon1Leg_hlt_isomu->push_back(hasIsoTriggered);
                cosmicMuon1Leg_hlt_mu->push_back(hasTriggered);
                cosmicMuon1Leg_hlt_isoDeltaR->push_back(isoMatchDeltaR);
                cosmicMuon1Leg_hlt_deltaR->push_back(matchDeltaR);
                cosmicMuon1Leg_passesSingleMuon->push_back(passesSingleMuonFlag);
            } else {
                cosmicMuon1Leg_hlt_isomu->push_back(-999);
                cosmicMuon1Leg_hlt_mu->push_back(-999);
                cosmicMuon1Leg_hlt_isoDeltaR->push_back(-999);
                cosmicMuon1Leg_hlt_deltaR->push_back(-999);
                cosmicMuon1Leg_passesSingleMuon->push_back(-999);
            }

            // extrapolation of track coordinates
            TrajectoryStateOnSurface stateAtMuSt1 = muPropagator1st_.extrapolate(muon);
            if (stateAtMuSt1.isValid()) {
                cosmicMuon1Leg_etaSt1->push_back(stateAtMuSt1.globalPosition().eta());
                cosmicMuon1Leg_phiSt1->push_back(stateAtMuSt1.globalPosition().phi());
            } else {
                cosmicMuon1Leg_etaSt1->push_back(-9999);
                cosmicMuon1Leg_phiSt1->push_back(-9999);
            }

            TrajectoryStateOnSurface stateAtMuSt2 = muPropagator2nd_.extrapolate(muon);
            if (stateAtMuSt2.isValid()) {
                cosmicMuon1Leg_etaSt2->push_back(stateAtMuSt2.globalPosition().eta());
                cosmicMuon1Leg_phiSt2->push_back(stateAtMuSt2.globalPosition().phi());
            } else {
                cosmicMuon1Leg_etaSt2->push_back(-9999);
                cosmicMuon1Leg_phiSt2->push_back(-9999);
            }

        }
        (*cosmicMuon1Leg_size) = CosmicMuons1Leg_->size();
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
    auto dispMuon_handle = make_handle(DispMuons_);
    auto cosmicMuon_handle = make_handle(CosmicMuons_);
    auto cosmicMuon1Leg_handle = make_handle(CosmicMuons1Leg_);
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

    if (useDispMuons_) {
        if (!DispMuonToken_.isUninitialized()) {
            iEvent.getByToken(DispMuonToken_, dispMuon_handle);
        }
        if (!dispMuon_handle.isValid()) {
            if (firstEvent_)
                edm::LogError("NtupleMaker") << "Cannot get the product: " << DispMuonTag_;
            DispMuons_ = nullptr;
        } else {
            DispMuons_ = dispMuon_handle.product();
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
        DispMuons_ = nullptr;
        Vertices_ = nullptr;
    }


    if (useCosmicMuons_) {
        if (!CosmicMuonToken_.isUninitialized()) {
            iEvent.getByToken(CosmicMuonToken_, cosmicMuon_handle);
        }
        if (!cosmicMuon_handle.isValid()) {
            if (firstEvent_)
                edm::LogError("NtupleMaker") << "Cannot get the product: " << CosmicMuonTag_;
            CosmicMuons_ = nullptr;
        } else {
            CosmicMuons_ = cosmicMuon_handle.product();
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
        CosmicMuons_ = nullptr;
        Vertices_ = nullptr;
    }

    if (useCosmicMuons1Leg_) {
        if (!CosmicMuon1LegToken_.isUninitialized()) {
            iEvent.getByToken(CosmicMuon1LegToken_, cosmicMuon1Leg_handle);
        }
        if (!cosmicMuon1Leg_handle.isValid()) {
            if (firstEvent_)
                edm::LogError("NtupleMaker") << "Cannot get the product: " << CosmicMuon1LegTag_;
            CosmicMuons1Leg_ = nullptr;
        } else {
            CosmicMuons1Leg_ = cosmicMuon1Leg_handle.product();
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
        CosmicMuons1Leg_ = nullptr;
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



    // Disp muon info pointers
    dispMuon_size = std::make_unique<int32_t>(0);
    dispMuon_e = std::make_unique<std::vector<float>>();
    dispMuon_et = std::make_unique<std::vector<float>>();
    dispMuon_pt = std::make_unique<std::vector<float>>();
    dispMuon_eta = std::make_unique<std::vector<float>>();
    dispMuon_phi = std::make_unique<std::vector<float>>();
    dispMuon_dxy = std::make_unique<std::vector<float>>();
    dispMuon_dz = std::make_unique<std::vector<float>>();
    dispMuon_isLooseMuon = std::make_unique<std::vector<bool>>();
    dispMuon_isMediumMuon = std::make_unique<std::vector<bool>>();
    dispMuon_isTightMuon = std::make_unique<std::vector<bool>>();
    dispMuon_iso = std::make_unique<std::vector<float>>();
    dispMuon_hlt_isomu = std::make_unique<std::vector<short>>();
    dispMuon_hlt_mu = std::make_unique<std::vector<short>>();
    dispMuon_hlt_isoDeltaR = std::make_unique<std::vector<float>>();
    dispMuon_hlt_deltaR = std::make_unique<std::vector<float>>();
    dispMuon_passesSingleMuon = std::make_unique<std::vector<int>>();
    dispMuon_charge = std::make_unique<std::vector<int>>();
    dispMuon_etaSt1 = std::make_unique<std::vector<float>>();
    dispMuon_phiSt1 = std::make_unique<std::vector<float>>();
    dispMuon_etaSt2 = std::make_unique<std::vector<float>>();
    dispMuon_phiSt2 = std::make_unique<std::vector<float>>();
    dispMuon_vx = std::make_unique<std::vector<float>>();
    dispMuon_vy = std::make_unique<std::vector<float>>();
    dispMuon_vz = std::make_unique<std::vector<float>>();
    dispMuon_px = std::make_unique<std::vector<float>>();
    dispMuon_py = std::make_unique<std::vector<float>>();
    dispMuon_pz = std::make_unique<std::vector<float>>();
    dispMuon_isSAMuon = std::make_unique<std::vector<bool>>();
    dispMuon_isGlobalMuon = std::make_unique<std::vector<bool>>();
    dispMuon_isTrackerMuon = std::make_unique<std::vector<bool>>();
    dispMuon_isPFMuon = std::make_unique<std::vector<bool>>();
    dispMuon_nChambers = std::make_unique<std::vector<int>>();
    dispMuon_nChambersCSCorDT = std::make_unique<std::vector<int>>();
    dispMuon_nMatches = std::make_unique<std::vector<int>>();
    dispMuon_nMatchedStations = std::make_unique<std::vector<int>>();
    dispMuon_expectedNumberOfMatchedStations = std::make_unique<std::vector<unsigned int>>();
    dispMuon_stationMask = std::make_unique<std::vector<unsigned int>>();
    dispMuon_nMatchedRPCLayers = std::make_unique<std::vector<int>>();
    dispMuon_RPClayerMask = std::make_unique<std::vector<unsigned int>>();



    // Disp muon info pointers
    cosmicMuon_size = std::make_unique<int32_t>(0);
    cosmicMuon_e = std::make_unique<std::vector<float>>();
    cosmicMuon_et = std::make_unique<std::vector<float>>();
    cosmicMuon_pt = std::make_unique<std::vector<float>>();
    cosmicMuon_eta = std::make_unique<std::vector<float>>();
    cosmicMuon_phi = std::make_unique<std::vector<float>>();
    cosmicMuon_dxy = std::make_unique<std::vector<float>>();
    cosmicMuon_dz = std::make_unique<std::vector<float>>();
    cosmicMuon_isLooseMuon = std::make_unique<std::vector<bool>>();
    cosmicMuon_isMediumMuon = std::make_unique<std::vector<bool>>();
    cosmicMuon_isTightMuon = std::make_unique<std::vector<bool>>();
    cosmicMuon_iso = std::make_unique<std::vector<float>>();
    cosmicMuon_hlt_isomu = std::make_unique<std::vector<short>>();
    cosmicMuon_hlt_mu = std::make_unique<std::vector<short>>();
    cosmicMuon_hlt_isoDeltaR = std::make_unique<std::vector<float>>();
    cosmicMuon_hlt_deltaR = std::make_unique<std::vector<float>>();
    cosmicMuon_passesSingleMuon = std::make_unique<std::vector<int>>();
    cosmicMuon_charge = std::make_unique<std::vector<int>>();
    cosmicMuon_etaSt1 = std::make_unique<std::vector<float>>();
    cosmicMuon_phiSt1 = std::make_unique<std::vector<float>>();
    cosmicMuon_etaSt2 = std::make_unique<std::vector<float>>();
    cosmicMuon_phiSt2 = std::make_unique<std::vector<float>>();
    cosmicMuon_vx = std::make_unique<std::vector<float>>();
    cosmicMuon_vy = std::make_unique<std::vector<float>>();
    cosmicMuon_vz = std::make_unique<std::vector<float>>();
    cosmicMuon_px = std::make_unique<std::vector<float>>();
    cosmicMuon_py = std::make_unique<std::vector<float>>();
    cosmicMuon_pz = std::make_unique<std::vector<float>>();
    cosmicMuon_isSAMuon = std::make_unique<std::vector<bool>>();
    cosmicMuon_isGlobalMuon = std::make_unique<std::vector<bool>>();
    cosmicMuon_isTrackerMuon = std::make_unique<std::vector<bool>>();
    cosmicMuon_isPFMuon = std::make_unique<std::vector<bool>>();
    cosmicMuon_nChambers = std::make_unique<std::vector<int>>();
    cosmicMuon_nChambersCSCorDT = std::make_unique<std::vector<int>>();
    cosmicMuon_nMatches = std::make_unique<std::vector<int>>();
    cosmicMuon_nMatchedStations = std::make_unique<std::vector<int>>();
    cosmicMuon_expectedNumberOfMatchedStations = std::make_unique<std::vector<unsigned int>>();
    cosmicMuon_stationMask = std::make_unique<std::vector<unsigned int>>();
    cosmicMuon_nMatchedRPCLayers = std::make_unique<std::vector<int>>();
    cosmicMuon_RPClayerMask = std::make_unique<std::vector<unsigned int>>();



    // Disp muon info pointers
    cosmicMuon1Leg_size = std::make_unique<int32_t>(0);
    cosmicMuon1Leg_e = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_et = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_pt = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_eta = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_phi = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_dxy = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_dz = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_isLooseMuon = std::make_unique<std::vector<bool>>();
    cosmicMuon1Leg_isMediumMuon = std::make_unique<std::vector<bool>>();
    cosmicMuon1Leg_isTightMuon = std::make_unique<std::vector<bool>>();
    cosmicMuon1Leg_iso = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_hlt_isomu = std::make_unique<std::vector<short>>();
    cosmicMuon1Leg_hlt_mu = std::make_unique<std::vector<short>>();
    cosmicMuon1Leg_hlt_isoDeltaR = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_hlt_deltaR = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_passesSingleMuon = std::make_unique<std::vector<int>>();
    cosmicMuon1Leg_charge = std::make_unique<std::vector<int>>();
    cosmicMuon1Leg_etaSt1 = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_phiSt1 = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_etaSt2 = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_phiSt2 = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_vx = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_vy = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_vz = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_px = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_py = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_pz = std::make_unique<std::vector<float>>();
    cosmicMuon1Leg_isSAMuon = std::make_unique<std::vector<bool>>();
    cosmicMuon1Leg_isGlobalMuon = std::make_unique<std::vector<bool>>();
    cosmicMuon1Leg_isTrackerMuon = std::make_unique<std::vector<bool>>();
    cosmicMuon1Leg_isPFMuon = std::make_unique<std::vector<bool>>();
    cosmicMuon1Leg_nChambers = std::make_unique<std::vector<int>>();
    cosmicMuon1Leg_nChambersCSCorDT = std::make_unique<std::vector<int>>();
    cosmicMuon1Leg_nMatches = std::make_unique<std::vector<int>>();
    cosmicMuon1Leg_nMatchedStations = std::make_unique<std::vector<int>>();
    cosmicMuon1Leg_expectedNumberOfMatchedStations = std::make_unique<std::vector<unsigned int>>();
    cosmicMuon1Leg_stationMask = std::make_unique<std::vector<unsigned int>>();
    cosmicMuon1Leg_nMatchedRPCLayers = std::make_unique<std::vector<int>>();
    cosmicMuon1Leg_RPClayerMask = std::make_unique<std::vector<unsigned int>>();


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

    // Disp muons
    if (useDispMuons_) {
        tree->Branch("dispMuon_size", &(*dispMuon_size));
        tree->Branch("dispMuon_e", &(*dispMuon_e));
        tree->Branch("dispMuon_et", &(*dispMuon_et));
        tree->Branch("dispMuon_pt", &(*dispMuon_pt));
        tree->Branch("dispMuon_eta", &(*dispMuon_eta));
        tree->Branch("dispMuon_phi", &(*dispMuon_phi));
        tree->Branch("dispMuon_dxy", &(*dispMuon_dxy));
        tree->Branch("dispMuon_dz", &(*dispMuon_dz));
        tree->Branch("dispMuon_isLooseMuon", &(*dispMuon_isLooseMuon));
        tree->Branch("dispMuon_isMediumMuon", &(*dispMuon_isMediumMuon));
        tree->Branch("dispMuon_isTightMuon", &(*dispMuon_isTightMuon));
        tree->Branch("dispMuon_iso", &(*dispMuon_iso));
        tree->Branch("dispMuon_hlt_isomu", &(*dispMuon_hlt_isomu));
        tree->Branch("dispMuon_hlt_mu", &(*dispMuon_hlt_mu));
        tree->Branch("dispMuon_hlt_isoDeltaR", &(*dispMuon_hlt_isoDeltaR));
        tree->Branch("dispMuon_hlt_deltaR", &(*dispMuon_hlt_deltaR));
        tree->Branch("dispMuon_passesSingleMuon", &(*dispMuon_passesSingleMuon));
        tree->Branch("dispMuon_charge", &(*dispMuon_charge));
        tree->Branch("dispMuon_etaSt1", &(*dispMuon_etaSt1));
        tree->Branch("dispMuon_phiSt1", &(*dispMuon_phiSt1));
        tree->Branch("dispMuon_etaSt2", &(*dispMuon_etaSt2));
        tree->Branch("dispMuon_phiSt2", &(*dispMuon_phiSt2));
        tree->Branch("dispMuon_vx", &(*dispMuon_vx));
        tree->Branch("dispMuon_vy", &(*dispMuon_vy));
        tree->Branch("dispMuon_vz", &(*dispMuon_vz));
        tree->Branch("dispMuon_px", &(*dispMuon_px));
        tree->Branch("dispMuon_py", &(*dispMuon_py));
        tree->Branch("dispMuon_pz", &(*dispMuon_pz));
        tree->Branch("dispMuon_isSAMuon", &(*dispMuon_isSAMuon));
        tree->Branch("dispMuon_isTrackerMuon", &(*dispMuon_isTrackerMuon));
        tree->Branch("dispMuon_isGlobalMuon", &(*dispMuon_isGlobalMuon));
        tree->Branch("dispMuon_isPFMuon", &(*dispMuon_isPFMuon));
        tree->Branch("dispMuon_nChambers", &(*dispMuon_nChambers));
        tree->Branch("dispMuon_nChambersCSCorDT", &(*dispMuon_nChambersCSCorDT));
        tree->Branch("dispMuon_nMatches", &(*dispMuon_nMatches));
        tree->Branch("dispMuon_nMatchedStations", &(*dispMuon_nMatchedStations));
        tree->Branch("dispMuon_expectedNumberOfMatchedStations", &(*dispMuon_expectedNumberOfMatchedStations));
        tree->Branch("dispMuon_stationMask", &(*dispMuon_stationMask));
        tree->Branch("dispMuon_nMatchedRPCLayers", &(*dispMuon_nMatchedRPCLayers));
        tree->Branch("dispMuon_RPClayerMask", &(*dispMuon_RPClayerMask));
    }

    // Cosmic muons
    if (useCosmicMuons_) {
        tree->Branch("cosmicMuon_size", &(*cosmicMuon_size));
        tree->Branch("cosmicMuon_e", &(*cosmicMuon_e));
        tree->Branch("cosmicMuon_et", &(*cosmicMuon_et));
        tree->Branch("cosmicMuon_pt", &(*cosmicMuon_pt));
        tree->Branch("cosmicMuon_eta", &(*cosmicMuon_eta));
        tree->Branch("cosmicMuon_phi", &(*cosmicMuon_phi));
        tree->Branch("cosmicMuon_dxy", &(*cosmicMuon_dxy));
        tree->Branch("cosmicMuon_dz", &(*cosmicMuon_dz));
        tree->Branch("cosmicMuon_isLooseMuon", &(*cosmicMuon_isLooseMuon));
        tree->Branch("cosmicMuon_isMediumMuon", &(*cosmicMuon_isMediumMuon));
        tree->Branch("cosmicMuon_isTightMuon", &(*cosmicMuon_isTightMuon));
        tree->Branch("cosmicMuon_iso", &(*cosmicMuon_iso));
        tree->Branch("cosmicMuon_hlt_isomu", &(*cosmicMuon_hlt_isomu));
        tree->Branch("cosmicMuon_hlt_mu", &(*cosmicMuon_hlt_mu));
        tree->Branch("cosmicMuon_hlt_isoDeltaR", &(*cosmicMuon_hlt_isoDeltaR));
        tree->Branch("cosmicMuon_hlt_deltaR", &(*cosmicMuon_hlt_deltaR));
        tree->Branch("cosmicMuon_passesSingleMuon", &(*cosmicMuon_passesSingleMuon));
        tree->Branch("cosmicMuon_charge", &(*cosmicMuon_charge));
        tree->Branch("cosmicMuon_etaSt1", &(*cosmicMuon_etaSt1));
        tree->Branch("cosmicMuon_phiSt1", &(*cosmicMuon_phiSt1));
        tree->Branch("cosmicMuon_etaSt2", &(*cosmicMuon_etaSt2));
        tree->Branch("cosmicMuon_phiSt2", &(*cosmicMuon_phiSt2));
        tree->Branch("cosmicMuon_vx", &(*cosmicMuon_vx));
        tree->Branch("cosmicMuon_vy", &(*cosmicMuon_vy));
        tree->Branch("cosmicMuon_vz", &(*cosmicMuon_vz));
        tree->Branch("cosmicMuon_px", &(*cosmicMuon_px));
        tree->Branch("cosmicMuon_py", &(*cosmicMuon_py));
        tree->Branch("cosmicMuon_pz", &(*cosmicMuon_pz));
        tree->Branch("cosmicMuon_isSAMuon", &(*cosmicMuon_isSAMuon));
        tree->Branch("cosmicMuon_isTrackerMuon", &(*cosmicMuon_isTrackerMuon));
        tree->Branch("cosmicMuon_isGlobalMuon", &(*cosmicMuon_isGlobalMuon));
        tree->Branch("cosmicMuon_isPFMuon", &(*cosmicMuon_isPFMuon));
        tree->Branch("cosmicMuon_nChambers", &(*cosmicMuon_nChambers));
        tree->Branch("cosmicMuon_nChambersCSCorDT", &(*cosmicMuon_nChambersCSCorDT));
        tree->Branch("cosmicMuon_nMatches", &(*cosmicMuon_nMatches));
        tree->Branch("cosmicMuon_nMatchedStations", &(*cosmicMuon_nMatchedStations));
        tree->Branch("cosmicMuon_expectedNumberOfMatchedStations", &(*cosmicMuon_expectedNumberOfMatchedStations));
        tree->Branch("cosmicMuon_stationMask", &(*cosmicMuon_stationMask));
        tree->Branch("cosmicMuon_nMatchedRPCLayers", &(*cosmicMuon_nMatchedRPCLayers));
        tree->Branch("cosmicMuon_RPClayerMask", &(*cosmicMuon_RPClayerMask));
    }

    // Disp muons
    if (useDispMuons_) {
        tree->Branch("cosmicMuon1Leg_size", &(*cosmicMuon1Leg_size));
        tree->Branch("cosmicMuon1Leg_e", &(*cosmicMuon1Leg_e));
        tree->Branch("cosmicMuon1Leg_et", &(*cosmicMuon1Leg_et));
        tree->Branch("cosmicMuon1Leg_pt", &(*cosmicMuon1Leg_pt));
        tree->Branch("cosmicMuon1Leg_eta", &(*cosmicMuon1Leg_eta));
        tree->Branch("cosmicMuon1Leg_phi", &(*cosmicMuon1Leg_phi));
        tree->Branch("cosmicMuon1Leg_dxy", &(*cosmicMuon1Leg_dxy));
        tree->Branch("cosmicMuon1Leg_dz", &(*cosmicMuon1Leg_dz));
        tree->Branch("cosmicMuon1Leg_isLooseMuon", &(*cosmicMuon1Leg_isLooseMuon));
        tree->Branch("cosmicMuon1Leg_isMediumMuon", &(*cosmicMuon1Leg_isMediumMuon));
        tree->Branch("cosmicMuon1Leg_isTightMuon", &(*cosmicMuon1Leg_isTightMuon));
        tree->Branch("cosmicMuon1Leg_iso", &(*cosmicMuon1Leg_iso));
        tree->Branch("cosmicMuon1Leg_hlt_isomu", &(*cosmicMuon1Leg_hlt_isomu));
        tree->Branch("cosmicMuon1Leg_hlt_mu", &(*cosmicMuon1Leg_hlt_mu));
        tree->Branch("cosmicMuon1Leg_hlt_isoDeltaR", &(*cosmicMuon1Leg_hlt_isoDeltaR));
        tree->Branch("cosmicMuon1Leg_hlt_deltaR", &(*cosmicMuon1Leg_hlt_deltaR));
        tree->Branch("cosmicMuon1Leg_passesSingleMuon", &(*cosmicMuon1Leg_passesSingleMuon));
        tree->Branch("cosmicMuon1Leg_charge", &(*cosmicMuon1Leg_charge));
        tree->Branch("cosmicMuon1Leg_etaSt1", &(*cosmicMuon1Leg_etaSt1));
        tree->Branch("cosmicMuon1Leg_phiSt1", &(*cosmicMuon1Leg_phiSt1));
        tree->Branch("cosmicMuon1Leg_etaSt2", &(*cosmicMuon1Leg_etaSt2));
        tree->Branch("cosmicMuon1Leg_phiSt2", &(*cosmicMuon1Leg_phiSt2));
        tree->Branch("cosmicMuon1Leg_vx", &(*cosmicMuon1Leg_vx));
        tree->Branch("cosmicMuon1Leg_vy", &(*cosmicMuon1Leg_vy));
        tree->Branch("cosmicMuon1Leg_vz", &(*cosmicMuon1Leg_vz));
        tree->Branch("cosmicMuon1Leg_px", &(*cosmicMuon1Leg_px));
        tree->Branch("cosmicMuon1Leg_py", &(*cosmicMuon1Leg_py));
        tree->Branch("cosmicMuon1Leg_pz", &(*cosmicMuon1Leg_pz));
        tree->Branch("cosmicMuon1Leg_isSAMuon", &(*cosmicMuon1Leg_isSAMuon));
        tree->Branch("cosmicMuon1Leg_isTrackerMuon", &(*cosmicMuon1Leg_isTrackerMuon));
        tree->Branch("cosmicMuon1Leg_isGlobalMuon", &(*cosmicMuon1Leg_isGlobalMuon));
        tree->Branch("cosmicMuon1Leg_isPFMuon", &(*cosmicMuon1Leg_isPFMuon));
        tree->Branch("cosmicMuon1Leg_nChambers", &(*cosmicMuon1Leg_nChambers));
        tree->Branch("cosmicMuon1Leg_nChambersCSCorDT", &(*cosmicMuon1Leg_nChambersCSCorDT));
        tree->Branch("cosmicMuon1Leg_nMatches", &(*cosmicMuon1Leg_nMatches));
        tree->Branch("cosmicMuon1Leg_nMatchedStations", &(*cosmicMuon1Leg_nMatchedStations));
        tree->Branch("cosmicMuon1Leg_expectedNumberOfMatchedStations", &(*cosmicMuon1Leg_expectedNumberOfMatchedStations));
        tree->Branch("cosmicMuon1Leg_stationMask", &(*cosmicMuon1Leg_stationMask));
        tree->Branch("cosmicMuon1Leg_nMatchedRPCLayers", &(*cosmicMuon1Leg_nMatchedRPCLayers));
        tree->Branch("cosmicMuon1Leg_RPClayerMask", &(*cosmicMuon1Leg_RPClayerMask));
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



    // Clear Disp Muons
    (*dispMuon_size) = 0;
    dispMuon_e->clear();
    dispMuon_et->clear();
    dispMuon_pt->clear();
    dispMuon_eta->clear();
    dispMuon_phi->clear();
    dispMuon_dxy->clear();
    dispMuon_dz->clear();
    dispMuon_isLooseMuon->clear();
    dispMuon_isMediumMuon->clear();
    dispMuon_isTightMuon->clear();
    dispMuon_iso->clear();
    dispMuon_hlt_isomu->clear();
    dispMuon_hlt_mu->clear();
    dispMuon_hlt_isoDeltaR->clear();
    dispMuon_hlt_deltaR->clear();
    dispMuon_passesSingleMuon->clear();
    dispMuon_charge->clear();
    dispMuon_etaSt1->clear();
    dispMuon_phiSt1->clear();
    dispMuon_etaSt2->clear();
    dispMuon_phiSt2->clear();
    dispMuon_isSAMuon->clear() ;
    dispMuon_isTrackerMuon->clear();
    dispMuon_isGlobalMuon->clear();
    dispMuon_isPFMuon->clear();
    dispMuon_nChambers->clear();
    dispMuon_nChambersCSCorDT->clear();
    dispMuon_nMatches->clear();
    dispMuon_nMatchedStations->clear();
    dispMuon_expectedNumberOfMatchedStations->clear();
    dispMuon_stationMask->clear();
    dispMuon_nMatchedRPCLayers->clear();
    dispMuon_RPClayerMask->clear();
    
    dispMuon_vx->clear();
    dispMuon_vy->clear();
    dispMuon_vz->clear();
    dispMuon_px->clear();
    dispMuon_py->clear();
    dispMuon_pz->clear();

    // Clear Cosmic Muons
    (*cosmicMuon_size) = 0;
    cosmicMuon_e->clear();
    cosmicMuon_et->clear();
    cosmicMuon_pt->clear();
    cosmicMuon_eta->clear();
    cosmicMuon_phi->clear();
    cosmicMuon_dxy->clear();
    cosmicMuon_dz->clear();
    cosmicMuon_isLooseMuon->clear();
    cosmicMuon_isMediumMuon->clear();
    cosmicMuon_isTightMuon->clear();
    cosmicMuon_iso->clear();
    cosmicMuon_hlt_isomu->clear();
    cosmicMuon_hlt_mu->clear();
    cosmicMuon_hlt_isoDeltaR->clear();
    cosmicMuon_hlt_deltaR->clear();
    cosmicMuon_passesSingleMuon->clear();
    cosmicMuon_charge->clear();
    cosmicMuon_etaSt1->clear();
    cosmicMuon_phiSt1->clear();
    cosmicMuon_etaSt2->clear();
    cosmicMuon_phiSt2->clear();
    cosmicMuon_isSAMuon->clear() ;
    cosmicMuon_isTrackerMuon->clear();
    cosmicMuon_isGlobalMuon->clear();
    cosmicMuon_isPFMuon->clear();
    cosmicMuon_nChambers->clear();
    cosmicMuon_nChambersCSCorDT->clear();
    cosmicMuon_nMatches->clear();
    cosmicMuon_nMatchedStations->clear();
    cosmicMuon_expectedNumberOfMatchedStations->clear();
    cosmicMuon_stationMask->clear();
    cosmicMuon_nMatchedRPCLayers->clear();
    cosmicMuon_RPClayerMask->clear();
    
    cosmicMuon_vx->clear();
    cosmicMuon_vy->clear();
    cosmicMuon_vz->clear();
    cosmicMuon_px->clear();
    cosmicMuon_py->clear();
    cosmicMuon_pz->clear();

    // Clear Disp Muons
    (*cosmicMuon1Leg_size) = 0;
    cosmicMuon1Leg_e->clear();
    cosmicMuon1Leg_et->clear();
    cosmicMuon1Leg_pt->clear();
    cosmicMuon1Leg_eta->clear();
    cosmicMuon1Leg_phi->clear();
    cosmicMuon1Leg_dxy->clear();
    cosmicMuon1Leg_dz->clear();
    cosmicMuon1Leg_isLooseMuon->clear();
    cosmicMuon1Leg_isMediumMuon->clear();
    cosmicMuon1Leg_isTightMuon->clear();
    cosmicMuon1Leg_iso->clear();
    cosmicMuon1Leg_hlt_isomu->clear();
    cosmicMuon1Leg_hlt_mu->clear();
    cosmicMuon1Leg_hlt_isoDeltaR->clear();
    cosmicMuon1Leg_hlt_deltaR->clear();
    cosmicMuon1Leg_passesSingleMuon->clear();
    cosmicMuon1Leg_charge->clear();
    cosmicMuon1Leg_etaSt1->clear();
    cosmicMuon1Leg_phiSt1->clear();
    cosmicMuon1Leg_etaSt2->clear();
    cosmicMuon1Leg_phiSt2->clear();
    cosmicMuon1Leg_isSAMuon->clear() ;
    cosmicMuon1Leg_isTrackerMuon->clear();
    cosmicMuon1Leg_isGlobalMuon->clear();
    cosmicMuon1Leg_isPFMuon->clear();
    cosmicMuon1Leg_nChambers->clear();
    cosmicMuon1Leg_nChambersCSCorDT->clear();
    cosmicMuon1Leg_nMatches->clear();
    cosmicMuon1Leg_nMatchedStations->clear();
    cosmicMuon1Leg_expectedNumberOfMatchedStations->clear();
    cosmicMuon1Leg_stationMask->clear();
    cosmicMuon1Leg_nMatchedRPCLayers->clear();
    cosmicMuon1Leg_RPClayerMask->clear();
    
    cosmicMuon1Leg_vx->clear();
    cosmicMuon1Leg_vy->clear();
    cosmicMuon1Leg_vz->clear();
    cosmicMuon1Leg_px->clear();
    cosmicMuon1Leg_py->clear();
    cosmicMuon1Leg_pz->clear();
    

    // Clear flags
    (*HLT_IsoMu24) = false;
    (*HLT_Mu50_L1SingleMuShower) = false;
    (*HLT_Mu50) = false;
}

void MuonAODAnalyzer::endRun(const edm::Run &run, const edm::EventSetup &eventSetup) {
}

// define this as a plug-in
DEFINE_FWK_MODULE(MuonAODAnalyzer);