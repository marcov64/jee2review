#include "fun_head.h"

MODELBEGIN

//DEBUG_AT(0);
/******************************************
*
*	DEMOGRAPHY
*
******************************************/




EQUATION("Health")
/*
Measure the sustainability of the firm comparing the ratio of (smoothed) monetary sales to cumulated profits.
Values close to 0 approach exit, 1 is perfect health.
*/
v[2]=V("aHealth");

v[1]=V("NetWorth");
if(v[1]>0)
 {
 END_EQUATION(v[2]*CURRENT+ (1-v[2]));
 }
else
 END_EQUATION(v[2]*CURRENT);
v[0]=V("smoothMonSales");

v[4]=min(1,v[0]/v[1]);//minimum between 1 and the ration of sales to (neg.) profits

v[5]=v[2]*CURRENT+(1-v[2])*v[4];

//if(v[5]<0)
// INTERACT("neg.health", v[3]);
RESULT(v[5] )

EQUATION("smoothMonSales")
/*
Smoothed value of MonetarySales
*/
V("Trade");
v[0]=V("aMonSales");
v[1]=V("MonetarySales");
RESULT(CURRENT*v[0]+(1-v[0])*v[1] )


EQUATION("Entry")
/*
Entry of new firms, function triggered by SectorEntry
*/

v[1]=VS(c,"IdGood");
cur1=ADDOBJ("Firm");
cur1->hook=c;
WRITELS(cur1,"Age",0, t);
WRITELS(cur1,"Visibility",0.1, t);
WRITES(cur1,"IdFirm",v[20]=V("CounterIdFirm"));
cur5=ADDOBJS(c,"sFirm");

WRITES(cur5,"IdsFirm",v[20]);
cur5->hook=cur1;
cur1->hook=cur5;

WRITES(cur1,"product",v[1]);
cur=SEARCHS(cur1,"PNeed");
cur2=ADDOBJS(cur,"Ch");
WRITES(cur2,"IdCh",2);
v[2]=VS(c,"FrontierX");
WRITELS(cur2,"x",v[2],t);

cur3=SEARCH_CNDS(cur1,"IdLabor",1);
cur4=SEARCH_CND("NumClass",1);
cur3->hook=cur4;
cur=ADDOBJS(cur1,"Labor");
WRITES(cur,"IdLabor",2);
cur4=SEARCH_CND("NumClass",2);
cur->hook=cur4;

cur5=SEARCH_CND("IdLabor",2);//
v[78]=VS(cur5,"wagecoeff");
WRITES(cur,"wagecoeff",v[78]);


v[4]=V("MinWage");
CYCLES(cur1, cur2, "Labor")
 {
  v[5]=VS(cur2,"wagecoeff");
  v[6]=v[5]*v[4];
  WRITELS(cur2,"wage",v[6],t);
  v[4]=v[6];
  
 }

cur2=SEARCHS(cur1,"BankF");
cur3=SEARCHS(p->up,"Bank");
cur2->hook=cur3;

v[7]=VS(cur1->hook,"SNetWorth")*0.1;
v[8]=VS(cur1->hook,"SNumFirms");
if(v[8]!=0)
 v[9]=(v[7]);
else
 v[9]=V("AvPrice")*100;
WRITELS(cur2,"BalanceF",v[9], t);

v[10]=V("AvCurrProd");
cur4=SEARCHS(cur1,"Capital");
WRITES(cur4,"IncProductivity",v[10]);
RESULT( 1)


EQUATION("ClearExitRecord")
/*
Prepare the computation of the exit record
*/	

CYCLE(cur, "Sectors")
 {
  WRITES(cur,"numExit",0);
  WRITES(cur,"AvAgeDeath",0);
 }

cur=SEARCH("Bank");
WRITES(cur,"CapitalDestroyed",0);
WRITES(cur,"CapitalDemand",0);
RESULT(1 )

EQUATION("sBalanceF")
/*
Comment
*/
v[0]=V("BalanceF");

v[1]=CURRENT*0.95+0.05*v[0];
v[2]=V("DebtF");
if(v[2]!=0)
 WRITE("RRoK",v[1]/v[2]);
RESULT(v[1] )

EQUATION("Exit")
/*
Remove firms with too poor Health
*/

V("ClearExitRecord");
v[4]=v[70]=0;
v[18]=V("minAgeExit");
v[19]=V("minRRoKExit");
CYCLE_SAFE(cur, "Firm")
 {

  v[6]=VS(cur,"Waiting");
  v[16]=VS(cur,"Age");

  if(v[6]==0 && v[16]>v[18]) //don't kill firms waiting for a capital good to be delivered.
   {
    v[2]=VS(cur->hook,"minHealth");
    v[3]=VS(cur,"Health");
    cur2=SEARCHS(cur,"BankF");
    v[20]=VS(cur2,"DebtF");
    v[70]+=v[20];
    v[7]=VS(cur,"NetWorth");
    v[17]=VS(cur,"RRoK");
//    if(v[3]<v[2])
//     if(v[7]<0)
    if(v[17]<v[19] && VS(cur->hook->up,"NFirmsS")>1)
     {
      INCRS(cur->hook->up,"NFirmsS",-1);
      v[5]=VS(cur,"Age");
      if(V("ExitFlag")==1 )
        INTERACTS(cur,"Dying", v[7]);
      INCRS(cur->hook->up,"AvAgeDeath",v[5]);
      INCRS(cur->hook->up,"numExit",1);      
      INCRS(cur2->hook,"CapitalDestroyed",v[20]);
      DELETE(cur->hook); 
      DELETE(cur);
      v[4]++;
      
     }
   }
 }

WRITE("TotDebt",v[70]);
RESULT(v[4] )

EQUATION("Demography")
/*
Comment
*/
cur=SEARCHS(p->up,"Supply");


VS(cur,"Exit");
v[3]=V("numExit");
//v[5]=INCR("NFirmsS",-v[3]);
//if(v[5]<=0)
 //INTERACT("Removed last firm in sector. Likely crash will follow", v[3]);
if(v[3]>0)
 MULT("AvAgeDeath",1/v[3]);

v[4]=V("probEntry");
if(RND<v[4])
 { v[1]=VS_CHEAT(cur,"Entry",p);
   INCR("NFirmsS",v[1]);
 }
RESULT( 1)




EQUATION("CounterIdFirm")
/*
Issue idFirm progressive numbers
*/

RESULT(CURRENT+1 )


/******************************************
*
*	DEMAND
*
******************************************/


EQUATION("x")
/*
Characteristics' values, constant unless they are a of a specified Id.
*/

v[0]=V("IdCh");
if(v[0]==1)
 v[1]=VLS(p->up->up,"price",1);
else
 {
  v[1]=CURRENT;
 } 
RESULT(v[1] )


EQUATION("TotIterations")
/*
Total number of iterations for a class
Thios is computed separately, in order to take into account the redistribution of expenditure due to missing goods, and avoid that expenditure level is modified by rounding the number of iterations to an integer
*/

v[5]=0;
CYCLE(cur, "Need")
 {
  v[1]=VS(cur,"ProdExists");
  if(v[1]==1)
   {
    v[2]=VS(cur,"NumIterations");
    v[3]=VS(cur,"TempIterations");
    v[4]=v[2]+v[3];
    v[5]+=v[4];
   }
 }

RESULT(v[5] )


EQUATION("Visibility")
/*
Comment
*/
//END_EQUATION(1);
v[0]=V("backlog");
v[1]=CURRENT;
//v[2]=V("MonetarySales");
v[2]=V("ExpectedSales");
if(v[2]<1)
 v[2]=1;

v[3]=(v[2]-v[0])/v[2];

v[4]=max(v[3],0.001);

v[5]=v[1]*0.9+0.1*v[4];
//v[5]=CURRENT*0.9+0.1;
RESULT(v[5] )


EQUATION("TTB_multiplWinner")
/*

The changes cannot work, since you get the optimum before making any selection.


Standard TTB, but all the winners remain marking "app" to 1, and returning the number of winners.

Choose one option on the base of their characteristics "x", using "tau" as tolerance and "Delta" as random bias of the observed characteristics' values.

Consumers' preferences are the order by which the characteristics are used to filter out (perceived) inferior products.

For each characteristics, the perceived value for each option is a random value drawn from a normal centered around the true value and variance equal to Delta.

All options scoring more than tau times the max value for the current characteristics are considered equivalent to the maximum, and therefore not filtered away.

Before starting the selection procedure, options are scanned to remove those scoring less than a minimum on some characteristic.
*/

//V("MinimumPriceSet"); // make sure the minimum parameter for the price characteristic has been set
v[0]=0;
v[31]=VS(c->up,"Expenditure")/VS(c->up,"TotIterations"); //amount to spend on this iteration
v[30]=VS(c,"IdNeed");

cur9=SEARCH_CND("IdGood",v[30]);

CYCLES(c, cur, "DCh")
  WRITES(cur,"temp",-1);//set to -1 the max value

v[72]=1;
while(v[0]==0)
{//repeat for increasing levels of visibility as long no firms have been found
//   CYCLE(cur, "Firm") 
 v[50]=0;
 CYCLES(cur9,cur8, "sFirm")
   v[50]+=VLS(cur8->hook,"Visibility",1);  
 CYCLES(cur9,cur8, "sFirm")
   {
    v[51]=VLS(cur8->hook,"Visibility",1);  
    WRITES(cur8,"app1",v[51]/v[50]);
   } 
    
  CYCLES(cur9,cur8, "sFirm")
  {
   cur=cur8->hook;
   v[24]=1; //assume the option to be viable
   //cur3=SEARCH_CNDS(cur,"IdPNeed",v[30]);
   v[37]=VS(cur,"product");
   if(v[37]!=v[30] || RND>VLS(cur8,"app1",1)*v[72])
    {// if the the firm does not produce the product the consumer is looking for, exclude it from the avaialble options
     WRITES(cur,"app",0);
    }
   else
    { // if the firm produces the required good
     WRITES(cur,"app",1);   
     v[0]++;
     CYCLES(cur, cur1, "Ch")
      {//for any characteristic
       v[20]=VS(cur1,"IdCh");
       cur2=SEARCH_CNDS(c,"IdDCh",v[20]); //find the ch. of the option you are browsing
       v[4]=VS(cur2,"Delta"); // observation error in the quality of the good
       v[22]=VS(cur1,"x");
       v[71]=norm(v[22],v[4]*v[22]); //this is the observed value
       //v[71]=norm(v[22],v[4]); //ABSOLUTE VARIANCE
       v[23]=max(0,v[71]);
       WRITES(cur1,"obs_x",v[23]); //write the observed value 
      }
    }
  }	
 if(v[0]==0)
  {//if no firm managed to get visible push up visibility and try again.
   v[72]++;
  } 
  if(v[72]>100)
   INTERACTS(c, "No visible firms", v[72]);
}  

//Do a proper choice among the (more than one) viable options

//INTERACT("First", v[0]);
v[80]=v[0];
CYCLES(c, cur, "DCh")
 {//for each characteristic, in the order of the decision maker
  v[27]=1;
  v[1]=VS(cur,"IdDCh");
  v[25]=VS(cur,"NegativeQuality"); // to control for negative value of quality such as pice
  v[3]=VS(cur,"tau"); // the tolerance parameter

  //CYCLE(cur1, "Firm")
  CYCLES(cur9,cur8, "sFirm")
   { //find the maximum
    cur1=cur8->hook;
    v[7]=VS(cur1,"app");
    if(v[7]==1)
    {
     //v[8]=VS(cur1,"curr_x");
     cur3=SEARCH_CNDS(cur1,"IdCh",v[1]);
     v[8]=VS(cur3,"obs_x");
     if(v[27]==1 ||v[6]*v[25]<v[8]*v[25])
       {cur->hook=cur3;
        v[27]=0;
        v[6]=v[8];
       }
     }
   }    
  
  v[74]=100000000;
  //CYCLE(cur1, "Firm")
  CYCLES(cur9,cur8, "sFirm")
   { //second cycle: remove options below maximum * tau
    cur1=cur8->hook;
    v[7]=VS(cur1,"app");
    if(v[7]==1)
    {
     cur3=SEARCH_CNDS(cur1,"IdCh",v[1]);
     v[8]=VS(cur3,"obs_x");
     if(v[25]==-1)
      v[33]=1/v[3];
     else
      v[33]=v[3]; 
     v[78]=v[8]*v[25];
     v[79]=v[6]*v[33]*v[25]; 
     v[77]=v[79]-v[78];
     if(v[8]*v[25]<v[6]*v[33]*v[25]-0.00001)//approximations sometime makes things weird
      {//too low value: remove
      if(abs(v[6]-v[8]) < 0.00001)
       INTERACTS(cur3,"Merda1",v[33]);

       WRITES(cur1,"app",-1);
       v[0]--;
      //sprintf(msg, " in need %g of class %g\n", v[30],VS(c,"NumClass"));
      //plog(msg);
      }
      
    }
   }
//INTERACT("Subsequ", v[0]);
 }
if(v[0]==0)
 INTERACTS(c,"No firms left",v[0]);//error control: v[0] must be >=1

if(v[80]!=v[0])
 {
 // sprintf(msg, " %g in need %g of class %g\n", v[0]/v[80], v[30],VS(c,"NumClass"));
  //plog(msg);
 }
//CYCLE(cur, "Firm")
CYCLES(cur9,cur8, "sFirm") 
 {
  v[32]=VS(cur8->hook,"app");
  if(v[32]==1)
   INCRS(cur8->hook,"MonetarySales",v[31]/v[0]);
 }

RESULT(v[0] )

EQUATION("Production")
/*
After trading fix any remaining variable to compute
*/

v[0]=v[1]=v[5]=v[6]=v[7]=v[8]=v[9]=v[70]=0;
CYCLE(cur, "Sectors")
 WRITES(cur,"BLrecouped",0);
CYCLE(cur, "Supply")
 {
  CYCLES(cur, cur1, "Firm")
   {
    VS(cur1,"Profit");
    VS(cur1,"InvestmentDecision");
    v[5]+=VS(cur1,"Age");
    v[6]++;
    VS(cur1,"WagePrem");
    
    v[8]=0;
    CYCLE_SAFES(cur1, cur2, "Labor")
     {
      v[4]=VS(cur2,"NumWorkers");
      v[8]+=v[4];  
      //INCRS(cur2->hook,"tempWage",v[2]*v[4]);
      if(v[4]==0 && VS(cur2,"IdLabor")!=1)
       DELETE(cur2);
               
     }
   v[9]+=v[8];  
   cur5=SEARCHS(cur1,"BankF");
   VS(cur1,"NetWorth");
   }
 }
CYCLE(cur, "KFirm")
 {
  CYCLE_SAFES(cur, cur1, "KLabor")
   {
    v[4]=VS(cur1,"KWage");
    v[3]=VS(cur1,"KNbrWorkers");
    VS(cur1,"KWagePrem");
    if(v[4]==0 && VS(cur2,"IdKLabor")!=1)
      DELETE(cur2);
   }
  v[70]+=VS(cur,"DebtK"); 
 }
/* 
 v[5]=V("TotPremia");
 v[6]=V("TotWage");
 if(v[5]!=v[0] || v[6]!=v[1])
  {
   INTERACT("Failed income",v[1]);
  } 
*/
WRITE("AvAge",v[5]/v[6]);

V("AllocateBlRecouped");
RESULT(1 )

EQUATION("AllocateBlRecouped")
/*
Comment
*/
v[0]=v[1]=v[2]=v[3]=v[4]=v[5]=0;

CYCLE(cur, "Class")
 {
  v[11]=VS(cur,"shareConsumption");
  v[2]=0;
  CYCLES(cur, cur1, "Need")
   {
    if(cur1->hook==NULL)
     {
      v[10]=VS(cur1,"IdNeed");     
      cur1->hook=SEARCH_CND("IdGood",v[10]);

     }
   v[12]=VS(cur1->hook,"BLrecouped");
   v[2]+=v[12]*v[11];
     
   }
  WRITES(cur,"Recouped",v[2]);
 }
RESULT(1 )


EQUATION("Trade")
/*
Set a trading cycle:
- initialize "sales" to zero in firms;
- compute the sales for each firm as the total of classes and needs 
*/

CYCLE(cur1, "Supply")
 {
  CYCLES(cur1, cur, "Firm")
  {
   v[1]=VS(cur,"MonetarySales");
   WRITES(cur,"MonetarySalesL",v[1]); // before setting the sales to 0 for the current period computation, register the lagged value of monetary sales
   WRITES(cur,"MonetarySales",0);
  }
 }

