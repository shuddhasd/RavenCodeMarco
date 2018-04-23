//First part of RavenCode v1.2 Beta

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

//-------- function for the decoding

int hextoint(char * hex, int part){         //function to choose the first or the second part of the 32bit word
	char thex[4];                           //(needed because the first and the second part must be swapped)
	if (part == 1){ 
		thex[0] = hex[0];
		thex[1] = hex[1];
		thex[2] = hex[2];
		thex[3] = hex[3]; 
	}
	if (part == 2){ 
		thex[0] = hex[4];
		thex[1] = hex[5];
		thex[2] = hex[6];
		thex[3] = hex[7]; 
	}
	return strtoul(thex, NULL, 16);
}

int logicanalizer(int value){               //function to check if it is a digital or analog value
	if (value <1360){return 1;}             //the constant value can be change depending on the GND used
	if (value >2800){return 0;}
	else {return 2;}
}

//-------- MAPPING FUNCTION

int apvrow, apvcolumn;                      //global variables for the mapping, names are self-explicative
int detrow, detcolumn;
std::vector<int> apvmapping;
std::vector<int> detmapping;

    

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

// -------------------------------- LOAD ANALYSIS SETTINGS -------------------------------- //
    string filename_in;             //variables for the analysis run stored in the txt file from the GUI
    string filename_out;
    int totalevents =0;

    std::ifstream settings("pedestal_analysis_settings");                 //analysis settings file
    if(!settings){
        std::cout<<"ERROR -- settings file not found"<<std::endl;
    } 

    while(settings.is_open()){
        if(settings.eof()){ printf("Settings LOADED\n"); break; }
       settings>>filename_in>>filename_out>>totalevents;
    }

    std::string filename_out_path = filename_out.substr(0, filename_out.find_last_of('.')); //create the path for the other files
    settings.close();

// -------------------------------- LOAD MAPPINGS -------------------------------- //

    int apvdim=0, detdim=0;                 //I know, there are some better ways to do this, but this is vary simple and self-explicative
    int apvmat=0, detmat=0;
    char readvar[10];
    int i =0;

    std::ifstream apvmap("apv_mapping");     //apv mapping file
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

    std::ifstream detmap("detector_mapping_hybrid");     //apv mapping file

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

// -------------------------------- LOAD DETECTOR SETTINGS -------------------------------- //
    int ntotFEC = 0;
    std::vector<int> ntotAPV;
    int ntotCh_Pad = 0;
    int totTiming = 0;

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
        detector>>ntotCh_Pad>>totTiming;
    }
    detector.close();

