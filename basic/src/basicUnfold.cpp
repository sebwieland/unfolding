#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"


#include "../include/helpers.hpp"

using namespace std;

int main(){
	//Reset Output ROOTFILE
	std::remove("output.root");
	TFile *histofile = new TFile("histofile.root", "open");
	TH1F* Gen = (TH1F*)histofile->Get("h_Gen");
	TH1F* Reco = (TH1F*)histofile->Get("h_Reco");
	TH1F* Data = (TH1F*)histofile->Get("h_Data");
	Draw1D(Gen, "Gen", "Gen Pt", "#events");
	Draw1D(Reco, "Reco", "Reco Pt", "#events");
	Draw1D(Data, "Data", "Data Pt", "#events");

	TH2F* MigrationMatrix = (TH2F*)histofile->Get("MigrationMatrix");
	Draw2D(MigrationMatrix, "MigrationMatrix", false, "Reco", "Gen");






}