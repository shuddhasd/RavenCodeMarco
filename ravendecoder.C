//Second part of RavenCode v1.3 Beta

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


int main(){
    std::cout<<"Start"<<std::endl;

// -------------------------------- LOAD ANALYSIS SETTINGS -------------------------------- //
    string filename_in;                     //variables for the analysis run stored in the txt file from the GUI
    string filename_out;
    int totalevents;
    int zerosup_limit;
    string sigma_filename; 
    string pedestal_filename;
    int commonmode =0;

    std::ifstream settings("decoder_settings");                 //analysis settings file
    
    if(!settings){
        std::cout<<"ERROR -- settings file not found"<<std::endl;
    } 
    
    while(settings.is_open()){
        if(settings.eof()){ printf("Settings LOADED\n"); break; }
        settings>>filename_in>>filename_out>>totalevents>>zerosup_limit>>sigma_filename>>pedestal_filename>>commonmode;
    }
        std::cout<<"qui"<<std::endl;
    std::string filename_out_path = filename_out.substr(0, filename_out.find_last_of('.')); //create the path for the other files (now not used)
    settings.close();

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
    TFile * rootfile = new TFile(filename_out.c_str(),"recreate");

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
    Bool_t Fzerosup = kFALSE;                      //flag to enable the zerosuppression
    Bool_t Fpedestal = kFALSE;                     //flag to eneble pedestal subtraction


    if (!(pedestal_filename=="no")){
        Fpedestal=kTRUE;
        std::cout<<"pedestal subtraction enable"<<std::endl;
    }
    else{
        Fpedestal=kFALSE;
        std::cout<<"pedestal subtraction disable"<<std::endl;
    }
    if (zerosup_limit == 0){
        Fzerosup = kFALSE;         // zero suppression flag
        std::cout<<"zero suppression disable"<<std::endl;
    }
    else {
        Fzerosup = kTRUE;          // zero suppression flag
        std::cout<<"zero suppression enable"<<std::endl;
    }
    if (commonmode == 0){
        Fzerosup = kFALSE;         // zero suppression flag
        std::cout<<"Common mode subtraction disable"<<std::endl;
    }
    else {
        Fzerosup = kTRUE;          // zero suppression flag
        std::cout<<"Common mode subtraction enable"<<std::endl;
    }

    ///////////////////////////////////////////////////////


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
    Double_t value_buffer[ntotCh_Pad] ;             //hit buffer used to store data before the common mode subtraction
    memset( value_buffer, 0, ntotCh_Pad*sizeof(Double_t) );
    Double_t nEVENT_buffer[ntotCh_Pad];
    memset( nEVENT_buffer, 0, ntotCh_Pad*sizeof(Double_t) );
    Double_t nFEC_buffer[ntotCh_Pad]  ;  
    memset( nFEC_buffer, 0, ntotCh_Pad*sizeof(Double_t) );
    Double_t nAPV_buffer[ntotCh_Pad]  ; 
    memset( nAPV_buffer, 0, ntotCh_Pad*sizeof(Double_t) );
    Double_t nPAD_buffer[ntotCh_Pad]  ;
    memset( nPAD_buffer, 0, ntotCh_Pad*sizeof(Double_t) );
    Double_t nTIME_buffer[ntotCh_Pad] ; 
    memset( nTIME_buffer, 0, ntotCh_Pad*sizeof(Double_t) );

    int hit=0, commonmode_events = 0, commonmode_sum =0;              //hit iterable
    Double_t commonmode_value =0;                  //commonmode

    ///////////////////////////////////////
    const int oneFECAPV = ntotAPV.at(nFEC_it);                  //for simplycity now limit on 1 FEC

    /////////////////////////////////////////////////////////////////////////////////////////////
    //---- Common-mode/pedestal and sigma variables 
    Double_t pedestal_map[ntotFEC][oneFECAPV][ntotCh_Pad];
    memset( pedestal_map, 0, ntotFEC*oneFECAPV*ntotCh_Pad*sizeof(Double_t) );
    Double_t sigma_map[ntotFEC][oneFECAPV][ntotCh_Pad];
    memset( sigma_map, 0, ntotFEC*oneFECAPV*ntotCh_Pad*sizeof(Double_t) );
	
    //---- fill the array from the txt pedestal file
    if (Fpedestal){
        int fec,apv,pad;
        Double_t value;
        std::ifstream file_ped(pedestal_filename.c_str());
        if(!file_ped){
            std::cout<<"ERROR -- pedestal file not found"<<std::endl;
        } 
        else {
            std::cout<<"Opened "<<pedestal_filename<<std::endl;
            while(file_ped.good()){
                if(file_ped.eof()){ printf("END\n"); break; }
                file_ped>>fec>>apv>>pad>>value;
                pedestal_map[fec][apv][pad] = value;
            }   
        }
        file_ped.close();
    }

    //---- fill the array from the txt sigma file
    if (Fzerosup){
        int fec,apv,pad;
        Double_t value;
        std::ifstream file_sigma(sigma_filename.c_str());
        if(!file_sigma){
            std::cout<<"ERROR -- sigma file not found"<<std::endl;
        } 
        else {
            std::cout<<"Opened "<<sigma_filename<<std::endl;
            while(file_sigma.good()){
                if(file_sigma.eof()){ printf("END\n"); break; }
                file_sigma>>fec>>apv>>pad>>value;
                sigma_map[fec][apv][pad] = value;
            }   
        }
        file_sigma.close();
    }
    /////////////////////////////////////////////////////////////////////////////////////////////

	Int_t nEVENT, nFEC, nAPV, nPAD, nTIME;
    Double_t value;
    TTree * t = new TTree("treedecoded","treedecoded");            //tree initialization
    t->Branch("value", &value);
    t->Branch("nEVENT", &nEVENT);
    t->Branch("nFEC", &nFEC);
    t->Branch("nAPV", &nAPV);
    t->Branch("nPAD", &nPAD);
    t->Branch("nTIME", &nTIME);
	

	std::ifstream file(filename_in.c_str(), std::ios::in | std::ios::binary);          //opening raw data file
	
	if(!file){
    	std::cout<<"ERROR -- raw file not found"<<std::endl;
	} 
	
	else {
		std::cout<<"Opened "<<filename_in<<std::endl;
    	while(file.good()){

            ///// Preparing Raw data
        	if(file.eof()){ printf("END\n"); break; }
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
                    std::cout<<"APV number from data > ntotAPV"<<std::endl;
                    std::cout<<Raw32bit<<" "<< logicanalizer(word16bit) <<" "<<word16bit<<" "<<std::endl;
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

                //percent display
                timing=0;
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

                if (nAPV_it > ntotAPV[nFEC_it]){                //sometime it reads the data not in the right way, so it reads more APV than the total number
                    std::cout<<"APV number from data > ntotAPV"<<std::endl;
                    std::cout<<Raw32bit<<" "<< logicanalizer(word16bit) <<" "<<word16bit<<" "<<std::endl;
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
                                // IMPORTANT POINT pedestal subtraction
                                if (Fpedestal){
                                    word16bit = -(word16bit-Double_t(pedestal_map[nFEC_it][nAPV_it][nPAD_it_ordered]));

                                }
                                // IMPORTANT POINT Zero suppression enable
                                if (Fzerosup){
                                    if (commonmode==0){
                                        if (word16bit > zerosup_limit*Double_t(sigma_map[nFEC_it][nAPV_it][nPAD_it_ordered])){
                                            value      = word16bit;
                                            nEVENT     = eventTriggered;
                                            nFEC       = nFEC_it;
                                            nAPV       = nAPV_it;
                                            nPAD       = nPAD_it_ordered;
                                            nTIME      = timing;
                                            t->Fill();              //fill a new entry in the tree
                                        }
                                    }
                                    if  (commonmode==1){
                                        if (word16bit > zerosup_limit*Double_t(sigma_map[nFEC_it][nAPV_it][nPAD_it_ordered])){
                                            value_buffer[hit]      = word16bit;
                                            nEVENT_buffer[hit]     = eventTriggered;
                                            nFEC_buffer[hit]       = nFEC_it;
                                            nAPV_buffer[hit]       = nAPV_it;
                                            nPAD_buffer[hit]       = nPAD_it_ordered;
                                            nTIME_buffer[hit]      = timing;
                                            hit++; 
                                        }
                                        else{
                                            commonmode_sum += word16bit;
                                            commonmode_events++;
                                        }
                                    }
                                }
                                // IMPORTANT POINT Zero suppression disable
                                if (!Fzerosup){
                                    value      = word16bit;
                                    nEVENT     = eventTriggered;
                                    nFEC       = nFEC_it;
                                    nAPV       = nAPV_it;
                                    nPAD       = nPAD_it_ordered;
                                    nTIME      = timing;
                                    t->Fill();              //fill a new entry in the tree
                                }

                            if (debug2){
                               std::cout<<"event n "<<eventTriggered<<", fec "<<nFEC_it<<", APV "<<nAPV_it<<", pad "<<nPAD_it<<", time "<<timing<<", value "<<word16bit<<", raw "<<Raw32bit<<std::endl;
                            }

     	  					nPAD_it++;

                            if ( nPAD_it > ntotCh_Pad-1){              //if the pads are finished restart count the 12bit of the divisor

                                //filling the entries in the tree subtracting common mode
                                if (commonmode==1){
                                    commonmode_value =Double_t(commonmode_sum)/Double_t(commonmode_events);
                                    //std::cout<<"common mode "<<commonmode_value<<std::endl;
                                    for (int k = 0; k < hit; ++k){
                                        value      = value_buffer[k]-commonmode_value;
                                        nEVENT     = nEVENT_buffer[k];
                                        nFEC       = nFEC_buffer[k];
                                        nAPV       = nAPV_buffer[k];
                                        nPAD       = nPAD_buffer[k] ;
                                        nTIME      = nTIME_buffer[k] ;
                                        t->Fill();
                                    }
                                    hit = 0;
                                    commonmode_value=0;
                                    commonmode_events=0;
                                    commonmode_sum=0;
                                    memset( value_buffer , 0, ntotCh_Pad*sizeof(Double_t) );
                                    memset( nEVENT_buffer, 0, ntotCh_Pad*sizeof(Double_t) );
                                    memset( nFEC_buffer  , 0, ntotCh_Pad*sizeof(Double_t) );
                                    memset( nAPV_buffer  , 0, ntotCh_Pad*sizeof(Double_t) );
                                    memset( nPAD_buffer  , 0, ntotCh_Pad*sizeof(Double_t) );
                                    memset( nTIME_buffer , 0, ntotCh_Pad*sizeof(Double_t) );
                                }

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
                        if (logicanalizer(word16bit)==0){
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

/////////// Prima occhiata /////////////////

    std::cout<<"Decoding complete of "<<filename_in<<std::endl;
    std::cout<<"Found "<<eventTriggered<<" events"<<std::endl;
    std::cout<<"Processed entries "<<processedentries<<", a mean of "<< float(processedentries)/float(eventTriggered) << ", with "<<ntotCh_Pad*totTiming<<" entries per pad per APV per event"<< std::endl;
    if (debug){
        std::cout<<"it will take a long time"<<std::endl;

        TCanvas * vvsp= new TCanvas("Value per pad (All APV)","Value per pad (All APV)",1920,1024);
        t->Draw("value:nPAD","nAPV==0");
        TGraph * graphvvsp = (TGraph*)gPad->GetPrimitive("Graph");
        //graphvvsp ->SetMarkerStyle(2);
        graphvvsp ->SetMarkerColor(1);
        TH1F *histovvsp = (TH1F*)gPad->GetPrimitive("htemp");
        histovvsp ->SetTitle("Value per pad (All APV)");
        histovvsp ->GetYaxis()->SetTitle("Value (Ch)");
        histovvsp ->GetXaxis()->SetTitle("Pad (n)");
        graphvvsp->Write();
    
        TCanvas * vpa= new TCanvas("Value per APV (All pad)","Value per APV (All pad)",1920,1024);
        t->Draw("value:nAPV");
        TGraph * graphvpa = (TGraph*)gPad->GetPrimitive("Graph");
        graphvpa->SetMarkerStyle(2);
        graphvpa->SetMarkerColor(1);
        TH1F *histovpa = (TH1F*)gPad->GetPrimitive("htemp");
        histovpa ->SetTitle("Value per APV (All pad)");
        histovpa ->GetYaxis()->SetTitle("Value (Ch)");
        histovpa ->GetXaxis()->SetTitle("APV (n)");
        graphvpa->Write();
    
        TCanvas * v= new TCanvas("Value (All APV and pad)","Value (All APV and pad)",1920,1024);
        t->Draw("value","nAPV==0");
        TH1F *histov = (TH1F*)gPad->GetPrimitive("htemp");
        histov->SetTitle("Value (All APV and pad)");
        histov->GetYaxis()->SetTitle("Value (Ch)");
        histov->GetXaxis()->SetTitle("Time frame (n)");
        histov->Write();
    }
    printf("saving...\n");
    t->Write();
    rootfile->Write();
    rootfile->Close();
    std::cout<<"File closed"<<std::endl;

// -------------------------------- Write in the log file -------------------------------- //
    time_t now = time(0);
    char* dt = ctime(&now);
    ofstream log;
    log.open ("logfile", std::ofstream::out | std::ofstream::app);
    log<<dt<<" Decoder "<<filename_in<<", pedestal= "<<pedestal_filename<<", sigma= "<<sigma_filename<<", pedestal subtraction= "<<Fpedestal<< ", zero suppression= "<<Fzerosup<<", number of sigma zerosup.= "<<zerosup_limit<<", detector_settings: ntotFEC= "<<ntotFEC<<", ntotAPV= "<<ntotAPV[0]<<", "<<ntotAPV[1]<<", "<<ntotAPV[2]<<"... , ntotCh_Pad= "<<ntotCh_Pad<<", totTiming= "<<totTiming<<"\n";
    log.close();
    std::cout<<"COMPLETE"<<std::endl;
}

