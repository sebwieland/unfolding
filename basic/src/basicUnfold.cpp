#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TUnfoldDensity.h"
#include <TString.h>
#include "TStopwatch.h"
#include "TH1F.h"

#include "../include/helpers.hpp"
#include "../include/TreeToClasses.hpp"

#include "TMVA/DataLoader.h"
#include "TMVA/Factory.h"
#include "TMVA/TMVAGui.h"
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"
#include "TMVA/TMVAMultiClassGui.h"


using namespace std;

int
main(int argc, char* argv[])
{
	// Reset Output ROOTFILE
	std::string arg = argv[1];
	std::remove("output.root");
	TFile* histofile = new TFile("histofile.root", "open");
	TH1F* Gen = (TH1F*)histofile->Get("h_Gen");
	TH1F* Reco = (TH1F*)histofile->Get("h_Reco");
	TH1F* Data = (TH1F*)histofile->Get("h_Data");
	Draw1D(Gen, "Gen", "Gen Pt", "#events");
	Draw1D(Reco, "Reco", "Reco Pt", "#events");
	Draw1D(Data, "Data", "Data Pt", "#events");
	DrawDataMC(Data, { Reco }, { "Reco" }, "DatavsReco", false, false, "Pt", "#events");

	TH2F* MigrationMatrix = (TH2F*)histofile->Get("MigrationMatrix");
	Draw2D(MigrationMatrix, "MigrationMatrix", true, "Reco", "Gen");

	int nBins_Gen = MigrationMatrix->GetNbinsY();
	histofile->Close();

	int nBins = 10;
	int xmin = 0;
	int xmax = 1000;
	int bindwidth = (xmax - xmin) / nBins;


	if (arg == "train") {
		cout << "Unfolding using ML Techniques" << endl;
		TFile* inFile = TFile::Open("TreeFile.root", "OPEN");
		TChain* chain = (TChain*) inFile->Get("Tree");

		TreeToClasses treeToClasses = TreeToClasses(chain, {"Gen"}, {"Reco", "Gen"}, {nBins}, {{xmin, xmax}, {xmin, xmax}});
		treeToClasses.BinVars();

		// This loads the library
		TMVA::Tools::Instance();

		// Create a new root output file.
		TString outfileName = "TMVAMulticlass.root";
		TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

		TMVA::Factory *factory = new TMVA::Factory( "TMVAMulticlass", outputFile,
		        "V=True:!Silent:Color:DrawProgressBar:Transformations=:AnalysisType=multiclass" );
		// Create DataLoader
		TMVA::DataLoader *dataloader = new TMVA::DataLoader("dataset");
		// Define the input variables used for the MVA training
		dataloader->AddVariable( "Reco", 'F' );

		// Load the signal and background event samples from ROOT trees
		TFile *input(0);
		TString fname = TString("classes.root");
		input = TFile::Open( fname );

		TTree *Tree  = (TTree*)input->Get("UnfoldTree");

		gROOT->cd( outfileName + TString(":/") );
		for (unsigned int i = 1; i <= nBins; i++) {
			TString index = std::to_string(i);
			dataloader->AddTree(Tree, "Bin" + index, 1.0, TCut("Gen_classes ==" + index));
		}

		dataloader->PrepareTrainingAndTestTree( "", "SplitMode=Random:NormMode=NumEvents:!V" );

		// General layout.
		TString layoutString("Layout=TANH|100,TANH|10,LINEAR");
		// Training strategies.
		TString training0("LearningRate=1e-1,Momentum=0.9,Repetitions=10,"
		                  "ConvergenceSteps=20,BatchSize=256,TestRepetitions=10,"
		                  "WeightDecay=1e-4,Regularization=L2,"
		                  "DropConfig=0.0+0.5+0.5, Multithreading=True");
		TString training1("LearningRate=1e-4,Momentum=0.9,Repetitions=10,"
		                  "ConvergenceSteps=20,BatchSize=256,TestRepetitions=10,"
		                  "WeightDecay=1e-4,Regularization=L2,"
		                  "DropConfig=0.0+0.0+0.0, Multithreading=True");
		TString training2("LearningRate=1e-4,Momentum=0.0,Repetitions=10,"
		                  "ConvergenceSteps=20,BatchSize=256,TestRepetitions=10,"
		                  "WeightDecay=1e-4,Regularization=L2,"
		                  "DropConfig=0.0+0.0+0.0, Multithreading=True");
		TString trainingStrategyString("TrainingStrategy=");
		trainingStrategyString += training0 + "|" + training1 + "|" + training2;
		TString nnOptions("H=True:V=True:ErrorStrategy=CROSSENTROPY:VarTransform=N:"
		                  "WeightInitialization=XAVIERUNIFORM");
		nnOptions.Append(":");
		nnOptions.Append(layoutString);
		nnOptions.Append(":");
		nnOptions.Append(trainingStrategyString);

		// Cuda implementation.
		TString gpuOptions = nnOptions + ":Architecture=GPU";
		factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN_GPU", gpuOptions);

		// Multi-core CPU implementation.
		// TString cpuOptions = dnnOptions + ":Architecture=CPU";
		// factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN CPU", cpuOptions);


		// Train MVAs using the set of training events
		factory->TrainAllMethods();

		// Evaluate all MVAs using the set of test events
		factory->TestAllMethods();

		// Evaluate and compare performance of all configured MVAs
		factory->EvaluateAllMethods();

		// --------------------------------------------------------------

		// Save the output
		outputFile->Close();

		std::cout << "==> Wrote root file: " << outputFile->GetName() << std::endl;
		std::cout << "==> TMVAMulticlass is done!" << std::endl;

		delete factory;
		delete dataloader;

		// Launch the GUI for the root macros
		// if (!gROOT->IsBatch()) TMVAMultiClassGui( outfileName );

		// Launch the GUI for the root macros
		// gROOT->SetBatch(false);
		// if (!gROOT->IsBatch()) {
		// 	gROOT->ProcessLine("TMVA::TMVAGui()");
		// 	do
		// 	{
		// 		cout << '\n' << "Press a key to continue...";
		// 	} while (cin.get() != '\n');
		// }
	}
	if (arg == "predict") {

		std::cout << "apply Network" << std::endl;

		// create the Reader object
		TMVA::Reader *reader = new TMVA::Reader( "!Color:!Silent" );
		// create a set of variables and declare them to the reader
		// - the variable names must corresponds in name and type to
		// those given in the weight file(s) that you use
		Float_t Reco;
		Float_t Gen;

		reader->AddVariable( "Reco", &Reco );
		// book the MVA methods
		TString dir    = "dataset/weights/";
		TString prefix = "TMVAMulticlass";
		TString methodName = TString("DNN_GPU");
		TString weightfile = dir + prefix + TString("_") + methodName + TString(".weights.xml");
		reader->BookMVA( methodName, weightfile );
		// // book output histograms
		TH1F *h_RecoPred = new TH1F( "DNN_GPU_Reco",    "DNN_GPU_Reco",    nBins, xmin, xmax );
		h_RecoPred->Sumw2();
		TH1F *h_Gen = new TH1F( "GenDNN",    "GenDNN",    nBins, xmin, xmax );
		h_Gen->Sumw2();

		TFile *input(0);
		TString fname = "./TreeFile.root";
		if (!gSystem->AccessPathName( fname )) {
			input = TFile::Open( fname ); // check if file in local directory exists
		}
		if (!input) {
			std::cout << "ERROR: could not open data file, please generate example data first!" << std::endl;
			exit(1);
		}
		std::cout << "--- TMVAMulticlassApp : Using input file: " << input->GetName() << std::endl;
		// // prepare the tree
		// // - here the variable names have to corresponds to your tree
		// // - you can use the same variables as above which is slightly faster,
		// //   but of course you can use different ones and copy the values inside the event loop
		TTree* theTree = (TTree*)input->Get("Tree");
		theTree->SetBranchAddress( "data", &Reco );
		theTree->SetBranchAddress( "dataGen", &Gen );

		std::cout << "--- Processing: " << theTree->GetEntries() << " events" << std::endl;
		TStopwatch sw;
		sw.Start();
		for (Long64_t ievt = 0; ievt < theTree->GetEntries(); ievt++) {
			if (ievt % 10000 == 0) {
				std::cout << "--- ... Processing event: " << ievt << std::endl;
			}
			theTree->GetEntry(ievt);
			std::vector<float> pred = reader->EvaluateMulticlass( "DNN_GPU" );
			// for (auto& pr : pred) {
				// std::cout << pr << std::endl;
			// }
			auto predclass =  max_element (pred.begin(), pred.end());
			auto pos = std::distance(pred.begin(), predclass); 
			std::cout << pos << std::endl;

			h_RecoPred->Fill((pos) * bindwidth);
			h_Gen->Fill(Gen);
		}
		// // get elapsed time
		sw.Stop();
		std::cout << "--- End of event loop: "; sw.Print();
		TFile *histofile  = new TFile( "histofile.root", "UPDATE" );
		h_RecoPred->Write();
		h_Gen->Write();
		histofile->Close();
		delete reader;
		std::cout << "==> TMVAClassificationApplication is done!" << std::endl << std::endl;
		DrawDataMC(h_RecoPred, { h_Gen }, { "Gen" }, "PredvsGen", false, false, "Pt", "#events");


	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (arg == "t") {
		cout << "Unfolding using TUnfold" << endl;
		int xmin = 0;
		int xmax = 1000;
		int nScan = 50;
		double tauMin = 0.04;
		double tauMax = 2;

		cout << "(1) Unfolding using no Regularisation and no Area Constraint"
		     << endl;

		TUnfoldDensity* unfold1 =
		    new TUnfoldDensity(MigrationMatrix,
		                       TUnfold::kHistMapOutputVert,
		                       TUnfold::kRegModeNone,
		                       TUnfold::kEConstraintNone,
		                       TUnfoldDensity::kDensityModeBinWidthAndUser,
		                       0,
		                       0,
		                       0,
		                       "*[UOB]");
		unfold1->SetInput(Data);
		unfold1->DoUnfold(0, Data);
		TH1* unfolded1 = unfold1->GetOutput("unfolded1");
		DrawDataMC(unfolded1,
		{ Gen },
		{ "Gen" },
		"UnfoldedvsGen1",
		false,
		false,
		"Pt",
		"#events");

		TH2* ErrorMatrix1 = unfold1->GetEmatrixTotal("ErrorMatrix1");
		Draw2D(ErrorMatrix1, "CovMatrix1", false);
		TH1D* UnfoldedTotalError1 =
		    new TH1D("TotalError1", "Pt", nBins_Gen, xmin, xmax);
		for (Int_t bin = 1; bin <= nBins_Gen; bin++) {
			UnfoldedTotalError1->SetBinContent(bin, unfolded1->GetBinContent(bin));
			UnfoldedTotalError1->SetBinError(
			    bin, sqrt(ErrorMatrix1->GetBinContent(bin, bin)));
		}
		DrawDataMC(UnfoldedTotalError1,
		{ Gen },
		{ "Gen" },
		"UnfoldedEvsGen1",
		false,
		false,
		"Pt",
		"#events");

		TH2* L1 = unfold1->GetL("L1");
		Draw2D(L1, "L1", false);
		TH2* RhoTotal1 = unfold1->GetRhoIJtotal("RhoTotal1");
		Draw2D(RhoTotal1, "RhoTotal1", false, "Reco", "Gen");

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		cout << "(2) Unfolding using size Regularisation and no Area Constraint"
		     << endl;

		TUnfoldDensity* unfold2 =
		    new TUnfoldDensity(MigrationMatrix,
		                       TUnfold::kHistMapOutputVert,
		                       TUnfold::kRegModeSize,
		                       TUnfold::kEConstraintNone,
		                       TUnfoldDensity::kDensityModeBinWidthAndUser,
		                       0,
		                       0,
		                       0,
		                       "*[UOB]");
		unfold2->SetInput(Data);

		cout << "Finding BestTau using avg correlation" << endl;
		int iBest2avg;
		TSpline* scanResult2avg = 0;
		TGraph* lCurve2avg = 0;
		iBest2avg = unfold2->ScanTau(nScan,
		                             tauMin,
		                             tauMax,
		                             &scanResult2avg,
		                             TUnfoldDensity::kEScanTauRhoAvgSys,
		                             0,
		                             0,
		                             &lCurve2avg);
		std::cout << " Best tau=" << unfold2->GetTau() << "\n";
		std::cout << "iBest2avg=" << iBest2avg << "\n";
		VisualizeTau(nScan, iBest2avg, scanResult2avg, lCurve2avg, "tau2_avg");

		TH1* unfolded2 = unfold2->GetOutput("unfolded2");
		DrawDataMC(unfolded2,
		{ Gen },
		{ "Gen" },
		"UnfoldedvsGen2_avgtau",
		false,
		false,
		"Pt",
		"#events");

		TH2* ErrorMatrix2 = unfold2->GetEmatrixTotal("ErrorMatrix2");
		Draw2D(ErrorMatrix2, "CovMatrix2_avgtau", false);
		TH1D* UnfoldedTotalError2 =
		    new TH1D("TotalError2", "Pt", nBins_Gen, xmin, xmax);
		for (Int_t bin = 1; bin <= nBins_Gen; bin++) {
			UnfoldedTotalError2->SetBinContent(bin, unfolded2->GetBinContent(bin));
			UnfoldedTotalError2->SetBinError(
			    bin, sqrt(ErrorMatrix2->GetBinContent(bin, bin)));
		}
		DrawDataMC(UnfoldedTotalError2,
		{ Gen },
		{ "Gen" },
		"UnfoldedEvsGen2_avgtau",
		false,
		false,
		"Pt",
		"#events");
		TH2* L2 = unfold2->GetL("L2");
		Draw2D(L2, "L2", false);
		TH2* RhoTotal2 = unfold2->GetRhoIJtotal("RhoTotal2_avgtau");
		Draw2D(RhoTotal2, "RhoTotal2_avgtau", false, "Reco", "Gen");

		cout << "Finding BestTau using max correlation" << endl;
		int iBest2max;
		TSpline* scanResult2max = 0;
		TGraph* lCurve2max = 0;
		iBest2max = unfold2->ScanTau(nScan,
		                             tauMin,
		                             tauMax,
		                             &scanResult2max,
		                             TUnfoldDensity::kEScanTauRhoMax,
		                             0,
		                             0,
		                             &lCurve2max);
		std::cout << " Best tau=" << unfold2->GetTau() << "\n";
		std::cout << "iBest2max=" << iBest2max << "\n";
		VisualizeTau(nScan, iBest2max, scanResult2max, lCurve2max, "tau2_max");

		TH1* unfolded2max = unfold2->GetOutput("unfolded2max");
		DrawDataMC(unfolded2max,
		{ Gen },
		{ "Gen" },
		"UnfoldedvsGen2_maxtau",
		false,
		false,
		"Pt",
		"#events");

		TH2* ErrorMatrix2max = unfold2->GetEmatrixTotal("ErrorMatrix2max");
		Draw2D(ErrorMatrix2max, "CovMatrix2_maxtau", false);
		TH1D* UnfoldedTotalError2max =
		    new TH1D("TotalError2max", "Pt", nBins_Gen, xmin, xmax);
		for (Int_t bin = 1; bin <= nBins_Gen; bin++) {
			UnfoldedTotalError2max->SetBinContent(bin, unfolded2->GetBinContent(bin));
			UnfoldedTotalError2max->SetBinError(
			    bin, sqrt(ErrorMatrix2->GetBinContent(bin, bin)));
		}
		DrawDataMC(UnfoldedTotalError2max,
		{ Gen },
		{ "Gen" },
		"UnfoldedEvsGen2_maxtau",
		false,
		false,
		"Pt",
		"#events");

		TH2* L2max = unfold2->GetL("L2max");
		Draw2D(L2max, "L2maxtau", false);
		TH2* RhoTotal2_maxtau = unfold2->GetRhoIJtotal("RhoTotal2_maxtau");
		Draw2D(RhoTotal2_maxtau, "RhoTotal2_maxtau", false, "Reco", "Gen");

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		cout << "(3) Unfolding using 1st Derivative Regularisation and no Area "
		     "Constraint"
		     << endl;

		TUnfoldDensity* unfold3 =
		    new TUnfoldDensity(MigrationMatrix,
		                       TUnfold::kHistMapOutputVert,
		                       TUnfold::kRegModeDerivative,
		                       TUnfold::kEConstraintNone,
		                       TUnfoldDensity::kDensityModeBinWidthAndUser,
		                       0,
		                       0,
		                       0,
		                       "*[UOB]");
		unfold3->SetInput(Data);

		cout << "Finding BestTau using avg correlation" << endl;
		int iBest3avg;
		TSpline* scanResult3avg = 0;
		TGraph* lCurve3avg = 0;
		iBest3avg = unfold3->ScanTau(nScan,
		                             tauMin,
		                             tauMax,
		                             &scanResult3avg,
		                             TUnfoldDensity::kEScanTauRhoAvgSys,
		                             0,
		                             0,
		                             &lCurve3avg);
		std::cout << " Best tau=" << unfold3->GetTau() << "\n";
		std::cout << "iBest3avg=" << iBest3avg << "\n";
		VisualizeTau(nScan, iBest3avg, scanResult3avg, lCurve3avg, "tau3_avg");

		TH1* unfolded3 = unfold3->GetOutput("unfolded3");
		DrawDataMC(unfolded3,
		{ Gen },
		{ "Gen" },
		"UnfoldedvsGen3_avgtau",
		false,
		false,
		"Pt",
		"#events");

		TH2* ErrorMatrix3 = unfold3->GetEmatrixTotal("ErrorMatrix3");
		Draw2D(ErrorMatrix3, "CovMatrix3_avgtau", false);
		TH1D* UnfoldedTotalError3 =
		    new TH1D("TotalError3", "Pt", nBins_Gen, xmin, xmax);
		for (Int_t bin = 1; bin <= nBins_Gen; bin++) {
			UnfoldedTotalError3->SetBinContent(bin, unfolded3->GetBinContent(bin));
			UnfoldedTotalError3->SetBinError(
			    bin, sqrt(ErrorMatrix3->GetBinContent(bin, bin)));
		}
		DrawDataMC(UnfoldedTotalError3,
		{ Gen },
		{ "Gen" },
		"UnfoldedEvsGen3_avgtau",
		false,
		false,
		"Pt",
		"#events");
		TH2* L3 = unfold3->GetL("L3");
		Draw2D(L3, "L3", false);
		TH2* RhoTotal3 = unfold3->GetRhoIJtotal("RhoTotal3_avgtau");
		Draw2D(RhoTotal3, "RhoTotal3_avgtau", false, "Reco", "Gen");

		cout << "Finding BestTau using max correlation" << endl;
		int iBest3max;
		TSpline* scanResult3max = 0;
		TGraph* lCurve3max = 0;
		// tauMin=0.0;
		// tauMax=0;
		iBest3max = unfold3->ScanTau(nScan,
		                             tauMin,
		                             tauMax,
		                             &scanResult3max,
		                             TUnfoldDensity::kEScanTauRhoMaxSys,
		                             0,
		                             0,
		                             &lCurve3max);
		std::cout << " Best tau=" << unfold3->GetTau() << "\n";
		std::cout << "iBest3max=" << iBest3max << "\n";
		VisualizeTau(nScan, iBest3max, scanResult3max, lCurve3max, "tau3_max");

		TH1* unfolded3max = unfold3->GetOutput("unfolded3max");
		DrawDataMC(unfolded3max,
		{ Gen },
		{ "Gen" },
		"UnfoldedvsGen3_maxtau",
		false,
		false,
		"Pt",
		"#events");

		TH2* ErrorMatrix3max = unfold3->GetEmatrixTotal("ErrorMatrix3max");
		Draw2D(ErrorMatrix3max, "CovMatrix3_maxtau", false);
		TH1D* UnfoldedTotalError3max =
		    new TH1D("TotalError3max", "Pt", nBins_Gen, xmin, xmax);
		for (Int_t bin = 1; bin <= nBins_Gen; bin++) {
			UnfoldedTotalError3max->SetBinContent(bin, unfolded3->GetBinContent(bin));
			UnfoldedTotalError3max->SetBinError(
			    bin, sqrt(ErrorMatrix3->GetBinContent(bin, bin)));
		}
		DrawDataMC(UnfoldedTotalError3max,
		{ Gen },
		{ "Gen" },
		"UnfoldedEvsGen3_maxtau",
		false,
		false,
		"Pt",
		"#events");

		TH2* L3max = unfold3->GetL("L3max");
		Draw2D(L3max, "L3maxtau", false);
		TH2* RhoTotal3_maxtau = unfold3->GetRhoIJtotal("RhoTotal3_maxtau");
		Draw2D(RhoTotal3_maxtau, "RhoTotal3_maxtau", false, "Reco", "Gen");

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		cout << "(4) Unfolding using 2nd Derivative Regularisation and no Area "
		     "Constraint"
		     << endl;

		TUnfoldDensity* unfold4 =
		    new TUnfoldDensity(MigrationMatrix,
		                       TUnfold::kHistMapOutputVert,
		                       TUnfold::kRegModeCurvature,
		                       TUnfold::kEConstraintNone,
		                       TUnfoldDensity::kDensityModeBinWidthAndUser,
		                       0,
		                       0,
		                       0,
		                       "*[UOB]");
		unfold4->SetInput(Data);

		cout << "Finding BestTau using avg correlation" << endl;
		int iBest4avg;
		TSpline* scanResult4avg = 0;
		TGraph* lCurve4avg = 0;
		iBest4avg = unfold4->ScanTau(nScan,
		                             tauMin,
		                             tauMax,
		                             &scanResult4avg,
		                             TUnfoldDensity::kEScanTauRhoAvgSys,
		                             0,
		                             0,
		                             &lCurve4avg);
		std::cout << " Best tau=" << unfold4->GetTau() << "\n";
		std::cout << "iBest4avg=" << iBest4avg << "\n";
		VisualizeTau(nScan, iBest4avg, scanResult4avg, lCurve4avg, "tau4_avg");

		TH1* unfolded4 = unfold4->GetOutput("unfolded4");
		DrawDataMC(unfolded4,
		{ Gen },
		{ "Gen" },
		"UnfoldedvsGen4_avgtau",
		false,
		false,
		"Pt",
		"#events");

		TH2* ErrorMatrix4 = unfold4->GetEmatrixTotal("ErrorMatrix4");
		Draw2D(ErrorMatrix4, "CovMatrix4_avgtau", false);
		TH1D* UnfoldedTotalError4 =
		    new TH1D("TotalError4", "Pt", nBins_Gen, xmin, xmax);
		for (Int_t bin = 1; bin <= nBins_Gen; bin++) {
			UnfoldedTotalError4->SetBinContent(bin, unfolded4->GetBinContent(bin));
			UnfoldedTotalError4->SetBinError(
			    bin, sqrt(ErrorMatrix4->GetBinContent(bin, bin)));
		}
		DrawDataMC(UnfoldedTotalError4,
		{ Gen },
		{ "Gen" },
		"UnfoldedEvsGen4_avgtau",
		false,
		false,
		"Pt",
		"#events");
		TH2* L4 = unfold4->GetL("L4");
		Draw2D(L4, "L4", false);
		TH2* RhoTotal4 = unfold4->GetRhoIJtotal("RhoTotal4_avgtau");
		Draw2D(RhoTotal4, "RhoTotal4_avgtau", false, "Reco", "Gen");

		cout << "Finding BestTau using max correlation" << endl;
		int iBest4max;
		TSpline* scanResult4max = 0;
		TGraph* lCurve4max = 0;
		tauMin = 0.0;
		tauMax = 0;
		iBest4max = unfold4->ScanTau(nScan,
		                             tauMin,
		                             tauMax,
		                             &scanResult4max,
		                             TUnfoldDensity::kEScanTauRhoMaxSys,
		                             0,
		                             0,
		                             &lCurve4max);
		std::cout << " Best tau=" << unfold4->GetTau() << "\n";
		std::cout << "iBest4max=" << iBest4max << "\n";
		VisualizeTau(nScan, iBest4max, scanResult4max, lCurve4max, "tau4_max");

		TH1* unfolded4max = unfold4->GetOutput("unfolded4max");
		DrawDataMC(unfolded4max,
		{ Gen },
		{ "Gen" },
		"UnfoldedvsGen4_maxtau",
		false,
		false,
		"Pt",
		"#events");

		TH2* ErrorMatrix4max = unfold4->GetEmatrixTotal("ErrorMatrix4max");
		Draw2D(ErrorMatrix4max, "CovMatrix4_maxtau", false);
		TH1D* UnfoldedTotalError4max =
		    new TH1D("TotalError4max", "Pt", nBins_Gen, xmin, xmax);
		for (Int_t bin = 1; bin <= nBins_Gen; bin++) {
			UnfoldedTotalError4max->SetBinContent(bin, unfolded4->GetBinContent(bin));
			UnfoldedTotalError4max->SetBinError(
			    bin, sqrt(ErrorMatrix4->GetBinContent(bin, bin)));
		}
		DrawDataMC(UnfoldedTotalError4max,
		{ Gen },
		{ "Gen" },
		"UnfoldedEvsGen4_maxtau",
		false,
		false,
		"Pt",
		"#events");

		TH2* L4max = unfold4->GetL("L4max");
		Draw2D(L4max, "L4maxtau", false);
		TH2* RhoTotal4_maxtau = unfold4->GetRhoIJtotal("RhoTotal4_maxtau");
		Draw2D(RhoTotal4_maxtau, "RhoTotal4_maxtau", false, "Reco", "Gen");
	}
}