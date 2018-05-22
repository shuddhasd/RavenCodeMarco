//First part of RavenCode v1.4 Beta

//Author: Marco Baruzzo
//Institute: INFN Trieste
//For problems or questions: elbaru90@gmail.com

#include <stdint.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <list>
#include "TROOT.h"
#include "TChain.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "iostream"
#include "TCanvas.h"
#include "TStyle.h"
#include "TPostScript.h"
#include "TPaveLabel.h"
#include "TTree.h"
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TApplication.h"
#include "Rtypes.h"
#include "fstream"
#include "sstream"
#include "TMath.h"
#include "ctime"
#include "cerrno"
#include "cstring"
#include "cstdlib"
#include "TNtuple.h"
#include "string.h"
#include "stdio.h"
#include "TLine.h"
#include "TText.h"
using std::string;
using std::ofstream;
using std::ios_base;

//-------- MAPPING FUNCTION

int apvrow, apvcolumn;                      //global variables for the mapping, names are self-explicative
int detrow, detcolumn;
std::vector<int> apvmapping;
std::vector<int> detmapping;

void readmappings(const char * apv_mapping, const char * detector_mapping){    //function to read mappings from APV and Detector txt files
    int apvdim=0, detdim=0;                 //I know, there are some better ways to do this, but this is vary simple and self-explicative
    int apvmat=0, detmat=0;
    char readvar[10];
    int i;

    std::ifstream apvmap(apv_mapping);     //apv mapping file

    if(!apvmap){
        std::cout<<"ERROR -- apv mapping file not found"<<std::endl;
    }

    while(apvmap.is_open()){                //it start reading the dimensions (first 3 lines) and after create the vector mapping
        if(apvmap.eof()){ printf("APV mapping LOADED\n"); break; }
        apvmap>>readvar;
        if (apvdim==1){
            apvcolumn=atoi(readvar);
            apvdim=0;
        }
        if (apvdim==2){
            apvrow=atoi(readvar);
            apvdim=1;
        }
        if (std::string(readvar)=="dimensions"){
            apvdim=2;
        }
        if (apvmat==1){
            apvmapping[atoi(readvar)]=i;            //!!!MOST IMPORTANT THING: calling the apvmapping[APV_channel]
            i++;                                    //it return the position order in the histogram, row per row
        }
        if (std::string(readvar)=="matrix"){        //matrix mapping 
            apvmapping.resize(apvrow*apvcolumn);
            apvmat=1;
            i=0;
        }
    }
    apvmap.close();

    std::ifstream detmap(detector_mapping);     //apv mapping file

    if(!detmap){
        std::cout<<"ERROR -- detector mapping file not found"<<std::endl;
    }

    while(detmap.is_open()){               //it start reading the dimensions (first 3 lines) and after create the vector mapping
        if(detmap.eof()){ printf("Detector mapping LOADED\n"); break; }
        detmap>>readvar;
        if (detdim==1){
            detcolumn=atoi(readvar);
            detdim=0;
        }
        if (detdim==2){
            detrow=atoi(readvar);
            detdim=1;
        }
        if (std::string(readvar)=="dimensions"){
            detdim=2;
        }
        if (detmat==1){
            detmapping[atoi(readvar)]=i;            //!!!MOST IMPORTANT THING: calling the detmapping[APV_number]
            i++;                                    //it return the position order in the histogram, row per row
        }
        if (std::string(readvar)=="matrix"){        //matrix mapping 
            detmapping.resize(detrow*detcolumn);
            detmat=1;
            i=0;
        }
    }
    detmap.close();
}


int Xpadtograph(int nPAD, int nAPV){                //From PAD and APV values give the X coordinate for the 2D/3D histrograms
	int xvalue=-10;
	if (nAPV < (detrow*detcolumn)){
		xvalue =(apvmapping[nPAD]%apvcolumn+(detmapping[nAPV]%detcolumn)*apvcolumn);
	}
	return xvalue;
}

int Ypadtograph(int nPAD, int nAPV){
	int yvalue=-10;
	if (nAPV < (detrow*detcolumn)){                //From PAD and APV values give the Y coordinate for the 2D/3D histrograms
		yvalue = apvrow*detrow-(apvmapping[nPAD]/apvcolumn+(detmapping[nAPV]/detcolumn)*apvrow)-1;
	}
	return yvalue;
}

