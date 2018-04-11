#ifndef TreeToClasses_HPP_
#define TreeToClasses_HPP_

#include <TChain.h>



class TreeToClasses
{
public:
	TChain* chain;
	std::vector<std::string> genVars;
	std::vector<std::string> recoVars;
	std::vector<int> nBins;
	std::vector<std::vector<int>> ranges;
	
	TreeToClasses(TChain* InChain, std::vector<std::string> genvars,std::vector<std::string> recovars, std::vector<int> nbins, std::vector<std::vector<int>> Ranges);

	TFile* BinVars();


	};

#endif