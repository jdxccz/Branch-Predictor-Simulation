//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <string.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
int gshistoryBits;
int lcounter,rcounter;

struct selector s1,s2;
struct gshare gs;
struct tournament tm;
struct perceptrons pt;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

void init_selector(struct selector *s,int gBits){
  s->history = 0;
  s->CHT = (uint8_t*)malloc( (1 << gBits) *sizeof(uint8_t));
  memset(s->CHT, 0, (1 << gBits) * sizeof(uint8_t));
  s->lcounter = 0;
  s->rcounter = 0;
  s->mask = (1 << gBits) - 1;
}

uint8_t pred_selector(struct selector *s,uint8_t Lout,uint8_t Rout){
  uint32_t index = s->history & s->mask;
  if (s->CHT[index] < 2) s->lcounter += 1; else s->rcounter += 1;
  return s->CHT[index] < 2 ? Lout : Rout;
}

void train_selector(struct selector *s,uint8_t Lout,uint8_t Rout, uint8_t outcome){
  uint32_t index = s->history & s->mask;
  if (Lout == TAKEN && Rout == NOTTAKEN && outcome == TAKEN && s->CHT[index] != 0) s->CHT[index] -= 1;
  if (Lout == TAKEN && Rout == NOTTAKEN && outcome == NOTTAKEN && s->CHT[index] != 3) s->CHT[index] += 1;
  if (Lout == NOTTAKEN && Rout == TAKEN && outcome == NOTTAKEN && s->CHT[index] != 0) s->CHT[index] -= 0;
  if (Lout == NOTTAKEN && Rout == TAKEN && outcome == TAKEN && s->CHT[index] != 3) s->CHT[index] += 1;
  s->history <<= 1;
  s->history |= outcome;
}

void init_gshare(struct gshare *gs,int gBits){
  gs->history = 0;
  gs->mask = (1 << gBits) - 1;
  gs->gBHT = (uint8_t*)malloc((1 << gBits) *sizeof(uint8_t));
  memset(gs->gBHT, 0, (1 << gBits) * sizeof(uint8_t));
}

uint8_t pred_gshare(struct gshare *gs, uint32_t pc){
  uint32_t index = (pc ^ gs->history) & (gs->mask);
  uint8_t choice = gs->gBHT[index];
  switch (choice)
  {
  case SN:
    return NOTTAKEN;
  case WN:
    return NOTTAKEN;
  case WT:
    return TAKEN;
  case ST:
    return TAKEN;
  default:
    printf("wrong counter type!\n pred:%d\n",choice);
    return -1;
  }
}

void train_gshare(struct gshare *gs, uint32_t pc, uint8_t outcome){
  uint32_t index = (pc ^ gs->history) & (gs->mask);
  uint8_t choice = gs->gBHT[index];
  if(outcome == TAKEN && choice != ST){gs->gBHT[index] += 1;}
  if(outcome == NOTTAKEN && choice != SN){gs->gBHT[index] -= 1;}
  gs->history <<= 1;
  gs->history |= outcome;
}

void init_global(struct global *g,int gBits){
  g->history = 0;
  g->mask = (1 << gBits) - 1;
  g->gBHT = (uint8_t*)malloc((1 << gBits) *sizeof(uint8_t));
  memset(g->gBHT, 0, (1 << gBits) * sizeof(uint8_t));
}

uint8_t pred_global(struct global *g, uint32_t pc){
  uint32_t index = (g->history) & (g->mask);
  uint8_t choice = g->gBHT[index];
  uint8_t result = choice >1 ? TAKEN : NOTTAKEN;
  return result;
}

void train_global(struct global *g, uint8_t outcome){
  uint32_t index = (g->history) & (g->mask);
  uint8_t choice = g->gBHT[index];
  if(outcome == TAKEN && choice != ST){g->gBHT[index] += 1;}
  if(outcome == NOTTAKEN && choice != SN){g->gBHT[index] -= 1;}
  g->history <<= 1;
  g->history |= outcome;
}

void init_local(struct local *l,int pcBits,int lBits){
  l->pcmask = (1 << pcBits) - 1;
  l->lmask = (1 << lBits) - 1;
  l->lBHT = (uint16_t*)malloc( (1 << pcBits) *sizeof(uint16_t));
  l->lPT = (uint8_t*)malloc((1 << lBits) * sizeof(uint8_t));
  memset(l->lBHT, 0, (1 << pcBits)  * sizeof(uint16_t));
  memset(l->lPT, 0, (1 << lBits) * sizeof(uint8_t));
}

uint8_t pred_local(struct local *l,uint32_t pc){
  uint32_t pcindex = l->pcmask & pc;
  uint16_t lindex = l->lmask & l->lBHT[pcindex];
  uint8_t choice = l->lPT[lindex];
  uint8_t result = choice >1 ? TAKEN : NOTTAKEN;
  return result;
}

void train_local(struct local *l,uint32_t pc, uint8_t outcome){
  uint32_t pcindex = l->pcmask & pc;
  uint16_t lindex = l->lmask & l->lBHT[pcindex];
  if(outcome == TAKEN && l->lPT[lindex] != ST){l->lPT[lindex] += 1;}
  if(outcome == NOTTAKEN && l->lPT[lindex] != SN){l->lPT[lindex] -= 1;}
  l->lBHT[pcindex] <<= 1;
  l->lBHT[pcindex] |= outcome;
}

void init_tournament(struct tournament *tm,int gBits,int pcBits,int lBits){
  init_global(&(tm->G),gBits);
  init_local(&(tm->L),pcBits,lBits);
  init_selector(&(tm->S),gBits);
}

