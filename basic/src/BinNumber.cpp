#include "../include/BinNumber.hpp"
#include <TTreeReader.h>
#include <TFile.h>


BinNumber::BinNumber(TChain* InChain, std::vector<std::string> Vars, std::vector<int> nbins, std::vector<std::vector<int>> Ranges) {
	chain = InChain;
	vars = Vars;
	ranges = Ranges;
	for(auto& bin: nbins) nBins.push_back(bin+1);
}

TFile* BinNumber::BinVars() {
	TFile* classfile = new TFile("classes.root", "RECREATE");
	TTree* Tree = new TTree("TreeFile.root", "Tree");


	TTreeReader theReader(chain);
	std::vector<TTreeReaderValue<Float_t>> values;
	std::vector<int> classvals(vars.size());
	std::vector<int> binWidths;
	int i = 0;
	for (auto const& name : vars) {
		values.push_back(TTreeReaderValue<Float_t>(theReader, name.c_str()));
		TBranch *branch = Tree->Branch((name + "_classes").c_str(), &classvals.at(i), (name + "_classes" + "/I").c_str());
		binWidths.push_back((ranges[i][1] - ranges[i][0]) / nBins[i]);
		i++;
	}


	while (theReader.Next()) {
		for (unsigned int i = 0; i < values.size(); i++) {
			// std::cout << *values.at(i) << std::endl;
			int tmp = int(*values.at(i)) / binWidths.at(i) + 1;
			std::cout << vars.at(i)  << *values.at(i) << " in bin: " << tmp << std::endl;
			classvals.at(i) = tmp;
		}
		Tree->Fill();
	}
	Tree->Write();
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