#include <iostream>
#include "TFile.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TUnfold.h"
#include "TUnfoldDensity.h"
#include "TH2D.h"
#include "TSpline.h"
#include "TGraph.h"
#include "math.h"
#include "TStyle.h"
// #include "gstyle.h"

using namespace std;

template<typename T>
std::vector<T> to_array(const std::string& s)
{
  std::vector<T> result;
  std::stringstream ss(s);
  std::string item;
  while(std::getline(ss, item, ',')) result.push_back(boost::lexical_cast<T>(item));
  return result;
}

void unfold(TFile* histos)
{
	gStyle->SetStatY(0.9);
	gStyle->SetStatX(0.3);

	//output File
	TFile* f_result = new  TFile("result.root", "RECREATE");

	//book histos
	TH1F* h_pt_MET = (TH1F*)histos->Get("h_pt_MET");
	TH1F* h_pt_GenMET = (TH1F*)histos->Get("h_pt_GenMET");
	TH1F* h_pt_dummyData = (TH1F*)histos->Get("h_pt_dummyData");
	TH2F* A_pt = (TH2F*)histos->Get("A_pt");

	//Draw Pt
	TCanvas* c_pt = new TCanvas("pt_MET", "pt_MET");
	c_pt->Divide(2, 1);
	c_pt->cd(1);
	h_pt_MET->Draw("hist");

	c_pt->cd(2);
	h_pt_GenMET->Draw("hist");
	c_pt->SaveAs("pt.pdf");

	//Draw Matrix
	TCanvas* c_A = new TCanvas("Matrix_pt", "Matrix_pt");
	c_A->cd();
	A_pt->Draw("COLZ");
	A_pt->SetXTitle("p_T Reco");
	A_pt->SetYTitle("p_T Gen");
	c_A->SaveAs("pt_matrix.pdf");

	//SetUp Unfolding
	Double_t biasScale = 0;
	TUnfoldDensity unfold(A_pt, TUnfold::kHistMapOutputVert, TUnfold::kRegModeDerivative);
	unfold.SetInput(h_pt_dummyData);

	//find best tau
	Int_t nScan = 40;
	Double_t tauMin = pow(10, -2);
	Double_t tauMax = pow(10, 2);
	Int_t iBest;
	TSpline *scanResult = 0;
	TGraph *lCurve = 0;

	iBest = unfold.ScanTau(nScan, tauMin, tauMax, &scanResult, TUnfoldDensity::kEScanTauRhoAvg, 0, 0, &lCurve);

	std::cout << "tau=" << unfold.GetTau() << "\n";
	std::cout << "chi**2=" << unfold.GetChi2A() << "+" << unfold.GetChi2L() << " / " << unfold.GetNdf() << "\n";

	//Graphs to Visualize best choice of Tau
	Double_t t[1], rho[1], x[1], y[1];
	scanResult->GetKnot(iBest, t[0], rho[0]);
	lCurve->GetPoint(iBest, x[0], y[0]);
	TGraph *bestRhoLogTau = new TGraph(1, t, rho);
	TGraph *bestLCurve = new TGraph(1, x, y);
	Double_t *tAll = new Double_t[nScan], *rhoAll = new Double_t[nScan];
	for (Int_t i = 0; i < nScan; i++) {
		scanResult->GetKnot(i, tAll[i], rhoAll[i]);
	}
	TGraph *knots = new TGraph(nScan, tAll, rhoAll);

	//do unfolding
	unfold.DoUnfold(unfold.GetTau(), h_pt_dummyData, biasScale);

	//GetOutput
	TH1 *pt_unfolded = unfold.GetOutput("pt_unfolded");
	pt_unfolded->Print();
	pt_unfolded->SetXTitle("p_T unfolded");
	TH1 *pt_unfoldedback = unfold.GetFoldedOutput("pt_unfoldedback");

	//Ratios für Closure Test
	TH1F *ratio_Genpt = (TH1F*)pt_unfolded->Clone("ratio_Genpt");
	ratio_Genpt->Divide(h_pt_GenMET);
	ratio_Genpt->SetXTitle("p_T unfolded/Gen");

	TH1F *ratio_foldedbackpt = (TH1F*)pt_unfoldedback->Clone("ratio_foldedbackpt");
	ratio_foldedbackpt->Divide(h_pt_dummyData);
	ratio_foldedbackpt->SetXTitle("p_T unfolded/Gen");


	//Draw Plots
	TCanvas* c_unfolded = new TCanvas("unfolded_pt", "unfolded_pt");
	c_unfolded->Divide(3, 3);
	c_unfolded->cd(1);
	pt_unfolded->Draw("hist");
	pt_unfolded->Write();

	c_unfolded->cd(2);
	h_pt_GenMET->Draw("hist");
	h_pt_GenMET->Write();

	c_unfolded->cd(3);
	ratio_Genpt->Draw("hist");
	ratio_Genpt->Write();


	c_unfolded->cd(4);
	pt_unfoldedback->Draw("hist");
	pt_unfoldedback->Write();

	c_unfolded->cd(5);
	h_pt_dummyData->Draw("hist");
	h_pt_dummyData->Write();

	c_unfolded->cd(6);
	ratio_foldedbackpt->Draw("hist");
	ratio_foldedbackpt->Write();

	c_unfolded->cd(7);
	lCurve->Draw("AL");
	bestLCurve->SetMarkerColor(kRed);
	bestLCurve->Draw("*");
	lCurve->Write();

	c_unfolded->cd(8);
	scanResult->Draw();
	knots->Draw("*");
	bestRhoLogTau->SetMarkerColor(kRed);
	bestRhoLogTau->Draw("*");
	scanResult->Write();

	c_unfolded->SaveAs("unfolded_pt.pdf");
	c_unfolded->Write();

	//Draw unfolded Only
	TCanvas* unfolded = new TCanvas("unfolded", "unfolded");
	unfolded->Divide(2, 2);
	unfolded->cd(1);
	pt_unfolded->Draw("hist");

	unfolded->cd(3);
	h_pt_GenMET-> Draw("hist");

	unfolded->cd(2);
	ratio_Genpt->Draw("hist");

	unfolded->SaveAs("unfolded.pdf");

	//Draw foldedback only
	TCanvas* foldedback = new TCanvas("foldedback", "foldedback");
	foldedback->Divide(2, 2);
	foldedback->cd(1);
	pt_unfoldedback->Draw("hist");

	foldedback->cd(3);
	h_pt_dummyData->Draw("hist");

	foldedback->cd(2);
	ratio_foldedbackpt->Draw("hist");

	foldedback->SaveAs("foldedback.pdf");

	//Draw Tau Only
	TCanvas* tau = new TCanvas("tau", "tau");
	tau->cd();
	scanResult->Draw();
	knots->Draw("*");
	bestRhoLogTau->SetMarkerColor(kRed);
	bestRhoLogTau->Draw("*");
	tau->SaveAs("tau.pdf");
	tau->Write();

	f_result->Close();


}

int main()
{
	TFile* histos = new TFile("histos.root");
	unfold(histos);
}