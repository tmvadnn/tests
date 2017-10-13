void makeImages(int n = 10000) {

   auto h1 = new TH2D("h1","h1",8,0,10,8,0,10);
   auto h2 = new TH2D("h2","h2",8,0,10,8,0,10);

   auto f1 = new TF2("f1","xygaus"); 
   auto f2 = new TF2("f2","xygaus");
   double x1[64];
   double x2[64]; 
   TTree sgn("sgn","sgn");
   TTree bkg("bkg","bkg");
   TFile f("images.root","RECREATE");
  
   for(auto i=0;i<64;i++)
   {
        bkg.Branch(Form("var%d",i),&x1[i]);
        sgn.Branch(Form("var%d",i),&x2[i]);
   }
   
   sgn.SetDirectory(&f);
   bkg.SetDirectory(&f);
   
   f1->SetParameters(1,3,3,3,3);
   f2->SetParameters(1,7,3,7,3 );
   gRandom->SetSeed(0); 
   for (int i = 0; i < n; ++i) {
      h1->Reset();
      h2->Reset();

      h1->FillRandom("f1",500);
      h2->FillRandom("f2",500);


      for (int k = 0; k < 8 ; ++k) {
         for (int l = 0; l < 8 ; ++l)  {
            int m = k * 8 + l; 
            x1[m] = h1->GetBinContent(k+1,l+1);
            x2[m] = h2->GetBinContent(k+1,l+1);
         }
      }
      sgn.Fill();
      bkg.Fill();
      
//       if (n==1) { 
//          h1->Draw("COLZ");
//          new TCanvas();
//          h2->Draw("COLZ");
//       }
   }
   sgn.Write();
   bkg.Write();
   f.Close();
}
