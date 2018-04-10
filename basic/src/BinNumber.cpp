#include "../include/BinNumber.hpp"
#include <TTreeReader.h>
#include <TFile.h>


BinNumber::BinNumber(TChain* InChain, std::vector<std::string> genvars,std::vector<std::string> recovars, std::vector<int> nbins, std::vector<std::vector<int>> Ranges) {
	chain = InChain;
	genVars = genvars;
	recoVars = recovars;
	ranges = Ranges;
	for(auto& bin: nbins) nBins.push_back(bin+1);
}

TFile* BinNumber::BinVars() {
	TFile* classfile = new TFile("classes.root", "RECREATE");
	TTreeReader theReader(chain);


	std::vector<TTreeReaderValue<Float_t>> GenValues;
	std::vector<TTreeReaderValue<Float_t>> RecoValues;
	std::vector<int> classvalsGen(genVars.size());
	std::vector<int> classvalsReco(recoVars.size());
	std::vector<int> binWidths;

	TTree* GenTree = new TTree("Gen", "Gen");
	TTree* RecoTree = new TTree("Reco", "Reco");
	int i = 0;
	for (auto const& name : genVars) {
		GenValues.push_back(TTreeReaderValue<Float_t>(theReader, name.c_str()));
		TBranch *branch = GenTree->Branch((name + "_classes").c_str(), &classvalsGen.at(i), (name + "_classes" + "/I").c_str());
		binWidths.push_back((ranges[i][1] - ranges[i][0]) / nBins[i]);
		i++;
	}
	i=0;
	for (auto const& name : recoVars) {
		RecoValues.push_back(TTreeReaderValue<Float_t>(theReader, name.c_str()));
		TBranch *branch = RecoTree->Branch((name + "_classes").c_str(), &classvalsReco.at(i), (name + "_classes" + "/I").c_str());
		binWidths.push_back((ranges[i][1] - ranges[i][0]) / nBins[i]);
		i++;
	}


	while (theReader.Next()) {
		for (unsigned int i = 0; i < GenValues.size(); i++) {
			// std::cout << *values.at(i) << std::endl;
			int tmp = int(*GenValues.at(i)) / binWidths.at(i) + 1;
			//std::cout << vars.at(i)  << *values.at(i) << " in bin: " << tmp << std::endl;
			classvalsGen.at(i) = tmp;
		}
		for (unsigned int i = 0; i < RecoValues.size(); i++) {
			// std::cout << *values.at(i) << std::endl;
			int tmp = int(*RecoValues.at(i)) / binWidths.at(i) + 1;
			//std::cout << vars.at(i)  << *values.at(i) << " in bin: " << tmp << std::endl;
			classvalsReco.at(i) = tmp;
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