// -------------------------------- DECODER -------------------------------- //
    //It's complicated to explain exactly what it do, read the manual to ha some ideas
    TFile * rootfile = new TFile(filename_out.c_str(),"recreate");              //open the root file where we will store the data

	///////////////////////////////////////////////////////
    bool debug = kFALSE;                        // debug flag (simple)
    bool debug2 = kFALSE;                       // debug flag (hex)
    ///////////////////////////////////////////////////////

    //some flag used in the decoder section
	Bool_t FcloseTriggerEvent = kTRUE;             //xfafafafa flag, end event
	Bool_t FopenTriggerEvent = kFALSE;             //x00434441 flag, start new event
	Bool_t FreadingData = kFALSE;                  //x434441 flag, new APV to read
	Bool_t Fstart128data = kFALSE;                 //exceeded syncronization part
    Bool_t Fsrsheader = kFALSE;                    //flag whne the header is expected

    uint8_t block1, block2, block3, block4;        //for uint8_t that will be saved in Raw32bit (necessary to have the right format and order)
	char Raw32bit[9];                              //32bit data from the raw file, the two 16bit data must be flipped
	int word16bit=0;                               //one of the two 16bit data

	int eventTriggered=0;                          //event number (one for a complete time interval with all APV)
	int nFEC_it = 0;                               //FEC number (number counted)
	int nAPV_it=0;                                 //APV number find in the rawfile (before 434441)
	int nPAD_it=0;                                 //APV channel number (number counted)
	int nPAD_it_ordered=0;                         //PAD number (number counted)
	int timing=0;                                  //timestamp number
	int header111=3;                               //variable to count the bit in the header
	int logicDivisorcounter=0;                     //variable to count the bit in the division between timestamp values
	char APVchar[2];                               //simple way to obtain the APV number
    float percent;                                 //only for the terminal output
    int processedentries = 0;                      //entries number (one for each analog value)

	///////////////////////////////////////
    const int oneFECAPV = ntotAPV.at(nFEC_it);                      //for simplycity now limit on 1 FEC

    //---- pedestal mean variables
    int mean_on[ntotFEC][oneFECAPV][ntotCh_Pad];
    memset( mean_on, 0, ntotFEC*oneFECAPV*ntotCh_Pad*sizeof(int) );
    Double_t mean[ntotFEC][oneFECAPV][ntotCh_Pad];
    memset( mean, 0, ntotFEC*oneFECAPV*ntotCh_Pad*sizeof(Double_t) );

    //---- Sigma pedestal mean variables
    Double_t sigma[ntotFEC][oneFECAPV][ntotCh_Pad];
    memset( sigma, 0, ntotFEC*oneFECAPV*ntotCh_Pad*sizeof(Double_t) );
    int sigma_on[ntotFEC][oneFECAPV][ntotCh_Pad];
    memset( sigma_on, 0, ntotFEC*oneFECAPV*ntotCh_Pad*sizeof(int) );

    //////////////////////////////////////////////

    Int_t nEVENT, nFEC, nAPV, nPAD, nTIME;
    Double_t value;
	TTree * t = new TTree("pedestal","pedestal");            //tree initialization
    t->Branch("value", &value);
    t->Branch("nEVENT", &nEVENT);
    t->Branch("nFEC", &nFEC);
    t->Branch("nAPV", &nAPV);
    t->Branch("nPAD", &nPAD);
    t->Branch("nTIME", &nTIME);

    //preparing histograms for "some channel distribution"
    TH1D * value_dist1= new TH1D("ADC values distribution APV 0, single PAD","ADC values distribution APV 0, single PAD",400,2600,3000); 
    TH1D * value_dist2= new TH1D("ADC values distribution APV 3, single PAD","ADC values distribution APV 3, single PAD",400,2600,3000); 
    TH1D * value_dist3= new TH1D("ADC values distribution APV 5, single PAD","ADC values distribution APV 5, single PAD",400,2600,3000); 
    TH1D * value_dist4= new TH1D("ADC values distribution APV 8, single PAD","ADC values distribution APV 8, single PAD",400,2600,3000);

	std::ifstream file(filename_in.c_str(), std::ios::in | std::ios::binary);          //opening raw data file
	
	if(!file){
    	std::cout<<"error opening file"<<std::endl;
	} 
	
	else {
		std::cout<<"Opening file "<<filename_in<<std::endl;
    	while(file.good()){

        	if(file.eof()){ printf("END\n"); break; }

            ///// Preparing Raw data
            file>>std::noskipws;
            file>>block1>>block2>>block3>>block4;
            sprintf(Raw32bit, "%02x%02x%02x%02x", block1, block2, block3, block4);

            ///// search for END EVENT
            if (std::string(Raw32bit) == "fafafafa"){
                FopenTriggerEvent = kFALSE;
                FcloseTriggerEvent = kTRUE;
                FreadingData = kFALSE;

                logicDivisorcounter=0;
                nPAD_it=0;
                Fstart128data = kFALSE;
                timing++;
                eventTriggered++;

                if (debug){
                    std::cout<<Raw32bit<<" End event"<<std::endl;
                }
                if (debug2){
                    if (eventTriggered >0){
                       break;
                    }
                }
            }

            ///// search for NEW APV
            if (std::string(Raw32bit).substr(2,6) == "434441" && FopenTriggerEvent) {
                FreadingData = kTRUE;
                Fsrsheader = kTRUE;
                Fstart128data = kFALSE;
                header111=3;

                logicDivisorcounter=0;
                nPAD_it=0;

                //taking the APV number from data
                APVchar[0] = Raw32bit[0];
                APVchar[1] = Raw32bit[1];
                nAPV_it = strtoul(APVchar, NULL, 16);

                timing=0;
                if (debug){
                    std::cout<<"Apv number "<<nAPV_it<<std::endl;
                }
                if (nAPV_it > ntotAPV[nFEC_it]){                //sometime it reads the data not in the right way, so it reads more APV than the total number
                    std::cout<<"APV number from raw data > ntotAPV"<<std::endl;
                    std::cout<<Raw32bit<<" "<< logicanalizer(word16bit) <<" "<<word16bit<<" "<<nAPV_it<<" "<<ntotAPV.at(nFEC_it)<<std::endl;
                    break;
                }
            }

            // search for NEW EVENT 
            if (std::string(Raw32bit).substr(2,6) == "434441" && FcloseTriggerEvent){
                FcloseTriggerEvent = kFALSE;
                FopenTriggerEvent = kTRUE;
                Fsrsheader = kTRUE;
                FreadingData = kTRUE;
                header111=3;

                //taking the APV number from data
                APVchar[0] = Raw32bit[0];
                APVchar[1] = Raw32bit[1];
                nAPV_it = strtoul(APVchar, NULL, 16);

                timing=0;

                //percent display
                percent=(float(eventTriggered)/float(totalevents))*100.0;
                if ((fmod(percent,1))==0){
                    printf(" %i on total events %3.f %c \r",eventTriggered,percent,37);
                    fflush(stdout);
                    t->Write();
                }
                if (debug){
                    std::cout<<"New event, number "<<eventTriggered<<std::endl;
                    std::cout<<"Apv number "<<nAPV_it<<std::endl;
                }

                if (nAPV_it > ntotAPV.at(nFEC_it) ){
                    std::cout<<"APV number from data > ntotAPV"<<std::endl;
                    std::cout<<Raw32bit<<" "<< logicanalizer(word16bit) <<" "<<word16bit<<" "<<nAPV_it<<" "<<ntotAPV.at(nFEC_it)<<std::endl;
                    break;
                }
            }

            
     	  	///// reading data from a single apv
     	  	if (FreadingData){
     	  		for (int i = 0; i < 2; ++i){              //read fistly the second 16bit and after the first one
                    word16bit = hextoint(Raw32bit,2-i); 

	     	  		if (!Fsrsheader && timing < totTiming){      //there are at the maximum totTiming header in a single APV

     	  				if (!Fstart128data){                     //if used to exceed the other 9 digital values after the header (di divisor if formed by 12 bit)
                            if (debug2){
                                std::cout<<logicDivisorcounter<<" "<<Raw32bit<<" "<<word16bit<<" "<<logicanalizer(word16bit)<<std::endl;
     	  					}
                            logicDivisorcounter++;
     	  				}

     	  				if (!Fstart128data && logicDivisorcounter==13){ //exceeded the program start reading the analog values
     	  					Fstart128data = kTRUE;
     	  				}

     	  				if (Fstart128data){     //reading analog values

                                nPAD_it_ordered = 32 * (nPAD_it%4) + 8 * int(nPAD_it/4) - 31 * int( nPAD_it/16 );    //APV internal pin ordering
                                mean[nFEC_it][nAPV_it][nPAD_it_ordered] = mean[nFEC_it][nAPV_it][nPAD_it_ordered] + word16bit;    //starting from now to calculate pedestal
                                mean_on[nFEC_it][nAPV_it][nPAD_it_ordered]++;           //and the divisor for the mean
                                value      = word16bit;
                                nEVENT     = eventTriggered;
                                nFEC       = nFEC_it;
                                nAPV       = nAPV_it;
                                nPAD       = nPAD_it_ordered;
                                nTIME      = timing;
                                t->Fill();              //fill a new entry in the tree
                                if(nPAD_it_ordered==17 && nAPV_it==0){
                                    value_dist1->Fill(word16bit);
                                }
                                if(nPAD_it_ordered==55 && nAPV_it==3){
                                    value_dist2->Fill(word16bit);
                                }
                                if(nPAD_it_ordered==89 && nAPV_it==5){
                                    value_dist3->Fill(word16bit);
                                }
                                if(nPAD_it_ordered==117 && nAPV_it==8){
                                    value_dist4->Fill(word16bit);
                                }


                            if (debug2){
                               std::cout<<"event n "<<eventTriggered<<", fec "<<nFEC_it<<", APV "<<nAPV_it<<", pad "<<nPAD_it<<", time "<<timing<<", value "<<word16bit<<", raw "<<Raw32bit<<std::endl;
                            }
		
     	  					nPAD_it++;

     	  					if ( nPAD_it > ntotCh_Pad-1){              //if the pad ara finished restart count the 12bit of the divisor
     	  						logicDivisorcounter=0;
     	  						nPAD_it=0;
     	  						Fstart128data = kFALSE;
     	  						timing++;
     	  					}
                            processedentries++;

     	  				}
     	  			}

                    if (Fsrsheader && header111>0){         //generally first "if" to be reach, the goal is to find 3 digital values one after the other
                        if (logicanalizer(word16bit)==1){
                            header111--;
                            if (debug2){
                                std::cout<<header111<<" "<<Raw32bit<<" "<<word16bit<<" "<<logicanalizer(word16bit)<<std::endl;
                            }
                        }
                        if (logicanalizer(word16bit)==0 || logicanalizer(word16bit)==2){
                            header111=3;
                        }
                    }        

                    if (header111==0){                     //obtained 3 digital values, exceeded the apv header
                        Fsrsheader = kFALSE;
                        header111=-1;
                        logicDivisorcounter = 3;
                    }
	     	  	}
     	  	} 
    	}
    }
    file.close();               //rawfile close
    rootfile->cd();

    TCanvas * valdist= new TCanvas("Values distribution","Values distribution",1920,1024);      //save the single channel distribution histograms
    valdist->Divide(2,2);
    valdist->cd(1);
    value_dist1->Draw();
    value_dist1->Fit("gaus");
    valdist->cd(2);
    value_dist2->Draw();
    value_dist2->Fit("gaus");
    valdist->cd(3);
    value_dist3->Draw();
    value_dist3->Fit("gaus");
    valdist->cd(4);
    value_dist4->Draw();
    value_dist4->Fit("gaus");
    valdist->SaveAs((filename_out_path+"_Energy_dist.pdf").c_str());

    printf("saving...\n");
    t->Write();
    rootfile->Write();
    rootfile->Close();
    

    std::cout<<"Root file closed"<<std::endl;
    std::cout<<""<<std::endl;
    std::cout<<"Decoding complete of "<<filename_in<<std::endl;
    std::cout<<"Found "<<eventTriggered<<" events, processed entries "<<processedentries<<std::endl;
    std::cout<<""<<std::endl;

