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
#include "TLegend.h"
#include "THStack.h"
#include "TLine.h"

#include "../include/helpers.hpp"

template<typename Base, typename T>
inline bool instanceof(const T *ptr) {
	return dynamic_cast<const Base*>(ptr) != nullptr;
}

void SetHistoStyle(TH1* histo, int color, bool filled) {
	if (instanceof<TH1F>(histo)) histo->SetStats(false);
	histo->GetYaxis()->SetTitleOffset(1.4);
	histo->GetXaxis()->SetTitleOffset(1.2);
	histo->GetYaxis()->SetTitleSize(0.05);
	histo->GetXaxis()->SetTitleSize(0.05);
	histo->GetYaxis()->SetLabelSize(0.05);
	histo->GetXaxis()->SetLabelSize(0.05);
	histo->SetMarkerColor(color);
	if (filled) {
		histo->SetLineColor( kBlack );
		histo->SetFillColor( color );
		histo->SetLineWidth(2);
	}
	else {
		histo->SetLineColor(color);
		histo->SetLineWidth(2);
	}
}

TCanvas* getCanvas(TString name, bool ratiopad) {
	if (ratiopad) {
		TCanvas* c = new TCanvas(name, name, 1024, 1024);
		c->Divide(1, 2);
		c->cd(1)->SetPad(0., 0.3, 1.0, 1.0);
		c->cd(1)->SetBottomMargin(0.0);
		c->cd(2)->SetPad(0., 0.0, 1.0, 0.3);
		c->cd(2)->SetTopMargin(0.0);
		c->cd(1)->SetTopMargin(0.07);
		c->cd(2)->SetBottomMargin(0.4);
		c->cd(1)->SetRightMargin(0.05);
		c->cd(1)->SetLeftMargin(0.15);
		c->cd(2)->SetRightMargin(0.05);
		c->cd(2)->SetLeftMargin(0.15);
		c->cd(2)->SetTicks(1, 1);
		c->cd(1)->SetTicks(1, 1);
		return c;
	}
	else {
		TCanvas* c = new TCanvas(name, name, 1024, 768);
		c->SetRightMargin(0.05);
		c->SetTopMargin(0.07);
		c->SetLeftMargin(0.15);
		c->SetBottomMargin(0.15);
		c->SetTicks(1, 1);
		return c;
	}
}

TLegend* getLegend() {
	TLegend* legend = new TLegend(0.7, 0.7, 0.92, 0.9);
	legend->SetBorderSize(0);
	legend->SetLineStyle(0);
	legend->SetTextFont(42);
	legend->SetTextSize(0.05);
	legend->SetFillStyle(0);
	return legend;
}
void Draw1D(TH1* hist, TString name, TString xlabel, TString ylabel) {
	TFile *output = new TFile("output.root", "update");
	TCanvas* c = getCanvas(name, false);

	hist->Draw("histe");
	hist->SetTitle(name);

	if (xlabel == "none") {
		hist->SetXTitle(name);
	}
	else hist-> SetXTitle(xlabel);
	hist->SetYTitle(ylabel);

	c->SaveAs("pdfs/" + name + ".pdf");
	c->SaveAs("pdfs/" + name + ".png");
	c->Write();
	output->Close();
}

void Draw2D(TH2* hist, TString name, bool log, TString xlabel, TString ylabel) {
	TFile *output = new TFile("output.root", "update");
	TCanvas* c = new TCanvas(name, name);
	gStyle->SetStatY(0.9);
	gStyle->SetStatX(0.9);

	hist->Draw("COLZ1");
	hist->SetTitle(name);
	if (log) gPad->SetLogz();

	if (xlabel == "none") {
		hist->SetXTitle(name + "_Reco");
	}
	else hist-> SetXTitle(xlabel);
	if (ylabel == "none") {
		hist->SetYTitle(name + "_Gen");
	}
	else hist-> SetYTitle(ylabel);

	c->SaveAs("pdfs/" + name + ".pdf");
	c->SaveAs("pdfs/" + name + ".png");
	c->Write();
	output->Close();
}