CYCLE(cur, "Class")
 {
  v[4]=VS(cur,"Expenditure");
  WRITES(cur,"NoConsumption",0); // after having computed the expenditure set the non expenditure to 0, to be computed again inthis period for the following period expenditures
  v[14]=v[21]=0;
  CYCLES(cur, cur1, "Need")
   { // make a first cycle through needs to check which products are available in the market
    v[10]=VS(cur1,"IdNeed");
    v[12]=0;
    CYCLE(cur2, "Supply")
     {
      CYCLES(cur2, cur3, "Firm")
       { // cycle through all firms
        v[11]=VS(cur3,"product");
        if(v[11]==v[10]) //count how many firms sell the product needed
         {//you just need to know if there is 1 firm, don't waste time
          v[12]++;
          break;
         } 
       }

     }
    if(v[12]<1)
     {
      v[13]=VS(cur1,"NumIterations");
      v[14]+=v[13];
      v[20]=VS(cur1,"Share");
      v[21]+=v[20];
      WRITES(cur1,"ProdExists",0); // write that the product for the need does not exists
     }
    else
     WRITES(cur1,"ProdExists",1);

   }
  CYCLES(cur, cur1, "Need")
   { // cycle again to redistribute the iterations to the needs for which a product exists
    v[15]=VS(cur1,"Share"); // use the share
    v[22]=1-v[21];
    v[23]=v[15]/v[22];
    v[16]=v[23]*v[14]; // compute the number of iterations non sde from the other needs
    v[17]=round(v[16]); // round them
    WRITES(cur1,"TempIterations",v[17]);
   }


  CYCLES(cur, cur1, "Need")
   {
    v[19]=VS(cur1,"ProdExists");
    if(v[19]==1)
     {
      v[0]=0;   
      //v[5]=VS(cur1,"Share");
      //v[6]=v[5]*v[4];  
      v[9]=VS(cur1,"NumIterations");
      v[18]=VS(cur1,"TempIterations");
   
      for(v[8]=0; v[8]<v[9]+v[18]; v[8]++)
        V_CHEAT("TTB_multiplWinner", cur1);
        
     }
   }
 }

CYCLE(cur, "Sectors")
 {
 VS(cur,"RedistributeSales");   
 }


cur=SEARCH("Bank");
RESULT( 1)

EQUATION("RedistributeSales")
/*
This routine redistributes sales in order to avoid excessive backlogs. The logic is that customers choosing firms that cannot deliver will redirect their expenses to firms with available capacity.

The routine collects total capacity in excess of demand and demand in excess of capacity. The latter is redistributed in proportion to the former. 

In case total capacity of the industry is not sufficient to meet the demand, the residual is distributed in proportion to sales
*/

if(VS(p->up,"Redistribute")==0)
 END_EQUATION(0);

v[0]=v[1]=v[2]=v[3]=v[8]=v[10]=v[20]=v[21]=v[32]=0;

CYCLE(cur, "sFirm")
 {
  v[4]=VS(cur->hook,"MonetarySales");
  v[8]+=v[4]; //total sales
  v[5]=VLS(cur->hook,"price",1);
  v[6]=v[4]/v[5]; //unit sold
  v[7]=VLS(cur->hook,"Q",1);
  if(v[6]>v[7])
   {//insufficient capacity
    v[0]+=v[4]-v[7]*v[5]; //excess sales over production
    v[20]+=v[7]*v[5];  //sales at full capacity
    //WRITES(cur,"app3",0);
    //WRITES(cur,"app4",v[7]*v[5]);
   }
  else
   {//sufficient capacity
    v[1]+=v[4]; //fully served sales
    v[2]+=v[7]*v[5]-v[4];//excess capacity available to serve extra customers
    v[21]+=VLS(cur->hook,"Q",1);
    v[3]++;
    //WRITES(cur,"app3",1);
    //WRITES(cur,"app4",v[7]*v[5]);
   } 
 }
if(v[3]==0)
 END_EQUATION(-1);
v[9]=max(0,v[0]-v[2]); //over sales
v[13]=v[0]-v[9];
CYCLE(cur, "sFirm")
 {
  v[4]=VS(cur->hook,"MonetarySales");
  v[5]=VLS(cur->hook,"price",1);
  v[6]=v[4]/v[5]; //unit sold
  v[7]=VLS(cur->hook,"Q",1);
  if(v[6]>v[7])
   {//oversold
    v[11]=v[7]*v[5];//maximum sales
    //WRITES(cur,"app1",v[11]);
    v[12]=v[9]*v[4]/v[8];
    //WRITES(cur,"app2",v[12]);
   }
  else
   {//undersold
    v[14]=v[7]*v[5]-v[4];
    if(v[2]>0)
      v[11]=v[4]+v[13]*v[14]/v[2];     
    else
      v[11]=v[4]+v[13]*VLS(cur->hook,"Q",1)/v[21];
    //WRITES(cur,"app1",v[11]);
    v[12]=v[9]*v[4]/v[8];
    //WRITES(cur,"app2",v[12]);    
   } 
  v[10]+=v[11]+v[12]; 
  v[32]+=v[12];
  WRITES(cur->hook,"MonetarySales",v[11]+v[12]);
 }

RESULT((v[8]-v[10]) )

EQUATION("UnitDemand")
/*
Number of product units demanded
*/
V("Trade"); //ensure that Monetary sales is updated
v[0]=VL("price",1);
v[1]=V("MonetarySales");

RESULT(v[1]/v[0] )

EQUATION("Revenues")
/*
Money flowing in
*/
v[0]=V("UnitSales");
v[1]=VL("price",1);
v[2]=V("backlogSales");

RESULT(v[0]*v[1]+v[2] )

EQUATION("Control")
/*
Control the allocation of production
*/
v[0]=V("Q");
v[1]=V("UnitSales");
v[2]=V("Stocks");
v[3]=VL("Stocks",1);
v[4]=V("backlog");
v[5]=VL("backlog",1);

v[6]=v[1]+v[2]-v[3]+max(0,v[5]-v[4]);
if(abs(v[6]-v[0])>0.001)
 INTERACT("Control failed",v[6]);
RESULT( 1)

EQUATION("UnitSales")
/*

Actual sales
*/

v[0]=V("Q");
v[1]=V("UnitDemand");
v[2]=V("Stocks");
v[3]=VL("Stocks",1);

v[7]=min(v[1], v[0]+max(0,v[3]-v[2]) );
if(v[7]<0)
 INTERACT("NegSales", v[7]);
RESULT(v[7] )


EQUATION("backlog")
/*
If production Q exceeds demand then fill as much back orders as possible
*/

v[0]=V("Q");
v[1]=V("UnitDemand");
v[2]=VL("backlog",1);
v[3]=VL("Stocks",1);
WRITE("backlogSales",0);

v[4]=V("BLfriction");
v[5]=v[55]=0;
CYCLE(cur, "blItem")
 {
  v[55]+=VS(cur,"blQ")*VS(cur,"blPrice")*(1-v[4]);
  v[5]+=MULTS(cur,"blQ",v[4]);

 }

INCRS(p->hook->up,"BLrecouped",v[55]); 

v[2]=v[5];
v[24]=v[23]=v[77]=0;
if(v[0]>v[1]+v[3] && v[2]>0)
 {//fill some orders reducing the backlog
 // if(v[3]>0)
   //INTERACT("Positive stocks with backlog",v[3]);
  v[7]=v[0]-v[1]; //excess production available to fill backlog orders 
  v[5]=v[6]=0;
  v[9]=V("numBLI");
  if(v[9]>0)
  {
    CYCLE_SAFE(cur, "blItem")
     {
      v[4]=VS(cur,"blQ");
      v[8]=VS(cur,"blPrice");
      if(v[4]<=v[7])
       {//Q sufficient to fill completely the order
        v[24]++;
        v[5]+=v[4];//Q filled
        v[6]+=v[4]*v[8];//sales produced
        v[7]-=v[4];
  //      INTERACTS(cur, "Test2", v[7]);
        v[10]=INCR("numBLI",-1);
        if(v[10]>0)
         DELETE(cur);
        else
        {
         WRITES(cur,"blQ",0);
         v[77]=1;
         } 
       }
      else 
       {v[23]++;
        v[11]=v[4]-v[7];//units remaining in the BL
        v[5]+=v[7]; //units sold
        v[6]+=v[7]*v[8];
        INCRS(cur,"blQ",-v[7]);
        v[7]=0;
       }    
     }
    } 
 /****   
  v[78]=v[79]=0; 
  CYCLE(cur, "blItem")
    {v[78]++;
     if(VS(cur,"blQ")==0)
      {
       v[79]++;
      } 
    } 
  if(v[78]-v[79]!=V("numBLI") )
   INTERACT("WRONG numBLI",v[78]);
 ***********/     
  v[2]-=v[5];
  WRITE("backlogSales",v[6]);
  v[21]=1;
 }
 
if(v[0]+v[3]<v[1])
 {//generate a new backlog item
  v[13]=INCR("numBLI",1);
  if(v[13]==1)
   cur1=SEARCH("blItem");
  else
   cur1=ADDOBJ("blItem");
  WRITES(cur1,"blQ",v[1]-v[0]-v[3]);
  WRITES(cur1,"blPrice",VL("price",1));
  v[2]+=v[1]-v[0]-v[3];
  v[21]=2; 
 }
 
//if(v[2]<-0.001) 
 //INTERACT("Neg.bl",v[7]);
 
v[22]=0;
/*
CYCLE(cur, "blItem")
 {
  v[22]+=VS(cur,"blQ");
 }
if(abs(v[22]-v[2])>0.001)
  INTERACT("DiffSum",v[22]);
v[2]=v[22];  
*/
if(v[77]==1)
 {//killed last backlog, any non-zero value is due to approximation
  v[2]=0;
 }
RESULT(v[2] )



EQUATION("markup")
/*

Raise price for positive backlog and reduce it to normal levels otherwise
*/
/*
v[0]=V("Stocks"); //here it is computed the backlog
v[1]=V("backlog");
v[2]=V("Q");
v[4]=V("coefMarkupVar");
if(v[1]==0 || v[2]==0 )
 v[3]=1+V("minMarkup");//normal level of markup
else
 v[3]=1+V("minMarkup")+v[4]*log(1+v[1]/v[2]);
*/

v[0]=V("Stocks"); //here it is computed the backlog
//v[1]=V("UnitDemand");
v[1]=V("ExpectedSales");
v[2]=V("Q");
v[4]=V("coefMarkupVar");
v[5]=V("backlog");
if(v[1]==0 || v[2]==0 || v[0]>0 )
 v[3]=1+V("minMarkup");//normal level of markup
else
 v[3]=1+V("minMarkup")+v[4]*log(1+(v[1]+v[5])/v[2]);


RESULT(v[3])


EQUATION("Stocks")
/*
Stocks
*/
v[0]=VL("Stocks",1);
v[1]=V("Q");
v[3]=V("UnitDemand");
v[4]=VL("backlog",1);
v[6]=V("backlog");
v[5]=v[0]+v[1]-v[3]+v[6]-v[4];
if(v[5]<0)
 v[5]=0;
// INTERACT("NEG. Stocks",v[3]);

RESULT(v[5] )


EQUATION("DesiredQ")
/*
Production, as a function of the difference between past stocks and desired ones
*/
V("Trade");
v[0]=VL("Stocks",1);
v[1]=V("DesiredStocks"); // percentage of expected sales firms want to stock to face unexpected demand
v[2]=VL("backlog",1);
v[4]=V("ExpectedSales");
v[3]=v[1]*v[4];
v[6]=max(0,v[3]+v[2]+v[4]-v[0] );

RESULT(v[6])


EQUATION("ExpectedSales")
/*
Smoothed level demand
*/
v[0]=VL("ExpectedSales",1);
v[1]=V("UnitDemand");
v[2]=V("aES");
v[3]=v[0]*v[2]+(1-v[2])*v[1];

RESULT(v[3] )


EQUATION("Q")
/*
Actual production, which is the minimum between desired production and constraints
*/

v[0]=V("DesiredQ");
v[1]=V("LaborCapacity");
v[2]=min(v[0],v[1]);
v[3]=V("CapitalCapacity");
v[5]=min(v[3],v[2]);

RESULT(v[5] )


EQUATION("LaborCapacity")
/*
Maximum capacity allowed by currently employed workers on current capital stock
*/
v[0]=0;
v[1]=V("MaxLaborProductivity");
v[2]=VL("NumWorkers",0); // refers only to first layer workers, not to executives
v[4]=v[2]*v[1];

RESULT(v[4] )



EQUATION("NumWorkers")
/*
Number of workers, computed as the 120% of the workers required to fill expected sales
*/

v[15]=0;
CYCLES(p->up, cur, "Labor")
 { //check how many tiers already exist
  v[15]++;
 }
v[14]=V("IdLabor");
if(v[14]==1)
 { // compute the first tier workers given their productivity and production needs
  //V("aNWDynamic");
  v[0]=VL("NumWorkers",1);
  //v[1]=V("ExpectedSales");
  v[1]=V("DesiredQ");
  //v[10]=V("backlog");
  //v[11]=v[1]+v[10];
  v[2]=V("MaxLaborProductivity");
  v[8]=V("CapitalCapacity");
  v[9]=min(v[1],v[8]);
  v[4]=V("DesiredUnusedCapacity");
  v[3]=v[4]*(v[9]/v[2]);
  v[5]=V("aNW");
  v[6]=v[0]*v[5]+(1-v[5])*v[3]; // number of workers in the first layer
  
  v[33]=(v[3]>v[6])?(v[3]-v[6]):0; //number of vacancies for the first layer, if there are more desired workers than actual new hires
  WRITES(p->up,"Vacancies",v[33]);
  v[54]=v[33]/v[6];//ratio of vacancies to actual workers
  WRITES(p->up,"RatioVacancies",v[54]);
 }

else
 {// when above the first tier workers...
  v[18]=V("IdLabor");
  cur=SEARCH_CNDS(p->up,"IdLabor",v[18]-1);
  v[23]=VS(p->up,"IdFirm");
  v[24]=VS(cur->up,"IdFirm");
  if(v[23]!=v[24])
   v[25]=INTERACT("the firm ID is different from the current firm", v[24]);
  v[21]=VS(cur,"nu"); //given the worker ratio between tiers (defined by the tier below)
  v[19]=VS(cur,"NumWorkers"); //and the number of workers in the previous tier
  v[6]=v[19]/v[21]; // compute the required executives for the current tier
  INCRS(p->up,"Vacancies",v[54]*v[6]);

  v[17]=V("nu");
  if(v[6]>=v[17] && v[18]==v[15])
   { // if they are above the workers ratio of this tier and this is the last tier, create a new working class
    cur1=ADDOBJS_EX(p->up,"Labor",p);
    WRITES(cur1,"IdLabor",v[18]+1);
    v[20]=v[6]/v[17];
    WRITELS(cur1,"NumWorkers",v[20], t);
    WRITELLS(cur1,"NumWorkers",0, t,1); // write also that the number of workers in the previous period is equal to 0, as it is used in the inequality statistics
    v[22]=0;
    CYCLES(p->up->up->up,cur2, "Class")
     {
      if(v[22]==0)
       {
        cur7=SEARCHS(cur2,"BankC");
        cur7=cur7->hook;
       }
      v[22]++;
      
     }
    if(v[22]-1<v[18]+1)
     { //starting from the second class (the first are engineers), if it does not exist a class that represnt the new layer of executives, create it
      cur2=SEARCH_CNDS(p->up->up->up,"NumClass",v[18]);
      cur3=ADDOBJS_EX(cur2->up,"Class",cur2);
      cur8=SEARCHS(cur3,"BankC");
      WRITELS(cur8,"BalanceC",0, t-1);
      WRITELS(cur8,"DividendsC",0, t-1);
      WRITELS(cur8,"NumberUnits",0, t-1);
      cur8->hook=cur7;
      cur1->hook=cur3;
      v[44]=VS(cur2,"SavingRate");
      v[45]=VS(cur2->up,"SRMultiplier");
      v[46]=v[44]*(1-v[45])+v[45];
      WRITES(cur3,"SavingRate",v[46]);
//      INTERACTS(cur3,"Inspect SavingRate", v[46]);
      
      WRITES(cur3,"NumClass",v[18]+1);
      WRITELS(cur3,"Expenditure",0, t-1);
      WRITELLS(cur3,"Expenditure",0, t-1,1);
      WRITELS(cur3,"Income",0, t-1);
      WRITELS(cur3,"Consumption",0, t-1);      
      WRITELS(cur3,"ShareWageIncome",0, t-1);
      WRITELS(cur3,"SharePremiaIncome",0, t-1);
      //WRITELS(cur3,"ShareProfitIncome",0, t-1);
//      WRITELS(cur3,"Savings",0, t-1);
      // WRITELS(cur3,"NumIterations",0, t-1); to reactivate when NumIterations report the number of consumers in the labour class, as given in equation "ShareWageIncome". Otherwise the number of iterations simply define the number of representative conusmers (groups) in a class
      WRITELS(cur3,"ShareIncome",0, t-1); // reset the share income to be recomputed
      WRITES(cur3,"Individuals",v[20]); // set the number of individuals to nu;ber of workers of the new class
      v[35]=VS(cur2,"LorenzInd"); 
      WRITES(cur3,"LorenzInd",v[35]+v[20]); // set total number of workers as previous total plus new workers
      WRITELS(cur3,"NoConsumption",0, t-1); // set the savongs due to unavailability of the good to 0
      //INTERACTS(cur3,"Created class",v[44]);
      CYCLES(cur3, cur, "Need")
       { // enter in neds and characterisitcs to change the tau parameter (the minimum is set every period)
        v[27]=VS(cur,"IdNeed");
        CYCLES(cur, cur1, "DCh")
         {
          v[26]=VS(cur1,"IdDCh");
          CYCLES(cur2, cur4, "Need")
           { // cycle among the needs of the preceding class
            v[28]=VS(cur4,"IdNeed");
            if(v[28]==v[27])
             { // when in the same need as the one the new class is cycling cycle through the char of the preceding class
              CYCLES(cur4, cur5, "DCh")
               { 
                v[29]=VS(cur5,"IdDCh");
                if(v[29]==v[26])// when in the same characteristic the new class is cycling
                 v[30]=VS(cur5,"tau"); // read the value of the tau parameter
               }

             }
           }
          v[31]=VS(cur1,"tauMultiplier"); // the speed of adjustment of the tolerance level from one class to the following one
          v[34]=VS(cur1,"NegativeQuality");
          if(v[34]>0)
           v[33]=VS(p->up->up->up,"tauMax"); // the asympthotic level of the tolerance level (1 for qualities and 0 for price)
          if(v[34]<0)
           v[33]=VS(p->up->up->up,"tauMin"); // the asympthotic level of the tolerance level (1 for qualities and 0 for price)
          v[32]=v[30]*(1-v[31])+v[31]*v[33]; // adjustment in the treshold level of tolerance
          WRITES(cur1,"tau",v[32]); // finally write the tau for the new consumer class in each of its characteristic for each need

         }

       }
     v[35]=VS(cur3,"ComputeShare"); // set the distribution of expenditure shares across needs for the new class
     
     }
    else
     {
      cur3=SEARCH_CND("NumClass",v[18]+1);
      cur1->hook=cur3;
     
     } 

   }
  if(v[18]>2 && v[19]<v[21])
   v[6]=0;
 }