int main(){
  gStyle->SetOptTitle(0);
	std::cout<<"Start"<<std::endl;

// -------------------------------- LOAD ANALYZER SETTINGS -------------------------------- //  

    string filename_in; 					//variables for the analysis run stored in the txt file from the GUI
    string filename_out;
    string find_max;
    Double_t totalevents;	

    std::ifstream settings("analyzer_settings");				//analysis settings file
    if(!settings){
        std::cout<<"ERROR -- settings file not found"<<std::endl;
    } 
    while(settings.is_open()){
        if(settings.eof()){ printf("Settings LOADED\n"); break; }
        settings>>filename_in>>filename_out>>find_max>>totalevents;
    }
    std::string filename_out_path = filename_out.substr(0, filename_out.find_last_of('/'));    //create the path for the other files
    settings.close();

// -------------------------------- LOAD DETECTOR SETTINGS -------------------------------- //

    int ntotFEC = 0;
    std::vector<int> ntotAPV;
    int ntotCh_Pad = 0;
    int totTiming = 0;
    string file_APV_mapping;
    string file_DET_mapping;

    std::ifstream detector("detector_settings");                    //detector settings file

    if(!detector){
        std::cout<<"ERROR -- detector file not found"<<std::endl;
    } 
    while(detector.is_open()){
        if(detector.eof()){ printf("detector LOADED\n"); break; }
        detector>>ntotFEC;
        ntotAPV.resize(ntotFEC);
        for (int i = 0; i < ntotFEC; ++i){               //to load the number of APV for all FEC
            detector>>ntotAPV[i];
        }
        detector>>ntotCh_Pad>>totTiming>>file_APV_mapping>>file_DET_mapping;
    }
    detector.close();

// -------------------------------- LOAD MAPPINGS -------------------------------- //

    readmappings(file_APV_mapping.c_str(),file_DET_mapping.c_str());

// -------------------------------- ANALYSIS -------------------------------- //
    std::cout<<"Start"<<std::endl;

	///////////////////////////////////////////////////////
    bool debug = kFALSE;                        // debug flag (simple)
    bool debug2 = kFALSE;                       // debug flag (hex)
    ///////////////////////////////////////////////////////

	bool fFirstcycle=kTRUE;							//flag for the first cycle needed for inizialyze the variable for the first time
	float percent;                             		//only for the terminal output
    int processedentries = 0;                  		//entries number (one for each analog value)
	Int_t iter=0;									//numerator of the data analyzed
	Double_t value_it=0;							//ADC value for that cycle
	Int_t nEVENT_before=-1;							//varible to check if the event is changed inizialyze the variable

	Double_t hit[16][128];						//hit mapping with increment each single pad		
	Int_t itera[16][128];						//mapping for the hit mapping (I know complicanted... but it works)
	Int_t used_apv[2048];						//values saved, they can be found starting from the itera values
	Int_t used_pad[2048];						//values saved, they can be found starting from the itera values						
	Int_t used_time[2048];						//values saved, they can be found starting from the itera values
	memset( hit, 0, 16*128*sizeof(Double_t) );
	memset( itera, 0, 16*128*sizeof(Double_t) );
	memset( used_apv, 0, 2048*sizeof(Int_t) );
	memset( used_pad, 0, 2048*sizeof(Int_t) );
	memset( used_time, 0, 2048*sizeof(Int_t) );
	std::vector<int> iter_clustering;

	//Reopen the ROOT file with physics data
	TFile *file = TFile::Open(filename_in.c_str());
	if(!file){
    	std::cout<<"ERROR -- root file not found"<<std::endl;
	} 
	TTreeReader reader("treedecoded", file);
	TTreeReaderValue<int> it_nEVENT(reader, "nEVENT");
	TTreeReaderValue<int> it_nFEC(reader, "nFEC");
	TTreeReaderValue<int> it_nAPV(reader, "nAPV");
	TTreeReaderValue<int> it_nPAD(reader, "nPAD");
	TTreeReaderValue<int> it_nTIME(reader, "nTIME");
	TTreeReaderValue<Double_t> it_value(reader, "value");

	//--------- find max value on time stamp
	if (find_max == "max"){	
		std::cout<<"Finding max values"<<std::endl;


		TFile * rootfile = new TFile(filename_out.c_str(),"recreate");
		Double_t value_max;
		Int_t nEVENT_max, nFEC_max, nAPV_max, nPAD_max, nTIME_max;

    	TTree * tmax = new TTree("treemax","treemax");						//tree initialization
    	tmax->Branch("value_max", &value_max);
    	tmax->Branch("nEVENT_max", &nEVENT_max);
    	tmax->Branch("nFEC_max", &nFEC_max);
    	tmax->Branch("nAPV_max", &nAPV_max);
    	tmax->Branch("nPAD_max", &nPAD_max);
    	tmax->Branch("nTIME_max", &nTIME_max);

		while (reader.Next()) {//search the maximum for each channel
			if ( *it_nEVENT==nEVENT_before){
				value_it = *it_value;
				if ( value_it > hit[*it_nAPV][*it_nPAD] && value_it>0){			//substitute the maximum
					if (hit[*it_nAPV][*it_nPAD]==0){
						iter++;
						itera[*it_nAPV][*it_nPAD]=iter;
					}
					hit[*it_nAPV][*it_nPAD] = value_it;
					used_apv[itera[*it_nAPV][*it_nPAD]]=*it_nAPV;
					used_pad[itera[*it_nAPV][*it_nPAD]]=*it_nPAD;
					used_time[itera[*it_nAPV][*it_nPAD]]=*it_nTIME;
				}
			}
			else{
				if(!fFirstcycle){					//not touched in the first cycle (I don't to fill with zeros) 
					for (int i = 0; i < iter; ++i){
						nEVENT_max 	= *it_nEVENT;
						nFEC_max 	= 1;
						nAPV_max 	= used_apv[i];
						nPAD_max 	= used_pad[i];
						nTIME_max 	= used_time[i];
						value_max 	= hit[used_apv[i]][used_pad[i]];
						tmax->Fill();
					}
					memset( hit, 0, 16*128*sizeof(Double_t) );
					memset( used_apv, 0, 2048*sizeof(Int_t) );
					memset( used_pad, 0, 2048*sizeof(Int_t) );
					memset( used_time, 0, 2048*sizeof(Int_t) );
					iter=0;
					value_it=0;
				}
				fFirstcycle=kFALSE;				//open the tree filling
				processedentries++;
				nEVENT_before = *it_nEVENT;		//change number of event

				value_it = *it_value;										//like the fisrt if, but for a new event
				itera[*it_nAPV][*it_nPAD]=iter;
				if (value_it>hit[*it_nAPV][*it_nPAD]){						//substitute the maximum
					hit[*it_nAPV][*it_nPAD] = value_it;
					used_apv[itera[*it_nAPV][*it_nPAD]]=*it_nAPV;
					used_pad[itera[*it_nAPV][*it_nPAD]]=*it_nPAD;
					used_time[itera[*it_nAPV][*it_nPAD]]=*it_nTIME;
				}

				percent=(float(processedentries)/float(totalevents))*100.0;									//only for the percentage
                if ((fmod(percent,1))==0){
                    printf(" %i on total events %3.f %c \r",processedentries,percent,37);
                    fflush(stdout);
                }

			}
		}
		printf("saving...\n");
    	tmax->Write();
    	rootfile->Write();

		if(!rootfile){
    		std::cout<<"ERROR -- root file not found"<<std::endl;
		} 
    	TTreeReader readermax("treemax", rootfile);							//analyze the max values and generate histograms
		TTreeReaderValue<int> it_nEVENT_max(readermax, "nEVENT_max");
		TTreeReaderValue<int> it_nFEC_max(readermax, "nFEC_max");
		TTreeReaderValue<int> it_nAPV_max(readermax, "nAPV_max");
		TTreeReaderValue<int> it_nPAD_max(readermax, "nPAD_max");
		TTreeReaderValue<int> it_nTIME_max(readermax, "nTIME_max");
		TTreeReaderValue<Double_t> it_value_max(readermax, "value_max");

		//3D lego plot
		TH2F * MapGraphLego_max= new TH2F ("Mapdistlego", "mapdistlego", apvcolumn*detcolumn, 0, apvcolumn*detcolumn, apvrow*detrow, 0, apvrow*detrow);
		//2D histogram
		TH2F * MapGraph_max= new TH2F ("Mapdist", "mapdist", apvcolumn*detcolumn, 0, apvcolumn*detcolumn, apvrow*detrow, 0, apvrow*detrow);
		//Energy distribution histogram
		TH1F * EnergyHisto_max = new TH1F("Energydist","Energydist",200,0,2000);
		//Timing distribution histrogram
		TH1F * Timing_max = new TH1F("Timing","Timing",totTiming,0,totTiming);

		std::cout<<apvcolumn<<" "<<detcolumn<<" xnum "<<apvcolumn*detcolumn<<", ynum "<<apvrow*detrow<<std::endl;

		while (readermax.Next()) {  //filling the histograms
			MapGraphLego_max->Fill(Xpadtograph(*it_nPAD_max,*it_nAPV_max),Ypadtograph(*it_nPAD_max,*it_nAPV_max));
			MapGraph_max->Fill(Xpadtograph(*it_nPAD_max,*it_nAPV_max),Ypadtograph(*it_nPAD_max,*it_nAPV_max));
			Timing_max->Fill(*it_nTIME_max);
			EnergyHisto_max->Fill(*it_value_max);
			processedentries++;
		}
		std::cout<<"processed entries "<<processedentries<<std::endl;

		gStyle->SetOptStat(0000);
		TCanvas * csinglepad= new TCanvas("Mapping","Mapping",1920,1024);

		MapGraphLego_max->Draw("LEGO2Z");
		MapGraphLego_max->SetTitle("Hit Spatial distribution (Lego plot)");
		MapGraphLego_max->GetXaxis()->SetTitleOffset(1.6);
		MapGraphLego_max->GetYaxis()->SetTitleOffset(1.8);
		MapGraphLego_max->GetYaxis()->SetTitleOffset(1.8);
    	MapGraphLego_max->GetYaxis()->SetTitle("Y axis [PAD]");
    	MapGraphLego_max->GetXaxis()->SetTitle("X axis [PAD]");
    	MapGraphLego_max->GetZaxis()->SetTitle("# Counts");
    	MapGraphLego_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/Spatial_dist(lego).eps").c_str());
    	

    	MapGraph_max->Draw("colz");
		MapGraph_max->SetTitle("Hit Spatial distribution");
    	MapGraph_max->GetYaxis()->SetTitle("Y axis [PAD]");
    	MapGraph_max->GetXaxis()->SetTitle("X axis [PAD]");
    	MapGraph_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/Spatial_dist(2D).eps").c_str());

    	EnergyHisto_max->Draw();
		EnergyHisto_max->SetTitle("Energy distribution");
    	EnergyHisto_max->GetYaxis()->SetTitle("# Count");
    	EnergyHisto_max->GetXaxis()->SetTitle("Energy [ADC Ch]");
    	EnergyHisto_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/Energy_dist.eps").c_str());

    	Timing_max->Draw();
		Timing_max->SetTitle("Timing distribution");
    	Timing_max->GetYaxis()->SetTitle("# Count");
    	Timing_max->GetXaxis()->SetTitle("Time [timestamp units]");
    	Timing_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/Time_dist.eps").c_str());

    	rootfile->Close();
    	std::cout<<"File closed"<<std::endl;

	}

// find max value on time stamp for NO TRIGGERED EVENT
	if (find_max == "max_notrigger"){	
		std::cout<<"Finding max values (notrigger)"<<std::endl;
		TFile * rootfile = new TFile(filename_out.c_str(),"recreate");
		Double_t value_max;
		Int_t nEVENT_max, nFEC_max, nAPV_max, nPAD_max, nTIME_max;
    	TTree * tmax = new TTree("treemax","treemax");
    	tmax->Branch("value_max", &value_max);
    	tmax->Branch("nEVENT_max", &nEVENT_max);
    	tmax->Branch("nFEC_max", &nFEC_max);
    	tmax->Branch("nAPV_max", &nAPV_max);
    	tmax->Branch("nPAD_max", &nPAD_max);
    	tmax->Branch("nTIME_max", &nTIME_max);
		while (reader.Next()) {
			//std::cout<<*it_nEVENT<<std::endl;
			if ( *it_nEVENT==nEVENT_before){
				value_it = *it_value;
				if ( value_it > hit[*it_nAPV][*it_nPAD] && value_it>0 && value_it<1500){    //the same of max but not saving data if the peaks are in the head or in the tail of the time distribution
					if (hit[*it_nAPV][*it_nPAD]==0){
						iter++;
						itera[*it_nAPV][*it_nPAD]=iter;
					}
					//std::cout<<hit[*it_nAPV][*it_nPAD]<<std::endl;
					hit[*it_nAPV][*it_nPAD] = value_it;
					used_apv[itera[*it_nAPV][*it_nPAD]]=*it_nAPV;
					used_pad[itera[*it_nAPV][*it_nPAD]]=*it_nPAD;
					used_time[itera[*it_nAPV][*it_nPAD]]=*it_nTIME;
				}
			}
			else{
				//std::cout<<nEVENT_before<<" "<<iter<<std::endl;
				if(!fFirstcycle){
					percent=(float(processedentries)/float(totalevents))*100.0;
                	if ((fmod(percent,1))==0){
                    	printf(" %i on total events %3.f %c \r",processedentries,percent,37);
                    	fflush(stdout);
                	}
					for (int i = 1; i < iter+1; ++i){
						//std::cout<<nEVENT_before<<" "<<iter<<" "<<nAPV<<std::endl;
						if ((used_time[i]>1) && used_time[i]<(totTiming-2)){
							nEVENT_max 	= *it_nEVENT;
							nFEC_max 	= 1;
							nAPV_max 	= used_apv[i];
							nPAD_max 	= used_pad[i];
							nTIME_max 	= used_time[i];
							value_max 	= hit[used_apv[i]][used_pad[i]];
							tmax->Fill();
						}
					}
					memset( hit, 0, 16*128*sizeof(Double_t) );
					memset( used_apv, 0, 2048*sizeof(Int_t) );
					memset( used_pad, 0, 2048*sizeof(Int_t) );
					memset( used_time, 0, 2048*sizeof(Int_t) );
					iter=0;
					value_it=0;
				}
				fFirstcycle=kFALSE;
				processedentries++;
				nEVENT_before = *it_nEVENT;
				value_it = *it_value;
				if ( value_it > hit[*it_nAPV][*it_nPAD] && value_it>0 && value_it<1500){ 
					if (hit[*it_nAPV][*it_nPAD]==0){
						iter++;
						itera[*it_nAPV][*it_nPAD]=iter;
					}
					hit[*it_nAPV][*it_nPAD] = value_it;
					used_apv[itera[*it_nAPV][*it_nPAD]]=*it_nAPV;
					used_pad[itera[*it_nAPV][*it_nPAD]]=*it_nPAD;
					used_time[itera[*it_nAPV][*it_nPAD]]=*it_nTIME;
				}

			}
		}
		printf("saving...\n");
    	tmax->Write();
    	rootfile->Write();
    	//rootfile->Close();

    	//TFile *file2 = TFile::Open(filename_out);
		if(!rootfile){
    		std::cout<<"ERROR -- root file not found"<<std::endl;
		} 
    	TTreeReader readermax("treemax", rootfile);
		TTreeReaderValue<int> it_nEVENT_max(readermax, "nEVENT_max");
		TTreeReaderValue<int> it_nFEC_max(readermax, "nFEC_max");
		TTreeReaderValue<int> it_nAPV_max(readermax, "nAPV_max");
		TTreeReaderValue<int> it_nPAD_max(readermax, "nPAD_max");
		TTreeReaderValue<int> it_nTIME_max(readermax, "nTIME_max");
		TTreeReaderValue<Double_t> it_value_max(readermax, "value_max");

		TH2F * MapGraphLego2_max= new TH2F ("Mapdistlego", "mapdistlego", apvcolumn*detcolumn, 0, apvcolumn*detcolumn, apvrow*detrow, 0, apvrow*detrow);
		TH2F * MapGraph2_max= new TH2F ("Mapdist2", "mapdist2", apvcolumn*detcolumn, 0, apvcolumn*detcolumn, apvrow*detrow, 0, apvrow*detrow);
		TH1F * EnergyHisto2_max = new TH1F("Energydist","Energydist",300,0,1500);
		TH1F * Timing2_max = new TH1F("Timing","Timing",totTiming,0,totTiming);
		TH1F EnergyHisto_singlePADS_max[2048];
		for (int i = 0; i < 128*ntotAPV[0]; ++i){
			EnergyHisto_singlePADS_max[i]=  TH1F("EnergyHisto_singlePADS","EnergyHisto_singlePADS",250,0,1500); 
		}
		TH1F * EnergyHisto_singlePADS_peaks_max = new TH1F("EnergyHisto_singlePADS_peaks","EnergyHisto_singlePADS_peaks",250,0,1500);
		std::cout<<apvcolumn<<" "<<detcolumn<<" xnum "<<apvcolumn*detcolumn<<", ynum "<<apvrow*detrow<<std::endl;
		while (readermax.Next()) {
			MapGraphLego2_max->Fill(Xpadtograph(*it_nPAD_max,*it_nAPV_max),Ypadtograph(*it_nPAD_max,*it_nAPV_max));
			MapGraph2_max->Fill(Xpadtograph(*it_nPAD_max,*it_nAPV_max),Ypadtograph(*it_nPAD_max,*it_nAPV_max));
			Timing2_max->Fill(*it_nTIME_max);
			EnergyHisto2_max->Fill(*it_value_max);
			EnergyHisto_singlePADS_max[*it_nPAD_max].Fill(*it_value_max);
			processedentries++;
		}
		std::cout<<"processed entries "<<processedentries<<std::endl;
		gStyle->SetOptStat(0000);
		TCanvas * csinglepad= new TCanvas("Mapping","Mapping",1920,1024);

		gPad->SetLogz();
		MapGraphLego2_max->Draw("LEGO2");
		MapGraphLego2_max->SetTitle("Hit Spatial distribution (Lego plot)");
		MapGraphLego2_max->GetXaxis()->SetTitleOffset(1.6);
		MapGraphLego2_max->GetYaxis()->SetTitleOffset(1.8);
		MapGraphLego2_max->GetYaxis()->SetTitleOffset(1.8);
    	MapGraphLego2_max->GetYaxis()->SetTitle("Y axis [PAD]");
    	MapGraphLego2_max->GetXaxis()->SetTitle("X axis [PAD]");
    	MapGraphLego2_max->GetZaxis()->SetTitle("# Counts");
    	MapGraphLego2_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/Spatial_dist(lego).eps").c_str());
    	
    	MapGraph2_max->Draw("colz");
		MapGraph2_max->SetTitle("Hit Spatial distribution");
    	MapGraph2_max->GetYaxis()->SetTitle("Y axis [PAD]");
    	MapGraph2_max->GetXaxis()->SetTitle("X axis [PAD]");
    	MapGraph2_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/Spatial_dist(2D).eps").c_str());

    	gPad->SetLogz(0);
    	EnergyHisto2_max->Draw();
		EnergyHisto2_max->SetTitle("Energy distribution");
    	EnergyHisto2_max->GetYaxis()->SetTitle("# Counts");
    	EnergyHisto2_max->GetXaxis()->SetTitle("Energy [ADC Ch]");
    	EnergyHisto2_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/Energy_dist.eps").c_str());

    	Timing2_max->Draw();
		Timing2_max->SetTitle("Timing distribution");
    	Timing2_max->GetYaxis()->SetTitle("# Counts");
    	Timing2_max->GetXaxis()->SetTitle("Time {timestamp units}");
    	Timing2_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/Time_dist.eps").c_str());
	
	TF1 * gaus1 = new TF1("gaus1","gaus(0)",600,1500);
    	for (int i = 0; i < 128*ntotAPV[0]; ++i){
	  if (EnergyHisto_singlePADS_max[i].Integral()>1500){
    		EnergyHisto_singlePADS_max[i].Draw();
    		EnergyHisto_singlePADS_max[i].Fit(gaus1);
    		EnergyHisto_singlePADS_peaks_max->Fill(gaus1->GetParameter(1));
	    
	  }
    	}
    	EnergyHisto_singlePADS_peaks_max->Draw();
    	EnergyHisto_singlePADS_peaks_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/EnergyHisto_singlePADS_peaks.eps").c_str());


    	rootfile->Close();
    	std::cout<<"File closed"<<std::endl;

	}

// find max value on time stamp for NO TRIGGERED EVENT
	if (find_max == "max_notrigger_gainscan"){	
		std::cout<<"Gain scan with max values (notrigger)"<<std::endl;
		TFile * rootfile = new TFile(filename_out.c_str(),"recreate");
		Double_t value_max;
		Int_t nEVENT_max, nFEC_max, nAPV_max, nPAD_max, nTIME_max;
    	TTree * tmax = new TTree("treemax","treemax");
    	tmax->Branch("value_max", &value_max);
    	tmax->Branch("nEVENT_max", &nEVENT_max);
    	tmax->Branch("nFEC_max", &nFEC_max);
    	tmax->Branch("nAPV_max", &nAPV_max);
    	tmax->Branch("nPAD_max", &nPAD_max);
    	tmax->Branch("nTIME_max", &nTIME_max);
		while (reader.Next()) {
			//std::cout<<*it_nEVENT<<std::endl;
			if ( *it_nEVENT==nEVENT_before){
				value_it = *it_value;
				if ( value_it > hit[*it_nAPV][*it_nPAD] && value_it>0 && value_it<1500){    //the same of max but not saving data if the peaks are in the head or in the tail of the time distribution
					if (hit[*it_nAPV][*it_nPAD]==0){
						iter++;
						itera[*it_nAPV][*it_nPAD]=iter;
					}
					//std::cout<<hit[*it_nAPV][*it_nPAD]<<std::endl;
					hit[*it_nAPV][*it_nPAD] = value_it;
					used_apv[itera[*it_nAPV][*it_nPAD]]=*it_nAPV;
					used_pad[itera[*it_nAPV][*it_nPAD]]=*it_nPAD;
					used_time[itera[*it_nAPV][*it_nPAD]]=*it_nTIME;
				}
			}
			else{
				//std::cout<<nEVENT_before<<" "<<iter<<std::endl;
				if(!fFirstcycle){
					percent=(float(processedentries)/float(totalevents))*100.0;
                	if ((fmod(percent,1))==0){
                    	printf(" %i on total events %3.f %c \r",processedentries,percent,37);
                    	fflush(stdout);
                	}
					for (int i = 1; i < iter+1; ++i){
						//std::cout<<nEVENT_before<<" "<<iter<<" "<<nAPV<<std::endl;
						if ((used_time[i]>1) && used_time[i]<(totTiming-2)){
							nEVENT_max 	= *it_nEVENT;
							nFEC_max 	= 1;
							nAPV_max 	= used_apv[i];
							nPAD_max 	= used_pad[i];
							nTIME_max 	= used_time[i];
							value_max 	= hit[used_apv[i]][used_pad[i]];
							tmax->Fill();
						}
					}
					memset( hit, 0, 16*128*sizeof(Double_t) );
					memset( used_apv, 0, 2048*sizeof(Int_t) );
					memset( used_pad, 0, 2048*sizeof(Int_t) );
					memset( used_time, 0, 2048*sizeof(Int_t) );
					iter=0;
					value_it=0;
				}
				fFirstcycle=kFALSE;
				processedentries++;
				nEVENT_before = *it_nEVENT;
				value_it = *it_value;
				if ( value_it > hit[*it_nAPV][*it_nPAD] && value_it>0 && value_it<1500){ 
					if (hit[*it_nAPV][*it_nPAD]==0){
						iter++;
						itera[*it_nAPV][*it_nPAD]=iter;
					}
					hit[*it_nAPV][*it_nPAD] = value_it;
					used_apv[itera[*it_nAPV][*it_nPAD]]=*it_nAPV;
					used_pad[itera[*it_nAPV][*it_nPAD]]=*it_nPAD;
					used_time[itera[*it_nAPV][*it_nPAD]]=*it_nTIME;
				}

			}
		}
		printf("saving...\n");
    	tmax->Write();
    	rootfile->Write();
    	//rootfile->Close();

    	//TFile *file2 = TFile::Open(filename_out);
		if(!rootfile){
    		std::cout<<"ERROR -- root file not found"<<std::endl;
		} 
    	TTreeReader readermax("treemax", rootfile);
		TTreeReaderValue<int> it_nEVENT_max(readermax, "nEVENT_max");
		TTreeReaderValue<int> it_nFEC_max(readermax, "nFEC_max");
		TTreeReaderValue<int> it_nAPV_max(readermax, "nAPV_max");
		TTreeReaderValue<int> it_nPAD_max(readermax, "nPAD_max");
		TTreeReaderValue<int> it_nTIME_max(readermax, "nTIME_max");
		TTreeReaderValue<Double_t> it_value_max(readermax, "value_max");

		TH2F * MapGraphLego2_max= new TH2F ("Mapdistlego", "mapdistlego", apvcolumn*detcolumn, 0, apvcolumn*detcolumn, apvrow*detrow, 0, apvrow*detrow);
		TH2F * MapGraph2_max= new TH2F ("Mapdist2", "mapdist2", apvcolumn*detcolumn, 0, apvcolumn*detcolumn, apvrow*detrow, 0, apvrow*detrow);
		TH1F * EnergyHisto2_max = new TH1F("Energydist","Energydist",300,0,1500);
		TH1F * Timing2_max = new TH1F("Timing","Timing",totTiming,0,totTiming);
		TH1F EnergyHisto_singlePADS_max[2048];
		for (int i = 0; i < 128*ntotAPV[0]; ++i){
			EnergyHisto_singlePADS_max[i]=  TH1F("EnergyHisto_singlePADS","EnergyHisto_singlePADS",1500,0,1500); 
		}
		TH1F * EnergyHisto_singlePADS_peaks_max = new TH1F("EnergyHisto_singlePADS_peaks","EnergyHisto_singlePADS_peaks",1500,0,1500);
		std::cout<<apvcolumn<<" "<<detcolumn<<" xnum "<<apvcolumn*detcolumn<<", ynum "<<apvrow*detrow<<std::endl;
		while (readermax.Next()) {
			MapGraphLego2_max->Fill(Xpadtograph(*it_nPAD_max,*it_nAPV_max),Ypadtograph(*it_nPAD_max,*it_nAPV_max));
			MapGraph2_max->Fill(Xpadtograph(*it_nPAD_max,*it_nAPV_max),Ypadtograph(*it_nPAD_max,*it_nAPV_max));
			Timing2_max->Fill(*it_nTIME_max);
			EnergyHisto2_max->Fill(*it_value_max);
			EnergyHisto_singlePADS_max[(*it_nPAD_max+1)*(*it_nAPV_max+1)-1].Fill(*it_value_max);
			processedentries++;
		}
		std::cout<<"processed entries "<<processedentries<<std::endl;
		gStyle->SetOptStat(0000);
		TCanvas * csinglepad= new TCanvas("Mapping","Mapping",1920,1024);

		gPad->SetLogz();
		MapGraphLego2_max->Draw("LEGO2");
		MapGraphLego2_max->SetTitle("Hit Spatial distribution (Lego plot)");
		MapGraphLego2_max->GetXaxis()->SetTitleOffset(1.6);
		MapGraphLego2_max->GetYaxis()->SetTitleOffset(1.8);
		MapGraphLego2_max->GetYaxis()->SetTitleOffset(1.8);
    	MapGraphLego2_max->GetYaxis()->SetTitle("Y axis [PAD]");
    	MapGraphLego2_max->GetXaxis()->SetTitle("X axis [PAD]");
    	MapGraphLego2_max->GetZaxis()->SetTitle("# Counts");
    	MapGraphLego2_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/Spatial_dist(lego).eps").c_str());
    	
    	MapGraph2_max->Draw("colz");
		MapGraph2_max->SetTitle("Hit Spatial distribution");
    	MapGraph2_max->GetYaxis()->SetTitle("Y axis [PAD]");
    	MapGraph2_max->GetXaxis()->SetTitle("X axis [PAD]");
    	MapGraph2_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/Spatial_dist(2D).eps").c_str());

    	gPad->SetLogz(0);
    	EnergyHisto2_max->Draw();
		EnergyHisto2_max->SetTitle("Energy distribution");
    	EnergyHisto2_max->GetYaxis()->SetTitle("# Counts");
    	EnergyHisto2_max->GetXaxis()->SetTitle("Energy [ADC Ch]");
    	EnergyHisto2_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/Energy_dist.eps").c_str());

    	Timing2_max->Draw();
		Timing2_max->SetTitle("Timing distribution");
    	Timing2_max->GetYaxis()->SetTitle("# Counts");
    	Timing2_max->GetXaxis()->SetTitle("Time {timestamp units}");
    	Timing2_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/Time_dist.eps").c_str());
	
	TF1 * gaus1 = new TF1("gaus1","gaus(0)",0,1500);
	TH2F * MapGain_max= new TH2F ("Mapgain", "mapgain", apvcolumn*detcolumn, 0, apvcolumn*detcolumn, apvrow*detrow, 0, apvrow*detrow);
	gStyle->SetOptStat(1111);
	int gausmean_approx=0;
    for (int i = 0; i < 128*ntotAPV[0]; ++i){
	  if (EnergyHisto_singlePADS_max[i].Integral()>2000){
	  		gausmean_approx=0;
	  		EnergyHisto_singlePADS_max[i].Rebin(10);
	  		EnergyHisto_singlePADS_max[i].GetXaxis()->SetRange(25,150);
	  		gausmean_approx = EnergyHisto_singlePADS_max[i].GetMaximumBin()*10;

	  		EnergyHisto_singlePADS_max[i].GetXaxis()->SetRange(0,1500);
	  		EnergyHisto_singlePADS_max[i].Fit(gaus1,"","",gausmean_approx-200,gausmean_approx+200);
	  		EnergyHisto_singlePADS_max[i].SetMaximum(150);
    		EnergyHisto_singlePADS_max[i].Draw();
    		EnergyHisto_singlePADS_max[i].Write();
    		if (i == 108){
    			csinglepad->SaveAs((filename_out_path+"/picco.pdf").c_str());
    		}

    		EnergyHisto_singlePADS_peaks_max->Fill(gaus1->GetParameter(1));
    		std::cout<<i<<" gain "<<gausmean_approx<<" "<<Xpadtograph(i%128,i/128)<<" "<<Ypadtograph(i%128,i/128)<<" "<<gaus1->GetParameter(1)<<std::endl;

    		if (gaus1->GetParameter(1) > 0 && gaus1->GetParameter(1)<1500){
    			MapGain_max->SetBinContent(Xpadtograph(i%128,i/128)+1,Ypadtograph(i%128,i/128)+1,gaus1->GetParameter(1));
    		}
    		if ( gaus1->GetParameter(1)>1500){
    			MapGain_max->SetBinContent(Xpadtograph(i%128,i/128)+1,Ypadtograph(i%128,i/128)+1,1500);
    		}
    		
	  }
    	}

    	gStyle->SetOptStat(1111);
    	EnergyHisto_singlePADS_peaks_max->Rebin(10);
    	EnergyHisto_singlePADS_peaks_max->Draw();
    	EnergyHisto_singlePADS_peaks_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/EnergyHisto_singlePADS_peaks.eps").c_str());

    	//gPad->SetLogz();
    	MapGain_max->Draw("colz text");
		MapGain_max->SetTitle("Gain scan Spatial distribution");
    	MapGain_max->GetYaxis()->SetTitle("Y axis [PAD]");
    	MapGain_max->GetXaxis()->SetTitle("X axis [PAD]");
    	MapGain_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/gain_dist(2D).eps").c_str());
    	//gPad->SetLogz(0);


    	rootfile->Close();
    	std::cout<<"File closed"<<std::endl;

	}

// find max value on time stamp for NO TRIGGERED EVENT MINIPAD
	if (find_max == "max_notrigger_cluster"){	
		std::cout<<"Finding max values, integrating over event (notrigger) with cluster calculation"<<std::endl;
		TFile * rootfile = new TFile(filename_out.c_str(),"recreate");
		Double_t value_max;
		Double_t total_value_max;
		int h=0, hit_peak;
		Int_t nEVENT_max, nFEC_max, nAPV_max, nPAD_max, nTIME_max, XPAD_max, YPAD_max; 
		Double_t totCluster_charge, X_Cluster, Y_Cluster, hit_limit;
		Int_t hit_max = 0, nHIT_percluster = 0, nCluster=0;
		bool hit_found;

		TH1F * Event_multiplicity_max = new TH1F("Event Multiplicity","Event Multiplicity",20,0,20);
		TH1F * Hit_per_cluster_max = new TH1F("Hit per cluster","Hit per cluster",20,0,20);
		TH1F * Cluster_per_event_max = new TH1F("Cluster per event","Cluster per event",20,0,20);
		TH1F * Event_total_value_max = new TH1F("Event Total Energy","Event Total Energy",1000,0,4000);
		TH1F * Cluster_total_value_max = new TH1F("Cluster total value","Cluster total value",1000,0,4000);
		TH2F * cluster_hit = new TH2F ("cluster_hit", "cluster_hit", apvcolumn*detcolumn, 0, apvcolumn*detcolumn, apvrow*detrow, 0, apvrow*detrow);

    	TTree * tmax = new TTree("treemax","treemax");
    	tmax->Branch("value_max", &value_max);
    	tmax->Branch("nEVENT_max", &nEVENT_max);
    	tmax->Branch("nFEC_max", &nFEC_max);
    	tmax->Branch("nAPV_max", &nAPV_max);
    	tmax->Branch("nPAD_max", &nPAD_max);
    	tmax->Branch("nTIME_max", &nTIME_max);
    	tmax->Branch("XPAD_max", &XPAD_max);
    	tmax->Branch("YPAD_max", &YPAD_max);


    	Double_t value_cluster, XPOS_cluster, YPOS_cluster;
		Int_t nEVENT_cluster, nFEC_cluster, nAPV_cluster, nHIT_cluster, nTIME_cluster, nCLUSTER_cluster; 
    	TTree * tcluster = new TTree("treecluster","treecluster");
    	tcluster->Branch("value_cluster", &value_cluster);
    	tcluster->Branch("nEVENT_cluster", &nEVENT_cluster);
    	tcluster->Branch("nFEC_cluster", &nFEC_cluster);
    	tcluster->Branch("nAPV_cluster", &nAPV_cluster);
    	tcluster->Branch("nTIME_cluster", &nTIME_cluster);
    	tcluster->Branch("XPOS_cluster", &XPOS_cluster);
    	tcluster->Branch("YPOS_cluster", &YPOS_cluster);
    	tcluster->Branch("nHIT_cluster", &nHIT_cluster);
    	tcluster->Branch("nCLUSTER_cluster", &nCLUSTER_cluster);

		while (reader.Next()) {
			//std::cout<<*it_nEVENT<<std::endl;
			if ( *it_nEVENT==nEVENT_before){
				value_it = *it_value;
				if ( value_it > hit[*it_nAPV][*it_nPAD] && value_it>0 && value_it<1500){    //the same of max but not saving data if the peaks are in the head or in the tail of the time distribution
					if (hit[*it_nAPV][*it_nPAD]==0){
						iter++;
						itera[*it_nAPV][*it_nPAD]=iter;
					}
					//std::cout<<hit[*it_nAPV][*it_nPAD]<<" "<<*it_nPAD<<std::endl;
					hit[*it_nAPV][*it_nPAD] = value_it;
					used_apv[itera[*it_nAPV][*it_nPAD]]=*it_nAPV;
					used_pad[itera[*it_nAPV][*it_nPAD]]=*it_nPAD;
					used_time[itera[*it_nAPV][*it_nPAD]]=*it_nTIME;
				}
			}
			else{
				//std::cout<<nEVENT_before<<" "<<iter<<std::endl;
				if(!fFirstcycle){
					percent=(float(processedentries)/float(totalevents))*100.0;
                	if ((fmod(percent,1))==0){
                    	printf(" %i on total events %3.f %c \r",processedentries,percent,37);
                    	fflush(stdout);
                	}
					for (int i = 1; i < iter+1; ++i){
						//std::cout<<nEVENT_before<<" "<<iter<<" "<<nAPV<<std::endl;
						if ((used_time[i]>1) && used_time[i]<(totTiming-2)){
							nEVENT_max 	= *it_nEVENT;
							nFEC_max 	= 1;
							nAPV_max 	= used_apv[i];
							nPAD_max 	= used_pad[i];
							nTIME_max 	= used_time[i];
							value_max 	= hit[used_apv[i]][used_pad[i]];
							XPAD_max	= Xpadtograph(used_pad[i],used_apv[i]);
							YPAD_max	= Ypadtograph(used_pad[i],used_apv[i]);
							iter_clustering.push_back(i);
							tmax->Fill();
							std::cout<<"max_search "<<iter<<" "<<hit[used_apv[i]][used_pad[i]]<<" "<<used_pad[i]<<" "<<used_time[i]<<std::endl;
						}
						total_value_max = total_value_max + hit[used_apv[i]][used_pad[i]];
					}
					std::cout<<nEVENT_max<<" "<<iter_clustering.size()<<std::endl;
					std::vector<int>::iterator it_peak_erase = iter_clustering.begin();
					while(1==1){
						if (iter_clustering.size()==0){
							nCluster =0;
							break;
						}
						for (std::vector<int>::iterator it = iter_clustering.begin(); it !=iter_clustering.end();){
							if (hit[used_apv[*it]][used_pad[*it]] > hit_max){
								hit_max = hit[used_apv[*it]][used_pad[*it]];
								hit_peak = *it;
								it_peak_erase = it;
							}
							++it;
						}
						iter_clustering.erase(it_peak_erase);

						totCluster_charge += hit[used_apv[hit_peak]][used_pad[hit_peak]];
						X_Cluster += hit[used_apv[hit_peak]][used_pad[hit_peak]] * Xpadtograph(used_pad[hit_peak],used_apv[hit_peak]);
						Y_Cluster += hit[used_apv[hit_peak]][used_pad[hit_peak]] * Ypadtograph(used_pad[hit_peak],used_apv[hit_peak]);
						nHIT_percluster++;
						nCluster++;
						std::cout<<nEVENT_max<<" cluster central hit "<<hit_peak<<" "<<used_pad[hit_peak]<<" "<<used_time[hit_peak]<<" "<<iter_clustering.size()<<std::endl;

						for (std::vector<int>::iterator it2 = iter_clustering.begin(); it2 !=iter_clustering.end();){
							std::cout<<nEVENT_max<<" checking "<<*it2<<std::endl;
							if (it2 != iter_clustering.end() &&
								Xpadtograph(used_pad[*it2],used_apv[*it2])>Xpadtograph(used_pad[hit_peak],used_apv[hit_peak])-1.1 && 
								Xpadtograph(used_pad[*it2],used_apv[*it2])<Xpadtograph(used_pad[hit_peak],used_apv[hit_peak])+1.1 &&
								Ypadtograph(used_pad[*it2],used_apv[*it2])>Ypadtograph(used_pad[hit_peak],used_apv[hit_peak])-1.1 && 
								Ypadtograph(used_pad[*it2],used_apv[*it2])<Ypadtograph(used_pad[hit_peak],used_apv[hit_peak])+1.1 &&
								used_time[*it2]<=used_time[hit_peak]+1 &&
								used_time[*it2]>=used_time[hit_peak]-1 ) {
								totCluster_charge += hit[used_apv[*it2]][used_pad[*it2]];
								X_Cluster += hit[used_apv[*it2]][used_pad[*it2]] * Xpadtograph(used_pad[*it2],used_apv[*it2]);
								Y_Cluster += hit[used_apv[*it2]][used_pad[*it2]] * Ypadtograph(used_pad[*it2],used_apv[*it2]);
								std::cout<<nEVENT_max<<" add to cluster "<<*it2<<" "<<used_pad[*it2]<<" "<<used_time[*it2]<<std::endl;
								nHIT_percluster++;
								it2 = iter_clustering.erase(it2);
							}
							else{
								++it2;
							}
						}
						X_Cluster = X_Cluster/totCluster_charge;
						Y_Cluster = Y_Cluster/totCluster_charge;
						Hit_per_cluster_max->Fill(nHIT_percluster);
						cluster_hit->Fill(X_Cluster,Y_Cluster);
						Cluster_total_value_max->Fill(totCluster_charge);
						std::cout<<"end cluster "<<nCluster<<", with "<<nHIT_percluster<<" hit, Xpos "<<X_Cluster<<" Ypos "<<Y_Cluster<<", charge "<<totCluster_charge<<std::endl;

						value_cluster	= totCluster_charge;
    					nEVENT_cluster	= *it_nEVENT;
    					nFEC_cluster	= 1;
    					nAPV_cluster	= used_apv[hit_peak];
    					nTIME_cluster	= used_time[hit_peak];
    					XPOS_cluster	= X_Cluster;
    					YPOS_cluster	= Y_Cluster;
    					nHIT_cluster	= nHIT_percluster;
    					nCLUSTER_cluster= nCluster;
    					tcluster->Fill();

    					totCluster_charge = 0;
						X_Cluster = 0;
						Y_Cluster = 0;
						hit_peak = 0;
						hit_max = 0;
						nHIT_percluster=0;
						if (iter_clustering.size()==0){
							std::cout<<"end clustering "<<*it_nEVENT<<std::endl;
							break;
						}
					}
					Cluster_per_event_max->Fill(nCluster);
					nCluster =0;
					Event_multiplicity_max->Fill(iter);
					iter_clustering.clear();
					memset( hit, 0, 16*128*sizeof(Double_t) );
					memset( used_apv, 0, 2048*sizeof(Int_t) );
					memset( used_pad, 0, 2048*sizeof(Int_t) );
					memset( used_time, 0, 2048*sizeof(Int_t) );
					iter=0;
					value_it=0;
					Event_total_value_max->Fill(total_value_max);
					total_value_max = 0;
				}
				fFirstcycle=kFALSE;
				processedentries++;
				nEVENT_before = *it_nEVENT;
				value_it = *it_value;
				itera[*it_nAPV][*it_nPAD]=iter;
				if ( value_it > hit[*it_nAPV][*it_nPAD] && value_it>0 && value_it<1500){    //the same of max but not saving data if the peaks are in the head or in the tail of the time distribution
					if (hit[*it_nAPV][*it_nPAD]==0){
						iter++;
						itera[*it_nAPV][*it_nPAD]=iter;
					}
					hit[*it_nAPV][*it_nPAD] = value_it;
					used_apv[itera[*it_nAPV][*it_nPAD]]=*it_nAPV;
					used_pad[itera[*it_nAPV][*it_nPAD]]=*it_nPAD;
					used_time[itera[*it_nAPV][*it_nPAD]]=*it_nTIME;
				}

			}
		}
		printf("saving...\n");
    	tmax->Write();
    	rootfile->Write();
    	//rootfile->Close();

    	//TFile *file2 = TFile::Open(filename_out);
		if(!rootfile){
    		std::cout<<"ERROR -- root file not found"<<std::endl;
		} 

    	TTreeReader readermax("treemax", rootfile);
		TTreeReaderValue<int> it_nEVENT_max(readermax, "nEVENT_max");
		TTreeReaderValue<int> it_nFEC_max(readermax, "nFEC_max");
		TTreeReaderValue<int> it_nAPV_max(readermax, "nAPV_max");
		TTreeReaderValue<int> it_nPAD_max(readermax, "nPAD_max");
		TTreeReaderValue<int> it_nTIME_max(readermax, "nTIME_max");
		TTreeReaderValue<Double_t> it_value_max(readermax, "value_max");
		TTreeReaderValue<Int_t> it_XPAD_max(readermax, "XPAD_max");
		TTreeReaderValue<Int_t> it_YPAD_max(readermax, "YPAD_max");

		TH2F * MapGraphLego2_max= new TH2F ("Mapdistlego", "mapdistlego", apvcolumn*detcolumn, 0, apvcolumn*detcolumn, apvrow*detrow, 0, apvrow*detrow);
		TH2F * MapGraph2_max= new TH2F ("Mapdist2", "mapdist2", apvcolumn*detcolumn, 0, apvcolumn*detcolumn, apvrow*detrow, 0, apvrow*detrow);
		TH1F * EnergyHisto2_max = new TH1F("Energydist","Energydist",300,0,1500);
		TH1F * Timing2_max = new TH1F("Timing","Timing",totTiming,0,totTiming);
		std::cout<<apvcolumn<<" "<<detcolumn<<" xnum "<<apvcolumn*detcolumn<<", ynum "<<apvrow*detrow<<std::endl;
		while (readermax.Next()) {
			MapGraphLego2_max->Fill(*it_XPAD_max,*it_YPAD_max);
			MapGraph2_max->Fill(*it_XPAD_max,*it_YPAD_max);
			Timing2_max->Fill(*it_nTIME_max);
			EnergyHisto2_max->Fill(*it_value_max);
			processedentries++;
		}
		std::cout<<"processed entries "<<processedentries<<std::endl;
		gStyle->SetOptStat(0000);
		TCanvas * csinglepad= new TCanvas("Mapping","Mapping",1920,1024);

		MapGraphLego2_max->Draw("LEGO2");
		MapGraphLego2_max->SetTitle("Hit Spatial distribution (Lego plot)");
		MapGraphLego2_max->GetXaxis()->SetTitleOffset(1.6);
		MapGraphLego2_max->GetYaxis()->SetTitleOffset(1.8);
		MapGraphLego2_max->GetYaxis()->SetTitleOffset(1.8);
    	MapGraphLego2_max->GetYaxis()->SetTitle("Y axis [PAD]");
    	MapGraphLego2_max->GetXaxis()->SetTitle("X axis [PAD]");
    	MapGraphLego2_max->GetZaxis()->SetTitle("# Counts");
    	MapGraphLego2_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/Spatial_dist(lego).eps").c_str());
    	
    	MapGraph2_max->Draw("colz");
		MapGraph2_max->SetTitle("Hit Spatial distribution");
    	MapGraph2_max->GetYaxis()->SetTitle("Y axis [PAD]");
    	MapGraph2_max->GetXaxis()->SetTitle("X axis [PAD]");
    	MapGraph2_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/Spatial_dist(2D).eps").c_str());

		cluster_hit->Draw("colz");
		cluster_hit->SetTitle("Cluster distribution");
		cluster_hit->GetYaxis()->SetTitle("Y axis [PAD]");
    	cluster_hit->GetXaxis()->SetTitle("X axis [PAD]");
    	cluster_hit->GetZaxis()->SetTitle("# Counts");
    	cluster_hit->Write();
    	csinglepad->SaveAs((filename_out_path+"/cluster_dist.eps").c_str());

    	gPad->SetLogz(0);
    	EnergyHisto2_max->Draw();
		EnergyHisto2_max->SetTitle("Energy distribution");
    	EnergyHisto2_max->GetYaxis()->SetTitle("# Counts");
    	EnergyHisto2_max->GetXaxis()->SetTitle("Energy [ADC Ch]");
    	EnergyHisto2_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/Energy_dist.eps").c_str());

    	Timing2_max->Draw();
		Timing2_max->SetTitle("Timing distribution");
    	Timing2_max->GetYaxis()->SetTitle("# Counts");
    	Timing2_max->GetXaxis()->SetTitle("Time [timestamp units]");
    	Timing2_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/Time_dist.eps").c_str());

    	Event_multiplicity_max->Draw();
    	Event_multiplicity_max->SetTitle("Event Multiplicity distribution");
    	Event_multiplicity_max->GetYaxis()->SetTitle("# Counts");
    	Event_multiplicity_max->GetXaxis()->SetTitle("Multiplicity (N)");
    	Event_multiplicity_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/Multiplicity_dist.eps").c_str());

    	Hit_per_cluster_max->Draw();
    	Hit_per_cluster_max->SetTitle("Hit per Cluster");
    	Hit_per_cluster_max->GetYaxis()->SetTitle("# Counts");
    	Hit_per_cluster_max->GetXaxis()->SetTitle("Hit per cluster (N)");
    	Hit_per_cluster_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/hit_per_cluster_dist.eps").c_str());

    	Cluster_per_event_max->Draw();
    	Cluster_per_event_max->SetTitle("Cluster per event");
    	Cluster_per_event_max->GetYaxis()->SetTitle("# Counts");
    	Cluster_per_event_max->GetXaxis()->SetTitle("Cluster per event (N)");
    	Cluster_per_event_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/cluster_per event_dist.eps").c_str());

    	Event_total_value_max->Draw();
    	Event_total_value_max->SetTitle("Event total energy distribution");
    	Event_total_value_max->GetYaxis()->SetTitle("# Counts");
    	Event_total_value_max->GetXaxis()->SetTitle("Values (ADC Ch)");
    	Event_total_value_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/event_total_energy_dist.eps").c_str());

    	Cluster_total_value_max->Draw();
    	Cluster_total_value_max->SetTitle("Cluster total energy distribution");
    	Cluster_total_value_max->GetYaxis()->SetTitle("# Counts");
    	Cluster_total_value_max->GetXaxis()->SetTitle("Values (ADC Ch)");
    	Cluster_total_value_max->Write();
    	csinglepad->SaveAs((filename_out_path+"/cluster_total_energy_dist.eps").c_str());

    	rootfile->Close();
    	std::cout<<"File closed"<<std::endl;

	}

// find the max value from the fit on time stamp
	if (find_max =="fit"){
		find_max = "other";
	}

// find the max value from the fit on time stamp

	if (find_max =="testmapping"){
		std::cout<<"Test mapping"<<std::endl;
		gStyle->SetOptStat(0000);
		TCanvas * c= new TCanvas("Mapping","Mapping",1920,1024);
		TH2F * mappingtest= new TH2F ("Mappingtest", "mappingtest", apvcolumn*detcolumn, 0, apvcolumn*detcolumn, apvrow*detrow, 0, apvrow*detrow);
		for (int x = 0; x < ntotAPV[0]; x++){
			for (int y = 0; y < ntotCh_Pad; y++){
				std::cout<<y<<" "<<x<<" "<<Xpadtograph(y,x)+1<<" "<<Ypadtograph(y,x)+1<<std::endl;
				mappingtest->SetBinContent(Xpadtograph(y,x)+1,Ypadtograph(y,x)+1,1000*x+y);
			}
		}
		mappingtest->Draw("text");
		c->SaveAs((filename_out_path+"/mappingtest.eps").c_str());

	}

// collect all data of the time stamp
	if (find_max=="All" ){ 														
		std::cout<<"All values"<<std::endl;
		TH2F * MapGraph= new TH2F ("Maptest", "maptest", apvcolumn*detcolumn, 0, apvcolumn*detcolumn, apvrow*detrow, 0, apvrow*detrow);
		std::cout<<apvcolumn<<" "<<detcolumn<<" xnum "<<apvcolumn*detcolumn<<", ynum "<<apvrow*detrow<<std::endl;
		while (reader.Next()) {
			MapGraph->Fill(Xpadtograph(*it_nPAD,*it_nAPV),Ypadtograph(*it_nPAD,*it_nAPV));
			processedentries++;
		}
		std::cout<<"processed entries "<<processedentries<<std::endl;
		TCanvas * csinglepad= new TCanvas("Mapping","Mapping",960,1024);
		MapGraph->Draw("colz");
		MapGraph->SetTitle("Hit Spatial distribution (All data)");
    	MapGraph->GetYaxis()->SetTitle("Y axis (pads)");
    	MapGraph->GetXaxis()->SetTitle("X axis (pads)");
    	MapGraph->Write();
    	csinglepad->SaveAs((filename_out_path+"/Spatial_dist(2D).eps").c_str());
	}

	time_t now = time(0);
   	char* dt = ctime(&now);
   	ofstream log;
    log.open ("logfile", std::ofstream::out | std::ofstream::app);
    log<<dt<<" Analyzer "<<filename_in<<", max mode= "<<find_max<<", detector_settings: ntotFEC= "<<ntotFEC<<", ntotAPV= "<<ntotAPV[0]<<", ... , ntotCh_Pad= "<<ntotCh_Pad<<", totTiming= "<<totTiming<<"\n";
	log.close();
	std::cout<<"COMPLETE"<<std::endl;
}