// -------------------------------- PEDESTAL calculation -------------------------------- //
    std::cout<<"Mean pedestal calculation"<<std::endl;

    TCanvas * pedval= new TCanvas("Pedestal Value","Pedestal Value",1920,1024);
    pedval->Divide(detcolumn,detrow);
    int canvaspad =1;
    pedval->cd(canvaspad);
    gStyle->SetOptStat(0000);

    TH1D * ped_dist= new TH1D("Pedestal distribution","Pedestal distribution",128,0,128);
    ped_dist ->SetTitle("APV pedestal distribution");
    ped_dist ->GetYaxis()->SetTitle("Mean value [ADC Ch]");
    ped_dist ->GetXaxis()->SetTitle("PAD No.");
    ped_dist->SetMaximum(2850);
    ped_dist->SetMinimum(2450);

    TH2D * MapGraph= new TH2D ("pedestal map", "pedestal map", apvcolumn*detcolumn, 0, apvcolumn*detcolumn, apvrow*detrow, 0, apvrow*detrow);
    MapGraph ->SetTitle("Pedestal map");
    MapGraph ->GetXaxis()->SetTitle("X axis [PAD]");
    MapGraph ->GetYaxis()->SetTitle("Y axis [PAD]");
    MapGraph ->GetZaxis()->SetTitle("Mean value [ADC Ch]");

    TH1D * Mean_dist= new TH1D("Mean distribution","Mean distribution",250,2000,3000);
    Mean_dist ->SetTitle("Mean distribution");
    Mean_dist ->GetXaxis()->SetTitle("Mean value [ADC Ch]");
    Mean_dist ->GetYaxis()->SetTitle("# Counts");

    ofstream fileped;
    fileped.open((filename_out_path+".pede").c_str());
    for (int i = 0; i < ntotFEC; ++i){
        for (int a = 0; a < ntotAPV.at(i); ++a){
            for (int b = 0; b < ntotCh_Pad; ++b){
                //mean for each channel to calculate the pedestal
                mean[i][a][b]=mean[i][a][b]/float(mean_on[i][a][b]);
                ped_dist->SetBinContent(b,0);
                if (!std::isnan(mean[i][a][b])){
                    MapGraph->SetBinContent(Xpadtograph(b,a)+1,Ypadtograph(b,a)+1,mean[i][a][b]);
                    Mean_dist->Fill(mean[i][a][b]);
                    ped_dist->SetBinContent(b,mean[i][a][b]);
                    if (debug){
                        std::cout<<mean[i][a][b]<<"\t x "<<Xpadtograph(b,a)+1<<"\t y "<<Ypadtograph(b,a)+1<<std::endl;
                    }
                }
                fileped<<i<<"\t"<<a<<"\t"<<b<<"\t"<<mean[i][a][b]<<"\n";            //saving pedestal data in the txt file
            }
            if (canvaspad<detrow*detcolumn+1){
                ped_dist->DrawClone();
                canvaspad++;
                pedval->cd(canvaspad);
            }
        }
    }
    fileped.close();
    pedval->SaveAs((filename_out_path+"_apvpeddist.pdf").c_str());

    gStyle->SetOptStat(1111);
    TCanvas * vp= new TCanvas("Distribution of the mean values per channel","Distribution of the mean values per channel",1920,1024);

    MapGraph->Draw("colz");
    vp->SaveAs((filename_out_path+"_mean.pdf").c_str());

    Mean_dist->Draw();
    vp->SaveAs((filename_out_path+"_meandist.pdf").c_str());

    std::cout<<"END - Mean pedestal calculation"<<std::endl;