void DrawRatio(TH1* hist1, TH1* hist2, TString name, TString xlabel, TString ylabel) {
	TFile *output = new TFile("output.root", "update");
	TCanvas* c = getCanvas(name, false);
	TH1F* ratio = (TH1F*) hist1->Clone();
	ratio->Divide(hist2);
	ratio->Draw("histe");
	ratio->SetTitle(name);

	if (xlabel == "none") {
		ratio->SetXTitle(name);
	}
	else ratio-> SetXTitle(xlabel);

	ratio-> SetYTitle(ylabel);
	c->Update();
	TLine *line = new TLine(c->GetUxmin(), 1.0, c->GetUxmax(), 1.0);
	line->SetLineColor(kRed);
	line->Draw();

	c->SaveAs("pdfs/" + name + ".pdf");
	c->SaveAs("pdfs/" + name + ".png");
	c->Write();
	output->Close();
}

void DrawDataMC(TH1* data, std::vector<TH1*> MC, std::vector<std::string> names, TString name, bool log, bool normalize, TString xlabel, TString ylabel) {
	TFile *output = new TFile("output.root", "update");
	TCanvas* c = getCanvas(name, true);
	TLegend* legend = getLegend();
	legend->AddEntry(data, "Data", "P");
	gStyle->SetErrorX(0.);
	if (log) gPad->SetLogy();

	THStack* stack = new THStack(name, name);
	int fillcolor = 2;
	int index = 0;
	for (auto const& mc : MC) {
		SetHistoStyle(mc, fillcolor, true);
		mc->SetFillColor(fillcolor);
		if (normalize) mc ->Scale(1 / (MC.size()* mc->Integral()));
		stack->Add(mc);
		legend->AddEntry(mc, TString(names.at(index)), "F");
		index += 1;
		fillcolor += 1;
	}
	TH1F* lastStack = (TH1F*) (TH1*)stack->GetStack()->Last();
	if (normalize) 	data->Scale(1 / data->Integral());

	float max_data = data->GetMaximum();
	float max_Stack = lastStack->GetMaximum();
	if (max_data > max_Stack) stack->SetMaximum(max_data);
	else stack->SetMaximum(max_Stack);

	stack->Draw("histe");
	data->Draw("samee");
	legend->Draw("same");
	data->SetStats(false);


	data->SetMarkerStyle(20);
	// data->SetMarkerSize();
	if (xlabel == "none") {
		data->SetXTitle(name);
	}
	else data-> SetXTitle(xlabel);

	c->cd(2);
	TH1* ratio = (TH1*) data->Clone();
	ratio->Divide((TH1*)stack->GetStack()->Last());
	ratio->Draw("E0");
	ratio->GetYaxis()->SetRangeUser(0.5, 1.5);
	ratio->GetYaxis()->SetTitle("#frac{Data}{MC Sample}");
	ratio->GetXaxis()->SetLabelSize(ratio->GetXaxis()->GetLabelSize() * 2.4);
	ratio->GetYaxis()->SetLabelSize(ratio->GetYaxis()->GetLabelSize() * 2.4);
	ratio->GetXaxis()->SetTitleSize(ratio->GetXaxis()->GetTitleSize() * 3);
	ratio->GetYaxis()->SetTitleSize(ratio->GetYaxis()->GetTitleSize() * 1.5);
	ratio->GetYaxis()->SetTitleOffset(0.9);
	ratio->GetYaxis()->SetNdivisions(505);
	ratio->SetTitle("");

	c->Update();
	TLine *line = new TLine(c->cd(2)->GetUxmin(), 1.0, c->cd(2)->GetUxmax(), 1.0);
	line->SetLineColor(kBlack);
	line->Draw();

	c->cd(1);


	c->SaveAs("pdfs/" + name + "_stacked.pdf");
	c->SaveAs("pdfs/" + name + "_stacked.png");
	c->Write();
	output->Close();
}


void VisualizeTau(int nScan, int iBest, TSpline* scanResult, TGraph*lCurve, TString name) {
	// int iBest = get<0>(tuple);
	// TSpline* scanResult = get<1>(tuple);
	// TGraph* lCurve = get<2>(tuple);

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

	TFile *output = new TFile("output.root", "UPDATE");
	//Draw Tau Graphs
	TCanvas* tau = new TCanvas("tau_" + name, "tau_" + name);
	tau->cd();
	scanResult->Draw();
	knots->Draw("*");
	bestRhoLogTau->SetMarkerColor(kRed);
	bestRhoLogTau->Draw("*");
	tau->SaveAs("pdfs/" + name + ".pdf");
	tau->SaveAs("pdfs/" + name + ".png");
	tau->Write();
	output->Close();
}