RESULT(v[6] )


EQUATION("MinimumPriceSet")
/*
System equation that mantains the `minimum' parameter for the price characteristic, of the first tyer working class and engineers, above the minimum price among firms (times the variance of the consumer percetion error). 
It also sets the Minimum parameter across needs for the price
*/


v[4]=1000000;
CYCLE(cur, "Supply")
 {
  CYCLES(cur, cur1, "Firm")
   {
    v[3]=VLS(cur1,"price",1);
    if(v[3]<v[4])
     v[4]=v[3];
   }

 }
v[10]=V("AvWagecoeff");
CYCLE(cur, "Demand")
 {
  CYCLES(cur, cur1, "Class")
   {
    v[1]=VS(cur1,"NumClass");
    if(v[1]<=1)
     { // start with the engineers and the first tier working class (they both set their minimum independently)
      CYCLES(cur1, cur2, "Need")
       {
        CYCLES(cur2, cur3, "DCh")
         {
          v[2]=VS(cur3,"IdDCh");
          if(v[2]==1)
           { // maintain the minimum above the minimum of the characteritic (price) only for the fist (price) characteritic
            v[5]=VS(cur3,"Minimum"); // check the initialised value of the minimum
            v[6]=VS(cur3,"Delta"); // variance of the distribution of the perceived characteritic around the actaul value
            v[7]=v[6]*v[4];
            v[8]=max(v[5],v[7]); // use either the initially set minimum or the price of the cheapest firm
            WRITES(cur3,"Minimum",v[8]);
           }
         }

       }

     }
    if(v[1]>1)
     { // layers above the first one (different types of executives)
      CYCLES(cur1, cur2, "Need")
       {
        CYCLES(cur2, cur3, "DCh")
         {
          v[9]=VS(cur3,"IdDCh");
          if(v[9]==1)
           {
            cur4=SEARCH_CNDS(cur,"NumClass",v[1]-1);
            CYCLES(cur4, cur5, "DCh")
             { // cycle the DCh for any need (we assume that the minimum for price is equal across needs)
              v[11]=VS(cur5,"IdDCh");
              if(v[11]==1)
               v[12]=VS(cur5,"Minimum"); // read the minimum par of the previsous class for char price
             }
            v[8]=v[12]*v[10];
            WRITES(cur3,"Minimum",v[8]); // write the value of the parameter, as the previous Minimum times the multiplier
           }
         }
       }
     }
   }
 }


RESULT(v[8] )


EQUATION("AvWagecoeff")
/*
Average of the wage multiplier, in case they are different between firms (or capital and final good firms), to be used as a multiplier of the `minimum' parameter for consumers classes of executives
*/

v[3]=v[4]=0;
CYCLE(cur, "Supply")
 {
  CYCLES(cur, cur1, "Firm")
   {
    CYCLES(cur1, cur2, "Labor")
     {
      v[1]=VS(cur2,"IdLabor");
      if(v[1]>1)
       {
        v[2]=VS(cur2,"wagecoeff");
        v[3]+=v[2];
        v[4]++;
       }
     }

   }

 }
CYCLE(cur, "Machinery")
 {
  CYCLES(cur, cur1, "KFirm")
   {
    CYCLES(cur1, cur2, "KLabor")
     {
      v[5]=VS(cur2,"IdKLabor");
      if(v[5]>1)
       {
        v[6]=VS(cur2,"KWagecoeff");
        v[3]+=v[6];
        v[4]++;
       }
     }

   }

 }
v[7]=v[3]/v[4];

RESULT(v[7] )

EQUATION("FinancialTrading")
/*
Collect from household classes and firms (C and K) the relevant financial aggregate
*/
V("Production");
v[0]=v[1]=v[2]=v[3]=v[4]=v[5]=v[6]=v[7]=0;


CYCLE(cur, "Class")
 {
  v[6]=VS(cur,"BalanceC");
  if(v[6]>0)
   v[0]+=v[6];
  else
   v[1]-=v[6]; 
 }


cur2=SEARCH("Bank");

WRITES(cur2,"TotalSavings",v[0]);
WRITES(cur2,"TotalWithdrawals",v[1]);

CYCLE(cur, "Firm")
 {
  cur1=SEARCHS(cur,"BankF");
  v[6]=VS(cur1,"BalanceF");
  if(v[6]>0)
   v[2]+=v[6];
  else
   {
    v[3]-=v[6];
    INCRS(cur1,"DebtF",-v[6]);
   }
  v[7]+=VS(cur1,"DebtF");  
  
 }

CYCLE(cur, "KFirm")
 {
  cur1=SEARCHS(cur,"BankK");
  v[6]=VS(cur,"BalanceK");
  if(v[6]>0)
   v[2]+=v[6];
  else
   {
    v[3]-=v[6];
    INCRS(cur1,"DebtK",-v[6]);
   }
  v[7]+=VS(cur1,"DebtK");   
 }

WRITES(cur2,"TotalCapital",v[7]);
WRITES(cur2,"TotalDividends",v[2]);
WRITES(cur2,"TotalLosses",v[3]); 
RESULT(1)

EQUATION("UnitValue")
/*
The value of a unit varies exogenously depending on a discount rate expressing the growth or fall of value of existing assets
*/

v[0]=VL("TotalNumberUnits",1);
v[1]=VL("TotalValue",1);
v[2]=v[1]/v[0];
RESULT(v[2] )

EQUATION("TotalValue")
/*
Total value of the assets
*/

v[0]=V("Liquidity");
v[1]=V("TotalCapital");
WRITE("ShareCapital",v[1]/(v[0]+v[1]));

RESULT(v[0]+v[1] )

EQUATION("NewUnits")
/*
The number of new units is the new investment divided by the unit value 
*/

v[0]=V("TotalSavings");
v[1]=V("UnitValue");
RESULT(v[0]/v[1])

EQUATION("TotalNumberUnits")
/*
Total number of units
*/
v[0]=VL("TotalNumberUnits",1);
v[1]=V("NewUnits");
v[2]=V("SoldUnits");
RESULT(v[0]+v[1]-v[2] )

EQUATION("SoldUnits")
/*
Comment
*/
v[0]=V("TotalWithdrawals");
v[1]=V("UnitValue");
RESULT(v[0]/v[1] )

EQUATION("Liquidity")
/*
Available liquidity
*/

v[0]=VL("Liquidity",1);
v[1]=V("OutgoingLiquidity");
v[2]=V("TotalSavings");

RESULT(v[0]+v[2]+v[3]-v[1])

EQUATION("CapitalUsed")
/*
Total capital currently employed by firms 
*/

v[0]=VL("CapitalUsed",1);
v[1]=V("CapitalDemand");
v[2]=V("CapitalDestroyed");

RESULT(v[0]+v[1]-v[2]-v[1] )

EQUATION("OutgoingLiquidity")
/*
Lost liquidity
*/
V("FinancialTrading");
v[1]=V("TotalWithdrawals");
v[2]=V("CapitalDemand");
v[3]=V("TotalLosses");
RESULT(v[1]+v[2]+v[3])

EQUATION("Income")
/*
Comment
*/
V("Production");
v[0]=V("PremiaIncome");
v[1]=V("WageIncome");
v[3]=VL("ShareIncome",1);
v[4]=V("ExIncome");
v[14]=V("DividendsC");
v[12]=V("NoConsumption");
v[20]=V("LiquidityRentsC");

v[5]=v[0]+v[1]+v[3]*v[4]+v[14]+v[12];
RESULT(v[5])

EQUATION("Consumption")
/*
Desired level of consumption
*/

v[0]=VL("Income",1);
v[1]=V("SavingRate");
v[2]=v[0]*(1-v[1]);

RESULT(v[2] )


EQUATION("Expenditure")
/*
Total money spent by consumers, computed as a combination of past consumption and available resources (from wages and stock options)
*/
v[0]=VL("Expenditure",1);
v[1]=V("Consumption");
v[2]=V("aEx");
v[10]=v[0]*v[2]+(1-v[2])*(v[1]);
v[11]=max(v[10],0);
//if(t==13)
 //INTERACT("STOP EXP", v[1]);
RESULT(v[11] )

EQUATION("LiquidityRentsC")
/*
Comment
*/

v[0]=VLS(p->hook,"TotalNumberUnits",1);
v[1]=VLS(p->hook,"Liquidity",1);
v[4]=VS(p->hook,"InterestRateLiquidity");
v[2]=VL("NumberUnits",1);

if(v[0]>0)
  v[3]=v[4]*v[1]*v[2]/v[0];
else
  v[3]=0;  
RESULT(v[3])

EQUATION("DividendsC")
/*
Income coming from the dividend distributed to shareholder
*/

v[0]=VLS(p->hook,"TotalNumberUnits",1);
v[1]=VLS(p->hook,"TotalDividends",1);
v[2]=VL("NumberUnits",1);

if(v[0]>0)
  v[3]=v[1]*v[2]/v[0];
else
  v[3]=0;  
RESULT(v[3])

EQUATION("NumberUnits")
/*
Number of units of count defining the shares of dividends the class is entitled to 
*/

v[1]=VS(p->hook,"UnitValue");
v[0]=V("BalanceC");
v[2]=VL("NumberUnits",1);
v[3]=v[2]+v[0]/v[1];
v[5]=VS(p->hook,"TotalNumberUnits");
v[6]=v[3]/v[5];
WRITE("ShareUnits",v[6]);
RESULT( v[3])


EQUATION("BalanceC")
/*
Comment
*/
v[1]=V("Income");
v[2]=V("Expenditure");
v[4]=V("Recouped");

v[3]=v[1]+v[4]-v[2];

WRITE("shareRecouped",v[5]=v[4]/(v[4]+v[1]));
if(v[5] > 1 || v[5] <0 )
 INTERACT("SHARE recouped out of range",v[5]);
RESULT(v[3] )


EQUATION("IncomeCapita")
/*
Comment
*/
v[0]=V("Income");
v[1]=V("Individuals");
if(v[1]>0.001)
 v[2]=v[0]/v[1];
else
 v[2]=0; 
RESULT(v[2] )

EQUATION("TotIncomeCapita")
/*
Comment
*/
v[0]=V("TotIncome");
v[1]=V("TotIndividuals");
RESULT(v[0]/v[1] )

EQUATION("TotIncomeCapitaReal")
/*
Comment
*/
v[0]=V("TotIncomeCapita");
v[1]=V("AvPrice");
RESULT(v[0]/v[1] )

EQUATION("GDPConstantCapita")
/*
Comment
*/
v[0]=V("GdpConstant");
v[1]=V("TotIndividuals");
if(v[1]>0.001)
 v[2]=v[0]/v[1];
else
 v[2]=0; 
RESULT(v[2] )


EQUATION("TotConsumption")
/*
Total Consumption
*/

RESULT(SUM("Consumption") )

EQUATION("shareConsumption")
/*
Comment
*/
v[0]=V("Consumption");
v[1]=V("TotConsumption");
RESULT(v[0]/v[1] )

EQUATION("TotIncome")
/*
Total income from the wage
*/

v[1]=V("TotWage");
v[2]=V("TotPremia");

RESULT(v[1]+v[2] )

EQUATION("TotWage")
/*
Total wage
*/

v[0]=0;

CYCLE(cur, "Class")
 {
  v[0]+=VS(cur,"WageIncome");
 }

RESULT(v[0] )

EQUATION("TotPremia")
/*
Total premia
*/

V("Production");
v[0]=0;

CYCLE(cur, "Class")
 {
  v[0]+=VS(cur,"PremiaIncome");
 }

RESULT(v[0] )

EQUATION("PayTime")
/*
Semaphore resetting the parameters collecting the incomes from employees before being computed
*/
V("Trade");
CYCLE(cur, "Class")
 {
  WRITES(cur,"Individuals",0);
  WRITES(cur,"WageIncome",0);
  WRITES(cur,"PremiaIncome",0);
  WRITES(cur,"tempWage",0);
 }

RESULT(1)

EQUATION("ShareIncome")
/*
Share of total income for this class
*/

v[0]=V("Income");
v[1]=VS(p->up,"TotIncome");
RESULT(v[0]/v[1] )

EQUATION("ShareIndividuals")
/*
Share of total income for this class
*/

v[0]=V("Individuals");
v[1]=VS(p->up,"TotIndividuals");
RESULT(v[0]/v[1] )

EQUATION("ShareWageIncome")
/*
Share of total wage for this class
*/

v[0]=V("WageIncome");
v[1]=VS(p->up,"TotWage");
RESULT(v[0]/v[1] )

EQUATION("ShareExIncome")
/*
Comment
*/

RESULT(V("ShareWageIncome") )


EQUATION("SharePremiaIncome")
/*
Share of total premia for this class
*/

v[0]=V("PremiaIncome");
v[1]=VS(p->up,"TotPremia");
if(v[1]==0)
 v[2]=0;
else
 v[2]=v[0]/v[1]; 
RESULT(v[2] )

EQUATION("DecilesRatios")
/*
Compute a few statistics:
- Income of top 1%
- Income of top 10%
- Ratio of 1%/90%
- Ratio of 10%/90%
*/

v[0]=v[1]=v[2]=v[3]=v[21]=v[22]=v[23]=0;
SORT("Class","IncomeCapita", "DOWN");
CYCLE(cur, "Class")
 {
  v[4]=VS(cur,"ShareIndividuals");
  v[7]=VS(cur,"IncomeCapita");
  
  v[8]=0.01;
  if(v[0]<v[8])
   {//for the 1%
    v[5]=min(v[8], v[0]+v[4]);
    v[6]=v[5]-v[0];//it is v[4], if you did not reach the threshold, or the part of it necessary to reach the top
    v[1]+=v[7]*v[6];
    v[21]+=v[6];
   }
  v[8]=0.1;
  if(v[0]<v[8])
   {//for the 10%
    v[5]=min(v[8], v[0]+v[4]);
    v[6]=v[5]-v[0];//it is v[4], if you did not reach the threshold, or the part of it necessary to reach the top
    v[2]+=v[7]*v[6];
    v[22]+=v[6];
   }
  v[0]+=v[4]; 
 }
WRITE("IncomeTop1",v[41]=v[1]*100);
WRITE("IncomeTop10",v[42]=v[2]*10);

v[0]=0;
SORT("Class","IncomeCapita", "UP");
CYCLE(cur, "Class")
 {
  v[4]=VS(cur,"ShareIndividuals");
  v[7]=VS(cur,"IncomeCapita");
  
  v[8]=0.90;
  if(v[0]<v[8])
   {//for the 90%
    v[5]=min(v[8], v[0]+v[4]);
    v[6]=v[5]-v[0];//it is v[4], if you did not reach the threshold, or the part of it necessary to reach the top
    v[3]+=v[7]*v[6];
    v[23]+=v[6];
   }
  v[0]+=v[4];
 }
WRITE("IncomeBottom90",v[43]=v[3]/(0.9));
 
WRITE("Ratio1-90",v[41]/v[43]);
WRITE("Ratio10-90",v[42]/v[43]);
SORT("Class","NumClass", "UP");
RESULT(1 )

EQUATION("GlobalIncome")
/*
Differentiate from TotIncome because considers also the financial rents
*/

v[0]=0;
CYCLE(cur, "Class")
 {
  v[0]+=VS(cur,"Income");
 }

RESULT(v[0] )


EQUATION("UnitLaborCost")
/*
Unit labor cost
*/

v[21]=v[22]=v[4]=0;
CYCLE(cur, "Labor")
 {
  v[16]=VS(cur,"wage");
  v[17]=VS(cur,"NumWorkers");
  if(v[4]==0)
   {
    v[5]=v[17];
    v[4]=1;
   } 
  v[21]+=v[16]*v[17];
  v[22]+=v[17]; // total labour force
 }

if(v[22]>0)
 v[23]=v[21]/v[22];
else
 v[23]=0; 
WRITE("AvWage",v[21]/v[22]);
WRITE("LaborForce",v[22]);
/*
v[0]=V("CapitalCapacity");
v[1]=V("LaborCapacity");
v[2]=min(v[0],v[1]);
v[24]=v[21]/v[2];
*/

v[2]=V("MaxLaborProductivity");
if(v[5]>0)
 v[24]=v[21]/(v[2]*v[5]);
else
 v[24]=v[21]; 
RESULT(v[24] )


EQUATION("price")
/*
Markup on the unit production cost
*/
v[10]=V("markup");
v[22]=V("UnitLaborCost"); // labour in the first tier (the ones which define the production capacity)

v[14]=v[10]*v[22];

RESULT(v[14] )


EQUATION("Profit")
/*
Profit, difference between revenues, total costs and RD
*/
v[0]=V("Revenues");
v[2]=V("LaborCost");
v[3]=V("RdExpenditure");


v[4]=v[0]-v[2]-v[3];

RESULT(v[4] )


