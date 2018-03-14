#include "TCanvas.h"

void Draw1D(TH1* hist, TString name, TString xlabel = "none", TString ylabel = "# Events");
void Draw2D(TH2 * hist, TString name, bool log = false, TString xlabel = "none", TString ylabel = "none");
void DrawRatio(TH1* hist1, TH1* hist2, TString name, TString xlabel = "none", TString ylabel = "Ratio");
void DrawDataMC(TH1* data, std::vector<TH1*> MC, std::vector<std::string> names, TString name, bool log = false, bool normalize = false, TString xlabel = "none", TString ylabel = "# Events");
void SetHistoStyle(TH1* histo, int color, bool filled = false);
TCanvas* getCanvas(TString name, bool ratiopad = false);
TLegend* getLegend();
void VisualizeTau(int nScan, int iBest, TSpline* scanResult, TGraph*lCurve, TString name);
