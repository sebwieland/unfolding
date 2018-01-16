from ROOT import *
import math

# Open File
# f = TFile("/afs/desy.de/user/s/swieland/DM_ntuples/DMV_NNPDF30_Axial_Mphi-1000_Mchi-1_gSM-0p25_gDM-1p0_v2_13TeV-powheg/DMV_NNPDF30_Axial_Mphi-1000_Mchi-1_gSM-0p25_gDM-1p0_v2_13TeV-powheg_1_nominal_Tree.root")
tree = TChain("MVATree", "filechain")
tree.Add("/afs/desy.de/user/s/swieland/DM_ntuples/DMV_NNPDF30_Axial_Mphi-1000_Mchi-1_gSM-0p25_gDM-1p0_v2_13TeV-powheg/DMV_NNPDF30_Axial_Mphi-1000_Mchi-1_gSM-0p25_gDM-1p0_v2_13TeV-powheg_1_nominal_Tree.root")
tree.Add("/afs/desy.de/user/s/swieland/DM_ntuples/DMV_NNPDF30_Axial_Mphi-1000_Mchi-1_gSM-0p25_gDM-1p0_v2_13TeV-powheg/DMV_NNPDF30_Axial_Mphi-1000_Mchi-1_gSM-0p25_gDM-1p0_v2_13TeV-powheg_2_nominal_Tree.root")
tree.Add("/afs/desy.de/user/s/swieland/DM_ntuples/DMV_NNPDF30_Axial_Mphi-1000_Mchi-1_gSM-0p25_gDM-1p0_v2_13TeV-powheg/DMV_NNPDF30_Axial_Mphi-1000_Mchi-1_gSM-0p25_gDM-1p0_v2_13TeV-powheg_3_nominal_Tree.root")
tree.Add("/afs/desy.de/user/s/swieland/DM_ntuples/DMV_NNPDF30_Axial_Mphi-1000_Mchi-1_gSM-0p25_gDM-1p0_v2_13TeV-powheg/DMV_NNPDF30_Axial_Mphi-1000_Mchi-1_gSM-0p25_gDM-1p0_v2_13TeV-powheg_4_nominal_Tree.root")

# create File to Save Histos
f_histos = TFile("histos.root", "RECREATE")

# book histos
h_pt_MET = TH1F("h_pt_MET", "h_pt_MET", 50, 0, 1000)
h_pt_GenMET = TH1F("h_pt_GenMET", "h_pt_GenMET", 25, 0, 1000)
h_pt_dummyData = TH1F("h_pt_dummyData", "h_pt_dummyData", 50, 0, 1000)

A_pt = TH2F("A_pt", "A_pt", 50, 0, 1000, 25, 0, 1000)
h_pt_GenMET.Sumw2()
h_pt_GenMET.Sumw2()
A_pt.Sumw2()
# get Tree
# tree = chain.Get("MVATree")
# get #Entries
# print(tree)
n = tree.GetEntries()

# loop over all events
print n, " Events to Analyse"
for i in xrange(n):
    if i % 10000 == 0:
        print "analyzing event Nr. ", i
    # if i >30000: break
    tree.GetEntry(i)
    if tree.GenMETSelection == 1 and tree.GenBTagVetoSelection == 1 and tree.GenBTagVetoSelection == 1 and tree.GenMonoJetSelection == 1 :
    # if tree.GenMETSelection == 1 and tree.GenBTagVetoSelection == 1 and tree.GenBTagVetoSelection == 1 and tree.GenMonoJetSelection == 1 and tree.GenVertexSelection == 1:
        if i % 2 != 0:
            A_pt.Fill(tree.Evt_Pt_MET, tree.Evt_Pt_GenMET)
        if i % 2 == 0:
            h_pt_dummyData.Fill(tree.Evt_Pt_MET)
            h_pt_MET.Fill(tree.Evt_Pt_MET)
            h_pt_GenMET.Fill(tree.Evt_Pt_GenMET)


# end eventloop
h_pt_MET.Write()
h_pt_GenMET.Write()
A_pt.Write()
h_pt_dummyData.Write()

f_histos.Close()


raw_input("histos saved in histos.root \n Press any Key...")
