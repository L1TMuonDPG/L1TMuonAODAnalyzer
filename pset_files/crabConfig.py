import CRABClient
from CRABClient.UserUtilities import config

config = config()

config.General.requestName = 'JetMET1_2025G'
config.General.workArea = 'crab'
config.General.transferOutputs = True

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'run3_data.py'
config.JobType.numCores = 8

config.Data.inputDataset = '/JetMET1/Run2025G-PromptReco-v1/AOD' # /Muon0/Run2024C-PromptReco-v1/AOD
config.Data.inputDBS = 'global'
# config.Data.useParent = True
# config.Data.partialDataset = True
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 10
# config.Data.publication = True
config.Data.outputDatasetTag = 'JetMET1_2025G'
config.Site.storageSite = 'T3_CH_CERNBOX'
