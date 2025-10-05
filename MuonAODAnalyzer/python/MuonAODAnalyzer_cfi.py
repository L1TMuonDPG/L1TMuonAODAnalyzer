import FWCore.ParameterSet.Config as cms

MuonAODAnalyzer = cms.EDAnalyzer('MuonAODAnalyzer',

    debug           = cms.bool(True),


    CSCInputTag      = cms.InputTag('simCscTriggerPrimitiveDigis','MPCSORTED'),
    RPCInputTag      = cms.InputTag('simMuonRPCDigis'),
    CPPFInputTag     = cms.InputTag('simCPPFDigis'),
    GEMInputTag      = cms.InputTag('simMuonGEMPadDigiClusters'),

    IRPCInputTag     = cms.InputTag('simMuonRPCDigis'),
    ME0InputTag      = cms.InputTag('me0TriggerConvertedPseudoDigis'),
    DTInputTag       = cms.InputTag('simDtTriggerPrimitiveDigis'),

    EMTFHitTag       = cms.InputTag('simEmtfDigis'),
    EMTFUnpHitTag    = cms.InputTag('emtfStage2Digis'),

    EMTFTrackTag     = cms.InputTag('simEmtfDigisData'),
    EMTFUnpTrackTag  = cms.InputTag('emtfStage2Digis'),

    GMTMuonTag       = cms.InputTag('simGmtStage2Digis'),
    GMTUnpMuonTag    = cms.InputTag('gmtStage2Digis', 'Muon'),

    GENPartTag       = cms.InputTag('genParticles'),
    RecoMuonTag      = cms.InputTag('muons'),

    CSCSegmentTag    = cms.InputTag('cscSegments'),

    outFileName      = cms.string('MuonAODAnalyzer.root'),
    verbosity        = cms.untracked.int32(0),

    # Change defaults to enable/disable processing of various inputs
    enablePhase2     = cms.bool(False),

    useCSC           = cms.bool(False),
    useRPC           = cms.bool(False),
    useCPPF          = cms.bool(False),
    useGEM           = cms.bool(False),

    useIRPC          = cms.bool(False),
    useME0           = cms.bool(False),
    useDT            = cms.bool(False),


    useEMTFHits      = cms.bool(False),
    useEMTFUnpHits   = cms.bool(True),

    useEMTFTracks    = cms.bool(False),
    useEMTFUnpTracks = cms.bool(True),

    useGMTMuons      = cms.bool(False),
    useGMTUnpMuons   = cms.bool(False),

    useGENParts      = cms.bool(False),
    useRecoMuons      = cms.bool(False),
    useEventInfo     = cms.bool(False),

    useCSCSegments   = cms.bool(False),
    matchCSCSegments   = cms.bool(False),

    isReco           = cms.bool(False),

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
          useTrack = cms.string("tracker"),  # 'none' to use Candidate P4; or 'tracker', 'muon', 'global'
          useState = cms.string("atVertex"), # 'innermost' and 'outermost' require the TrackExtra
          useSimpleGeometry = cms.bool(True),
          useStation2 = cms.bool(True),
          fallbackToME1 = cms.bool(False),
          cosmicPropagationHypothesis = cms.bool(False),
          useMB2InOverlap = cms.bool(False),
          propagatorAlong = cms.ESInputTag("", "SteppingHelixPropagatorAlong"),
          propagatorAny = cms.ESInputTag("", "SteppingHelixPropagatorAny"),
          propagatorOpposite = cms.ESInputTag("", "SteppingHelixPropagatorOpposite")
    ),
  )