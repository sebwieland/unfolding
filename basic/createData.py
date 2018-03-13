from ROOT import *
import math

rnd = TRandom3()


def GenerateEvent(tau, minpt, maxpt, resA, resB):
        # generate Pt according to distribution  pow(ptGen,tau)
        # resA = resolution paramater ~ sqrt(pt)
        # resB = resolution paramater ~ pt
    t = rnd.Rndm()
    tau += 1.0
    if(tau == 0.0):
        x0 = math.log(minpt)
        ptGen = math.exp(t*(math.log(maxpt)-x0)+x0)
    else:
        x0 = pow(minpt, tau)
        ptGen = pow(t*(pow(maxpt, tau)-x0)+x0, 1./tau)

    # smearing in Pt with large asymmetric tail
    sigma = math.sqrt(resA*resA*ptGen+resB*resB*(ptGen*ptGen))
    ptObs = rnd.BreitWigner(ptGen, 1/sigma)

    pt = [ptGen, ptObs]
    return pt

def GenerateEventMass(mass, res):
    xGen=rnd.Gaus(mass,res);
    xReco=rnd.Gaus(xGen,res);
    x=[xGen,xReco]
    return x



tauMC = -1.5 	 # MC exponent
tauData = -1.5	 # Data exponent (steeper to illustrate Bin-by-Bin bias ??)
minpt = 100  # gen MinPt-Cut
maxpt = 800  # max MinPt-Cut
resA = 0.002  # resA = resolution paramater ~ sqrt(pt)
resB = 0.001 # resB = resolution paramater ~ pt


# print GenerateEvent(tauMC, minpt, maxpt, resA, resB)

nBinsGen = 20
nBinsReco = 40

xMin = 0
xMax = 1000

h_Data = TH1F("h_Data", "h_Data", nBinsReco, xMin, xMax)
h_Data.Sumw2()
h_Reco = TH1F("h_Reco", "h_Reco", nBinsReco, xMin, xMax)
h_Reco.Sumw2()
h_Gen = TH1F("h_Gen", "h_Gen", nBinsGen, xMin, xMax)
h_Gen.Sumw2()
MigrationMatrix = TH2F("MigrationMatrix", "MigrationMatrix",
                       nBinsReco, xMin, xMax, nBinsGen, xMin, xMax)
MigrationMatrix.Sumw2()
MigrationMatrixSquare = TH2F("MigrationMatrixSquare", "MigrationMatrixSquare",
                       5, xMin, xMax, 5, xMin, xMax)
MigrationMatrixSquare.Sumw2()

nEvents = 100000

for i in range(nEvents):
    MCevent = GenerateEvent(tauMC, minpt, maxpt, resA, resB)
    Dataevent = GenerateEvent(tauData, minpt, maxpt, resA, resB)
    h_Gen.Fill(MCevent[0])
    h_Reco.Fill(MCevent[1])
    h_Data.Fill(Dataevent[1])
    MigrationMatrix.Fill(MCevent[1], MCevent[0])
    MigrationMatrixSquare.Fill(MCevent[1], MCevent[0])


histofile = TFile("histofile.root", "RECREATE")
h_Gen.Write()
h_Reco.Write()
h_Data.Write()
MigrationMatrix.Write()
MigrationMatrixSquare.Write()
