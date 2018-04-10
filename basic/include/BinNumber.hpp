#ifndef BinNumber_HPP_
#define BinNumber_HPP_

#include <TChain.h>



class BinNumber
{
public:
	TChain* chain;
	std::vector<std::string> vars;
	std::vector<int> nBins;
	std::vector<std::vector<int>> ranges;

	BinNumber(TChain* InNtuple, std::vector<std::string> vars, std::vector<int> binwidths, std::vector<std::vector<int>> Ranges);
	TFile* BinVars();


	};

#endif