EQUATION("MovAvProfit")
/*
Moving average of the profits.
Test to be uases as an indicator of profits that is not oscillating as unit sales, and allows to use profits to trigger exit from market
*/

v[1]=VL("MovAvProfit",1);
v[2]=VL("Profit",1);
v[3]=V("aProfit");
v[4]=v[3]*v[2]+(1-v[3])*v[1];

RESULT(v[4] )


EQUATION("ExpectedProfit")
/*
Level of profits perceived by the firm, which reflects the expected gains from the sales.
To be used as an indicator of profit that is not oscillating as unit sales, and that would allow to use profits to trigger exit from markets 
*/

v[0]=V("ExpectedSales");
v[1]=V("price");
v[2]=V("LaborCost");
v[3]=v[0]*v[1]-v[2];

RESULT(v[3] )

EQUATION("LaborCost")
/*
Comment
*/
v[0]=0;
CYCLE(cur, "Labor")
 {
  v[1]=VS(cur,"NumWorkers");
  v[2]=VS(cur,"wage");
  v[0]+=v[1]*v[2];
 }

RESULT(v[0] )


EQUATION("ExpectedProfit2")
/*
Level of profits perceived by the firm, which reflects the gains from the desired production
*/

v[0]=V("DesiredQ");
v[1]=V("price");
v[2]=V("LaborCost");
v[3]=v[0]*v[1]-v[2];

RESULT(v[3] )


EQUATION("ExpectedProfit3")
/*
Level of profits perceived by the firm, which reflects the gains from the actual production
*/

v[0]=V("Q");
v[1]=V("price");
v[2]=V("LaborCost");
v[3]=v[0]*v[1]-v[2];

RESULT(v[3] )

EQUATION("IncLearningK")
/*
Express the skills of the firm in exploiting the capital, where 0 is the inability to use the capital and
1 is the maximum exploitation
*/

v[0]=V("aLearningK");
v[1]=CURRENT*v[0]+(1-v[0]);
RESULT(v[1] )


EQUATION("MaxKQ")
/*
Defines the Theoretical Labor Productivity of the Firm as incorporated in the various capital vintages of the firm.
*/
v[0]=0;
v[1]=0;
v[2]=V("CapitalDepress");//defines the depression rate of capital
v[3]=V("K");
v[4]=V("KAge");
v[9]=V("CapitalIntens");
v[6]=pow((1-v[2]),v[4]);//computes the depressiation of capital
v[19]=V("IncLearningK");
v[7]=v[3]*v[6]*v[19];//computes the actual stock of this capital vintage that can be used 

v[8]=v[7]/v[9];

RESULT(v[8] )


EQUATION("MaxLaborProductivity")
/*
Defines the Theoretical Labor Productivity of the Firm as incorporated in the various capital vintages of the firm.
*/

v[10]=V("CapitalStock");
v[0]=0;
v[1]=v[41]=v[42]=0;
v[2]=V("CapitalDepress");//defines the depression rate of capital
v[40]=V("CapitalIntens");

v[20]=0;
v[30]=VL("Inflation",1);
CYCLE_SAFE(cur, "Capital")
 {
//  MULTS(cur,"ResellPrice",(1+v[30]));

  v[3]=VS(cur,"K");
  v[4]=VS(cur,"KAge");
  v[5]=VS(cur, "IncProductivity");
  v[6]=pow((1-v[2]),v[4]);//computes the depressiation of capital
  v[7]=v[3]*v[6];//computes the actual stock of this capital vintage that can be used 
  
  if(v[7]/v[10]>0.01)
   {
    v[0]+=(v[7]*v[5]);
    v[1]+=v[7];
   }
  else
   DELETE(cur); 

}
v[8]=v[0]/v[1];//Max Labor productivity computed as the weighted average of the incorporated productivity in every capital vintages

v[50]=v[1]/v[40];
WRITE("CapitalStock",v[1]);
WRITE("CapitalCapacity",v[50]);
RESULT(v[8] )



EQUATION("KAge")
v[0]=VL("KAge",1);
RESULT((v[0]+1) );


EQUATION("wage")
/*
Wage of the workers in this layer of the firm
*/
v[5]=V("NumWorkers");
v[2]=V("IdLabor");
if(v[2]==1)
 { // first tier workers
  v[0]=V("MinWage");
  v[1]=V("wagecoeff"); // wage coefficient as the minimum wage multiplier
 }
else
 { // executives
  cur=SEARCH_CNDS(p->up,"IdLabor",v[2]-1);
  v[0]=VS(cur,"wage");
  v[1]=V("wagecoeff"); // wage coefficient as the wage tier multiplier
 }

VS(p->hook,"PayTime");
INCRS(p->hook,"Individuals",v[5]);
INCRS(p->hook,"WageIncome",v[5]*v[0]*v[1]);

RESULT((v[0]*v[1]) )



EQUATION("WagePrem")
/*
Wage premia distributed, when available to all classes of executives. 
*/

v[0]=V("Profit");

v[2]=V("roPremia");
v[5]=max(0,(v[0])*v[2]);
v[3]=0;

if(v[5]>0)
 {
  CYCLE(cur, "Labor")
   {
    v[10]=VS(cur,"IdLabor");
    if(v[10]>1)
     {
      v[2]=VS(cur,"wage");
      v[3]+=v[2];
     }
   }
   
  CYCLE(cur, "Labor")
   {
    v[10]=VS(cur,"IdLabor");
    if(v[10]>1)
     {v[2]=VS(cur,"wage");
      WRITES(cur,"Premia",v[5]*v[2]/v[3]);
      INCRS(cur->hook,"PremiaIncome",v[5]*v[2]/v[3]);
     }
   }
 }

RESULT(v[5] )

EQUATION("BalanceF")
/*
Balance of the current account for firms
*/
v[0]=V("Profit");
v[1]=V("WagePrem");

v[3]=v[0]-v[1];

RESULT(v[3])

EQUATION("LaborCostK")
/*
Comment
*/
v[0]=0;
CYCLE(cur, "KLabor")
 {
  v[0]+=VS(cur,"KNbrWorkers")*VS(cur,"KWage");
 }

v[0]+=V("KNbrEngineers")*V("KWageEngineers");  
RESULT(v[0] )

EQUATION("BalanceK")
/*
Balance of the current account for firms
*/

V("KProductionFlow");
v[0]=V("KRevenues");
v[1]=V("KWagePrem");
v[10]=V("LaborCostK");


v[5]=v[0]-v[10]-v[1]; //total liquidity after labor costs and premia
 
RESULT(v[5] )

EQUATION("KLaborCost") 
/* 
Comment 
*/ 
v[0]=0; 
CYCLE(cur, "KLabor") 
 { 
  v[1]=VS(cur,"KNbrWorkers"); 
  v[2]=VS(cur,"KWage"); 
  v[0]+=v[1]*v[2]; 
 } 

CYCLE(cur, "KEngineers") 
 { 
  v[1]=VS(cur,"KNbrEngineers"); 
  v[2]=VS(cur,"KWageEngineers"); 
  v[0]+=v[1]*v[2]; 
 } 

RESULT(v[0] ) 

/****************************************************************/
/******************** PROUCT INNOVATION *************************/
/****************************************************************/


EQUATION("RdExpenditure")
/*
Cumulated profits (not used to invest in kapital) devoted to product R&D
*/

V("InvestmentDecision");
v[0]=V("MovAvExpSales");

v[2]=V("roRD");
v[3]=v[2]*v[0];
v[1]=log(1+v[3]);

RESULT(v[1] )



EQUATION("ProdInno")
/*
The actual product innovation: extraction of a quality given the quality of the good the firm is currently producing
*/


if(INCR("tInno",1)<0)
 END_EQUATION(0);

v[17]=V("RdExpenditure");
 
V("Trade");
cur1=SEARCH_CND("IdPNeed",0);
cur=SEARCH_CNDS(cur1,"IdCh",2); 
v[2]=VS(cur,"x"); // check the current quality level of the produced good
v[4]=V("product"); // the sector in which the firm is currently producing
v[6]=V("ProdShockP")*v[2]; // productivity shock that determines the variance of the product innovation


v[0]=V("zInno"); //probability of hitting an innovation
for(v[14]=v[9]=0; v[9]<v[17]; v[9]++)
{
 if(RND<v[0]) 
 {
   v[8]=norm(v[2],v[6]); // outcome of the product innovation
   if(v[8]>v[2])
    {
     v[14]++;
     WRITELS(cur,"x",v[8],t);
     v[2]=v[8];
     v[10]=V("innoInterval");
     WRITE("tInno",-1*v[10]);
    }
 }   
}

RESULT(v[14] )

EQUATION("MovAvExpSales")
/*
Moving average of expected sales.
Used by the firm to decide wether to adopt a prototipe
*/

v[1]=VL("MovAvExpSales",1);
v[2]=VL("ExpectedSales",1);
v[3]=V("aExpSales");
v[4]=v[3]*v[2]+(1-v[3])*v[1];
RESULT(v[4])

EQUATION("KapitalNeed")
/*
Decide whether to order new capital.
*/


v[0]=V("Waiting");
if(v[0]==1)
 END_EQUATION(CURRENT);
 
V("MaxLaborProductivity");
v[3]=V("CapitalCapacity");
v[4]=V("ExpectedSales");
v[5]=V("backlog")/10;//a tenth of backlog should be got rid of.
v[7]=V("DesiredUnusedCapacity");
v[8]=V("CapitalIntens");
v[9]=(v[4]+v[5])*v[7];

v[23]=V("LaborCapacity");
v[24]=min(v[9],v[23]*10);//increase K if it is the bottleneck, considering also LaborCapacity
v[10]=v[24]-v[3];
v[11]=max(v[10],0);
if(v[11]==0)
 END_EQUATION(0);
 
v[12]=v[11]*v[8];//desired capital

END_EQUATION(v[12]);

/////// CANCELED ///////////////
v[14]=VL("SmoothProfit",1);
v[16]=V("AvKPrice");
v[17]=V("InterestRate");
v[24]=V("BacklogValue");

v[18]=max(0,v[14]) ;//financial constraints


v[20]=VL("NetWorth",1)-V("DebtF");//Financial feasibility
if(v[20]>=v[12]*v[16])
 {
  WRITE("RationingRatioFirm",1); 
  END_EQUATION(v[12]);
 }

v[21]=v[20]/(v[16]*v[12]);
v[12]*=v[21];

WRITE("RationingRatioFirm",v[21]); 


RESULT(v[12] )


EQUATION("InvestmentDecision")
/*
Place on order of K if you need it and did not place an order as yet
*/

v[0]=V("Waiting");
if(v[0]==1)
 END_EQUATION(1); //skip the equation if you already placed an order. To be edited to give the possibility to remove a too late order
//we are here only if there is no pending order

v[1]=V("KapitalNeed");
v[2]=V("AvKPrice");

v[3]=VL("TotalValue",1);
v[4]=VL("Liquidity",1);


if(v[1]>0 && RND<v[4]/v[3] )
 {
  V("PlaceOrder");
  WRITE("Waiting",1);
 } 

RESULT( 1)

EQUATION("SmoothProfit")
/*
Comment
*/
v[1]=CURRENT;
if(v[1]==-1)
  v[0]=V("Profit");
else
  v[0]=v[1]*0.9+0.1*V("Profit");  
RESULT(v[0] )

EQUATION("BacklogValue")
/*
Value of the backlog, future revenues
*/

v[0]=0;
CYCLES(p->up, cur, "blItem")
 {
  v[0]+=VS(cur,"blQ")*VS(cur,"blPrice");
 }

RESULT(v[0] )

EQUATION("NetWorth")
/*
Measure of the value of the firm in case of sales
*/

v[0]=V("KPresentValue")*V("PrudenceLending");
v[1]=V("BalanceF");
v[2]=V("DebtF");
v[4]=V("BacklogValue");

v[3]=v[0]+v[4]	-v[2]+v[1];
RESULT(v[3] )

EQUATION("KPresentValue")
/*
Present value of capital
*/

v[0]=v[1]=0;
v[5]=V("AvCurrProd");
v[6]=V("CapitalDepress");
v[2]=V("AvKPrice");
CYCLES(p->up, cur, "Capital")
 {
  //v[2]=VS(cur,"ResellPrice");
  v[3]=VS(cur,"IncProductivity");
  v[4]=VS(cur,"K");
  v[7]=VS(cur,"KAge");
  v[8]=pow(1-v[6],v[7]);
  
  v[0]+=v[2]*v[4]*v[8]*v[3]/v[5];
 }

RESULT(v[0] )


EQUATION("PlaceOrder")
/*
Place the order from the calling firm to a Kapital producer adopting the technology of the firm
*/

v[44]=VLS(c,"NetWorth",1);
v[0]=VS(c,"IdTech"); //this is the technology of the firm

//assuming there are many firms producing K with the same technologies, firm select the one they prefer in terms of price and productivity of the capital, and waiting time (insert also durability of the capital if we include depreciation as a function of production quantity and not time)
v[51]=v[53]=v[59]=v[60]=v[58]=v[70]=0;
//given the preference of the buyer firm wth respect to the features of the capital production, namely price and current productivity of the capital, and approximate time to wait for receiving the order
v[30]=VS(c,"betaPrice");
v[31]=VS(c,"betaProd");
v[32]=VS(c,"betaTime");
v[3]=VS(c,"KapitalNeed");
//check and evaluate the available supply

CYCLE(cur, "KFirm")
 {
  v[50]=VLS(cur,"KPrice",1);
  v[51]+=v[50];
  cur2=SEARCHS(cur,"KCapital");
  v[52]=VS(cur2,"CurrentProductivity");
  v[53]+=v[52];
  v[60]++;
  v[64]=VS(cur,"NumOrders");

  v[54]=VS(cur,"KQ");//number of productive workers
  // WRITES(cur,"WaitTime",ceil(v[3]/v[54]));
  WRITES(cur,"WaitTime",1);
  if(v[64]>0)
   {
    CYCLES(cur, cur1, "Order")
     {
      
      v[55]=VS(cur1,"KCompletion");
      v[56]=VS(cur1,"KAmount");
      v[57]=(v[56]-v[55])/v[54];
      INCRS(cur,"WaitTime",v[57]);
     }
   }

  v[65]=VS(cur,"WaitTime");
  if(v[65]<0)
   INTERACTS(cur, "Neg. waittime", v[65]);
  v[58]+=v[65];
// an index that gives the amount of time needed or a firm to complete the production of the capital already ordered
 }
 // write the average values for price  and producitivty of capital, and for the index of awaiting time. The averates are updated each time a firm place an order, as this would affectthe choice of a following firm in the same time period.
// NOTE THAT THIS PROVIDES A FIRST MOVER 'ADVANTAGE' TO THE FIRST FIRMS.
v[61]=v[51]/v[60];
WRITE("AvKPrice",v[61]);
v[62]=v[53]/v[60];
WRITE("AvCurrProd",v[62]);
v[63]=v[58]/v[60];
WRITE("AvWaitTime",v[63]);

v[81]=0;
CYCLE(cur, "KFirm")
 {
  v[1]=VS(cur,"IdKTech");
  if(v[0]!=v[1])
   WRITES(cur,"kapp",0);
  else
   WRITES(cur,"kapp",1);
  v[20]=VLS(cur,"KPrice",1); 
  v[21]=VS(cur,"CurrentProductivity");
  v[26]=VS(cur,"WaitTime");
  // normalise relative values (shares) in order to compare firms on the different indicators using the same evaluation for different units of measurement
  v[27]=v[20]/v[61]+1;
  v[28]=v[21]/v[62]+1;
  v[29]=v[26]/v[63]+1;
  if(v[27]*v[28]*v[29]==0)
   v[33]=0;
  else 
   v[33]=pow(v[28],v[31])*pow(v[29],-v[32])*pow(v[27],-v[30]);
  WRITES(cur,"kselect",v[33]*VS(cur,"kapp"));
  v[70]+=v[33];
  if(v[33]>v[81])
   {
    v[81]=v[33];
    cur1=cur;
   }

 }



//Legend:
//c: it is the final producer firm ordering the K
//cur: is the K producer

cur=cur1;
v[6]=VLS(cur,"KPrice",1);
if(VS(cur,"NumOrders")==0)
 cur1=SEARCHS(cur,"Order");
else
 cur1=ADDOBJS(cur,"Order");

//if(v[44]<v[3]*v[6])
  //v[63]=v[44]/v[6]; //REMOVED THE RATIONING DUE TO INSUFFICIENT NETWORTH
//else
 v[63]=v[3];

WRITES(c,"RationingRatioFirm",v[63]/v[3]); 
WRITES(cur1,"KAmount",v[63]);
if(v[63]<0)
 INTERACT("Neg.KAmount",v[63]);
WRITES(cur1,"KCompletion",0);
WRITES(cur1,"TimeWaited",1);
cur1->hook=c; //useful to retrieve quickly the ordering firm

INCRS(cur,"NumOrders",1);
v[4]=VLS(cur,"CurrentProductivity",1); //current state of the K art
WRITES(cur1,"Kproductivity",v[4]); //tech characteristics of the capital stock order
v[5]=VLS(cur,"CurrentSkillBiais",1);
WRITES(cur1,"KSkillBiais",v[5]);
WRITES(cur1,"KP",v[6]);// write the price of the capital in the period in which it is ordered, and use it to compute the actual expenditure using the `agreed' price.

RESULT(1 )



EQUATION("KProductionFlow")
/*
*/
//Activity of the K producing firm
v[0]=V("KQ"); //production capacity of the firm
v[1]=V("NumOrders");
if(v[1]==0)
 END_EQUATION(0);
