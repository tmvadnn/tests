#include<TROOT.h>

#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/DataSetInfo.h"
#include "TMVA/Config.h"

#include "TFile.h"
#include "TTree.h"

void RNN_test() {

//    ROOT::EnableImplicitMT(1);
   TMVA::Config::Instance();
   TFile *input(0);

   std::cout << "nthreads  = " << ROOT::GetImplicitMTPoolSize() << std::endl;

   TString fname = "imagesRnn.root";
   input = TFile::Open( fname ); // check if file in local directory exists
 
   if (!input) {
      std::cout << "ERROR: could not open data file" << std::endl;
      exit(1);
   }

   std::cout << "--- RNNClassification  : Using input file: " << input->GetName() << std::endl;


  // Create a ROOT output file where TMVA will store ntuples, histograms, etc.
   TString outfileName( "data.root" );
   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

   // Creating the factory object
   TMVA::Factory *factory = new TMVA::Factory( "TMVAClassification", outputFile,
                                               "!V:!Silent:Color:DrawProgressBar:Transformations=I:AnalysisType=Classification:!ModelPersistence" );
   TMVA::DataLoader *dataloader=new TMVA::DataLoader("dataset");

   TTree *signalTree     = (TTree*)input->Get("sgn");
   TTree *background     = (TTree*)input->Get("bkg");


   signalTree->Print();

   background->Print();
// add variables (time zero and time 1)
// for (int i = 0; i < 32; ++i) {
//     for (int j = 0; j < 32; ++j) {
//         if(i>=12&&i<20)
//         {
//             if(j>=12&&j<20)
//             {
//                 int ivar=i*32+j;
//                 TString varName = TString::Format("EBenergyRed[%d]",ivar);
// //                 std::cout<<Form("(%d,%d) = pos = %d\n",i,j,ivar);
//                 dataloader->AddVariable(varName,'F');
//             }
//         }
//     }
// }


for(auto i=0;i<40;i++)
 {
     dataloader->AddVariable(Form("var1[%d]",i),'F');
     dataloader->AddVariable(Form("var2[%d]",i),'F');
 }

    // dataloader->AddVariable(Form("var%d",10),'F');
    //  dataloader->AddVariable(Form("var%d",20),'F');
    //   dataloader->AddVariable(Form("var%d",30),'F');
    //    dataloader->AddVariable(Form("var%d",40),'F');

  

// exit(0);


// for (int j = 0; j < 1024; ++j) {
//   TString varName = TString::Format("EBenergyRed[%d]",j);
//   dataloader->AddVariable(varName,'F');
// }

dataloader->AddSignalTree    ( signalTree,     1.0 );
dataloader->AddBackgroundTree( background,   1.0 );

// check given input
auto & datainfo = dataloader->GetDataSetInfo();
auto vars = datainfo.GetListOfVariables(); 
std::cout << "number of variables is " << vars.size() << std::endl;
for ( auto & v : vars) std::cout << v << ","; 
std::cout << std::endl;

int ntrainEvts = 256;
int ntestEvts =  256; 
TString trainAndTestOpt = TString::Format("nTrain_Signal=%d:nTrain_Background=%d:nTest_Signal=%d:nTest_Background=%d:SplitMode=Random:NormMode=NumEvents:V",ntrainEvts,ntrainEvts,ntestEvts,ntestEvts );
TCut mycuts = "";//Entry$<1000"; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
 TCut mycutb = "";//Entry$<1000"; 
dataloader->PrepareTrainingAndTestTree( mycuts, mycutb,trainAndTestOpt);

std::cout << "prepared DATA LOADER " << std::endl;

// Input Layout


TString inputLayoutString("InputLayout=1|2|40");
//TString inputLayoutString("InputLayout=1|2|2");
// Batch Layout
TString batchLayoutString("BatchLayout=256|2|40");
//TString batchLayoutString("BatchLayout=256|1|4");

// // General layout.
// General layout.
// for dense layer
//TString layoutString("Layout=RESHAPE|1|1|64|FLAT,DENSE|64|TANH,DENSE|32|TANH,DENSE|1|LINEAR");

// TString layoutString("Layout=CONV|12|2|2|1|1|1|1|TANH,MAXPOOL|6|6|1|1,RESHAPE|1|1|192|FLAT,DENSE|32|TANH,DENSE|32|"
//                       "TANH,DENSE|1|LINEAR");

   TString layoutString ("Layout=RNN|128|40|2|0,RESHAPE|1|2|128|FLAT,DENSE|64|TANH,DENSE|1|LINEAR");
//    TString layoutString ("Layout=RNN|50|10|4|0,RESHAPE|1|4|50|FLAT,DENSE|32|TANH,DENSE|1|LINEAR");
   // Training strategies.
   TString training0("LearningRate=1e-1,Momentum=0.9,Repetitions=1,"
                     "ConvergenceSteps=100,BatchSize=256,TestRepetitions=1,"
                     "WeightDecay=1e-4,Regularization=None,"
                     "DropConfig=0.0+0.5+0.5+0.5, Multithreading=False");
   TString training1("LearningRate=1e-2,Momentum=0.9,Repetitions=1,"
                     "ConvergenceSteps=20,BatchSize=256,TestRepetitions=10,"
                     "WeightDecay=1e-4,Regularization=L2,"
                     "DropConfig=0.0+0.0+0.0+0.0, Multithreading=True");
   TString training2("LearningRate=1e-3,Momentum=0.0,Repetitions=1,"
                     "ConvergenceSteps=20,BatchSize=256,TestRepetitions=10,"
                     "WeightDecay=1e-4,Regularization=L2,"
                     "DropConfig=0.0+0.0+0.0+0.0, Multithreading=True");
   TString trainingStrategyString ("TrainingStrategy=");
   trainingStrategyString += training0; // + "|" + training1 + "|" + training2;

   // General Options.
   TString rnnOptions ("!H:V:ErrorStrategy=CROSSENTROPY:VarTransform=N:"
                       "WeightInitialization=XAVIERUNIFORM");

   rnnOptions.Append(":"); rnnOptions.Append(inputLayoutString);
   rnnOptions.Append(":"); rnnOptions.Append(batchLayoutString);
   rnnOptions.Append(":"); rnnOptions.Append(layoutString);
   rnnOptions.Append(":"); rnnOptions.Append(trainingStrategyString);
   rnnOptions.Append(":Architecture=CPU");

   factory->BookMethod(dataloader, TMVA::Types::kDL, "DL_CPU", rnnOptions);
//    factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTG",
//                            "!H:!V:NTrees=1000:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:BaggedSampleFraction=0.5:nCuts=20:MaxDepth=2" );




//      // General layout.
//    {
//     TString layoutString ("Layout=TANH|64,TANH|64,TANH|64,LINEAR");
// // 
// //       // Training strategies.
//       TString training0("LearningRate=1e-1,Momentum=0.9,Repetitions=1,"
//                         "ConvergenceSteps=20,BatchSize=256,TestRepetitions=10,"
//                         "WeightDecay=1e-4,Regularization=L2,"
//                         "DropConfig=0.0+0.5+0.5+0.5, Multithreading=True");
//       TString training1("LearningRate=1e-2,Momentum=0.9,Repetitions=1,"
//                         "ConvergenceSteps=20,BatchSize=256,TestRepetitions=10,"
//                         "WeightDecay=1e-4,Regularization=L2,"
//                         "DropConfig=0.0+0.0+0.0+0.0, Multithreading=True");
//       TString training2("LearningRate=1e-3,Momentum=0.0,Repetitions=1,"
//                         "ConvergenceSteps=20,BatchSize=256,TestRepetitions=10,"
//                         "WeightDecay=1e-4,Regularization=L2,"
//                         "DropConfig=0.0+0.0+0.0+0.0, Multithreading=True");
//       TString trainingStrategyString ("TrainingStrategy=");
//       trainingStrategyString += training0 + "|" + training1 + "|" + training2;
// 
//       // General Options.
//       TString dnnOptions ("!H:V:ErrorStrategy=CROSSENTROPY:VarTransform=N:"
//                           "WeightInitialization=XAVIERUNIFORM");
//       dnnOptions.Append (":"); dnnOptions.Append (layoutString);
//       dnnOptions.Append (":"); dnnOptions.Append (trainingStrategyString);
// 
//       TString cpuOptions = dnnOptions + ":Architecture=CPU";
//       factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN_CPU", cpuOptions);
//    }


//       factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTG",
//                            "!H:!V:NTrees=1000:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:BaggedSampleFraction=0.5:nCuts=20:MaxDepth=2" );

   std::cout << "nthreads  = " << ROOT::GetImplicitMTPoolSize() << std::endl;


   
  
 factory->TrainAllMethods();

  std::cout << "nthreads  = " << ROOT::GetImplicitMTPoolSize() << std::endl;
 
 // ---- Evaluate all MVAs using the set of test events
 factory->TestAllMethods();
 
 // ----- Evaluate and compare performance of all configured MVAs
 factory->EvaluateAllMethods();


 outputFile->Close();


}
int main() {
   RNN_test();
}
