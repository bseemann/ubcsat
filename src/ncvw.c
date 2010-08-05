/*

      ##  ##  #####    #####   $$$$$   $$$$   $$$$$$    
      ##  ##  ##  ##  ##      $$      $$  $$    $$      
      ##  ##  #####   ##       $$$$   $$$$$$    $$      
      ##  ##  ##  ##  ##          $$  $$  $$    $$      
       ####   #####    #####  $$$$$   $$  $$    $$      
  ======================================================
  SLS SAT Solver from The University of British Columbia
  ======================================================
  ...Developed by Dave Tompkins (davet [@] cs.ubc.ca)...
  ------------------------------------------------------
  .......consult legal.txt for legal information........
  ......consult revisions.txt for revision history......
  ------------------------------------------------------
  ... project website: http://www.satlib.org/ubcsat ....
  ------------------------------------------------------
  .....e-mail ubcsat-help [@] cs.ubc.ca for support.....
  ------------------------------------------------------

*/

#include "ubcsat.h"

void PickNCVW();
void InitNCVW();
void PostFlipNCVW();

FLOAT fNcvwGamma;
FLOAT fNcvwDelta;
FLOAT fNcvwPi;

UINT32 iSelectedAlg;

void AddNCVW() {

  ALGORITHM *pCurAlg;

  pCurAlg = CreateAlgorithm("ncvw","",FALSE,
   "NCVWr: Switch between non (adaptG2WSAT*) clause (RSAPS) and variable (VW) weighting (paper version)",
    "Wei, Li, Zhang  [CP 08]",
    "PickNCVW,InitNCVW,InitHybrid1,InitRSAPS,PostFlipNCVW",
    "DefaultProcedures,Flip+TrackChanges+FCL,DecPromVars,FalseClauseList,VarLastChange,VW2Weights,MakeBreakPenaltyFL,VarInFalse",
    "default","default");

  AddParmFloat(&pCurAlg->parmList,"-gamma","NCVW switching criteria [default %s]","paramater to adjust selecting VW over others~use VW if max.vw2w > gamma * avg.vw2w","",&fNcvwGamma,7.5);
  AddParmFloat(&pCurAlg->parmList,"-delta","NCVW switching criteria [default %s]","use RSAPS if max.cw >= delta * avg.cw ","",&fNcvwDelta,3.0f);
  AddParmFloat(&pCurAlg->parmList,"-pi","NCVW switching criteria [default %s]","use RSAPS if avg.cw <= pi","",&fNcvwPi,15.0f);
  // No VW smoothing parameter for now -- ZERO
  AddParmProbability(&pCurAlg->parmList,"-wp","RSAPS walk probability [default %s]","within RSAPS, when a local minimum is encountered,~flip a random variable with probability PR","",&iWp,0.05);
  AddParmFloat(&pCurAlg->parmList,"-alpha","RSAPS scaling parameter alpha [default %s]","when a local minimum is encountered,~multiply all unsatisfied cluase penalties by FL","",&fAlpha,1.3f);
  AddParmFloat(&pCurAlg->parmList,"-rho","RSAPS smoothing parameter rho [default %s]","when smoothing occurs, smooth penalties by a factor of FL","",&fRho,0.8f);

  CreateTrigger("PickNCVW",ChooseCandidate,PickNCVW,"","");
  CreateTrigger("InitNCVW",InitStateInfo,InitNCVW,"","");
  CreateTrigger("PostFlipNCVW",PostFlip,PostFlipNCVW,"","PostFlipRSAPS");


  pCurAlg = CreateAlgorithm("ncvw","2009",FALSE,
   "NCVWr: NCVW 2009 SAT competition version",
    "Wei, Li, Zhang  [CP 08]",
    "PickNCVW,InitNCVW,InitVW2Auto,UpdateVW2Auto,InitRSAPS,PostFlipNCVW",
    "DefaultProcedures,Flip+TrackChanges+FCL,DecPromVars,FalseClauseList,VarLastChange,VW2Weights,MakeBreakPenaltyFL,VarInFalse",
    "default","default");

  AddParmFloat(&pCurAlg->parmList,"-gamma","NCVW switching criteria [default %s]","paramater to adjust selecting VW over others~use VW if max.vw2w > gamma * avg.vw2w","",&fNcvwGamma,1.0122);
  AddParmFloat(&pCurAlg->parmList,"-delta","NCVW switching criteria [default %s]","use RSAPS if max.cw >= delta * avg.cw ","",&fNcvwDelta,2.75f);
  AddParmFloat(&pCurAlg->parmList,"-pi","NCVW switching criteria [default %s]","use RSAPS if avg.cw <= pi","",&fNcvwPi,15.0f);
  // No VW smoothing parameter for now -- randomized
  AddParmProbability(&pCurAlg->parmList,"-wp","RSAPS walk probability [default %s]","within RSAPS, when a local minimum is encountered,~flip a random variable with probability PR","",&iWp,0.05);
  AddParmFloat(&pCurAlg->parmList,"-alpha","RSAPS scaling parameter alpha [default %s]","when a local minimum is encountered,~multiply all unsatisfied cluase penalties by FL","",&fAlpha,1.3f);
  AddParmFloat(&pCurAlg->parmList,"-rho","RSAPS smoothing parameter rho [default %s]","when smoothing occurs, smooth penalties by a factor of FL","",&fRho,0.8f);

}

void InitNCVW() {
  InitHybridInfo();
  fPenaltyImprove = -1.0e-01f;
}

void PickNCVW() {
  FLOAT fAveragePenalty = fTotalPenaltyFL / (FLOAT) iNumClauses;
  if (fVW2WeightMax >= fNcvwGamma * fVW2WeightMean) {
    PickVW2Auto();
    iMultiAlgCurrent = 3;
  } else {
    fAveragePenalty = fTotalPenaltyFL / (FLOAT) iNumClauses;
    if ((fAveragePenalty <= fNcvwPi) || (fMaxPenaltyFL >= fNcvwDelta * fAveragePenalty)) {
      PickSAPS();
      iMultiAlgCurrent = 4;
    } else {
      PickG2WSatNoveltyPlusOldest();
    }
  }
}

void PostFlipNCVW() {
  if (iFlipCandidate) {
    /* as per original NCVW implementation, only perform updates for non-null flips */
    UpdateHybridInfo();
    UpdateVW2Auto();
    AdaptG2WSatNoise();
  }
  if (iMultiAlgCurrent == 4) {
    PostFlipRSAPS();
  }
}
