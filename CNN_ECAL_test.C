#include<TROOT.h>
void CNN_ECAL_test() {

   ROOT::EnableImplicitMT(0);
   TMVA::Config::Instance();
//    std::cout<<ROOT::GetImplicitMTPoolSize()<<std::endl;
   TFile *input(0);

   TString fname = "images.root";
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
                                               "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification:!ModelPersistence" );
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
for(auto i=0;i<64;i++)
{
    dataloader->AddVariable(Form("var%d",i),'F');
}
   
// dataloader->AddVariable(Form("var%d",10),'D');
// dataloader->AddVariable(Form("var%d",20),'D');
// dataloader->AddVariable(Form("var%d",30),'D');
// dataloader->AddVariable(Form("var%d",40),'D');

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
TString inputLayoutString("InputLayout=1|8|8");

// // General layout.
// TString layoutString("Layout=CONV|12|2|2|1|1|1|1|TANH,MAXPOOL|6|6|1|1,RESHAPE|1|1|192|FLAT,DENSE|512|TANH,DENSE|32|"
//                      "TANH,DENSE|2|LINEAR");
// General layout.
TString layoutString("Layout=CONV|12|2|2|1|1|1|1|TANH,MAXPOOL|6|6|1|1,RESHAPE|1|1|192|FLAT,DENSE|2|TANH,DENSE|2|TANH,DENSE|1|LINEAR");
// TString layoutString("Layout=CONV|12|2|2|1|1|1|1|TANH,MAXPOOL|6|6|1|1,RESHAPE|1|1|192|FLAT,DENSE|32|TANH,DENSE|32|TANH,DENSE|32|LINEAR");
   // Batch Layout
   TString batchLayoutString("BatchLayout=256|1|64");

   // Training strategies.
   TString training0("LearningRate=1e-1,Momentum=0.9,Repetitions=1,"
                     "ConvergenceSteps=20,BatchSize=256,TestRepetitions=1,"
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
//       TString layoutString ("Layout=TANH|128,TANH|128,TANH|128,LINEAR");
// 
//       // Training strategies.
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

 factory->TrainAllMethods();

 
 // ---- Evaluate all MVAs using the set of test events
 factory->TestAllMethods();
 
 // ----- Evaluate and compare performance of all configured MVAs
 factory->EvaluateAllMethods();


 outputFile->Close();


}
