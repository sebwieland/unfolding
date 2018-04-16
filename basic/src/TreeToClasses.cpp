#include "../include/TreeToClasses.hpp"
#include <TTreeReader.h>
#include <TFile.h>


TreeToClasses::TreeToClasses(TChain* InChain, std::vector<std::string> genvars, std::vector<std::string> recovars, std::vector<int> nbins, std::vector<std::vector<int>> Ranges) {
	chain = InChain;
	genVars = genvars;
	recoVars = recovars;
	ranges = Ranges;
	for (auto& bin : nbins) nBins.push_back(bin+1);
}

TFile* TreeToClasses::BinVars() {
	std::cout << "converting GenDistribution to Classes" << std::endl;
	TFile* classfile = new TFile("classes.root", "RECREATE");
	TTreeReader theReader(chain);


	std::vector<TTreeReaderValue<Float_t>> GenValues;
	std::vector<TTreeReaderValue<Float_t>> RecoValues;
	std::vector<int> classvalsGen(genVars.size());
	std::vector<Float_t> classvalsReco(genVars.size());
	std::vector<int> binWidths;
	TTree* Tree = new TTree("UnfoldTree", "UnfoldTree");

	int i = 0;
	for (auto const& genName : genVars) { //index i
		GenValues.push_back(TTreeReaderValue<Float_t>(theReader, genName.c_str()));
		TBranch *branch = Tree->Branch((genName + "_classes").c_str(), &classvalsGen.at(i), (genName + "_classes" + "/I").c_str());
		binWidths.push_back((ranges[i][1] - ranges[i][0]) / nBins[i]);

		int k = 0;
		for (auto const& recoName : recoVars) { //index k
			RecoValues.push_back(TTreeReaderValue<Float_t>(theReader, recoName.c_str()));
			TBranch *branch = Tree->Branch((recoName).c_str(), &classvalsReco[k], (recoName + "/F").c_str());
			k++;
		}
		i++;
	}
	i = 0;


	while (theReader.Next()) {
		for (unsigned int i = 0; i < GenValues.size(); i++) {
			int classnumber = int(*GenValues.at(i) / binWidths.at(i) )+1;
			// std::cout <<  *GenValues.at(i) << " in bin: " << classnumber << std::endl;
			classvalsGen.at(i) = classnumber;

			for (unsigned int j = 0; j < RecoValues.size(); j++) {
				Float_t tmp = (*RecoValues.at(j)) ;
				classvalsReco[j] = tmp;
			}
		}

		Tree->Fill();
	}
	Tree->Write();

	classfile->Close();
	std::cout << "converted Gen Distribution to Classes" << std::endl;
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