v[2]=v[0]/v[1]; //one way to determine the amount of K production capacity per order. Otherwise...

v[3]=0;
CYCLE(cur, "Order")
 {
  v[4]=VS(cur,"KAmount");
  v[5]=VS(cur,"KCompletion");
  v[3]+=v[4]-v[5];
 }
cur5=SEARCH("BankK");
WRITES(cur5,"KRevenues",0);

CYCLE_SAFE(cur, "Order")
 {//increase the level of advancement of the orders and, if completed, remove the order. Given the production capacity, devote it respecting oreders' order (first comes first go, which allows to respect the priority given by customers, on side, and to reduce the dofferences between the price agreed upon ordering and the price at which the kapital is sold)
  v[4]=VS(cur,"KAmount");
  v[5]=VS(cur,"KCompletion");
  v[6]=(v[4]-v[5]); // given the missing quantity of the current order
  //v[7]=v[6]*v[0]; //share of production capacity devoted to this order
  v[8]=min(v[0], v[4]-v[5]); //use the production capacity needed actually neded to produce the order, or exhaust here the production capacity (for the current period)
  INCRS(cur,"KCompletion",v[8]);
  v[0]=v[0]-v[8];
  v[5]=VS(cur,"KCompletion"); //update the completion level  in order to cancel the order if done
  if(v[5]>=v[4])
   {//order fulfilled. Either search for the ordering firm, or simply use the hook
    if(v[5]>0)
     {//stupid control needed to not be confused by the very initial object
      INCRS(cur->hook,"NumK",1);
      //cur1=ADDOBJS(cur->hook,"Capital");
//      cur1=cur->hook->add_an_object("Capital");
      cur1=ADDOBJS(cur->hook,"Capital");
      WRITELS(cur1,"K",v[5],t);
      v[9]=VS(cur,"Kproductivity");
      WRITELS(cur1,"IncProductivity",v[9],t);
      v[10]=VS(cur,"KSkillBiais");
      WRITELS(cur1,"IncSkillBiais",v[10],t);
      WRITELS(cur1,"IncLearningK",0.1,t);
      WRITELS(cur1,"KAge",0,t);
      v[11]=VS(cur,"KP");
      v[12]=v[11]*v[5];
      WRITELS(cur1,"KExpenditures",v[12], t);
      WRITES(cur->hook,"Waiting",0); //tell the firms it has the new capital
      SORTS(cur->hook,"Capital","IncProductivity", "DOWN");
      cur5=SEARCHS(cur->hook,"BankF");
      INCRS(cur5,"DebtF",v[4]*v[11]); // sprintf(msg, " KF(%g)\n", v[4]*v[11]); plog(msg);  
      INCRS(cur5->hook,"CapitalDemand",v[4]*v[11]);
      cur5=SEARCH("BankK");
      INCRS(cur5,"KRevenues",v[4]*v[11]);
      
//      WRITES(cur1,"ResellPrice",v[11]*V("DiscountUsedK"));

      v[20]=INCR("NumOrders",-1);
      if(v[20]>0)
       DELETE(cur);
      else
       {
        WRITES(cur,"KAmount",0);
        WRITES(cur,"KCompletion",0);
        WRITES(cur,"TimeWaited",0);
        WRITES(cur,"Kproductivity",0);
        
       } 
     }
   }
  else
   {
    if(v[4]>0)
     INCRS(cur,"TimeWaited",1); // if orders remain non completed increase the time needed to go through future orders
   }
   
 }

v[13]=min(V("KQ"),v[3]); 
v[15]=V("KQ")-v[3];
v[16]=v[15]-v[0];
//if(v[15]>0 && v[15]!=v[0])
 //INTERACT("check the correspondence between production and KQ",v[16]);
//if(v[15]<0 && v[0]!=0)
 //INTERACT("check the correspondence between production and KQ",v[0]);
 
RESULT(v[13] )


EQUATION("KInnovation")
/*
Changes in the characteristics of kapital
*/

v[0]=VL("CurrentProductivity",1);
v[1]=VL("CurrentSkillBiais",1);
v[2]=VL("KNbrEngineers",1);
v[3]=V("z");
v[4]=V("ProductivityShock");
v[5]=V("SkillBiaisShock");
v[6]=-(v[2]*v[3]);
v[7]=1-exp(v[6]);//Proba to innovate increases with the nbr of engineers
//first random draw to define the outcome of R&D i.e. success or failure
if (RND<v[7])
	{
	v[8]=norm(0,v[4]);//second stockastic variable, defines the level of productivity out of the successful R&D process	
	v[9]=norm(0,v[5]);
	}
else
	{
	v[8]=0;
	v[9]=0;
	}
v[10]=max(v[8],0);
v[11]=v[0] + v[10];
WRITE("CurrentProductivity",v[11]);
v[12]=v[1] +v[9];
v[13]=max(v[12],0);
v[14]=min(v[13],1);
WRITE("CurrentSkillBiais",v[14]);
RESULT(1 )


EQUATION("KNbrEngineers")
/*
Number of enginers is a share of the number of blue collars. Though they are mantained only when there are the available cumulated profits (their cost should not enter in teh price determination)
*/

v[2]=VL("KWageEngineers",1);
v[3]=V("EngineersHiring"); // share of cuimulated profits devoted to increase the amount of engineers

v[1]=VL("BalanceK",1);
v[4]=0;
v[7]=max(v[1],v[4]); // in case in which profits are negative for a long time but the firm achieve to sell some macineries, we assume it decides to increase the attractiveness of its capital
v[5]=v[3]*(v[7]/v[2]);
//REMOVED
	
v[8]=V("EngineersShare");
v[9]=VS(p->up,"KNbrWorkers"); // number of first tier worker as a max to chose the number of engineers
v[11]=v[9]*v[8];
v[55]=VS(p->up,"KRatioVacancies");
INCRS(p->up,"KVacancies",v[55]*v[11]);

RESULT(v[11] )


EQUATION("KWageEngineers")
/*
Wage of engineers
*/
v[0]=V("MinWage");
v[1]=V("KEWagecoeff");
VS(p->hook->up,"PayTime");
v[3]=V("KNbrEngineers");
VS(p->hook->up,"PayTime");
INCRS(p->hook,"WageIncome",v[1]*v[0]*v[3]);
INCRS(p->hook,"Individuals",v[3]);
RESULT((v[0]*v[1]) )


EQUATION("KProfit")
/*
Comment
*/

V("KNbrEngineers"); // compute first the number of engineers which use the past value of cumulated profits
v[0]=V("KPrice");
v[1]=V("KProductionFlow");
v[8]=V("LaborCostK");

v[6]=(v[0]*v[1])-v[8];
v[7]=INCR("KCumProfit",v[6]);

RESULT(v[6] )


EQUATION("KPrice")
/*
Comment
*/
v[0]=V("Kmarkup");
v[8]=V("KQ"); // productive capacity of the firm
v[4]=V("LaborCostK");
v[7]=(1+v[0])*(v[4])/v[8];


RESULT(v[7] )


EQUATION("KWage")
/*
Comment
*/
v[12]=V("KNbrWorkers");
v[2]=V("IdKLabor");
if(v[2]==1)
 {
  v[0]=V("MinWage");
  v[1]=V("KWagecoeff");
 }
else
 {
  cur=SEARCH_CNDS(p->up,"IdKLabor",v[2]-1);
  v[0]=VS(cur,"KWage");
  v[1]=V("KWagecoeff"); // wage coefficient as the wage tier multiplier
 }
VS(p->hook,"PayTime");
INCRS(p->hook,"Individuals",v[12]);
INCRS(p->hook,"WageIncome",v[12]*v[0]*v[1]);

RESULT((v[0]*v[1]) )



EQUATION("KWagePrem")
/*
Wage premia distributed, when available to all classes of executives. 
*/

v[0]=V("KProfit");
v[2]=V("roPremia");
v[5]=max(0,v[0]*v[2]);

v[3]=0;
if(v[5]>0)
 {
  CYCLE(cur, "KLabor")
   {
    v[10]=VS(cur,"IdKLabor");
    if(v[10]>1)
     {
      v[2]=VS(cur,"KWage");
      v[3]+=v[2];
     }
   }
   
  CYCLE(cur, "KLabor")
   {
    v[10]=VS(cur,"IdKLabor");
    if(v[10]>1)
     {v[2]=VS(cur,"KWage");
      WRITES(cur,"KPremia",v[5]*v[2]/v[3]);
      INCRS(cur->hook,"PremiaIncome",v[5]*v[2]/v[3]);
     }
   }
 }

RESULT(v[5] )


EQUATION("KNbrWorkers")
/*
Number of workesr in the capital sector firms
*/
v[1]=0;
CYCLES(p->up, cur, "Order")
 {
  v[11]=VS(cur,"KAmount");
  v[12]=VS(cur,"KCompletion"); 
  v[1]+=v[11]-v[12];
 }
v[15]=0;
CYCLE_SAFES(p->up, cur, "KLabor")
 { //check how many tiers already exist
  if(VLS(cur,"KNbrWorkers",1)>0)
    v[15]++;
  else
    DELETE(cur);  
 }
v[14]=V("IdKLabor");
if(v[14]==1)
 {
  v[0]=VL("KNbrWorkers",1);
  v[2]=VL("KLaborProductivity",1);
  v[4]=V("KDesiredUnusedCapacity");
  v[3]=v[4]*(v[1]/v[2]);
  v[5]=V("KaNW");
  v[6]=v[0]*v[5]+(1-v[5])*v[3];
  v[33]=v[3]>v[6]?v[3]-v[6]:0;
  v[54]=v[33]/v[6];
  WRITES(p->up,"KVacancies",v[33]);
  WRITES(p->up,"KRatioVacancies",v[54]);
  if(v[6]<1)
   v[6]=1; //limit the minimum number of workers to 1 to avoid crazy errors.
 }
else
 {// when above the first tier workers...
  v[18]=V("IdKLabor");
  cur=SEARCH_CNDS(p->up,"IdKLabor",v[18]-1); 
  v[19]=VS(cur,"KNbrWorkers"); //and the number of workers in the previous tier
  v[21]=VS(cur,"knu"); //given the worker ratio between tiers n the tier below
  v[6]=v[19]/v[21]; // compute the required executives
  v[54]=VS(p->up,"KRatioVacancies");
  INCRS(p->up,"KVacancies",v[6]*v[54]);
  v[17]=V("knu"); //given the worker ratio between tiers in the present tier
  if(v[6]>=v[17] && v[18]==v[15])
   { 
    cur1=ADDOBJS_EX(p->up,"KLabor",p);
    WRITES(cur1,"IdKLabor",v[18]+1);
    v[20]=v[6]/v[17];
    WRITELS(cur1,"KNbrWorkers",v[20], t);
    WRITELLS(cur1,"KNbrWorkers",0, t,1); // write also that the number of workers in the previous period is 0, to be used in the statistics
    v[22]=0;
    CYCLES(p->up->up->up,cur2, "Class")
     {if(v[22]==0)
       {
        cur7=SEARCHS(cur2,"BankC");
        cur7=cur7->hook;
       }
      v[22]++;
     }
    if(v[22]-1<v[18]+1)
     { /// if there is still not a class for the new tyoe of wage earner create one
      cur2=SEARCH_CNDS(p->up->up->up,"NumClass",v[18]);
      cur3=ADDOBJS_EX(cur2->up,"Class",cur2);
      cur6=SEARCHS(cur3,"BankC");
      cur6->hook=cur7;

      cur1->hook=cur3;
      v[44]=VS(cur2,"SavingRate");
      v[45]=VS(cur2->up,"SRMultiplier");
      v[46]=v[44]*(1-v[45])+v[45];
      WRITES(cur3,"SavingRate",v[46]);
      
      WRITES(cur3,"NumClass",v[18]+1);
      WRITELS(cur3,"Expenditure",0, t-1);
      WRITELS(cur3,"ShareWageIncome",0, t-1);
      WRITELS(cur3,"SharePremiaIncome",0, t-1);
      //WRITELS(cur3,"ShareProfitIncome",0, t-1);
 //     WRITELS(cur3,"Savings",0, t-1);
      // WRITELS(cur3,"NumIterations",0, t-1); to reactivate when NumIterations report the number of consumers in the labour class, as given in equation "ShareWageIncome". Otherwise the number of iterations simply define the number of representative conusmers (groups) in a class
      WRITELS(cur3,"ShareIncome",0, t-1); // reset the share income to be recomputed
      WRITES(cur3,"Individuals",v[20]); // set the number of individuals to nu;ber of workers of the new class
      v[35]=VS(cur2,"LorenzInd"); 
      WRITES(cur3,"LorenzInd",v[35]+v[20]); // set total number of workers as previous total plus new workers
      WRITELS(cur3,"NoConsumption",0, t-1); // set the available income due to unavailability of goods to 0
      CYCLES(cur3, cur, "Need")
       { // enter in neds and characterisitcs to change the tau parameter (the minimum is set every period)
        v[27]=VS(cur,"IdNeed");
        CYCLES(cur, cur1, "DCh")
         {
          v[26]=VS(cur1,"IdDCh");
          CYCLES(cur2, cur4, "Need")
           { // cycle among the needs of the preceding class
            v[28]=VS(cur4,"IdNeed");
            if(v[28]==v[27])
             { // when in the same need as the one the new class is cycling cycle through the char of the preceding class
              CYCLES(cur4, cur5, "DCh")
               { 
                v[29]=VS(cur5,"IdDCh");
                if(v[29]==v[26])// when in the same characteristic the new class is cycling
                 v[30]=VS(cur5,"tau"); // read the value of the tau parameter
               }

             }
           }
          v[31]=VS(cur1,"tauMultiplier"); // the speed of adjustment of the tolerance level from one class to the following one
          v[34]=VS(cur1,"NegativeQuality");
          if(v[34]>0)
           v[33]=VS(p->up->up->up,"tauMax"); // the asympthotic level of the tolerance level (1 for qualities and 0 for price)
          if(v[34]<0)
           v[33]=VS(p->up->up->up,"tauMin"); // the asympthotic level of the tolerance level (1 for qualities and 0 for price)
          v[32]=v[30]*(1-v[31])+v[31]*v[33]; // adjustment in the treshold level of tolerance
          WRITES(cur1,"tau",v[32]); // finally write the tau for the new consumer class in each of its characteristic for each need      
         }

       }
     v[35]=VS(cur3,"ComputeShare"); // set the expenditure shares for the needs in the new class
     }
    else
     {
      cur3=SEARCH_CND("NumClass",v[18]+1);
      cur1->hook=cur3;
     } 
   }
  if(v[18]>2 && v[19]<v[21])
   v[6]=0;
 }

RESULT(v[6] )


EQUATION("KQ")
/*
Comment
*/

v[0]=V("KNbrWorkers");
v[1]=V("KLaborProductivity");

RESULT((v[0]*v[1]) )


EQUATION("MinWage")
/*
Sets the minimum wage for all categories, as an aggregate relation. Variables influecing overall wage are: aggregate productivity, inflation, and unemployment. 
Aggregate productivity?
Unemployment: to account for Beveridge curves we could use the suggishness in the hiring process, which gnerates rates of vacancies...
NOTE: probably it makes sense to use levels for all variables. That is, when the variable reaches a certain level, a wage resetting is unedergone: if inflation runs too high, wages are renegotiated, if aggregate productivity increase evidently, wage are renegotiated.

*/
V("NbrWorkers");
v[6]=(double)t;
v[0]=VL("MinWage",1);
//END_EQUATION(v[0]);
v[10]=V("InitAggProd"); //the reference level of productivity 
v[2]=V("MovAvAggProd");
v[11]=V("IncrAggProd"); 
v[12]=v[10]+v[10]*v[11]; //required increase in productity to change the min wage
v[13]=V("MovAvPrice");
v[14]=V("InitAvPrice"); //the reference level of prices
v[15]=V("IncrAvPrice");
v[16]=v[14]+v[14]*v[15]; //required increase in prices to change the min wage
v[17]=VL("LTUnemployment",1);
v[18]=VL("STUnemployment",1);

v[30]=V("minMinWage");
v[31]=V("maxMinWage");
v[32]=V("elasMinWage");

v[5]=v[30]+v[31]*pow(v[18],-v[32]);
v[6]=0.95*v[0]+0.05*v[5];

if(v[2]>v[12])
 {
 
  v[30]=(v[12]/v[10])*v[30];
  v[31]=(v[12]/v[10])*v[31];  
  WRITE("InitAggProd",v[2]);
 // WRITE("IncrAggProd",v[11]*v[2]/v[10]);  
 }

if(v[13]>v[16])
 {
 
  v[30]=(v[16]/v[14])*v[30];
  v[31]=(v[16]/v[14])*v[31];  
  WRITE("InitAvPrice",v[13]);
 // WRITE("IncrAvPrice",v[15]*v[13]/v[14]);  
 }

WRITE("minMinWage",v[30]);
WRITE("maxMinWage",v[31]);

RESULT(v[6] )


EQUATION("AggProductivity")
/*
Comment
*/
v[0]=0;
v[1]=v[30]=v[31]=v[32]=0;
CYCLE(cur2, "Supply")
 {
  CYCLES(cur2, cur, "Firm")
   {
    v[2]=VS(cur,"Q");
    v[0]+=v[2];
    v[30]++;
    v[31]+=VS(cur,"Profit");
//    v[32]+=VS(cur,"CumProfit");
    CYCLES(cur, cur1, "Labor")
     {
      v[3]=VS(cur1,"NumWorkers");
      v[1]+=v[3];
     }
   }
 }