uint8_t pred_tournament(struct tournament *tm,uint32_t pc){
  uint8_t gpred,lpred;
  gpred = pred_global(&(tm->G),pc);
  lpred = pred_local(&(tm->L),pc);
  return pred_selector(&(tm->S),gpred,lpred);
}

void train_tournament(struct tournament *tm,uint32_t pc, uint8_t outcome){
  uint8_t gpred,lpred;
  gpred = pred_global(&(tm->G),pc);
  lpred = pred_local(&(tm->L),pc);
  train_global(&(tm->G),outcome);
  train_local(&(tm->L),pc,outcome);
  train_selector(&(tm->S),gpred,lpred,outcome);
}

void init_perceptrons(struct perceptrons *pt,int pcBits,int weightnum,int theta,int bound){
  pt->history = 0;
  pt->mask = (1 << pcBits) - 1;
  pt->PLT = (int8_t*)malloc((1 << pcBits) * weightnum *sizeof(int8_t));
  memset(pt->PLT, 0, (1 << pcBits) * weightnum *sizeof(int8_t));
  pt->weightnum =weightnum;
  pt->theta =theta;
  pt->bound =bound;
}

uint8_t pred_perceptrons(struct perceptrons *pt,uint32_t pc){
  int pcindex = (int)pc & pt->mask;
  int result = 0;
  int8_t w;
  for(int i = 0;i<pt->weightnum;i++){
    w = pt->PLT[pcindex * pt->weightnum + i];
    if (((1 << i) & pt->history) != 0) {
      result += w;
    }
    else {
      result -= w;
    }
  }
  if (result >= 0) return TAKEN;
  else return NOTTAKEN;
}

void train_perceptrons(struct perceptrons *pt,uint32_t pc, uint8_t outcome) {
  int pcindex = (int)pc & pt->mask;
  int result = 0;
  int8_t w;
  for(int i = 0;i < pt->weightnum;i++){
    w = pt->PLT[pcindex * pt->weightnum + i];
    if (((1 << i) & pt->history) != 0) {
      result += w;
    }
    else {
      result -= w;
    }
  }
  if (result >= -(pt->theta) && outcome == NOTTAKEN){
    for(int i = 0;i<pt->weightnum;i++){
      if (((1 << i) & pt->history) != 0) {
        if (pt->PLT[pcindex * (pt->weightnum) + i] > -(pt->bound))pt->PLT[pcindex * (pt->weightnum) + i] -= 1;
      }
      else {
        if (pt->PLT[pcindex * (pt->weightnum) + i] < pt->bound-1)pt->PLT[pcindex * (pt->weightnum) + i] += 1;
      }
    }
  }
  else if (result <= pt->theta && outcome == TAKEN)
  {
    for(int i = 0;i<pt->weightnum;i++){
      if (((1 << i) & pt->history) != 0) {
        if (pt->PLT[pcindex * (pt->weightnum) + i] < pt->bound - 1)pt->PLT[pcindex * (pt->weightnum) + i] += 1;
      }
      else{
        if (pt->PLT[pcindex * (pt->weightnum) + i] > -(pt->bound))pt->PLT[pcindex * (pt->weightnum) + i] -= 1;
      }
    }
  }
  
  pt->history <<= 1;
  pt->history |= outcome;
}

void init_custom(){
  ghistoryBits = 9; 
  lhistoryBits = 10; 
  pcIndexBits = 10;
  gshistoryBits = 13;

  int cusweight = 12;
  int cuspcIndexBits = 8;
  int theta = 8;
  int bound = 10;
  init_tournament(&tm,ghistoryBits,pcIndexBits,lhistoryBits);
  init_gshare(&gs,gshistoryBits);
  init_perceptrons(&pt,cuspcIndexBits,cusweight,theta,bound);
  init_selector(&s1,ghistoryBits);
  init_selector(&s2,ghistoryBits);
}

uint8_t pred_custom(uint32_t pc){
  uint8_t tpred = pred_tournament(&tm,pc);
  uint8_t gpred = pred_gshare(&gs,pc);
  uint8_t ppred = pred_perceptrons(&pt,pc);
  uint8_t tgpred = pred_selector(&s2,tpred,gpred);
  return pred_selector(&s1,ppred,tgpred);
}

void train_custom(uint32_t pc, uint8_t outcome){
  uint8_t tpred = pred_tournament(&tm,pc);
  uint8_t gpred = pred_gshare(&gs,pc);
  uint8_t ppred = pred_perceptrons(&pt,pc);
  uint8_t tgpred = pred_selector(&s2,tpred,gpred);
  train_tournament(&tm,pc,outcome);
  train_gshare(&gs,pc,outcome);
  train_perceptrons(&pt,pc,outcome);
  train_selector(&s2,tpred,gpred,outcome);
  train_selector(&s1,ppred,tgpred,outcome);
}

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      init_gshare(&gs,gshistoryBits);
      break;
    case TOURNAMENT:
      init_tournament(&tm,ghistoryBits,pcIndexBits,lhistoryBits);
      break;
    case CUSTOM:
      init_custom();
      break;
    default:
      printf("wrong bytype!\n");
      break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return pred_gshare(&gs,pc);
    case TOURNAMENT:
      return pred_tournament(&tm,pc);
    case CUSTOM:
      return pred_custom(pc);
    default:
      printf("wrong bytype!\n");
      return -1;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      train_gshare(&gs,pc,outcome);
      break;
    case TOURNAMENT:
      train_tournament(&tm,pc,outcome);
      break;
    case CUSTOM:
      train_custom(pc,outcome);
      break;
    default:
      printf("wrong bytype!\n");
      break;
  }
}
