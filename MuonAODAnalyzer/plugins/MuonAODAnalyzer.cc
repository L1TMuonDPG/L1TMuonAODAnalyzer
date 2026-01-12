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

#include "MuonAODAnalyzer.h"


//
// constructors and destructor
//
MuonAODAnalyzer::MuonAODAnalyzer(const edm::ParameterSet& iConfig)
    :
    muonToken_(consumes< std::vector< reco::Muon> >(iConfig.getParameter<edm::InputTag>("Muons"))),
    l1MuonToken_(consumes<l1t::MuonBxCollection>(edm::InputTag("gmtStage2Digis","Muon"))),
    l1BMTFRegionalMuonCandToken_(consumes<BXVector<l1t::RegionalMuonCand>>(edm::InputTag("gmtStage2Digis","BMTF"))),
    verticesToken_(consumes<std::vector<Vertex> > (iConfig.getParameter<edm::InputTag>("Vertices"))),
    trgresultsToken_(consumes<TriggerResults>(iConfig.getParameter<edm::InputTag>("Triggers"))),
    UnprefirableEventToken_(consumes<GlobalExtBlkBxCollection>(edm::InputTag("simGtExtUnprefireable"))),
    l1GtToken_(consumes<BXVector<GlobalAlgBlk>>(iConfig.getParameter<edm::InputTag>("l1GtSrc"))),

    MuonPtCut_(iConfig.getParameter<double>("MuonPtCut")),
    SaveTree_(iConfig.getParameter<bool>("SaveTree")),
    IsMC_(iConfig.getParameter<bool>("IsMC")),
    Debug_(iConfig.getParameter<bool>("Debug")),

    muPropagatorSetup1st_(iConfig.getParameter<edm::ParameterSet>("muProp1st"), consumesCollector()),
    muPropagatorSetup2nd_(iConfig.getParameter<edm::ParameterSet>("muProp2nd"), consumesCollector())

{
  //now do what ever initialization is needed
  // usesResource("TFileService"); // shared resources

  edm::Service<TFileService> fs;
  outputTree = fs->make<TTree>("tree","tree");

}

MuonAODAnalyzer::~MuonAODAnalyzer() {}

//
// member functions
//