WRITE("AvProfit",v[31]/v[30]);
//WRITE("AvCumProfit",v[32]/v[30]);
v[4]=0;
v[5]=0;
v[10]=v[20]=v[21]=v[22]=v[23]=0;
CYCLE(cur, "KFirm")
 {v[20]++;
 	v[7]=VS(cur,"KNbrEngineers");	
 	v[10]+=v[7];
    v[8]=VS(cur,"KProductionFlow");
    v[5]+=v[8];
  v[21]+=VS(cur,"KProfit");
  v[22]+=VS(cur,"KCumProfit"); 
  v[23]+=VS(cur,"CurrentProductivity"); 
   CYCLES(cur, cur1, "KLabor")
    {
     v[6]=VS(cur1,"KNbrWorkers");
  	  v[4]+=v[6];
    }

 }
 
WRITE("AvCurrentProductivity",v[23]/v[20]);
WRITE("AvKProfit",v[21]/v[20]);
//WRITE("AvKCumProfit",v[22]/v[0]);
v[9]=(v[0]+v[5])/(v[1]+v[4]+v[10]);
if(v[5]<0)
 INTERACT("Neg.Kprod", v[5]);
WRITE("TotalKProduction",v[5]);
RESULT(v[9] )


EQUATION("MovAvAggProd")
/*
Exponential Moving Average of the aggregate productivity
*/

v[1]=VL("MovAvAggProd",1);
//v[2]=VL("AggProductivity",1);
v[2]=VL("AvCurrentProductivity",0);
v[3]=V("aAgProd");
v[4]=v[3]*v[2]+(1-v[3])*v[1];

RESULT(v[4] )



EQUATION("Ms")
/*
Market share of each firm, computed differently only for firms in the consumable market
*/

v[1]=V("UnitSales");
v[2]=VS(p->up->up,"TotUSales");
v[3]=VL("Ms",1);
if(v[2]>0)
 v[4]=v[1]/v[2];
else
 v[4]=v[3];

RESULT(v[4] )


EQUATION("TotUSales")
/*
Total unit sold by the consumabe sector
*/

v[2]=0;
CYCLE(cur1, "Supply")
 {
  CYCLES(cur1, cur, "Firm")
   {
    v[1]=VS(cur,"UnitSales");
    v[2]+=v[1];
   }
 }

RESULT(v[2] )



EQUATION("AvPrice")
/*
Average prices
*/

v[3]=0;
CYCLE(cur1, "Supply")
 {
  CYCLES(cur1, cur, "Firm")
   {
    v[1]=VS(cur,"price");
    v[2]=VS(cur,"Ms");
    v[3]+=v[1]*v[2];
   }
 }

RESULT(v[3] )

EQUATION("Age")
/*
Age of the firm
*/

RESULT(CURRENT+1 )


EQUATION("MovAvPrice")
/*
Exponential Moving Average of the inflation
*/

v[1]=VL("MovAvPrice",1);
v[2]=VL("AvPrice",1);
v[3]=V("aAvPrice");
v[4]=v[3]*v[2]+(1-v[3])*v[1];


RESULT(v[4] )

EQUATION("Inflation")
/*
One period change in price in the conumables market
*/

v[1]=VL("AvPrice",1);
v[2]=V("AvPrice");
v[3]=v[2]/v[1]-1;

RESULT(v[3] )




EQUATION("TotVacancies")
/*
Number of vacancies
*/

v[1]=V("NbrWorkers");
v[0]=0;

CYCLE(cur,"Firm")
 {
  v[0]+=VS(cur,"Vacancies");
  v[1]++;
 }
 
CYCLE(cur,"KFirm")
 {
  v[0]+=VS(cur,"KVacancies");  
 }

WRITE("AvRatioVacancies",v[0]/v[1]);
RESULT(v[0] )


EQUATION("MovAvTotVac")
/*
Exponential Moving Average of Total vacancies
*/

v[1]=VL("MovAvTotVac",1);
v[2]=VL("TotVacancies",1);
v[3]=V("aTotVac");
v[4]=v[3]*v[2]+(1-v[3])*v[1];

RESULT(v[4] )





EQUATION("UnemploymentRate")
/*
We derive the level of unemployment using directly the Beveridge curve, without generateing it. Two options:
Linear equation: U = Constant - beta*V (Nickell et al: beta=0.23; Wall & Zoega: beta=0.5; Teo et al: 0.3>beta<0.9)
Hyperbolic equation: U = Constant + beta/V (Börsch-Supan: c<1, beta=6)
Note that both Constant and beta should be quite different when using one or the other relation. See the Earnings_and_Consumption file
*/

v[1]=V("c");
v[2]=V("AvRatioVacancies");
v[3]=V("beta");
//v[4]=v[1]-v[3]*v[2];
v[4]=v[1]+pow(v[3],3*v[2]+0.5);

RESULT(v[4] )

EQUATION("LTUnemployment")
/*
Long term unemployment rate
*/

v[0]=VL("LTUnemployment",1);
v[1]=V("UnemploymentRate");
v[2]=V("aLTUR");

v[3]=v[0]*v[2]+(1-v[2])*v[1];
RESULT( v[3])

EQUATION("STUnemployment")
/*
Short term unemployment rate
*/

v[0]=VL("STUnemployment",1);
v[1]=V("UnemploymentRate");
v[2]=V("aSTUR");

v[3]=v[0]*v[2]+(1-v[2])*v[1];
RESULT( v[3])


EQUATION("NbrWorkers")
/*

*/

v[3]=v[8]=0;
CYCLE(cur2, "Supply")
 {
  CYCLES(cur2, cur, "Firm")
   {
    CYCLES(cur, cur1, "Labor")
     {
      v[1]=VS(cur1,"NumWorkers");
      v[2]=VLS(cur1,"NumWorkers",1);
      v[3]+=v[1];
      v[8]+=v[2];
     }
  
   }
 }
CYCLE(cur, "KFirm")
 {
  CYCLES(cur, cur1, "KLabor")
   {
    v[4]=VS(cur1,"KNbrWorkers");
    v[5]=VLS(cur1,"KNbrWorkers",1);
    v[3]+=v[4];
    v[8]+=v[5];
   }
  cur2=SEARCHS(cur,"KEngineers");
  v[6]=VS(cur2,"KNbrEngineers");
  v[7]=VLS(cur2,"KNbrEngineers",1);
  v[3]+=v[6];
  v[8]+=v[7];
 }

RESULT(v[3] )


EQUATION("MovAvNbrWork")
/*
Exponential Moving Average of the number of worwers
*/

v[1]=VL("MovAvNbrWork",1);
v[2]=VL("NbrWorkers",1);
v[3]=V("aNbrWork");
v[4]=v[3]*v[2]+(1-v[3])*v[1];

RESULT(v[4] )


EQUATION("ComputeShare")
/*
The Share is a function that is called when a classed is formed to determine the shares of expenditure across needs, as a change with respect to the previous class. 
he dynamic mimics engels laws (shiting the share of expenditures toward different needs, thus reducing the expenditure on 'basic' needs, as income increases)
This Share function simply normalise the shares computed in ExpShares
*/

v[1]=SUM("ExpShare"); //compute the sum of the newly computed expenditure shares
v[4]=v[7]=0;
CYCLE(cur, "Need")
 { // cycle through the needs
  v[2]=VS(cur,"ExpShare"); 
  v[3]=v[2]/v[1]; // normalise the share
  WRITES(cur,"Share",v[3]); // and fix it
  v[4]+=v[3]; // check that the sum is equal to 1
  v[5]=v[3]*151; // compute the number of iterations given the consumption shares
  v[6]=round(v[5]); // round to the closest integer
  WRITES(cur,"NumIterations",v[6]);
  v[7]+=v[6]; // check that the sum of interations is 100
 }

RESULT(v[4]+v[7] )


EQUATION("ExpShare")
/*
Change in the expenditure shares across classes
*/

v[1]=VS(p->up->up,"eta"); // iterrelations between the changes in the shares
v[2]=VS(p->up, "NumClass"); // check the current class for which the share is computed
v[5]=V("IdNeed");
v[3]=V("endExpShare"); // asymptotic value of the expenditure share for the current need (defined in the beginning symmetric to the first class distribution of shares)
CYCLES(p->up->up, cur, "Class")
 {
  v[4]=VS(cur,"NumClass"); 
  if(v[4]==v[2]-1)
   { // select the class below the one for which the shares are computed
    CYCLES(cur, cur1, "Need")
     { //  cycle through the different needs
      v[6]=VS(cur1,"IdNeed");
      if(v[6]==v[5])
       v[7]=VS(cur1,"Share"); // read the value of the Share
     }

   }
 }
v[8]=v[1]*(v[7]-v[3]);
v[9]=v[7]*(1-v[8]);

RESULT(v[9] )




/*******************************************************************************
INITIALISATIONS
********************************************************************************/




EQUATION("Init")
/*
Initialisation of initial values
*/

V("Init_x");

v[22]=VL("MinWage",1);
CYCLE(cur4, "Supply")
 {cur7=SEARCH("Bank");
  cur6=SEARCH("Demand");
  CYCLES(cur6, cur1, "Class")
   {
    cur5=SEARCHS(cur1,"BankC");
    cur5->hook=cur7;
   }

  CYCLES(cur4, cur1, "Firm")
   {// run a first cycle trough firms to set the number of labor in t-1
    cur=SEARCHS(cur1,"BankF");
    cur->hook=cur7;
    v[15]=0;
    v[16]=VS(cur1,"product");
    cur5=SEARCH_CND("IdGood",v[16]);
    cur6=SEARCHS(cur5,"sFirm");
    if(VS(cur6, "IdsFirm")!=-1)
     cur6=ADDOBJS(cur5,"sFirm");

    WRITES(cur6,"IdsFirm",VS(cur1, "IdFirm")); 
    cur1->hook=cur6;
    cur6->hook=cur1;
    CYCLES(cur1,cur, "Labor")
     { //check how many tiers already exist
      v[15]++;
     }

    CYCLES(cur1, cur, "Labor")
     {
      v[14]=VS(cur,"IdLabor");
      cur3=SEARCH_CND("NumClass",v[14]);
      cur->hook=cur3;
      if(v[14]==1)
       { // compute the first tier workers given their productivity and production needs
        //V("aNWDynamic");
        //v[0]=VL("NumWorkers",1);

        v[1]=VLS(cur1,"ExpectedSales",1); //use the value of expected sales to compute the number of workers in the initial period
        //v[1]=VS(cur1,"DesiredQ");
        //v[10]=V("backlog");
        //v[11]=v[1]+v[10];
        v[2]=VS(cur1,"MaxLaborProductivity");
        v[8]=VS(cur1,"CapitalCapacity");
        v[9]=min(v[1],v[8]);
        //v[2]=VLS(cur1,"MaxLaborProductivity",1);
        v[4]=VS(cur1,"DesiredUnusedCapacity");
        v[3]=v[4]*(v[9]/v[2]); // number of workers in the first layer in the first period	
        WRITELLS(cur,"NumWorkers",v[3],t,1);
        WRITELS(cur,"NumWorkers",v[3],t);
        v[21]=VS(cur,"wagecoeff");
        v[23]=v[22]*v[21];
        WRITELS(cur,"wage",v[23], t-1);
       }
      else
       {// when above the first tier workers...
        v[18]=VS(cur,"IdLabor");
        cur2=SEARCH_CNDS(cur->up,"IdLabor",v[18]-1); 
        v[17]=VS(cur2,"nu"); //given the worker ratio between tiers of the tier below
        v[19]=VS(cur2,"NumWorkers"); //and the number of workers in the previous tier
        v[6]=v[19]/v[17]; // compute the required executives
        v[24]=VS(cur,"nu"); // given the worker ration between tiers of the present tier (use different worker ratios in case we want to change it along the hierarchical structure)
        WRITELLS(cur,"NumWorkers",v[6], t,1);
        WRITELS(cur,"NumWorkers",v[6],t);
        v[25]=VLS(cur2,"wage",1);
        v[26]=VS(cur,"wagecoeff");
        v[27]=v[25]*v[26];
        WRITELS(cur,"wage",v[27], t-1);
        if(v[6]>=v[24] && v[18]==v[15])
         {
          cur3=ADDOBJS_EX(cur->up,"Labor",cur);
          WRITES(cur3,"IdLabor",v[18]+1);
          v[20]=v[6]/v[24];
          WRITELLS(cur3,"NumWorkers",v[20], t,1);
          WRITELS(cur3,"NumWorkers",v[20],t);
          v[28]=v[27]*v[26];
          WRITELS(cur3,"wage",v[28], t-1);
         }
       }
  
     }
   }
 }

CYCLE(cur2, "Supply")
 {
  CYCLES(cur2, cur, "Firm")
   {// a second cycle to set the price in t-1
    v[10]=1+V("minMarkup");
    WRITELS(cur,"markup",v[10], t);
    v[21]=v[22]=0;
    CYCLES(cur,cur1, "Labor")
     {
      v[16]=VLS(cur1,"wage",1);
      v[17]=VLS(cur1,"NumWorkers",1);
      v[21]+=v[16]*v[17];
      v[22]+=v[17];
     }
    //if(v[21]==0)
     //v[11]=V("AvWage");
    //else
     v[11]=v[21]/v[22];  //av. wage in the firm
    WRITES(cur,"AvWage",v[11]);
    v[0]=v[31]=v[32]=0;
    CYCLES(cur,cur1, "Capital")
     {
      v[3]=VS(cur1,"MaxKQ");
      v[5]=VS(cur1,"IncProductivity");
      v[0]+=v[3];
      v[32]+=v[5]*v[3];
     }
    v[9]=v[32]/v[0]; //weighted av. incProductivity
    WRITELS(cur,"MaxLaborProductivity",v[9],t-1);
    v[14]=v[11]*v[10]/v[9];
    if(v[14]<0)
     v[14]=INTERACT("nEG. price", v[14]);
    WRITELS(cur,"price",v[14], t-1);
    v[64]=VS(cur,"product");
    cur3=SEARCH_CND("IdGood",v[64]);
    INCRS(cur3,"NFirmsS",1);

   }
 }

v[3]=v[8]=0;
CYCLE(cur2, "Supply")
 {
  CYCLES(cur2, cur, "Firm")
   {
    CYCLES(cur, cur1, "Labor")
  	{
      v[1]=VS(cur1,"NumWorkers");
      v[2]=VLS(cur1,"NumWorkers",1);
      v[3]+=v[1];
      v[8]+=v[2];
     }
  
   }
 }
CYCLE(cur, "KFirm")
 {
  cur1=SEARCHS(cur,"BankK");
  cur1->hook=cur7;
  CYCLES(cur, cur1, "KLabor")
   {
    v[30]=VS(cur1,"IdKLabor");
    cur3=SEARCH_CND("NumClass",v[30]);
    cur1->hook=cur3;
    v[4]=VS(cur1,"KNbrWorkers");
    v[5]=VLS(cur1,"KNbrWorkers",1);
    v[3]+=v[4];
    v[8]+=v[5];
   }
  cur2=SEARCHS(cur,"KEngineers");
  cur3=SEARCH_CND("NumClass",0);
  cur2->hook=cur3;  
  v[6]=VS(cur2,"KNbrEngineers");
  v[7]=VLS(cur2,"KNbrEngineers",1);
  v[3]+=v[6];
  v[8]+=v[7];
 }
cur=SEARCH("Country");
WRITELS(cur,"MovAvNbrWork",v[3],t);
//v[10]=V("AvPrice");
v[10]=1;
WRITELS(cur,"AvPrice",v[10], t-1);
WRITELS(cur,"MovAvPrice",v[10], t-1);
WRITES(cur,"InitAvPrice",v[10]);
//WRITELS(cur,"MinWage",v[10],1);
//v[11]=V("AggProductivity");
v[11]=1;
WRITELS(cur,"MovAvAggProd",v[11], t-1);
WRITELS(cur,"AggProductivity",v[11], t-1);
WRITES(cur,"InitAggProd",v[11]);

V("MinimumInit"); // assign the value for the Minimum parameter to all classes
V("TauInit");
V("ShareInit"); // assign the expenniture shares to the classes above the first one

v[30]=0;
CYCLE(cur, "Need")
 {
  v[30]++;
 }
WRITE("totNeeds",v[30]); // write the total number of needs in the economy (to be used in the product innovation)

PARAMETER;
RESULT(1 )


EQUATION("Init_x")
/*
Initialisation of the charactheristics (quality)
*/

CYCLE(cur, "Supply")
 {
  CYCLES(cur, cur1, "Firm")
   {
    CYCLES(cur1, cur2, "PNeed")
     {
      v[5]=VS(cur2,"IdPNeed");
      if(v[5]==0)
       { // set the initial value of the quality characteristic only for the product that is currently produced. The others are set to 0
        CYCLES(cur2, cur3, "Ch")
         {
          v[1]=VS(cur3,"IdCh");
          if(v[1]>1)
           { // if it is not a price characteristic set the initial value
            v[2]=V("Min_x");
            v[3]=V("Max_x");
            v[4]=UNIFORM(v[2],v[3]);
            WRITELS(cur3,"x",v[4], t-1);
           }
         }
       }

     }

   }

 }

PARAMETER;
RESULT(1 )


EQUATION("InitMarkup")
/*
Set the initial level of mark-up
*/

