## This is a tool for analyzing AOD files and producing L1T muon ntuples.  

**NOTE:** Before setting up, check the CMSSW release used to create the AOD file you want to process. Older releases may not work! You can find the release in [DAS](https://cmsweb.cern.ch/das/), e.g.: 
```
release dataset=/Cosmics/Run2024I-PromptReco-v1/AOD
```

Setup
-----
```
cmsrel CMSSW_14_0_16
cd CMSSW_14_0_16/src
cmsenv
voms-proxy-init --voms cms --valid 24:00:00

git clone https://github.com/muonDPG/MuonAODAnalyzer.git
scram b -j8
```

Run
-----
To run locally on 1 file (testing):
```
cmsRun L1TMuonAODAnalyzer/pset_files/run3_data.py
```
To submit to CRAB:
```
cd L1TMuonAODAnalyzer/pset_files/
crab submit
```

Collections
-----------
- ```l1t::Muon``` objects  
- ```reco::Muon``` objects  
- ```reco::Track``` objects  
- BMTF ```l1t::RegionalMuonCand``` objects   

Useful links
-------------
CRAB tutorial: https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookCRAB3Tutorial  
CRAB config file: https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRAB3ConfigurationFile  
CRAB exit codes: https://twiki.cern.ch/twiki/bin/view/CMSPublic/JobExitCodes  


## Datasets 2024

### Cosmics
- /Cosmics/Run2024A-PromptReco-v1/AOD
- /Cosmics/Run2024B-PromptReco-v1/AOD
- /Cosmics/Run2024C-PromptReco-v1/AOD
- /Cosmics/Run2024D-PromptReco-v1/AOD
- /Cosmics/Run2024E-PromptReco-v1/AOD
- /Cosmics/Run2024E-PromptReco-v2/AOD
- /Cosmics/Run2024F-PromptReco-v1/AOD
- /Cosmics/Run2024G-PromptReco-v1/AOD
- /Cosmics/Run2024H-PromptReco-v1/AOD
- /Cosmics/Run2024I-PromptReco-v1/AOD
- /Cosmics/Run2024I-PromptReco-v2/AOD
- /Cosmics/Run2024J-PromptReco-v1/AOD

### Muon
- /Muon0/Run2024A-PromptReco-v1/AOD
- /Muon0/Run2024B-PromptReco-v1/AOD
- /Muon0/Run2024C-PromptReco-v1/AOD
- /Muon0/Run2024D-PromptReco-v1/AOD
- /Muon0/Run2024E-PromptReco-v1/AOD
- /Muon0/Run2024E-PromptReco-v2/AOD
- /Muon0/Run2024F-PromptReco-v1/AOD
- /Muon0/Run2024G-PromptReco-v1/AOD
- /Muon0/Run2024H-PromptReco-v1/AOD
- /Muon0/Run2024I-PromptReco-v1/AOD
- /Muon0/Run2024I-PromptReco-v2/AOD
- /Muon0/Run2024J-PromptReco-v1/AOD
- /Muon1/Run2024A-PromptReco-v1/AOD
- /Muon1/Run2024B-PromptReco-v1/AOD
- /Muon1/Run2024C-PromptReco-v1/AOD
- /Muon1/Run2024D-PromptReco-v1/AOD
- /Muon1/Run2024E-PromptReco-v1/AOD
- /Muon1/Run2024E-PromptReco-v2/AOD
- /Muon1/Run2024F-PromptReco-v1/AOD
- /Muon1/Run2024G-PromptReco-v1/AOD
- /Muon1/Run2024H-PromptReco-v1/AOD
- /Muon1/Run2024I-PromptReco-v1/AOD
- /Muon1/Run2024I-PromptReco-v2/AOD
- /Muon1/Run2024J-PromptReco-v1/AOD

### JetMET
- /JetMET0/Run2024A-PromptReco-v1/AOD
- /JetMET0/Run2024B-PromptReco-v1/AOD
- /JetMET0/Run2024C-PromptReco-v1/AOD
- /JetMET0/Run2024D-PromptReco-v1/AOD
- /JetMET0/Run2024E-PromptReco-v1/AOD
- /JetMET0/Run2024E-PromptReco-v2/AOD
- /JetMET0/Run2024F-PromptReco-v1/AOD
- /JetMET0/Run2024G-PromptReco-v1/AOD
- /JetMET0/Run2024H-PromptReco-v1/AOD
- /JetMET0/Run2024I-PromptReco-v1/AOD
- /JetMET0/Run2024I-PromptReco-v2/AOD
- /JetMET0/Run2024J-PromptReco-v1/AOD
- /JetMET1/Run2024A-PromptReco-v1/AOD
- /JetMET1/Run2024B-PromptReco-v1/AOD
- /JetMET1/Run2024C-PromptReco-v1/AOD
- /JetMET1/Run2024D-PromptReco-v1/AOD
- /JetMET1/Run2024E-PromptReco-v1/AOD
- /JetMET1/Run2024E-PromptReco-v2/AOD
- /JetMET1/Run2024F-PromptReco-v1/AOD
- /JetMET1/Run2024G-PromptReco-v1/AOD
- /JetMET1/Run2024H-PromptReco-v1/AOD
- /JetMET1/Run2024I-PromptReco-v1/AOD
- /JetMET1/Run2024I-PromptReco-v2/AOD
- /JetMET1/Run2024J-PromptReco-v1/AOD

## Datasets 2025

### Cosmics
- /Cosmics/Run2025A-PromptReco-v1/AOD
- /Cosmics/Run2025A-PromptReco-v2/AOD
- /Cosmics/Run2025B-PromptReco-v1/AOD
- /Cosmics/Run2025C-PromptReco-v1/AOD
- /Cosmics/Run2025C-PromptReco-v2/AOD
- /Cosmics/Run2025D-PromptReco-v1/AOD
- /Cosmics/Run2025E-PromptReco-v1/AOD
- /Cosmics/Run2025F-PromptReco-v1/AOD
- /Cosmics/Run2025F-PromptReco-v2/AOD

### Muon
- /Muon0/Run2025B-PromptReco-v1/AOD
- /Muon0/Run2025C-PromptReco-v1/AOD
- /Muon0/Run2025C-PromptReco-v2/AOD
- /Muon0/Run2025D-PromptReco-v1/AOD
- /Muon0/Run2025E-PromptReco-v1/AOD
- /Muon0/Run2025F-PromptReco-v1/AOD
- /Muon0/Run2025F-PromptReco-v2/AOD
- /Muon1/Run2025B-PromptReco-v1/AOD
- /Muon1/Run2025C-PromptReco-v1/AOD
- /Muon1/Run2025C-PromptReco-v2/AOD
- /Muon1/Run2025D-PromptReco-v1/AOD
- /Muon1/Run2025E-PromptReco-v1/AOD
- /Muon1/Run2025F-PromptReco-v1/AOD
- /Muon1/Run2025F-PromptReco-v2/AOD

### JetMET
- /JetMET0/Run2025B-PromptReco-v1/AOD
- /JetMET0/Run2025C-PromptReco-v1/AOD
- /JetMET0/Run2025C-PromptReco-v2/AOD
- /JetMET0/Run2025D-PromptReco-v1/AOD
- /JetMET0/Run2025E-PromptReco-v1/AOD
- /JetMET0/Run2025F-PromptReco-v1/AOD
- /JetMET0/Run2025F-PromptReco-v2/AOD
- /JetMET1/Run2025B-PromptReco-v1/AOD
- /JetMET1/Run2025C-PromptReco-v1/AOD
- /JetMET1/Run2025C-PromptReco-v2/AOD
- /JetMET1/Run2025D-PromptReco-v1/AOD
- /JetMET1/Run2025E-PromptReco-v1/AOD
- /JetMET1/Run2025F-PromptReco-v1/AOD
- /JetMET1/Run2025F-PromptReco-v2/AOD