// ------------ method called for each event  ------------
void MuonAODAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  InitandClearStuff();

  muPropagator1st_ = muPropagatorSetup1st_.init(iSetup);
  muPropagator2nd_ = muPropagatorSetup2nd_.init(iSetup);


  _runNb = iEvent.id().run();
  _eventNb = iEvent.id().event();
  _lumiBlock = iEvent.luminosityBlock();
  _bx=iEvent.bunchCrossing();

  //Triggers
  edm::Handle<TriggerResults> trigResults;
  iEvent.getByToken(trgresultsToken_, trigResults);
  if( !trigResults.failedToGet() ) {
    int N_Triggers = trigResults->size();
    edm::TriggerNames const& trigName = iEvent.triggerNames(*trigResults);
    for( int i_Trig = 0; i_Trig < N_Triggers; ++i_Trig ) {
      if (trigResults.product()->accept(i_Trig)) {
	      TString TrigPath = trigName.triggerName(i_Trig);
	      if(TrigPath.Contains("HLT_IsoMu27_v"))HLT_IsoMu27 =true;
        if(TrigPath.Contains("HLT_IsoMu24_v"))HLT_IsoMu24 =true;
      }
    }
  }

  //Unprefirable events
  edm::Handle<GlobalExtBlkBxCollection> handleUnprefEventResults;
  iEvent.getByToken(UnprefirableEventToken_, handleUnprefEventResults);
  if(handleUnprefEventResults.isValid()){
    if (handleUnprefEventResults->size() != 0) {
      Flag_IsUnprefirable = handleUnprefEventResults->at(0, 0).getExternalDecision(GlobalExtBlk::maxExternalConditions - 1);
    }
  }

  //first bunch in train
  edm::Handle<BXVector<GlobalAlgBlk>> l1GtHandle;
  iEvent.getByToken(l1GtToken_, l1GtHandle);
  for(int i =0; i <512; i++){
    if(!IsMC_){ 
      if(i==472){
        passL1_Final_bxmin1= l1GtHandle->begin(-1)->getAlgoDecisionFinal(i);
        passL1_Final_bxmin2= l1GtHandle->begin(-2)->getAlgoDecisionFinal(i);
      }
    }
    else {
      passL1_Final_bxmin1= false;
      passL1_Final_bxmin2= false;
    }
  }

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

  // BMTF RegionalMuonCand
  edm::Handle<BXVector<l1t::RegionalMuonCand>> l1RegionalMuoncoll;
  iEvent.getByToken(l1BMTFRegionalMuonCandToken_ , l1RegionalMuoncoll);
  for(int i = l1RegionalMuoncoll->getFirstBX() ; i<= l1RegionalMuoncoll->getLastBX() ;i++){
    for( BXVector<l1t::RegionalMuonCand>::const_iterator l1muonit= l1RegionalMuoncoll->begin(i); l1muonit != l1RegionalMuoncoll->end(i) ; ++l1muonit){
      BMTFMu_processor.push_back(l1muonit->processor());
      BMTFMu_hwPt.push_back(l1muonit->hwPt());
      BMTFMu_hwQual.push_back(l1muonit->hwQual());
      BMTFMu_hwSign.push_back(l1muonit->hwSign());
      BMTFMu_hwSignValid.push_back(l1muonit->hwSignValid());
      BMTFMu_hwEta.push_back(l1muonit->hwEta());
      BMTFMu_hwPhi.push_back(l1muonit->hwPhi());
    }
  }


  //Vertices
  edm::Handle<std::vector<Vertex> > theVertices;
  iEvent.getByToken(verticesToken_,theVertices) ;
  _n_PV = theVertices->size();
  Vertex::Point PV(0,0,0);
  if(_n_PV){ PV = theVertices->begin()->position();}

  // reco muons
  edm::Handle< std::vector<reco::Muon> > theMuons;
  iEvent.getByToken(muonToken_,theMuons);
  for( std::vector<reco::Muon>::const_iterator muon = (*theMuons).begin(); muon != (*theMuons).end(); muon++ ) {
    if((&*muon)->pt() <0) continue; //Loose cut  on uncorrected pt 

    double ptmuoncorr= (&*muon)->pt();

    // store all reco muons for now
    muon_size++;
    muon_eta.push_back((&*muon)->eta());
    muon_phi.push_back((&*muon)->phi());
    muon_pt.push_back((&*muon)->pt());
    muon_ptCorr.push_back( ptmuoncorr );
    muon_charge.push_back((&*muon)->charge());
    muon_PassTightID.push_back(  (&*muon)->passed(reco::Muon::CutBasedIdMediumPrompt )&& (&*muon)->passed(reco::Muon::PFIsoTight ) );
    muon_PassLooseID.push_back(  (&*muon)->passed(reco::Muon::CutBasedIdLoose )&& (&*muon)->passed(reco::Muon::PFIsoLoose ) );
    muon_isSAMuon.push_back( (&*muon)->isStandAloneMuon());
    muon_isTrackerMuon.push_back( (&*muon)->isTrackerMuon());
    muon_isGlobalMuon.push_back( (&*muon)->isGlobalMuon());
    muon_isPFMuon.push_back( (&*muon)->isPFMuon());

    muon_vx.push_back( (&*muon)->vx() );
    muon_vy.push_back( (&*muon)->vy() );
    muon_vz.push_back( (&*muon)->vz() );
    muon_px.push_back( (&*muon)->px() );
    muon_py.push_back( (&*muon)->py() );
    muon_pz.push_back( (&*muon)->pz() );

    muon_nChambers.push_back( (&*muon)->numberOfChambers() );
    muon_nChambersCSCorDT.push_back((&*muon)->numberOfChambersCSCorDT() );
    muon_nMatches.push_back( (&*muon)->numberOfMatches() );
    muon_nMatchedStations.push_back( (&*muon)->numberOfMatchedStations() );
    muon_expectedNumberOfMatchedStations.push_back( (&*muon)->expectedNnumberOfMatchedStations() );
    muon_stationMask.push_back( (&*muon)->stationMask() );
    muon_nMatchedRPCLayers.push_back( (&*muon)->numberOfMatchedRPCLayers() );
    muon_RPClayerMask.push_back( (&*muon)->RPClayerMask() );
    
    if( !((&*muon)->innerTrack()).isNull()){
      muon_dxy.push_back( (&*muon)->innerTrack()->dxy(PV));
      muon_dz.push_back( (&*muon)->innerTrack()->dz(PV));
      muon_hasInnerTrack.push_back(true);
    }
    else if(!((&*muon)->outerTrack()).isNull()){
      muon_dxy.push_back( (&*muon)->outerTrack()->dxy(PV));
      muon_dz.push_back( (&*muon)->outerTrack()->dz(PV));
      muon_hasInnerTrack.push_back(false);
    }
    else{
      muon_dxy.push_back(-999.);
      muon_dz.push_back(-999.);
      muon_hasInnerTrack.push_back(false);
    }

    // extrapolation of muon track coordinates
    TrajectoryStateOnSurface stateAtMuSt1 = muPropagator1st_.extrapolate(*muon);
    if (stateAtMuSt1.isValid()) {
        muon_etaAtSt1.push_back(stateAtMuSt1.globalPosition().eta());
        muon_phiAtSt1.push_back(stateAtMuSt1.globalPosition().phi());
    } else {
        muon_etaAtSt1.push_back(-999);
        muon_phiAtSt1.push_back(-999);
    }

    TrajectoryStateOnSurface stateAtMuSt2 = muPropagator2nd_.extrapolate(*muon);
    if (stateAtMuSt2.isValid()) {
        muon_etaAtSt2.push_back(stateAtMuSt2.globalPosition().eta());
        muon_phiAtSt2.push_back(stateAtMuSt2.globalPosition().phi());
    } else {
        muon_etaAtSt2.push_back(-999);
        muon_phiAtSt2.push_back(-999);
    }
  }

  if(SaveTree_)outputTree->Fill();

}