v[1]=V("Avx"); // average value of the quality characteristic across firms (computed from the initial setting)
v[2]=V("AvMarkup"); // average markup, used as a parameter to change the difference in firms pricing with respect to quality setting
v[8]=v[9]=0;
CYCLE(cur, "Supply")
 {
  CYCLES(cur, cur1, "Firm")
   {
    v[3]=VS(cur1,"AvxFirm");
    v[4]=(v[3]-v[1])/v[3]; // diffrence from the average value of quality
    v[5]=v[2]*v[4];
    v[6]=v[2]+v[5]; // equal change in the markup
    WRITES(cur1,"markup",1+v[6]);
    v[7]=VS(cur1,"markup");
    if(v[7]<1)
     INTERACT("A negative Markup setting. Check equation InitMarkup", v[6]);
    v[8]+=v[7];
    v[9]++;
   }

 }
v[10]=v[8]/v[9]; // check it is the same value of AvMarkup

PARAMETER;
RESULT(v[10] )


EQUATION("AvxFirm")
/*
Average level of quality characteristic in the firm (used to set the initial mark-up), turns into a parameter
*/

v[3]=v[4]=0;
CYCLE(cur, "PNeed")
 {
  CYCLES(cur, cur1, "Ch")
   {
    v[1]=VS(cur1,"IdCh");
    if(v[1]>1)
     {
      v[2]=VS(cur1,"x");
      v[3]+=v[2];
      v[4]++;
     }
   }

 }
v[5]=v[3]/v[4];

PARAMETER;
RESULT(v[5] )


EQUATION("MinimumInit")
/*
Initialisation of the minimum parameter across classes, needs and chars
*/

v[2]=V("MinimumPMinE"); // minimum value of the minimum parameter on price for engineers
v[3]=V("MinimumPMaxE"); // maximum value of the minimum parameter on price for engineers
v[4]=V("MinimumPMinW"); // minimum value of the minimum parameter on price for workers
v[5]=V("MinimumPMaxW"); // maximum value of the minimum parameter on price for workers
v[13]=V("MinimumCMinE"); // minimum value of the minimum parameter on any quality characteristic for engineers
v[14]=V("MinimumCMaxE"); // maximum value of the minimum parameter on any quality characteristic for engineers
v[15]=V("MinimumCMinW"); // minimum value of the minimum parameter on any quality characteristic for workers
v[16]=V("MinimumCMaxW"); // maximum value of the minimum parameter on any quality characteristic for workers
v[6]=1000000;
CYCLE(cur, "Supply")
 {
  CYCLES(cur, cur1, "Firm")
   {
    v[7]=VLS(cur1,"price",1);
    if(v[7]<v[6])
     v[6]=v[7];
   }

 }
v[10]=V("AvWagecoeff");
CYCLE(cur, "Demand")
 {
  CYCLES(cur, cur1, "Class")
   {
    v[1]=VS(cur1,"NumClass");
    if(v[1]<1)
     { // engineers
      CYCLES(cur1, cur2, "Need")
       {
        CYCLES(cur2, cur3, "DCh")
         {
          v[2]=VS(cur3,"IdDCh");
          if(v[2]==1)
           { // maintain the minimum above the minimum of the characteritic (price) only for the fist (price) characteritic
            v[8]=UNIFORM(v[2],v[3]);
            v[9]=VS(cur3,"Delta"); // variance of the distribution of the perceived characteritic around the actaul value
            v[11]=v[9]*v[6];
            v[12]=max(v[8],v[11]); // use either the initially set minimum or the price of the cheapest firm
            WRITES(cur3,"Minimum",v[12]);
           }
          else
           {
            v[17]=UNIFORM(v[13],v[14]);
            WRITES(cur3,"Minimum",v[17]);
           }
         }

       }

     }
    if(v[1]==1)
     {// first tier workers
      CYCLES(cur1, cur2, "Need")
       {
        CYCLES(cur2, cur3, "DCh")
         {
          v[17]=VS(cur3,"IdDCh");
          if(v[17]==1)
           {
            v[18]=UNIFORM(v[4],v[5]);
            v[19]=VS(cur3,"Delta");
            v[20]=v[19]*v[6];
            v[21]=max(v[18],v[20]);
            WRITES(cur3,"Minimum",v[21]);
           }
          else
           {
            v[22]=UNIFORM(v[15],v[16]);
            WRITES(cur3,"Minimum",v[22]);
           }
         }

       }

     }
    if(v[1]>1) 
     { // exectuives, above the first tier workers
      CYCLES(cur1, cur2, "Need")
       {
        CYCLES(cur2, cur3, "DCh")
         {
          v[23]=VS(cur3,"IdDCh");
          if(v[23]==1)
           {
            cur4=SEARCH_CNDS(cur,"NumClass",v[1]-1);
            CYCLES(cur4, cur5, "DCh")
             {
              v[24]=VS(cur5,"IdDCh");
              if(v[24]==1)
               v[25]=VS(cur5,"Minimum");
             }
            v[26]=v[25]*v[10];
            WRITES(cur3,"Minimum",v[26]);
           }
          else
           {
            v[27]=UNIFORM(v[15],v[16]);
            WRITES(cur3,"Minimum",v[27]);
           }
         }

       }

     }
   }
 }

PARAMETER
RESULT(1 )


EQUATION("TauInit")
/*
Set the initial values for the preferences (tolerance toward difference from max char in the market) across classes, needs and characteristics
*/

v[1]=V("tauMinE"); // minimum level of tolerance toward the best product in the market, for engineers
v[2]=V("tauMaxE"); // maximum level of tolerance toward the best product in the market, for engineers
CYCLE(cur, "Demand")
 {
  CYCLES(cur, cur1, "Class")
   {
    v[6]=VS(cur1,"NumClass");
    if(v[6]<1)
     {// engineers
      CYCLES(cur1, cur2, "Need")
       {
        CYCLES(cur2, cur3, "DCh")
         {
          v[7]=UNIFORM(v[1],v[2]);
          WRITES(cur3,"tau",v[7]);
         }

       }

     }
    if(v[6]==1)
     { // first tier workers
      CYCLES(cur1, cur2, "Need")
       {
        CYCLES(cur2, cur3, "DCh")         
        {
          v[3]=VS(cur3,"tauMinW"); // minimum level of tolerance toward the best product in the market, for workers
          v[4]=VS(cur3,"tauMaxW"); // maximum level of tolerance toward the best product in the market, for wokers
          v[8]=UNIFORM(v[3],v[4]);
          if(VS(cur3,"NegativeQuality")>0)
           v[8]=1-v[8];
          WRITES(cur3,"tau",v[8]);
         }

       }

     }
    if(v[6]>1)
     { // exectuives, above first tier
      CYCLES(cur1, cur2, "Need")
       {
        v[9]=VS(cur2,"IdNeed");
        CYCLES(cur2, cur3, "DCh")
         {
          v[10]=VS(cur3,"IdDCh");
          cur4=SEARCH_CNDS(cur,"NumClass",(v[6]-1));
          CYCLES(cur4, cur5, "Need")
           {
            v[11]=VS(cur5,"IdNeed");
            if(v[11]==v[9])
             { // check in the same need
              CYCLES(cur5, cur6, "DCh")
               { // and in the same characteristic
                v[12]=VS(cur6,"IdDCh");
                if(v[12]==v[10]) // if it is in the same characteristic and in the same need, in the previous class, take the value of tau
                 v[13]=VS(cur6,"tau");
               }

             }
           }
          v[5]=VS(cur3,"tauMultiplier"); // the speed of adjustment of the tolerance level from one class to the following one
          v[16]=VS(cur3,"NegativeQuality");
          if(v[16]>0)
           v[15]=V("tauMax"); // the asympthotic level of the tolerance level (1 for qualities and 0 for price)
          if(v[16]<0)
           v[15]=V("tauMin"); // the asympthotic level of the tolerance level (1 for qualities and 0 for price)
          v[14]=v[13]*(1-v[5])+v[5]*v[15]; // adjustment in the treshold level of tolerance
          WRITES(cur3,"tau",v[14]); // finally write the tau for the new consumer class in each of its characteristic for each need
          //INTERACTS(cur3, "tauini", v[13]);
         }

       }

     }
   }

 }

PARAMETER
RESULT(1 )


EQUATION("ShareInit")
/*
Set the initial distribution of shares (in the class above the first tier, engineers and firts tier are set exogenously)
*/

CYCLE(cur, "Demand")
 {
  CYCLES(cur, cur1, "Class")
   {
    v[1]=VS(cur1,"NumClass");
    if(v[1]>1)
     { // if the class is above the first tier of workers, compute the change in the expenditure shares with resepct to the first class
      v[2]=VS(cur1,"ComputeShare");
     }
   }

 }

PARAMETER
RESULT(1 )




/***** SOME STATISTICS ******/

EQUATION("MargPropConsume")
/*
Comment
*/
v[1]=v[2]=v[3]=v[4]=0;
CYCLE(cur, "Class")
 {
  v[5]=VS(cur,"Expenditure");
  v[6]=VLS(cur,"Expenditure",1);
  v[7]=VS(cur,"Income");
  v[8]=VLS(cur,"Income",1);
  v[1]+=v[5];
  v[2]+=v[6];
  v[3]+=v[7];
  v[4]+=v[8];
 }
v[9]=(v[1]-v[2])/(v[3]-v[4]);
RESULT(v[9] )



EQUATION("IncomeDistribution")
/*
Herfindahl Index for the Income
*/
v[2]=0;
v[3]=0;
v[4]=0;
CYCLE(cur, "Class")
 {
  v[5]=VS(cur,"ShareWageIncome");
  v[6]=VS(cur,"SharePremiaIncome");
  v[8]=v[5]+v[6];
  v[2]+=(v[8]*v[8]);
  v[3]+=(v[5]*v[5]);
  v[4]+=(v[6]*v[6]);
 }
WRITE("HerfTotalIncome",v[2]);
WRITE("HerfWageIncome",v[3]);
WRITE("HerfNonWageIncome",v[4]);
v[10]=V("TotWage");
v[11]=V("TotPremia");
v[12]=V("TotIncome");
v[13]=v[10]/v[12];
v[14]=v[11]/v[12];
WRITE("WageIncomeRatio",v[13]);
WRITE("PremiaIncomeRatio",v[14]);
RESULT(1 )

EQUATION("TopBotInc")
/*
Ratio between top class and bottom class income
*/
v[2]=0;
CYCLE(cur, "Class")
 {
  v[1]=VS(cur,"NumClass");
  if(v[1]>v[2])
   {
    v[2]=v[1];  
    v[3]=VS(cur,"Income"); // Top class income
   }
  if(v[1]==2)
   v[4]=VS(cur,"Income"); //Bottom class income
 }
v[5]=v[3]/v[4];

RESULT(v[5])

EQUATION("TopBotEarn")
/*
Ratio between top and bottom class wages
*/

v[2]=0;
CYCLE(cur, "Class")
 {
  v[1]=VS(cur,"NumClass");
  if(v[1]>v[2])
   {
    v[2]=v[1];
    v[3]=VS(cur,"WageIncome"); //Top class earings 
   }
  if(v[2]==2)
   v[4]=VS(cur,"WageIncome"); // Bottom class wage
 }
v[5] = v[3]/v[4];

RESULT(v[5] )


EQUATION("InvHerfIndex")
/*
Inverse Herfindahl Index for firm sales in each sector
*/
v[0]=0;
v[1]=0;
CYCLE(cur, "Firm")
 {
  v[2]=VS(cur,"UnitSales");
  v[0]+=v[2];
 }
WRITE("TotalSales",v[0]);
CYCLE(cur, "Firm")
 {
 v[3]=VS(cur,"UnitSales");
 v[4]=v[3]/v[0];
 v[1]+=(v[4]*v[4]);
 }
v[5]=1/v[1];

RESULT(v[5] )


EQUATION("Andre")
/*
Lorenz curve
*/

v[10]=(double)t;
if(v[10]>1)
 {
  v[5]=v[6]=v[9]=0;
  v[4]=V("TotPremia");
  v[11]=V("TotWage");
  CYCLE(cur, "Demand")
   {
    SORTS(cur,"Class","ShareIncome", "UP");
    CYCLES(cur, cur1, "Class")
     {
      v[1]=VS(cur1,"ShareWageIncome");
      v[2]=VS(cur1,"SharePremiaIncome");
      v[12]=(v[1]*v[11])/(v[4]+v[11]);
      v[13]=(v[2]*v[4])/(v[4]+v[11]);
      v[3]=(v[12]+v[13]);
      WRITES(cur1,"Lorenz",v[3]+v[6]);
      v[6]=v[3]+v[6];
      v[5]+=v[3];
      v[8]=VS(cur1,"Individuals");
      v[9]+=v[8];
      WRITES(cur1,"LorenzInd",v[9]);
     }
    SORTS(cur,"Class","NumClass", "UP");
  
   }
 }

RESULT(v[5] )



EQUATION("TotIndividuals")
/*
Total number of consumers (workers) in t-1
*/

V("ShareIncome");
v[2]=0;
CYCLE(cur, "Demand")
 {
  CYCLES(cur, cur1, "Class")
   {
    v[1]=VS(cur1,"Individuals");
    v[2]+=v[1];
   }

 }

RESULT(v[2] )


EQUATION("AvIncome")
/*
Average income across classes in time t-1
*/

v[5]=(double)t;
if(v[5]>1)
 {
  V("ShareIncome");
  v[1]=V("TotIncome");
  v[3]=V("TotIndividuals");
  v[4]=(v[1])/v[3];
 }
else
 v[4]=1;

RESULT(v[4] )


EQUATION("Atkinson")
/*
Atkinson index of inequality for income in period t-1
A=1-{SUM_i[y_i**(1-e)]**(1/(1-e))}/Av.y
*/

v[16]=(double)t;
if(v[16]>1)
 {
  v[10]=v[20]=v[30]=v[40]=0;
  //v[6]=V("AvIncome");
  v[8]=V("Aversion"); // parmeter for the aversion to inequality (changes which end of the income distribution has a higher weight in the index computation)
  v[11]=V("TotIndividuals");
  v[22]=V("TotIncome");
  CYCLE(cur, "Demand")
   {
    CYCLES(cur, cur1, "Class")
     {
      v[4]=VS(cur1,"Individuals");
      v[5]=VS(cur1,"Income"); 
      if(v[5]>0 && v[4]>0.01)
       {
        v[7]=v[5]/v[4]; // av. income of the class
        v[9]=pow(v[7],(1-v[8]));
        v[17]=v[9]*v[4];
        v[41]=pow(v[5],(1-v[8]));
       }
      else
      	{
       v[17]=0;
       v[41]=0;
       	}
      v[10]+=v[17];
      v[20]+=v[5];
      v[30]+=1;
      v[40]+=v[41];
     }
  
   }
  
  v[12]=v[10]/v[11];
  v[13]=1/(1-v[8]);
  v[14]=pow(v[12],v[13]);
  v[15]=1-v[14]/(v[20]/v[11]);//Atkinson Index for Individual inequality
  v[42]=v[41]/v[30];
  v[43]=pow(v[42],v[13]);
  v[44]=1-v[43]/(v[20]/v[30]);//Atkinson Index for Individual inequality
  WRITE("AtkinsonClass",v[44]);
 }
else
 v[15]=0;

RESULT(v[15] )

EQUATION("Gini")
/*
Gini coefficient in t-1
*/

v[20]=(double)t;
if(v[20]>1)
{
CYCLE(cur, "Demand")
 {
  v[14]=v[21]=0;
  v[2]=VS(cur,"TotWage");
  v[3]=VS(cur,"TotPremia");

  CYCLES(cur, cur1, "Class")
   {
    v[1]=VS(cur1,"ShareIncome");
    if(v[1]>0)
     {
      v[22]=VLS(cur1,"ShareWageIncome",1);
      v[23]=VLS(cur1,"SharePremiaIncome",1);
      v[24]=v[22]*v[2];
      v[25]=v[23]*v[3];
      v[5]=VS(cur1,"Individuals");
      v[6]=(v[24]+v[25]); // total class income of the comparing class
      v[7]=VS(cur1,"NumClass");
      v[21]++;
      CYCLES(cur, cur2, "Class")
       {
        v[8]=VS(cur2,"NumClass");
        if(v[8]!=v[7])
         { // if it is a different class 
          v[9]=VS(cur2,"ShareIncome");
          if(v[9]>0)
           {
            v[26]=VLS(cur2,"ShareWageIncome",1);
            v[27]=VLS(cur2,"SharePremiaIncome",1);
            v[28]=v[26]*v[2];
            v[29]=v[27]*v[3];
            v[11]=VS(cur2,"Individuals");
            v[12]=(v[28]+v[29]); // total class income of the compared class
            v[13]=abs(v[6]-v[12]);
            v[14]+=v[13];
           }
         }
       }

     }
   }

 }
v[15]=V("AvIncome");
v[30]=(v[2]+v[3])/v[21]; // average income per class
v[16]=V("TotIndividuals");
v[17]=pow(v[21],2);
v[18]=2*v[17]*v[30];
v[19]=v[14]/v[18];
}
else
 v[19]=0;

RESULT(v[19] )


EQUATION("GdpNominal")
/*
Nominal GDP, at varying prices
*/

v[4]=0;
v[8]=v[9]=v[10]=0;
CYCLE(cur, "Supply")
 {
  CYCLES(cur, cur1, "Firm")
   {
    v[1]=VS(cur1,"Revenues");
    v[4]+=v[1];
    v[5]=VS(cur1,"UnitSales");
    v[6]=VS(cur1,"ConstPrice");
    v[7]=v[5]*v[6];
    v[9]+=v[7];
   }

 }
CYCLE(cur, "Machinery")
 {
  CYCLES(cur, cur1, "KFirm")
   {
    v[5]=VS(cur1,"KProductionFlow");
    v[6]=VS(cur1,"KPrice");
    v[11]=VS(cur1,"KConstPrice");    
    v[8]+=v[6]*v[5];
    v[10]+=v[11]*v[5];
   }

 }
