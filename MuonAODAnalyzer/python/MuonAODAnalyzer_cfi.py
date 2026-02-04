import FWCore.ParameterSet.Config as cms

MuonAODAnalyzer = cms.EDAnalyzer('MuonAODAnalyzer',

    debug            = cms.bool(True),
    verbosity        = cms.untracked.int32(0),
    outFileName      = cms.string('L1TMuonNtuple.root'),

    # Input Tags for Reco, Trigger, and Vertices
    RecoMuonTag      = cms.InputTag('muons'),
    Triggers         = cms.InputTag("TriggerResults::HLT"),
    l1GtSrc = cms.InputTag("gtStage2Digis"),
    DispMuonTag=cms.InputTag("displacedMuons"),
    CosmicMuonTag=cms.InputTag("muonsFromCosmics"), 
    CosmicMuon1LegTag=cms.InputTag("muonsFromCosmics1Leg"),
    
    # Flags to enable processing
    useRecoMuons     = cms.bool(True),
    useEventInfo     = cms.bool(True),
    useDispMuons     = cms.bool(True),
    useCosmicMuons     = cms.bool(True),
    useCosmicMuons1Leg     = cms.bool(True),

    # Trigger Names for matching and flags
    isoTriggerNames = cms.vstring(
      "HLT_IsoMu24_v*",
      "HLT_IsoMu27_v*",
      "HLT_IsoMu30_v*",
    ),
    triggerNames = cms.vstring(
      "HLT_Mu50_v*",
      "HLT_Mu55_v*",
    ),

    # muon track extrapolation to 1st station
    muProp1st = cms.PSet(
          useTrack = cms.string("tracker"),  # 'none' to use Candidate P4; or 'tracker', 'muon', 'global'
          useState = cms.string("atVertex"), # 'innermost' and 'outermost' require the TrackExtra
          useSimpleGeometry = cms.bool(True),
          useStation2 = cms.bool(False),
          fallbackToME1 = cms.bool(False),
          cosmicPropagationHypothesis = cms.bool(False),
          useMB2InOverlap = cms.bool(False),
          propagatorAlong = cms.ESInputTag("", "SteppingHelixPropagatorAlong"),
          propagatorAny = cms.ESInputTag("", "SteppingHelixPropagatorAny"),
          propagatorOpposite = cms.ESInputTag("", "SteppingHelixPropagatorOpposite")
    ),
    # muon track extrapolation to 2nd station
    muProp2nd = cms.PSet(
          useTrack = cms.string("none"),  # 'none' to use Candidate P4; or 'tracker', 'muon', 'global'
          useState = cms.string("atVertex"), # 'innermost' and 'outermost' require the TrackExtra
          useSimpleGeometry = cms.bool(False),
          useStation2 = cms.bool(True),
          fallbackToME1 = cms.bool(False),
          cosmicPropagationHypothesis = cms.bool(False),
          useMB2InOverlap = cms.bool(True),
          propagatorAlong = cms.ESInputTag("", "SteppingHelixPropagatorAlong"),
          propagatorAny = cms.ESInputTag("", "SteppingHelixPropagatorAny"),
          propagatorOpposite = cms.ESInputTag("", "SteppingHelixPropagatorOpposite")
    ),
)