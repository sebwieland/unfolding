#include "../include/TreeToClasses.hpp"
#include <TTreeReader.h>
#include <TFile.h>


TreeToClasses::TreeToClasses(TChain* InChain, std::vector<std::string> genvars, std::vector<std::string> recovars, std::vector<int> nbins, std::vector<std::vector<int>> Ranges) {
	chain = InChain;
	genVars = genvars;
	recoVars = recovars;
	ranges = Ranges;
	for (auto& bin : nbins) nBins.push_back(bin + 1);
}

TFile* TreeToClasses::BinVars() {
	TFile* classfile = new TFile("classes.root", "RECREATE");
	TTreeReader theReader(chain);


	std::vector<TTreeReaderValue<Float_t>> GenValues;
	std::vector<TTreeReaderValue<Float_t>> RecoValues;
	std::vector<int> classvalsGen(genVars.size());
	std::vector<std::vector<Float_t> > classvalsReco(genVars.size(), std::vector<Float_t> (nBins.at(0)));
	std::vector<int> binWidths;
	TTree* GenTree = new TTree("Gen", "Gen");
	TTree* RecoTree = new TTree("Reco", "Reco");

	int i = 0;
	for (auto const& genName : genVars) { //index i
		GenValues.push_back(TTreeReaderValue<Float_t>(theReader, genName.c_str()));
		TBranch *branch = GenTree->Branch((genName + "_classes").c_str(), &classvalsGen.at(i), (genName + "_classes" + "/I").c_str());
		binWidths.push_back((ranges[i][1] - ranges[i][0]) / nBins[i]);
		int k = 0;

		for (auto const& recoName : recoVars) { //index k
			RecoValues.push_back(TTreeReaderValue<Float_t>(theReader, recoName.c_str()));
			for (unsigned int j = 0; j < nBins.at(i); j++) {
				TBranch *branch = RecoTree->Branch((recoName + "_class" + std::to_string(j)).c_str(), &classvalsReco[i][j], (recoName + "_class" + std::to_string(j) + "/F").c_str());
			}
			binWidths.push_back((ranges[i][1] - ranges[i][0]) / nBins[i]);
			k++;
		}
		i++;
	}
	i = 0;


	while (theReader.Next()) {
		for (unsigned int i = 0; i < GenValues.size(); i++) {
			// std::cout << *values.at(i) << std::endl;
			int classnumber = int(*GenValues.at(i)) / binWidths.at(i) + 1;
			//std::cout << vars.at(i)  << *values.at(i) << " in bin: " << tmp << std::endl;
			classvalsGen.at(i) = classnumber;

			for (unsigned int j = 0; j < RecoValues.size(); j++) {
				Float_t tmp = (*RecoValues.at(i)) ;
				classvalsReco[i][classnumber] = tmp;
			}
		}

		GenTree->Fill();
		RecoTree->Fill();
	}

	GenTree->Write();
	RecoTree->Write();
	classfile->Close();
	return classfile;


}

// TTree* HistMaker::CreateFriendTree(std::vector<string> BranchNames, long n_Events) {
// 	TFile* friendfile = new TFile(path.GetRootFilesPath() + "TreeFriend.root", "RECREATE");
// 	TreeFriend->SetEntries(n_Events);
// 	Long64_t dummyval = 1;
// 	for (auto const& name : BranchNames) {
// 		TBranch *branch = TreeFriend->Branch(name.c_str(), &dummyval, (name + "/L").c_str());
// 	}
// 	for (int j = 0; j < n_Events; j++) {
// 		dummyval = 1;
// 		TreeFriend->Fill();
// 	}

// 	TreeFriend->Write();
// 	friendfile->Close();
// 	return TreeFriend;
// }