v[11]=v[9]+v[10];
v[12]=v[4]+v[8];
if(v[12]==0)
	v[13]=0;
else
	v[13]=1/v[12];
v[14]=v[4]*v[13];
v[15]=v[8]*v[13];
WRITE("GdpConstant",v[11]);
WRITE("GdpConstantF",v[9]);
WRITE("GdpConstantK",v[10]);
WRITE("ConsumptionGdpRatio",v[14]);
WRITE("InvestmentGdpRatio",v[15]);
RESULT(v[12] )



EQUATION("GdpConstantK")
/*
GDP at constant prices in the capital sector
*/

v[1]=V("IndexYear");
v[2]=(double)t;
if(v[1]<v[2])
 {
  v[8]=0;
  CYCLE(cur, "Machinery")
   {
    CYCLES(cur, cur1, "KFirm")
     {
      v[9]=VS(cur1,"KProductionFlow");
      v[10]=VS(cur1,"KConstPrice");
      v[11]=v[9]*v[10];
      v[8]+=v[11];
     }

   }

 }
else
 v[8]=V("GdpNominal"); 

RESULT(v[8] )


EQUATION("GdpNGrowth")
/*
Growth rate of the nominal GDP
*/

v[4]=(double)t;
if(v[4]>2)
 {
  v[1]=VL("GdpNominal",1);
  v[2]=V("GdpNominal");
  v[3]=(v[2]/v[1])-1;
 }
else
 v[3]=0;

RESULT(v[3] )


EQUATION("GdpCGrowth")
/*
Growth rate of the GDP at constant prices
*/

v[4]=(double)t;
if(v[4]>2)
 {
  v[1]=VL("GdpConstant",1);
  v[2]=V("GdpConstant");
  v[3]=(v[2]/v[1])-1;
 }
else
 v[3]=0;

RESULT(v[3] )


EQUATION("AvProfit")
/*
Average profits across firms
*/

v[2]=v[3]=0;
CYCLE(cur, "Supply")
 {
  CYCLES(cur, cur1, "Firm")
   {
    v[1]=VS(cur1,"Profit");
    v[2]+=v[1];
    v[3]++;
   }

 }
v[4]=v[2]/v[3];

RESULT(v[4] )


EQUATION("AvKProfit")
/*
Average of the capital firms profits
*/

v[2]=v[3]=0;
CYCLE(cur, "Machinery")
 {
  CYCLES(cur, cur1, "KFirm")
   {
    v[1]=VS(cur1,"KProfit");
    v[2]+=v[1];
    v[3]++;
   }

 }
v[4]=v[2]/v[3];

RESULT(v[4] )

EQUATION("Avx")
/*
Average value of the quality characteristic (non price characteristic that is not price)

Compute also all the statistics for sectors.
*/

v[3]=v[4]=0;

CYCLE(cur, "Sectors")
 {
  VS(cur,"Demography");
  WRITES(cur,"AvxS",0);
  WRITES(cur,"SUnitSales",0);
  WRITES(cur,"SQ",0); 
  WRITES(cur,"SRevenues",0);   
  WRITES(cur,"SProfits",0);
  WRITES(cur,"SNumFirms",0); 
  WRITES(cur,"SMonetarySales",0); 
  WRITES(cur,"Sapp",VS(cur,"maxXS"));
  WRITES(cur,"maxXS",0); 
  WRITES(cur,"AvpS",0);   
  WRITES(cur,"SInvHerf",0);   
  WRITES(cur,"SAvHealth",0);
  WRITES(cur,"SAvStock",0); 
  WRITES(cur,"SAvBacklog",0); 
  WRITES(cur,"SKProductivity",0); 
  WRITES(cur,"SULC",0); 
  WRITES(cur,"SnumBLI",0);      
  WRITES(cur,"SNetWorth",0);      
  WRITES(cur,"SAvAge",0); 
  WRITES(cur,"SBacklogShare",0);

 }

CYCLE(cur, "Supply")
 {
  CYCLES(cur, cur1, "Firm")
   {
    INCRS(cur1->hook->up, "SBacklogShare", VS(cur1,"backlog"));
    INCRS(cur1->hook->up, "SAvAge", VS(cur1,"Age"));
    INCRS(cur1->hook->up, "SUnitSales", VS(cur1,"UnitSales"));
    INCRS(cur1->hook->up, "SnumBLI", VS(cur1,"numBLI"));
    INCRS(cur1->hook->up, "SNetWorth", VS(cur1,"NetWorth"));
    INCRS(cur1->hook->up, "SQ", v[30]=VS(cur1,"Q"));
    INCRS(cur1->hook->up, "SRevenues", VS(cur1,"Revenues"));
    INCRS(cur1->hook->up, "AvpS", v[30]*VS(cur1,"price"));
    INCRS(cur1->hook->up, "SProfits", VS(cur1,"Profit"));
    INCRS(cur1->hook->up, "SMonetarySales", VS(cur1,"MonetarySales"));
    INCRS(cur1->hook->up, "SNumFirms", 1);    
    WRITES(cur1,"MsSector",VS(cur1, "MonetarySales"));
    INCRS(cur1->hook->up, "SKProductivity", VS(cur1,"MaxLaborProductivity")*v[30]); 
    INCRS(cur1->hook->up, "SULC", VS(cur1,"UnitLaborCost")*v[30]);            

    CYCLES(cur1, cur2, "PNeed")
     {
      CYCLES(cur2, cur3, "Ch")
       {
        v[1]=VS(cur3,"IdCh");
        if(v[1]>1)
         { // to make it simple i assume that the first charateristic is price, this should be generalised
          v[2]=VS(cur3,"x");
          INCRS(cur1->hook->up, "AvxS", v[2]*v[30]);
          v[5]=VS(cur1->hook->up,"maxXS");
          if(v[2]>v[5])
           WRITES(cur1->hook->up,"maxXS",v[2]);
          v[3]+=v[2];
          v[4]++;
         }
       }

     }

   }

 }

v[40]=v[41]=0;
CYCLE(cur1, "Firm")
 {
  v[20]=VS(cur1->hook->up,"SMonetarySales");
  v[21]=VS(cur1,"MsSector");
  
  if(v[20]>0)
   v[23]=v[21]/v[20];
  else
   v[23]=0; 
  WRITES(cur1,"MsSector",v[23]);
  INCRS(cur1->hook->up,"SAvHealth",v[23]*VS(cur1,"Health"));
  INCRS(cur1->hook->up,"SInvHerf",v[23]*v[23]);
  INCRS(cur1->hook->up,"SAvStock",v[23]*VS(cur1,"Stocks"));
  INCRS(cur1->hook->up,"SAvBacklog",v[23]*VS(cur1,"backlog"));  
  v[42]=VS(cur1,"AvWage");
  v[43]=VS(cur1,"LaborForce");
  v[40]+=v[42]*v[43];
  v[41]+=v[43];
 }
if(t>1)
 v[44]=V("TotAvWage");
else
 v[44]=v[40]/v[41]; 
WRITE("TotAvWage",v[40]/v[41]);
v[45]=(v[40]/v[41]-v[44])/v[44];
WRITE("WageGrowth",v[45]);

CYCLE(cur1, "Sectors")
 {
  v[27]=VS(cur1,"SInvHerf");
  if(v[27]>0)
   WRITES(cur1,"SInvHerf",1/v[27]); 
  v[30]=VS(cur1,"SQ");
  if(v[30]>0)
   {
    MULTS(cur1,"AvxS",1/v[30]);
    MULTS(cur1,"SKProductivity",1/v[30]); 
    MULTS(cur1,"SULC",1/v[30]);        
    MULTS(cur1,"AvpS",1/v[30]); 
    MULTS(cur1,"SBacklogShare",1/v[30]);     
           
   }
  else
   {
    WRITES(cur1,"maxXS",VS(cur1,"Sapp"));
   } 
  v[17]=VS(cur1,"SNumFirms");
  if(v[17]>0)
   {
    MULTS(cur1,"SnumBLI",1/v[17]);
    MULTS(cur1,"SNetWorth",1/v[17]);
    MULTS(cur1,"SAvAge",1/v[17]);    
   }  
 }

v[5]=v[3]/v[4];

RESULT(v[5] )

EQUATION("Dtau")
/*
Difference between the minimum and the maximum asymptoti level of tau: gives the theoretical heterogeneity in consuemrs preferces, while the standard deviaiton depends on how many tiers develop through time (as it is affected by the speed of change)
*/

v[1]=V("tauMax");
v[2]=V("tauMin");
v[3]=v[1]-v[2];

RESULT(v[3] )


EQUATION("HerfIndexS")
/*
Herfindahl indexin each sector
*/

v[4]=v[8]=0;
v[1]=V("IdGood");
CYCLES(p->up, cur, "Supply")
 {
  CYCLES(cur, cur1, "Firm")
   {
    v[2]=VS(cur1,"product");
    if(v[2]==v[1])
     {
      v[3]=VS(cur1,"UnitSales");
      v[4]+=v[3];
     }
   }

 } 
WRITE("TotSalesS",v[4]);
CYCLES(p->up, cur, "Supply")
 {
  CYCLES(cur, cur1, "Firm")
   {
    v[5]=VS(cur1,"product");
    if(v[5]==v[1])
     {
      v[6]=VS(cur1,"UnitSales");
      v[7]=v[6]/v[4];
      WRITES(cur1,"MsSector",v[7]);
      v[8]+=(v[7]*v[7]);
     }
   }

 }

RESULT(v[8] )

EQUATION("SectEmployment")
/*
Herfindahl indexin each sector
*/

v[0]=v[10]=v[20]=v[30]=0;
v[1]=V("IdGood");
v[4]=V("NbrWorkers");
v[14]=V("GdpNominal");
v[24]=V("GdpConstant");
  CYCLE( cur1, "sFirm")
   {
      v[3]=VS(cur1->hook,"NumWorkers");
      v[11]=VS(cur1->hook,"Revenues");
      v[21]=VS(cur1->hook,"UnitSales");
      v[22]=VS(cur1->hook,"ConstPrice");
      v[31]=VS(cur1->hook,"CapitalStock");
      v[23]=v[21]*v[22];
      v[0]+=v[3];
      v[10]+=v[11];
      v[20]+=v[23];
      v[30]+=v[31];
   }

v[7]=v[0]/v[4];
v[17]=v[10]/v[14];
v[27]=v[20]/v[24];
WRITE("SectEmploymentShare",v[7]);
WRITE("SectSales",v[10]);
WRITE("SectNomGDPShare",v[17]);
WRITE("SectOutputShare",v[27]);
v[32]=v[30]/v[0];
WRITE("SectMechanisation",v[32]);
RESULT(v[0] )

EQUATION("ConsumptionConcentration")
/*
Comment
*/
v[0]=v[10]=0;
CYCLE(cur, "Sectors")
 {
 v[1]=VS(cur,"SectSales");
 v[0]+=v[1]; 
 }
v[2]=v[0];
CYCLE(cur, "Sectors")
 {
 v[1]=VS(cur,"SectSales");
 if(v[2]==0)
 	v[3]=0;
 else
	 v[3]=v[1]/v[2];
 WRITES(cur,"SectSalesShare",v[3]); 
 v[10]+=(v[3]*v[3]);
 }
if(v[10]==0)
	v[11]=0;
else 
	v[11]=1/v[10];
RESULT(v[11] )

EQUATION("Mechanisation")
/*
Comment
*/
v[0]=0;
CYCLE(cur, "Sectors")
 {
 v[1]=VS(cur,"SectMechanisation");
 v[2]=VS(cur,"SectEmploymentShare");
 v[0]+=(v[1]*v[2]); 
 }
v[3]=v[0];
RESULT(v[3] )

EQUATION("EmploymentConcentration")
/*
Comment
*/
v[0]=v[3]=0;
v[4]=V("NbrWorkers");
v[10]=v[11]=v[12]=0;
v[13]=V("GdpNominal");
v[20]=v[21]=v[22]=0;
v[23]=V("GdpConstant");
CYCLE(cur, "Sectors")
 {
 	v[5]=VS(cur,"SectEmploymentShare");
 	v[0]+=(v[5]*v[5]);
 	v[14]=VS(cur,"SectNomGDPShare");
 	if(v[14]==0)
		v[19]=0;
	else
		v[19]=v[14]*v[14];
	v[10]+=v[19];
 	v[24]=VS(cur,"SectOutputShare");
 	if(v[24]==0)
		v[29]=0;
	else
		v[29]=v[24]*v[24];
	v[20]+=v[29];
}

CYCLE(cur, "Machinery")
 {
 	v[1]=v[2]=0;
  CYCLES(cur, cur1, "KFirm")
   {
    CYCLES(cur1, cur2, "KLabor")
     {
      v[6]=VS(cur2,"KNbrWorkers");
      v[2]+=v[6];
     }
		CYCLES(cur1, cur2, "KEngineers")
 		{
  		v[7]=VS(cur2,"KNbrEngineers");
      v[1]+=v[7];	
 		}
	  v[15]=VS(cur1,"KProductionFlow"); 
    v[16]=VS(cur1,"KPrice");
    v[11]+=(v[15]*v[16]); 
    v[26]=VS(cur1,"KConstPrice");
    v[21]+=v[15]; 
   }
	v[8]=v[1]+v[2];
	v[9]=v[8]/v[4];
	v[3]+=(v[9]*v[9]);
	WRITES(cur,"KEmployment",v[8]);
	WRITES(cur,"KEmploymentShare",v[9]);
	v[17]=v[11]/v[13];
	if(v[11]==0)
		v[18]=0;
	else
		v[18]=v[17]*v[17];
	v[12]+=v[18];
	WRITES(cur,"KNomGDPShare",v[17]);
		v[27]=v[21]/v[23];
	if(v[21]==0)
		v[28]=0;
	else
		v[28]=v[27]*v[27];
	v[22]+=v[28];
	WRITES(cur,"KOutputShare",v[27]);
 }
v[99]=1/(v[3]+v[0]);
v[98]=v[12]+v[10];
if(v[98]==0)
	v[97]=0;
else
	v[97]=1/v[98];
WRITE("GDPConcentration",v[97]);
v[96]=v[22]+v[20];
if(v[96]==0)
	v[95]=0;
else
	v[95]=1/v[96];
WRITE("OutputConcentration",v[95]);
RESULT(v[99])




EQUATION("FrontierX")
/*
Imitable value of X for new entrants
*/

V("AvxS");
v[0]=V("maxXS");

v[1]=V("aFrontierX");

v[2]=v[1]*CURRENT+(1-v[1])*v[0];
RESULT(v[2] )



EQUATION("Dump")
/*
Dumping data for inequality study
*/

if(t!=100 && t%1000!=0)
 END_EQUATION(0)
sprintf(msg, "data%d.txt", t);
f=fopen(msg, "w");

fprintf(f,"Num\tIncome\tCons\tWealth\n");
cur2=SEARCH("Supply");

CYCLES(cur2, cur, "Firm")
 {
  CYCLES(cur, cur1, "Labor")
   {
    v[0]=VS(cur1,"NumWorkers");
    v[1]=VS(cur1,"wage");
    v[2]=V("Premia");
    v[3]=VS(cur1->hook,"Consumption");
    v[4]=VS(cur1->hook,"Individuals");
    v[5]=VS(cur1->hook,"BalanceC");
    fprintf(f, "%lf\t%lf\t%lf\t%lf\n", v[0],v[1]+v[2],(v[3]/v[4])*v[0],(v[5]/v[4])*v[0]);
   }

 }

fclose(f);
RESULT(1 )


/********************************************************************************************

STATISTICS OVER THE RANDOM RUNS

********************************************************************************************/

EQUATION("Init_Init")
/*
Requires first to initialise all the simulation runs
*/
CYCLE(cur, "Country")
 {
  VS(cur,"Init");
 }
PARAMETER
RESULT(1 )


// AVERAGES



EQUATION("Av_Computations")
/*
Activate the equation for income distribution HI, Lorenz curve
*/

CYCLE(cur, "Country")
 {VS(cur,"AvProfit");
  VS(cur,"Trade");
  VS(cur,"Production");
  cur1=SEARCHS(cur,"Demand");
  VS(cur1,"IncomeDistribution");
  VS(cur,"Andre");
 }

RESULT(1 )



EQUATION("TestFinance")
/*
Comment
*/
v[0]=v[1]=v[2]=v[3]=v[4]=v[5]=0;

CYCLE(cur, "Firm")
 {
  v[0]+=VS(cur,"DebtF");
  v[1]+=VS(cur,"BalanceF");
  
 }
CYCLE(cur, "KFirm")
 {
  v[0]+=VS(cur,"DebtK");
  v[1]+=VS(cur,"BalanceK");
 }

v[10]=V("TotalCapital");



RESULT(1 )



MODELEND



void close_sim(void)
{

}




/*

Notes Tum

- Why does the order of preference make the differences the way in whch the code is now? firms below the threshold are thrown cycling through any of the carachteristic. Got it, we actually need to define a truncation of the characteristics browsing to allow for the choice of specific goods.

- We need to insert a random order of characteritic in some INIT equation

- In the TTB we do not allow for a distribution of needs in a class: acounted for by the 'share' parameter, exogenous
*/


/*

Notes, just to not forget:

- ProdCost should be smoothed in order to avoid sudden movements of prices

- Among the characteristics of product should be included: visibility (i.e. smoothed market shares+marketing) and price (to be inverted, probably quite a mess).



*/