// ------------ method called once each job just before starting event loop  ------------
void MuonAODAnalyzer::beginJob() {
  // please remove this method if not needed

  outputTree->Branch("_eventNb",   &_eventNb,   "_eventNb/l");
  outputTree->Branch("_runNb",     &_runNb,     "_runNb/l");
  outputTree->Branch("_lumiBlock", &_lumiBlock, "_lumiBlock/l");
  outputTree->Branch("_bx", &_bx, "_bx/l");

  outputTree->Branch("muon_eta",&muon_eta);
  outputTree->Branch("muon_etaAtSt1",&muon_etaAtSt1);
  outputTree->Branch("muon_etaAtSt2",&muon_etaAtSt2);
  outputTree->Branch("muon_phi",&muon_phi);
  outputTree->Branch("muon_phiAtSt1",&muon_phiAtSt1);
  outputTree->Branch("muon_phiAtSt2",&muon_phiAtSt2);
  outputTree->Branch("muon_pt",&muon_pt);
  outputTree->Branch("muon_ptCorr",&muon_ptCorr);
  outputTree->Branch("muon_charge",&muon_charge);
  outputTree->Branch("muon_dz",&muon_dz);
  outputTree->Branch("muon_dzError",&muon_dzError);
  outputTree->Branch("muon_dxy",&muon_dxy);
  outputTree->Branch("muon_dxyError",&muon_dxyError);
  outputTree->Branch("muon_3dIP",&muon_3dIP);
  outputTree->Branch("muon_3dIPError",&muon_3dIPError);
  outputTree->Branch("muon_PassTightID",&muon_PassTightID);
  outputTree->Branch("muon_PassLooseID",&muon_PassLooseID);
  outputTree->Branch("muon_isSAMuon",&muon_isSAMuon);
  outputTree->Branch("muon_isTrackerMuon",&muon_isTrackerMuon);
  outputTree->Branch("muon_isGlobalMuon",&muon_isGlobalMuon);
  outputTree->Branch("muon_isPFMuon",&muon_isPFMuon);
  outputTree->Branch("muon_hasInnerTrack",&muon_hasInnerTrack);

  outputTree->Branch("muon_vx",&muon_vx);
  outputTree->Branch("muon_vy",&muon_vy);
  outputTree->Branch("muon_vz",&muon_vz);
  outputTree->Branch("muon_px",&muon_px);
  outputTree->Branch("muon_py",&muon_py);
  outputTree->Branch("muon_pz",&muon_pz);

  outputTree->Branch("muon_nChambers",&muon_nChambers);
  outputTree->Branch("muon_nChambersCSCorDT",&muon_nChambersCSCorDT);
  outputTree->Branch("muon_nMatches",&muon_nMatches);
  outputTree->Branch("muon_nMatchedStations",&muon_nMatchedStations);
  outputTree->Branch("muon_expectedNumberOfMatchedStations",&muon_expectedNumberOfMatchedStations);
  outputTree->Branch("muon_stationMask",&muon_stationMask);
  outputTree->Branch("muon_nMatchedRPCLayers",&muon_nMatchedRPCLayers);
  outputTree->Branch("muon_RPClayerMask",&muon_RPClayerMask);

  outputTree->Branch("muon_size", &muon_size, "muon_size/I");

  outputTree->Branch("l1mu_qual",&l1mu_qual);
  outputTree->Branch("l1mu_charge",&l1mu_charge);
  outputTree->Branch("l1mu_pt",&l1mu_pt);
  outputTree->Branch("l1mu_pt_dxy",&l1mu_pt_dxy);
  outputTree->Branch("l1mu_dxy",&l1mu_dxy);
  outputTree->Branch("l1mu_eta",&l1mu_eta);
  outputTree->Branch("l1mu_etaAtVtx",&l1mu_etaAtVtx);
  outputTree->Branch("l1mu_phi",&l1mu_phi);
  outputTree->Branch("l1mu_phiAtVtx",&l1mu_phiAtVtx);
  outputTree->Branch("l1mu_tfIdx",&l1mu_tfIdx);
  outputTree->Branch("l1mu_bx",&l1mu_bx);
  outputTree->Branch("l1mu_size", &l1mu_size, "l1mu_size/I");

  outputTree->Branch("BMTFMu_processor",&BMTFMu_processor);
  outputTree->Branch("BMTFMu_hwPt",&BMTFMu_hwPt);
  outputTree->Branch("BMTFMu_hwQual",&BMTFMu_hwQual);
  outputTree->Branch("BMTFMu_hwSign",&BMTFMu_hwSign);
  outputTree->Branch("BMTFMu_hwSignValid",&BMTFMu_hwSignValid);
  outputTree->Branch("BMTFMu_hwEta",&BMTFMu_hwEta);
  outputTree->Branch("BMTFMu_hwPhi",&BMTFMu_hwPhi);

  outputTree->Branch("HLT_IsoMu27",&HLT_IsoMu27,"HLT_IsoMu27/O");
  outputTree->Branch("HLT_IsoMu24",&HLT_IsoMu24,"HLT_IsoMu24/O");

  outputTree->Branch("Flag_IsUnprefirable",&Flag_IsUnprefirable,"Flag_IsUnprefirable/O");
  outputTree->Branch("passL1_Final_bxmin1",&passL1_Final_bxmin1,"passL1_Final_bxmin1/O");
  outputTree->Branch("passL1_Final_bxmin2",&passL1_Final_bxmin2,"passL1_Final_bxmin2/O");
}

