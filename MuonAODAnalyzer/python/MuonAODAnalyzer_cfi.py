import FWCore.ParameterSet.Config as cms

MuonAODAnalyzer = cms.EDAnalyzer('MuonAODAnalyzer',

    Muons=cms.InputTag("muons"),
    Vertices=cms.InputTag("offlinePrimaryVertices"),
    Triggers = cms.InputTag("TriggerResults::HLT"),
    l1GtSrc = cms.InputTag("gtStage2Digis"),

    MuonPtCut=cms.double(0),
    IsMC=cms.bool(False),
    SaveTree=cms.bool(True),
    Debug=cms.bool(False),

    isoTriggerNames = cms.vstring(
      "HLT_IsoMu27_v*",
      "HLT_IsoMu30_v*",
    ),
    triggerNames = cms.vstring(
      "HLT_Mu50_v*",
      "HLT_Mu55_v*",
      # pA triggers
      # "HLT_PAL3Mu12_v*",
      # "HLT_PAL3Mu15_v*",
      # "HLT_PAL2Mu12_v*",
      # "HLT_PAL2Mu15_v*",
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