// -------------------------------- SIGMA calculation -------------------------------- //

    std::cout<<"Sigma pedestal calculation"<<std::endl;

    //Reopen the ROOT file to compute the sigmas
        TFile *file_sigma_in = TFile::Open(filename_out.c_str());
        if(!file_sigma_in){
            std::cout<<"error opening file"<<std::endl;
        } 
    
        TTreeReader reader("pedestal", file_sigma_in);
        TTreeReaderValue<int> it_nEVENT(reader, "nEVENT");
        TTreeReaderValue<int> it_nFEC(reader, "nFEC");
        TTreeReaderValue<int> it_nAPV(reader, "nAPV");
        TTreeReaderValue<int> it_nPAD(reader, "nPAD");
        TTreeReaderValue<int> it_nTIME(reader, "nTIME");
        TTreeReaderValue<Double_t> it_value(reader, "value");
    
        while (reader.Next()) {             //sum for each channel to calculate the sigma
            sigma[*it_nFEC][*it_nAPV][*it_nPAD]=sigma[*it_nFEC][*it_nAPV][*it_nPAD]+TMath::Power( *it_value -mean[*it_nFEC][*it_nAPV][*it_nPAD],2);
            sigma_on[*it_nFEC][*it_nAPV][*it_nPAD]++;
        }
        file_sigma_in->Close();
    //close ROOT file
    
    TH2D * MapGraph2= new TH2D ("Sigma pedestal map", "Sigma pedestal map", apvcolumn*detcolumn, 0, apvcolumn*detcolumn, apvrow*detrow, 0, apvrow*detrow);
    MapGraph2 ->SetTitle("Sigma map");
    MapGraph2 ->GetXaxis()->SetTitle("X axis [PAD]");
    MapGraph2 ->GetYaxis()->SetTitle("Y axis [PAD]");
    MapGraph2 ->GetZaxis()->SetTitle("Sigma value [ADC Ch]"); 

    TH1D * Sigma_dist= new TH1D("Sigma distribution","Sigma distribution",500,0,100); 
    Sigma_dist ->SetTitle("Sigma distribution");
    Sigma_dist ->GetXaxis()->SetTitle("Sigma value [ADC Ch]");
    Sigma_dist ->GetYaxis()->SetTitle("# Counts");

    ofstream filesigma_out;
    filesigma_out.open ((filename_out_path+".sigma").c_str());
    for (int i = 0; i < ntotFEC; ++i){
        for (int a = 0; a < ntotAPV.at(i); ++a){
            for (int b = 0; b < ntotCh_Pad; ++b){
                sigma[i][a][b]=TMath::Power(sigma[i][a][b]/float(sigma_on[i][a][b]),0.5);    //calculate the sigma value
                if (!std::isnan(sigma[i][a][b])){
                    MapGraph2->SetBinContent(Xpadtograph(b,a)+1,Ypadtograph(b,a)+1,sigma[i][a][b]);
                    Sigma_dist->Fill(sigma[i][a][b]);
                    if (debug){
                        std::cout<<sigma[i][a][b]<<"\t x "<<Xpadtograph(b,a)+1<<"\t y "<<Ypadtograph(b,a)+1<<std::endl;
                    }
                }
                filesigma_out<<i<<"\t"<<a<<"\t"<<b<<"\t"<<sigma[i][a][b]<<"\n";         //save sigmas in txt file
            }
        }
    }
    filesigma_out.close();

    TCanvas * vp2= new TCanvas("Distribution of the sigma values per channel","Distribution of the sigma values per channel",1920,1024);
    MapGraph2->Draw("colz");
    vp2->SaveAs((filename_out_path+"_sigma.pdf").c_str());

    Sigma_dist->Draw();
    vp2->SaveAs((filename_out_path+"_sigmadist.pdf").c_str());

    std::cout<<"END - Sigma pedestal calculation"<<std::endl;
// -------------------------------- Write in the log file -------------------------------- //

    time_t now = time(0);
    char* dt = ctime(&now);
    ofstream log;
    log.open ("logfile", std::ofstream::out | std::ofstream::app);
    log<<dt<<" pedestal "<<filename_in<<", detector_settings: ntotFEC= "<<ntotFEC<<", ntotAPV= "<<ntotAPV[0]<<", "<<ntotAPV[1]<<", "<<ntotAPV[2]<<"... , ntotCh_Pad= "<<ntotCh_Pad<<", totTiming= "<<totTiming<<"\n";
    log.close();
    
    std::cout<<"COMPLETE"<<std::endl;
    return 0;
}