void MuonAODAnalyzer::endJob() {}

void MuonAODAnalyzer::InitandClearStuff() {

  muon_eta.clear();
  muon_etaAtSt1.clear();
  muon_etaAtSt2.clear();
  muon_phi.clear();
  muon_phiAtSt1.clear();
  muon_phiAtSt2.clear();
  muon_pt.clear();
  muon_ptCorr.clear();
  muon_charge.clear();
  muon_dz.clear();
  muon_dzError.clear();
  muon_dxy.clear();
  muon_dxyError.clear();
  muon_3dIP.clear();
  muon_3dIPError.clear();
  muon_PassTightID.clear();
  muon_PassLooseID.clear();
  muon_isSAMuon.clear() ;
  muon_isTrackerMuon.clear();
  muon_isGlobalMuon.clear();
  muon_isPFMuon.clear();
  muon_nChambers.clear();
  muon_nChambersCSCorDT.clear();
  muon_nMatches.clear();
  muon_nMatchedStations.clear();
  muon_expectedNumberOfMatchedStations.clear();
  muon_stationMask.clear();
  muon_nMatchedRPCLayers.clear();
  muon_RPClayerMask.clear();
  muon_hasInnerTrack.clear();
  
  muon_vx.clear();
  muon_vy.clear();
  muon_vz.clear();
  muon_px.clear();
  muon_py.clear();
  muon_pz.clear();

  muon_size = 0;

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

  BMTFMu_processor.clear();
  BMTFMu_hwPt.clear();
  BMTFMu_hwQual.clear();
  BMTFMu_hwSign.clear();
  BMTFMu_hwSignValid.clear();
  BMTFMu_hwEta.clear();
  BMTFMu_hwPhi.clear();

  HLT_IsoMu27 = false;
  HLT_IsoMu24 = false;

  Flag_IsUnprefirable = false;
  passL1_Final_bxmin1 = false;
  passL1_Final_bxmin2 = false;
}

//define this as a plug-in
DEFINE_FWK_MODULE(MuonAODAnalyzer);
