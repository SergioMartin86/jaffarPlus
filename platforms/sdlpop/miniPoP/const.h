/*
MiniPop, a barebones thread-safe version of SDLPop for routing
Copyright (C) 2021 Sergio Martin

based on

SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2020  Dávid Nagy

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

The authors of this program may be contacted at https://forum.princed.org
*/


#include "common.h"

struct directory_listing_type
{
  DIR *dp;
  char *found_filename;
  const char *extension;
};

#define VGA_PALETTE_DEFAULT \
  {                         \
    {0x00, 0x00, 0x00},     \
      {0x00, 0x00, 0x2A},   \
      {0x00, 0x2A, 0x00},   \
      {0x00, 0x2A, 0x2A},   \
      {0x2A, 0x00, 0x00},   \
      {0x2A, 0x00, 0x2A},   \
      {0x2A, 0x15, 0x00},   \
      {0x2A, 0x2A, 0x2A},   \
      {0x15, 0x15, 0x15},   \
      {0x15, 0x15, 0x3F},   \
      {0x15, 0x3F, 0x15},   \
      {0x15, 0x3F, 0x3F},   \
      {0x3F, 0x15, 0x15},   \
      {0x3F, 0x15, 0x3F},   \
      {0x3F, 0x3F, 0x15},   \
      {0x3F, 0x3F, 0x3F},   \
  }

#define backtable_count table_counts[0]
#define foretable_count table_counts[1]
#define wipetable_count table_counts[2]
#define midtable_count table_counts[3]
#define objtable_count table_counts[4]


#define SEQTBL_BASE 0x196E
#define SEQTBL_0 (seqtbl - SEQTBL_BASE)

// This expands a two-byte number into two comma-separated bytes, used for the JMP destinations
#define DW(data_word) (data_word) & 0x00FF, (((data_word)&0xFF00) >> 8)

// Shorter notation for the sequence table instructions
#define act(action) SEQ_ACTION, action
#define jmp(dest) SEQ_JMP, DW(dest)
#define jmp_if_feather(dest) SEQ_JMP_IF_FEATHER, DW(dest)
#define dx(amount) SEQ_DX, (byte)amount
#define dy(amount) SEQ_DY, (byte)amount
#define snd(sound) SEQ_SOUND, sound
#define set_fall(x, y) SEQ_SET_FALL, (byte)x, (byte)y

// This splits the byte array into labeled "sections" that are packed tightly next to each other
// However, it only seems to work correctly in the Debug configuration...
//#define LABEL(label) }; const byte label##_eventual_ptr[] __attribute__ ((aligned(1))) = {
#define LABEL(label) // disable
//#define OFFSET(label) label - seqtbl + SEQTBL_BASE

// Labels
#define running SEQTBL_BASE                // 0x196E
#define startrun 5 + running               //SEQTBL_BASE + 5     // 0x1973
#define runstt1 2 + startrun               //SEQTBL_BASE + 7     // 0x1975
#define runstt4 3 + runstt1                //SEQTBL_BASE + 10    // 0x1978
#define runcyc1 9 + runstt4                //SEQTBL_BASE + 19    // 0x1981
#define runcyc7 20 + runcyc1               //SEQTBL_BASE + 39    // 0x1995
#define stand 11 + runcyc7                 //SEQTBL_BASE + 50    // 0x19A0
#define goalertstand 6 + stand             //SEQTBL_BASE + 56    // 0x19A6
#define alertstand 2 + goalertstand        //SEQTBL_BASE + 58    // 0x19A8
#define arise 4 + alertstand               //SEQTBL_BASE + 62    // 0x19AC
#define guardengarde 21 + arise            //SEQTBL_BASE + 83    // 0x19C1
#define engarde 3 + guardengarde           //SEQTBL_BASE + 86    // 0x19C4
#define ready 14 + engarde                 //SEQTBL_BASE + 100   // 0x19D2
#define ready_loop 6 + ready               //SEQTBL_BASE + 106   // 0x19D8
#define stabbed 4 + ready_loop             //SEQTBL_BASE + 110   // 0x19DC
#define strikeadv 29 + stabbed             //SEQTBL_BASE + 139   // 0x19F9
#define strikeret 14 + strikeadv           //SEQTBL_BASE + 153   // 0x1A07
#define advance 12 + strikeret             //SEQTBL_BASE + 165   // 0x1A13
#define fastadvance 15 + advance           //SEQTBL_BASE + 180   // 0x1A22
#define retreat 12 + fastadvance           //SEQTBL_BASE + 192   // 0x1A2E
#define strike 14 + retreat                //SEQTBL_BASE + 206   // 0x1A3C
#define faststrike 6 + strike              //SEQTBL_BASE + 212   // 0x1A42
#define guy4 3 + faststrike                //SEQTBL_BASE + 215   // 0x1A45
#define guy7 5 + guy4                      //SEQTBL_BASE + 220   // 0x1A4A
#define guy8 3 + guy7                      //SEQTBL_BASE + 223   // 0x1A4D
#define blockedstrike 7 + guy8             //SEQTBL_BASE + 230   // 0x1A54
#define blocktostrike 6 + blockedstrike    //SEQTBL_BASE + 236   // 0x1A5A
#define readyblock 4 + blocktostrike       //SEQTBL_BASE + 240   // 0x1A5E
#define blocking 1 + readyblock            //SEQTBL_BASE + 241   // 0x1A5F
#define striketoblock 4 + blocking         //SEQTBL_BASE + 245   // 0x1A63
#define landengarde 5 + striketoblock      //SEQTBL_BASE + 250   // 0x1A68
#define bumpengfwd 6 + landengarde         //SEQTBL_BASE + 256   // 0x1A6E
#define bumpengback 7 + bumpengfwd         //SEQTBL_BASE + 263   // 0x1A75
#define flee 7 + bumpengback               //SEQTBL_BASE + 270   // 0x1A7C
#define turnengarde 7 + flee               //SEQTBL_BASE + 277   // 0x1A83
#define alertturn 8 + turnengarde          //SEQTBL_BASE + 285   // 0x1A8B
#define standjump 8 + alertturn            //SEQTBL_BASE + 293   // 0x1A93
#define sjland 29 + standjump              //SEQTBL_BASE + 322   // 0x1AB0
#define runjump 29 + sjland                //SEQTBL_BASE + 351   // 0x1ACD
#define rjlandrun 46 + runjump             //SEQTBL_BASE + 397   // 0x1AFB
#define rdiveroll 9 + rjlandrun            //SEQTBL_BASE + 406   // 0x1B04
#define rdiveroll_crouch 18 + rdiveroll    //SEQTBL_BASE + 424   // 0x1B16
#define sdiveroll 4 + rdiveroll_crouch     //SEQTBL_BASE + 428   // 0x1B1A
#define crawl 1 + sdiveroll                //SEQTBL_BASE + 429   // 0x1B1B
#define crawl_crouch 14 + crawl            //SEQTBL_BASE + 443   // 0x1B29
#define turndraw 4 + crawl_crouch          //SEQTBL_BASE + 447   // 0x1B2D
#define turn 12 + turndraw                 //SEQTBL_BASE + 459   // 0x1B39
#define turnrun 26 + turn                  //SEQTBL_BASE + 485   // 0x1B53
#define runturn 7 + turnrun                //SEQTBL_BASE + 492   // 0x1B5A
#define fightfall 43 + runturn             //SEQTBL_BASE + 535   // 0x1B85
#define efightfall 28 + fightfall          //SEQTBL_BASE + 563   // 0x1BA1
#define efightfallfwd 30 + efightfall      //SEQTBL_BASE + 593   // 0x1BBF
#define stepfall 28 + efightfallfwd        //SEQTBL_BASE + 621   // 0x1BDB
#define fall1 9 + stepfall                 //SEQTBL_BASE + 630   // 0x1BE4
#define patchfall 22 + fall1               //SEQTBL_BASE + 652   // 0x1BFA
#define stepfall2 7 + patchfall            //SEQTBL_BASE + 659   // 0x1C01
#define stepfloat 5 + stepfall2            //SEQTBL_BASE + 664   // 0x1C06
#define jumpfall 22 + stepfloat            //SEQTBL_BASE + 686   // 0x1C1C
#define rjumpfall 28 + jumpfall            //SEQTBL_BASE + 714   // 0x1C38
#define jumphangMed 28 + rjumpfall         //SEQTBL_BASE + 742   // 0x1C54
#define jumphangLong 21 + jumphangMed      //SEQTBL_BASE + 763   // 0x1C69
#define jumpbackhang 27 + jumphangLong     //SEQTBL_BASE + 790   // 0x1C84
#define hang 29 + jumpbackhang             //SEQTBL_BASE + 819   // 0x1CA1
#define hang1 3 + hang                     //SEQTBL_BASE + 822   // 0x1CA4
#define hangstraight 45 + hang1            //SEQTBL_BASE + 867   // 0x1CD1
#define hangstraight_loop 7 + hangstraight //SEQTBL_BASE + 874   // 0x1CD8
#define climbfail 4 + hangstraight_loop    //SEQTBL_BASE + 878   // 0x1CDC
#define climbdown 16 + climbfail           //SEQTBL_BASE + 894   // 0x1CEC
#define climbup 24 + climbdown             //SEQTBL_BASE + 918   // 0x1D04
#define hangdrop 33 + climbup              //SEQTBL_BASE + 951   // 0x1D25
#define hangfall 17 + hangdrop             //SEQTBL_BASE + 968   // 0x1D36
#define freefall 19 + hangfall             //SEQTBL_BASE + 987   // 0x1D49
#define freefall_loop 2 + freefall         //SEQTBL_BASE + 989   // 0x1D4B
#define runstop 4 + freefall_loop          //SEQTBL_BASE + 993   // 0x1D4F
#define jumpup 25 + runstop                //SEQTBL_BASE + 1018  // 0x1D68
#define highjump 21 + jumpup               //SEQTBL_BASE + 1039  // 0x1D7D
#define superhijump 30 + highjump          //SEQTBL_BASE + 1069  // 0x1D9B
#define fallhang 91 + superhijump          //SEQTBL_BASE + 1160  // 0x1DF6
#define bump 6 + fallhang                  //SEQTBL_BASE + 1166  // 0x1DFC
#define bumpfall 10 + bump                 //SEQTBL_BASE + 1176  // 0x1E06
#define bumpfloat 31 + bumpfall            //SEQTBL_BASE + 1207  // 0x1E25
#define hardbump 22 + bumpfloat            //SEQTBL_BASE + 1229  // 0x1E3B
#define testfoot 30 + hardbump             //SEQTBL_BASE + 1259  // 0x1E59
#define stepback 31 + testfoot             //SEQTBL_BASE + 1290  // 0x1E78
#define step14 5 + stepback                //SEQTBL_BASE + 1295  // 0x1E7D
#define step13 31 + step14                 //SEQTBL_BASE + 1326  // 0x1E9C
#define step12 31 + step13                 //SEQTBL_BASE + 1357  // 0x1EBB
#define step11 31 + step12                 //SEQTBL_BASE + 1388  // 0x1EDA
#define step10 29 + step11                 //SEQTBL_BASE + 1417  // 0x1EF7
#define step10a 5 + step10                 //SEQTBL_BASE + 1422  // 0x1EFC
#define step9 23 + step10a                 //SEQTBL_BASE + 1445  // 0x1F13
#define step8 6 + step9                    //SEQTBL_BASE + 1451  // 0x1F19
#define step7 26 + step8                   //SEQTBL_BASE + 1477  // 0x1F33
#define step6 21 + step7                   //SEQTBL_BASE + 1498  // 0x1F48
#define step5 21 + step6                   //SEQTBL_BASE + 1519  // 0x1F5D
#define step4 21 + step5                   //SEQTBL_BASE + 1540  // 0x1F72
#define step3 16 + step4                   //SEQTBL_BASE + 1556  // 0x1F82
#define step2 16 + step3                   //SEQTBL_BASE + 1572  // 0x1F92
#define step1 12 + step2                   //SEQTBL_BASE + 1584  // 0x1F9E
#define stoop 9 + step1                    //SEQTBL_BASE + 1593  // 0x1FA7
#define stoop_crouch 8 + stoop             //SEQTBL_BASE + 1601  // 0x1FAF
#define standup 4 + stoop_crouch           //SEQTBL_BASE + 1605  // 0x1FB3
#define pickupsword 23 + standup           //SEQTBL_BASE + 1628  // 0x1FCA
#define resheathe 16 + pickupsword         //SEQTBL_BASE + 1644  // 0x1FDA
#define fastsheathe 33 + resheathe         //SEQTBL_BASE + 1677  // 0x1FFB
#define drinkpotion 14 + fastsheathe       //SEQTBL_BASE + 1691  // 0x2009
#define softland 34 + drinkpotion          //SEQTBL_BASE + 1725  // 0x202B
#define softland_crouch 11 + softland      //SEQTBL_BASE + 1736  // 0x2036
#define landrun 4 + softland_crouch        //SEQTBL_BASE + 1740  // 0x203A
#define medland 32 + landrun               //SEQTBL_BASE + 1772  // 0x205A
#define hardland 66 + medland              //SEQTBL_BASE + 1838  // 0x209C
#define hardland_dead 9 + hardland         //SEQTBL_BASE + 1847  // 0x20A5
#define stabkill 4 + hardland_dead         //SEQTBL_BASE + 1851  // 0x20A9
#define dropdead 5 + stabkill              //SEQTBL_BASE + 1856  // 0x20AE
#define dropdead_dead 12 + dropdead        //SEQTBL_BASE + 1868  // 0x20BA
#define impale 4 + dropdead_dead           //SEQTBL_BASE + 1872  // 0x20BE
#define impale_dead 7 + impale             //SEQTBL_BASE + 1879  // 0x20C5
#define halve 4 + impale_dead              //SEQTBL_BASE + 1883  // 0x20C9
#define halve_dead 4 + halve               //SEQTBL_BASE + 1887  // 0x20CD
#define crush 4 + halve_dead               //SEQTBL_BASE + 1891  // 0x20D1
#define deadfall 3 + crush                 //SEQTBL_BASE + 1894  // 0x20D4
#define deadfall_loop 5 + deadfall         //SEQTBL_BASE + 1899  // 0x20D9
#define climbstairs 4 + deadfall_loop      //SEQTBL_BASE + 1903  // 0x20DD
#define climbstairs_loop 81 + climbstairs  //SEQTBL_BASE + 1984  // 0x212E
#define Vstand 4 + climbstairs_loop        //SEQTBL_BASE + 1988  // 0x2132
#define Vraise 4 + Vstand                  //SEQTBL_BASE + 1992  // 0x2136
#define Vraise_loop 21 + Vraise            //SEQTBL_BASE + 2013  // 0x214B
#define Vwalk 4 + Vraise_loop              //SEQTBL_BASE + 2017  // 0x214F
#define Vwalk1 2 + Vwalk                   //SEQTBL_BASE + 2019  // 0x2151
#define Vwalk2 3 + Vwalk1                  //SEQTBL_BASE + 2022  // 0x2154
#define Vstop 18 + Vwalk2                  //SEQTBL_BASE + 2040  // 0x2166
#define Vexit 7 + Vstop                    //SEQTBL_BASE + 2047  // 0x216D
#define Pstand 40 + Vexit                  //SEQTBL_BASE + 2087  // 0x2195
#define Palert 4 + Pstand                  //SEQTBL_BASE + 2091  // 0x2199
#define Pstepback 15 + Palert              //SEQTBL_BASE + 2106  // 0x21A8
#define Pstepback_loop 16 + Pstepback      //SEQTBL_BASE + 2122  // 0x21B8
#define Plie 4 + Pstepback_loop            //SEQTBL_BASE + 2126  // 0x21BC
#define Pwaiting 4 + Plie                  //SEQTBL_BASE + 2130  // 0x21C0
#define Pembrace 4 + Pwaiting              //SEQTBL_BASE + 2134  // 0x21C4
#define Pembrace_loop 30 + Pembrace        //SEQTBL_BASE + 2164  // 0x21E2
#define Pstroke 4 + Pembrace_loop          //SEQTBL_BASE + 2168  // 0x21E6
#define Prise 4 + Pstroke                  //SEQTBL_BASE + 2172  // 0x21EA
#define Prise_loop 14 + Prise              //SEQTBL_BASE + 2186  // 0x21F8
#define Pcrouch 4 + Prise_loop             //SEQTBL_BASE + 2190  // 0x21FC
#define Pcrouch_loop 64 + Pcrouch          //SEQTBL_BASE + 2254  // 0x223C
#define Pslump 4 + Pcrouch_loop            //SEQTBL_BASE + 2258  // 0x2240
#define Pslump_loop 1 + Pslump             //SEQTBL_BASE + 2259  // 0x2241
#define Mscurry 4 + Pslump_loop            //SEQTBL_BASE + 2263  // 0x2245
#define Mscurry1 2 + Mscurry               //SEQTBL_BASE + 2265  // 0x2247
#define Mstop 12 + Mscurry1                //SEQTBL_BASE + 2277  // 0x2253
#define Mraise 4 + Mstop                   //SEQTBL_BASE + 2281  // 0x2257
#define Mleave 4 + Mraise                  //SEQTBL_BASE + 2285  // 0x225B
#define Mclimb 19 + Mleave                 //SEQTBL_BASE + 2304  // 0x226E
#define Mclimb_loop 2 + Mclimb             //SEQTBL_BASE + 2306  // 0x2270

const word seqtbl_offsets[] = {
  0x0000, startrun, stand, standjump, runjump, turn, runturn, stepfall, jumphangMed, hang, climbup, hangdrop, freefall, runstop, jumpup, fallhang, jumpbackhang, softland, jumpfall, stepfall2, medland, rjumpfall, hardland, hangfall, jumphangLong, hangstraight, rdiveroll, sdiveroll, highjump, step1, step2, step3, step4, step5, step6, step7, step8, step9, step10, step11, step12, step13, step14, turnrun, testfoot, bumpfall, hardbump, bump, superhijump, standup, stoop, impale, crush, deadfall, halve, engarde, advance, retreat, strike, flee, turnengarde, striketoblock, readyblock, landengarde, bumpengfwd, bumpengback, blocktostrike, strikeadv, climbdown, blockedstrike, climbstairs, dropdead, stepback, climbfail, stabbed, faststrike, strikeret, alertstand, drinkpotion, crawl, alertturn, fightfall, efightfall, efightfallfwd, running, stabkill, fastadvance, goalertstand, arise, turndraw, guardengarde, pickupsword, resheathe, fastsheathe, Pstand, Vstand, Vwalk, Vstop, Palert, Pstepback, Vexit, Mclimb, Vraise, Plie, patchfall, Mscurry, Mstop, Mleave, Pembrace, Pwaiting, Pstroke, Prise, Pcrouch, Pslump, Mraise};

// data:196E
static const byte seqtbl[] = {

  LABEL(running) // running
  act(actions_1_run_jump),
  jmp(runcyc1), // goto running: frame 7

  LABEL(startrun) // startrun
  act(actions_1_run_jump),
  LABEL(runstt1) frame_1_start_run,
  frame_2_start_run,
  frame_3_start_run,
  LABEL(runstt4) frame_4_start_run,
  dx(8),
  frame_5_start_run,
  dx(3),
  frame_6_start_run,
  dx(3),
  LABEL(runcyc1) frame_7_run,
  dx(5),
  frame_8_run,
  dx(1),
  snd(SND_FOOTSTEP),
  frame_9_run,
  dx(2),
  frame_10_run,
  dx(4),
  frame_11_run,
  dx(5),
  frame_12_run,
  dx(2),
  LABEL(runcyc7) snd(SND_FOOTSTEP),
  frame_13_run,
  dx(3),
  frame_14_run,
  dx(4),
  jmp(runcyc1),

  LABEL(stand) // stand
  act(actions_0_stand),
  frame_15_stand,
  jmp(stand), // goto "stand"

  LABEL(goalertstand) // alert stand
  act(actions_1_run_jump),
  LABEL(alertstand) frame_166_stand_inactive,
  jmp(alertstand), // goto "alertstand"

  LABEL(arise) // arise (skeleton)
  act(actions_5_bumped),
  dx(10),
  frame_177_spiked,
  frame_177_spiked,
  dx(-7),
  dy(-2),
  frame_178_chomped,
  dx(5),
  dy(2),
  frame_166_stand_inactive,
  dx(-1),
  jmp(ready), // goto "ready"

  // guard engarde
  LABEL(guardengarde)
    jmp(ready), // goto "ready"

  // engarde
  LABEL(engarde)
    act(actions_1_run_jump),
  dx(2),
  frame_207_draw_1,
  frame_208_draw_2,
  dx(2),
  frame_209_draw_3,
  dx(2),
  frame_210_draw_4,
  dx(3),
  LABEL(ready) act(actions_1_run_jump),
  snd(SND_SILENT),
  frame_158_stand_with_sword,
  frame_170_stand_with_sword,
  LABEL(ready_loop) frame_171_stand_with_sword,
  jmp(ready_loop), // goto ":loop"

  LABEL(stabbed) // stabbed
  act(actions_5_bumped),
  set_fall(-1, 0),
  frame_172_jumpfall_2,
  dx(-1),
  dy(1),
  frame_173_jumpfall_3,
  dx(-1),
  frame_174_jumpfall_4,
  dx(-1),
  dy(2), // frame 175 is commented out in the Apple II source
  dx(-2),
  dy(1),
  dx(-5),
  dy(-4),
  jmp(guy8), // goto "guy8"

  // strike - advance
  LABEL(strikeadv)
    act(actions_1_run_jump),
  set_fall(1, 0),
  frame_155_guy_7,
  dx(2),
  frame_165_walk_with_sword,
  dx(-2),
  jmp(ready), // goto "ready"

  LABEL(strikeret) // strike - retreat
  act(actions_1_run_jump),
  set_fall(-1, 0),
  frame_155_guy_7,
  frame_156_guy_8,
  frame_157_walk_with_sword,
  frame_158_stand_with_sword,
  jmp(retreat), // goto "retreat"

  LABEL(advance) // advance
  act(actions_1_run_jump),
  set_fall(1, 0),
  dx(2),
  frame_163_fighting,
  dx(4),
  frame_164_fighting,
  frame_165_walk_with_sword,
  jmp(ready), // goto "ready"

  LABEL(fastadvance) // fast advance
  act(actions_1_run_jump),
  set_fall(1, 0),
  dx(6),
  frame_164_fighting,
  frame_165_walk_with_sword,
  jmp(ready), // goto "ready"

  LABEL(retreat) // retreat
  act(actions_1_run_jump),
  set_fall(-1, 0),
  dx(-3),
  frame_160_fighting,
  dx(-2),
  frame_157_walk_with_sword,
  jmp(ready), // goto "ready"

  LABEL(strike) // strike
  act(actions_1_run_jump),
  set_fall(-1, 0),
  frame_168_back,
  LABEL(faststrike) act(actions_1_run_jump),
  frame_151_strike_1,
  LABEL(guy4) act(actions_1_run_jump),
  frame_152_strike_2,
  frame_153_strike_3,
  frame_154_poking,
  LABEL(guy7) act(actions_5_bumped),
  frame_155_guy_7,
  LABEL(guy8) act(actions_1_run_jump),
  frame_156_guy_8,
  frame_157_walk_with_sword,
  jmp(ready), // goto "ready"

  LABEL(blockedstrike) // blocked strike
  act(actions_1_run_jump),
  frame_167_blocked,
  jmp(guy7), // goto "guy7"

  // block to strike
  LABEL(blocktostrike) // "blocktostrike"
  frame_162_block_to_strike,
  jmp(guy4), // goto "guy4"

  LABEL(readyblock) // ready block
  frame_169_begin_block,
  LABEL(blocking) frame_150_parry,
  jmp(ready), // goto "ready"

  LABEL(striketoblock) // strike to block
  frame_159_fighting,
  frame_160_fighting,
  jmp(blocking), // goto "blocking"

  LABEL(landengarde) // land en garde
  act(actions_1_run_jump),
  SEQ_KNOCK_DOWN,
  jmp(ready), // goto "ready"

  LABEL(bumpengfwd) // bump en garde (forward)
  act(actions_5_bumped),
  dx(-8),
  jmp(ready), // goto "ready"

  LABEL(bumpengback) // bump en garde (back)
  act(actions_5_bumped),
  frame_160_fighting,
  frame_157_walk_with_sword,
  jmp(ready), // goto "ready"

  LABEL(flee) // flee
  act(actions_7_turn),
  dx(-8),
  jmp(turn), // goto "turn"

  LABEL(turnengarde) // turn en garde
  act(actions_5_bumped),
  SEQ_FLIP,
  dx(5),
  jmp(retreat), // goto "retreat"

  LABEL(alertturn) // alert turn (for enemies)
  act(actions_5_bumped),
  SEQ_FLIP,
  dx(18),
  jmp(goalertstand), // goto "goalertstand"

  LABEL(standjump) // standing jump
  act(actions_1_run_jump),
  frame_16_standing_jump_1,
  frame_17_standing_jump_2,
  dx(2),
  frame_18_standing_jump_3,
  dx(2),
  frame_19_standing_jump_4,
  dx(2),
  frame_20_standing_jump_5,
  dx(2),
  frame_21_standing_jump_6,
  dx(2),
  frame_22_standing_jump_7,
  dx(7),
  frame_23_standing_jump_8,
  dx(9),
  frame_24_standing_jump_9,
  dx(5),
  dy(-6),
  /* "sjland" */ LABEL(sjland) frame_25_standing_jump_10,
  dx(1),
  dy(6),
  frame_26_standing_jump_11,
  dx(4),
  SEQ_KNOCK_DOWN,
  snd(SND_FOOTSTEP),
  frame_27_standing_jump_12,
  dx(-3),
  frame_28_standing_jump_13,
  dx(5),
  frame_29_standing_jump_14,
  snd(SND_FOOTSTEP),
  frame_30_standing_jump_15,
  frame_31_standing_jump_16,
  frame_32_standing_jump_17,
  frame_33_standing_jump_18,
  dx(1),
  jmp(stand), // goto "stand"

  LABEL(runjump) // running jump
  act(actions_1_run_jump),
  snd(SND_FOOTSTEP),
  frame_34_start_run_jump_1,
  dx(5),
  frame_35_start_run_jump_2,
  dx(6),
  frame_36_start_run_jump_3,
  dx(3),
  frame_37_start_run_jump_4,
  dx(5),
  snd(SND_FOOTSTEP),
  frame_38_start_run_jump_5,
  dx(7),
  frame_39_start_run_jump_6,
  dx(12),
  dy(-3),
  frame_40_running_jump_1,
  dx(8),
  dy(-9),
  frame_41_running_jump_2,
  dx(8),
  dy(-2),
  frame_42_running_jump_3,
  dx(4),
  dy(11),
  frame_43_running_jump_4,
  dx(4),
  dy(3),
  /* "rjlandrun" */ LABEL(rjlandrun) frame_44_running_jump_5,
  dx(5),
  SEQ_KNOCK_DOWN,
  snd(SND_FOOTSTEP),
  jmp(runcyc1), // goto "runcyc1"

  LABEL(rdiveroll) // run dive roll
  act(actions_1_run_jump),
  dx(1),
  frame_107_fall_land_1,
  dx(2),
  dx(2),
  frame_108_fall_land_2,
  dx(2),
  frame_109_crouch,
  dx(2),
  frame_109_crouch,
  dx(2),
  /* ":crouch" */ LABEL(rdiveroll_crouch) frame_109_crouch,
  jmp(rdiveroll_crouch), // goto ":crouch"

  LABEL(sdiveroll) 0x00, // stand dive roll; not implemented

  LABEL(crawl) // crawl
  act(actions_1_run_jump),
  dx(1),
  frame_110_stand_up_from_crouch_1,
  frame_111_stand_up_from_crouch_2,
  dx(2),
  frame_112_stand_up_from_crouch_3,
  dx(2),
  frame_108_fall_land_2,
  dx(2),
  /* ":crouch" */ LABEL(crawl_crouch) frame_109_crouch,
  jmp(crawl_crouch), // goto ":crouch"

  LABEL(turndraw) // turn draw
  act(actions_7_turn),
  SEQ_FLIP,
  dx(6),
  frame_45_turn,
  dx(1),
  frame_46_turn,
  jmp(engarde), // goto "engarde"

  LABEL(turn) // turn
  act(actions_7_turn),
  SEQ_FLIP,
  dx(6),
  frame_45_turn,
  dx(1),
  frame_46_turn,
  dx(2),
  frame_47_turn,
  dx(-1),
  /* "finishturn" */ frame_48_turn,
  dx(1),
  frame_49_turn,
  dx(-2),
  frame_50_turn,
  frame_51_turn,
  frame_52_turn,
  jmp(stand), // goto "stand"

  LABEL(turnrun) // turnrun (from frame 48)
  act(actions_1_run_jump),
  dx(-1),
  jmp(runstt1), // goto "runstt1"

  LABEL(runturn) // runturn
  act(actions_1_run_jump),
  dx(1),
  frame_53_runturn,
  dx(1),
  snd(SND_FOOTSTEP),
  frame_54_runturn,
  dx(8),
  frame_55_runturn,
  snd(SND_FOOTSTEP),
  frame_56_runturn,
  dx(7),
  frame_57_runturn,
  dx(3),
  frame_58_runturn,
  dx(1),
  frame_59_runturn,
  frame_60_runturn,
  dx(2),
  frame_61_runturn,
  dx(-1),
  frame_62_runturn,
  frame_63_runturn,
  frame_64_runturn,
  dx(-1),
  frame_65_runturn,
  dx(-14),
  SEQ_FLIP,
  jmp(runcyc7), // goto "runcyc7"

  LABEL(fightfall) // fightfall (backward)
  act(actions_3_in_midair),
  dy(-1),
  frame_102_start_fall_1,
  dx(-2),
  dy(6),
  frame_103_start_fall_2,
  dx(-2),
  dy(9),
  frame_104_start_fall_3,
  dx(-1),
  dy(12),
  frame_105_start_fall_4,
  dx(-3),
  set_fall(0, 15),
  jmp(freefall), // goto "freefall"

  LABEL(efightfall) // enemy fight fall
  act(actions_3_in_midair),
  dy(-1),
  dx(-2),
  frame_102_start_fall_1,
  dx(-3),
  dy(6),
  frame_103_start_fall_2,
  dx(-3),
  dy(9),
  frame_104_start_fall_3,
  dx(-2),
  dy(12),
  frame_105_start_fall_4,
  dx(-3),
  set_fall(0, 15),
  jmp(freefall), // goto "freefall"

  LABEL(efightfallfwd) // enemy fight fall forward
  act(actions_3_in_midair),
  dx(1),
  dy(-1),
  frame_102_start_fall_1,
  dx(2),
  dy(6),
  frame_103_start_fall_2,
  dx(-1),
  dy(9),
  frame_104_start_fall_3,
  dy(12),
  frame_105_start_fall_4,
  dx(-2),
  set_fall(1, 15),
  jmp(freefall), // goto "freefall"

  LABEL(stepfall) // stepfall
  act(actions_3_in_midair),
  dx(1),
  dy(3),
  jmp_if_feather(stepfloat), // goto "stepfloat"
  /* "fall1" */ LABEL(fall1) frame_102_start_fall_1,
  dx(2),
  dy(6),
  frame_103_start_fall_2,
  dx(-1),
  dy(9),
  frame_104_start_fall_3,
  dy(12),
  frame_105_start_fall_4,
  dx(-2),
  set_fall(1, 15),
  jmp(freefall), // goto "freefall"

  LABEL(patchfall) // patchfall
  dx(-1),
  dy(-3),
  jmp(fall1), // goto "fall1"

  LABEL(stepfall2) // stepfall2 (from frame 12)
  dx(1),
  jmp(stepfall), // goto "stepfall"

  LABEL(stepfloat) // stepfloat
  frame_102_start_fall_1,
  dx(2),
  dy(3),
  frame_103_start_fall_2,
  dx(-1),
  dy(4),
  frame_104_start_fall_3,
  dy(5),
  frame_105_start_fall_4,
  dx(-2),
  set_fall(1, 6),
  jmp(freefall), // goto "freefall"

  LABEL(jumpfall) // jump fall (from standing jump)
  act(actions_3_in_midair),
  dx(1),
  dy(3),
  frame_102_start_fall_1,
  dx(2),
  dy(6),
  frame_103_start_fall_2,
  dx(1),
  dy(9),
  frame_104_start_fall_3,
  dx(2),
  dy(12),
  frame_105_start_fall_4,
  set_fall(2, 15),
  jmp(freefall), // goto "freefall"

  LABEL(rjumpfall) // running jump fall
  act(actions_3_in_midair),
  dx(1),
  dy(3),
  frame_102_start_fall_1,
  dx(3),
  dy(6),
  frame_103_start_fall_2,
  dx(2),
  dy(9),
  frame_104_start_fall_3,
  dx(3),
  dy(12),
  frame_105_start_fall_4,
  set_fall(3, 15),
  jmp(freefall), // goto "freefall"

  LABEL(jumphangMed) // jumphang (medium: DX = 0)
  act(actions_1_run_jump),
  frame_67_start_jump_up_1,
  frame_68_start_jump_up_2,
  frame_69_start_jump_up_3,
  frame_70_jumphang,
  frame_71_jumphang,
  frame_72_jumphang,
  frame_73_jumphang,
  frame_74_jumphang,
  frame_75_jumphang,
  frame_76_jumphang,
  frame_77_jumphang,
  act(actions_2_hang_climb),
  frame_78_jumphang,
  frame_79_jumphang,
  frame_80_jumphang,
  jmp(hang), // goto "hang"

  LABEL(jumphangLong) // jumphang (long: DX = 4)
  act(actions_1_run_jump),
  frame_67_start_jump_up_1,
  frame_68_start_jump_up_2,
  frame_69_start_jump_up_3,
  frame_70_jumphang,
  frame_71_jumphang,
  frame_72_jumphang,
  frame_73_jumphang,
  frame_74_jumphang,
  frame_75_jumphang,
  frame_76_jumphang,
  frame_77_jumphang,
  act(actions_2_hang_climb),
  dx(1),
  frame_78_jumphang,
  dx(2),
  frame_79_jumphang,
  dx(1),
  frame_80_jumphang,
  jmp(hang), // goto "hang"

  LABEL(jumpbackhang) // jumpbackhang
  act(actions_1_run_jump),
  frame_67_start_jump_up_1,
  frame_68_start_jump_up_2,
  frame_69_start_jump_up_3,
  frame_70_jumphang,
  frame_71_jumphang,
  frame_72_jumphang,
  frame_73_jumphang,
  frame_74_jumphang,
  frame_75_jumphang,
  frame_76_jumphang,
  dx(-1),
  frame_77_jumphang,
  act(actions_2_hang_climb),
  dx(-2),
  frame_78_jumphang,
  dx(-1),
  frame_79_jumphang,
  dx(-1),
  frame_80_jumphang,
  jmp(hang), // goto "hang"

  LABEL(hang) // hang
  act(actions_2_hang_climb),
  frame_91_hanging_5,
  /* "hang1" */ LABEL(hang1) frame_90_hanging_4,
  frame_89_hanging_3,
  frame_88_hanging_2,
  frame_87_hanging_1,
  frame_87_hanging_1,
  frame_87_hanging_1,
  frame_88_hanging_2,
  frame_89_hanging_3,
  frame_90_hanging_4,
  frame_91_hanging_5,
  frame_92_hanging_6,
  frame_93_hanging_7,
  frame_94_hanging_8,
  frame_95_hanging_9,
  frame_96_hanging_10,
  frame_97_hanging_11,
  frame_98_hanging_12,
  frame_99_hanging_13,
  frame_97_hanging_11,
  frame_96_hanging_10,
  frame_95_hanging_9,
  frame_94_hanging_8,
  frame_93_hanging_7,
  frame_92_hanging_6,
  frame_91_hanging_5,
  frame_90_hanging_4,
  frame_89_hanging_3,
  frame_88_hanging_2,
  frame_87_hanging_1,
  frame_88_hanging_2,
  frame_89_hanging_3,
  frame_90_hanging_4,
  frame_91_hanging_5,
  frame_92_hanging_6,
  frame_93_hanging_7,
  frame_94_hanging_8,
  frame_95_hanging_9,
  frame_96_hanging_10,
  frame_95_hanging_9,
  frame_94_hanging_8,
  frame_93_hanging_7,
  frame_92_hanging_6,
  jmp(hangdrop), // goto "hangdrop"

  LABEL(hangstraight) // hangstraight
  act(actions_6_hang_straight),
  frame_92_hanging_6, // Apple II source has a bump sound here
  frame_93_hanging_7,
  frame_93_hanging_7,
  frame_92_hanging_6,
  frame_92_hanging_6,
  /* ":loop" */ LABEL(hangstraight_loop) frame_91_hanging_5,
  jmp(hangstraight_loop), // goto ":loop"

  LABEL(climbfail) // climbfail
  frame_135_climbing_1,
  frame_136_climbing_2,
  frame_137_climbing_3,
  frame_137_climbing_3,
  frame_138_climbing_4,
  frame_138_climbing_4,
  frame_138_climbing_4,
  frame_138_climbing_4,
  frame_137_climbing_3,
  frame_136_climbing_2,
  frame_135_climbing_1,
  dx(-7),
  jmp(hangdrop), // goto "hangdrop"

  LABEL(climbdown) // climbdown
  act(actions_1_run_jump),
  frame_148_climbing_14,
  frame_145_climbing_11,
  frame_144_climbing_10,
  frame_143_climbing_9,
  frame_142_climbing_8,
  frame_141_climbing_7,
  dx(-5),
  dy(63),
  SEQ_DOWN,
  act(actions_3_in_midair),
  frame_140_climbing_6,
  frame_138_climbing_4,
  frame_136_climbing_2,
  frame_91_hanging_5,
  act(actions_2_hang_climb),
  jmp(hang1), // goto "hang1"

  LABEL(climbup) // climbup
  act(actions_1_run_jump),
  frame_135_climbing_1,
  frame_136_climbing_2,
  frame_137_climbing_3,
  frame_138_climbing_4,
  frame_139_climbing_5,
  frame_140_climbing_6,
  dx(5),
  dy(-63),
  SEQ_UP,
  frame_141_climbing_7,
  frame_142_climbing_8,
  frame_143_climbing_9,
  frame_144_climbing_10,
  frame_145_climbing_11,
  frame_146_climbing_12,
  frame_147_climbing_13,
  frame_148_climbing_14,
  act(actions_5_bumped), // to clear flags
  frame_149_climbing_15,
  act(actions_1_run_jump),
  frame_118_stand_up_from_crouch_9,
  frame_119_stand_up_from_crouch_10,
  dx(1),
  jmp(stand), // goto "stand"

  LABEL(hangdrop) // hangdrop
  frame_81_hangdrop_1,
  frame_82_hangdrop_2,
  act(actions_5_bumped),
  frame_83_hangdrop_3,
  act(actions_1_run_jump),
  SEQ_KNOCK_DOWN,
  snd(SND_SILENT),
  frame_84_hangdrop_4,
  frame_85_hangdrop_5,
  dx(3),
  jmp(stand), // goto "stand"

  LABEL(hangfall) // hangfall
  act(actions_3_in_midair),
  frame_81_hangdrop_1,
  dy(6),
  frame_81_hangdrop_1,
  dy(9),
  frame_81_hangdrop_1,
  dy(12),
  dx(2),
  set_fall(0, 12),
  jmp(freefall), // goto "freefall"

  LABEL(freefall) // freefall
  act(actions_4_in_freefall),
  /* ":loop" */ LABEL(freefall_loop) frame_106_fall,
  jmp(freefall_loop), // goto :loop

  LABEL(runstop) // runstop
  act(actions_1_run_jump),
  frame_53_runturn,
  dx(2),
  snd(SND_FOOTSTEP),
  frame_54_runturn,
  dx(7),
  frame_55_runturn,
  snd(SND_FOOTSTEP),
  frame_56_runturn,
  dx(2),
  frame_49_turn,
  dx(-2),
  frame_50_turn,
  frame_51_turn,
  frame_52_turn,
  jmp(stand), // goto "stand"

  LABEL(jumpup) // jump up (and touch ceiling)
  act(actions_1_run_jump),
  frame_67_start_jump_up_1,
  frame_68_start_jump_up_2,
  frame_69_start_jump_up_3,
  frame_70_jumphang,
  frame_71_jumphang,
  frame_72_jumphang,
  frame_73_jumphang,
  frame_74_jumphang,
  frame_75_jumphang,
  frame_76_jumphang,
  frame_77_jumphang,
  frame_78_jumphang,
  act(actions_0_stand),
  SEQ_KNOCK_UP,
  frame_79_jumphang,
  jmp(hangdrop), // goto "hangdrop"

  LABEL(highjump) // highjump (no ceiling above)
  act(actions_1_run_jump),
  frame_67_start_jump_up_1,
  frame_68_start_jump_up_2,
  frame_69_start_jump_up_3,
  frame_70_jumphang,
  frame_71_jumphang,
  frame_72_jumphang,
  frame_73_jumphang,
  frame_74_jumphang,
  frame_75_jumphang,
  frame_76_jumphang,
  frame_77_jumphang,
  frame_78_jumphang,
  frame_79_jumphang,
  dy(-4),
  frame_79_jumphang,
  dy(-2),
  frame_79_jumphang,
  frame_79_jumphang,
  dy(2),
  frame_79_jumphang,
  dy(4),
  jmp(hangdrop), // goto "hangdrop"

  LABEL(superhijump) // superhijump (when weightless)
  frame_67_start_jump_up_1,
  frame_68_start_jump_up_2,
  frame_69_start_jump_up_3,
  frame_70_jumphang,
  frame_71_jumphang,
  frame_72_jumphang,
  frame_73_jumphang,
  frame_74_jumphang,
  frame_75_jumphang,
  frame_76_jumphang,
  dy(-1),
  frame_77_jumphang,
  dy(-3),
  frame_78_jumphang,
  dy(-4),
  frame_79_jumphang,
  dy(-10),
  frame_79_jumphang,
  dy(-9),
  frame_79_jumphang,
  dy(-8),
  frame_79_jumphang,
  dy(-7),
  frame_79_jumphang,
  dy(-6),
  frame_79_jumphang,
  dy(-5),
  frame_79_jumphang,
  dy(-4),
  frame_79_jumphang,
  dy(-3),
  frame_79_jumphang,
  dy(-2),
  frame_79_jumphang,
  dy(-2),
  frame_79_jumphang,
  dy(-1),
  frame_79_jumphang,
  dy(-1),
  frame_79_jumphang,
  dy(-1),
  frame_79_jumphang,
  frame_79_jumphang,
  frame_79_jumphang,
  frame_79_jumphang,
  dy(1),
  frame_79_jumphang,
  dy(1),
  frame_79_jumphang,
  dy(2),
  frame_79_jumphang,
  dy(2),
  frame_79_jumphang,
  dy(3),
  frame_79_jumphang,
  dy(4),
  frame_79_jumphang,
  dy(5),
  frame_79_jumphang,
  dy(6),
  frame_79_jumphang,
  set_fall(0, 6),
  jmp(freefall), // goto "freefall"

  LABEL(fallhang) // fall hang
  act(actions_3_in_midair),
  frame_80_jumphang,
  jmp(hang), // goto "hang"

  LABEL(bump) // bump
  act(actions_5_bumped),
  dx(-4),
  frame_50_turn,
  frame_51_turn,
  frame_52_turn,
  jmp(stand), // goto "stand"

  LABEL(bumpfall) // bumpfall
  act(actions_5_bumped),
  dx(1),
  dy(3),
  jmp_if_feather(bumpfloat),
  frame_102_start_fall_1,
  dx(2),
  dy(6),
  frame_103_start_fall_2,
  dx(-1),
  dy(9),
  frame_104_start_fall_3,
  dy(12),
  frame_105_start_fall_4,
  dx(-2),
  set_fall(0, 15),
  jmp(freefall), // goto "freefall"

  LABEL(bumpfloat) // bumpfloat
  frame_102_start_fall_1,
  dx(2),
  dy(3),
  frame_103_start_fall_2,
  dx(-1),
  dy(4),
  frame_104_start_fall_3,
  dy(5),
  frame_105_start_fall_4,
  dx(-2),
  set_fall(0, 6),
  jmp(freefall), // goto "freefall"

  LABEL(hardbump) // hard bump
  act(actions_5_bumped),
  dx(-1),
  dy(-4),
  frame_102_start_fall_1,
  dx(-1),
  dy(3),
  dx(-3),
  dy(1),
  SEQ_KNOCK_DOWN,
  dx(1),
  snd(SND_FOOTSTEP),
  frame_107_fall_land_1,
  dx(2),
  frame_108_fall_land_2,
  snd(SND_FOOTSTEP),
  frame_109_crouch,
  jmp(standup), // goto "standup"

  LABEL(testfoot) // test foot
  frame_121_stepping_1,
  dx(1),
  frame_122_stepping_2,
  frame_123_stepping_3,
  dx(2),
  frame_124_stepping_4,
  dx(4),
  frame_125_stepping_5,
  dx(3),
  frame_126_stepping_6,
  dx(-4),
  frame_86_test_foot,
  snd(SND_FOOTSTEP),
  SEQ_KNOCK_DOWN,
  dx(-4),
  frame_116_stand_up_from_crouch_7,
  dx(-2),
  frame_117_stand_up_from_crouch_8,
  frame_118_stand_up_from_crouch_9,
  frame_119_stand_up_from_crouch_10,
  jmp(stand), // goto "stand"

  LABEL(stepback) // step back
  dx(-5),
  jmp(stand), // goto "stand"

  LABEL(step14) // step forward 14 pixels
  act(actions_1_run_jump),
  frame_121_stepping_1,
  dx(1),
  frame_122_stepping_2,
  dx(1),
  frame_123_stepping_3,
  dx(3),
  frame_124_stepping_4,
  dx(4),
  frame_125_stepping_5,
  dx(3),
  frame_126_stepping_6,
  dx(-1),
  dx(3),
  frame_127_stepping_7,
  frame_128_stepping_8,
  frame_129_stepping_9,
  frame_130_stepping_10,
  frame_131_stepping_11,
  frame_132_stepping_12,
  jmp(stand), // goto "stand"

  LABEL(step13) // step forward 13 pixels
  act(actions_1_run_jump),
  frame_121_stepping_1,
  dx(1),
  frame_122_stepping_2,
  dx(1),
  frame_123_stepping_3,
  dx(3),
  frame_124_stepping_4,
  dx(4),
  frame_125_stepping_5,
  dx(3),
  frame_126_stepping_6,
  dx(-1),
  dx(2),
  frame_127_stepping_7,
  frame_128_stepping_8,
  frame_129_stepping_9,
  frame_130_stepping_10,
  frame_131_stepping_11,
  frame_132_stepping_12,
  jmp(stand), // goto "stand"

  LABEL(step12) // step forward 12 pixels
  act(actions_1_run_jump),
  frame_121_stepping_1,
  dx(1),
  frame_122_stepping_2,
  dx(1),
  frame_123_stepping_3,
  dx(3),
  frame_124_stepping_4,
  dx(4),
  frame_125_stepping_5,
  dx(3),
  frame_126_stepping_6,
  dx(-1),
  dx(1),
  frame_127_stepping_7,
  frame_128_stepping_8,
  frame_129_stepping_9,
  frame_130_stepping_10,
  frame_131_stepping_11,
  frame_132_stepping_12,
  jmp(stand), // goto "stand"

  LABEL(step11) // step forward 11 pixels (normal step)
  act(actions_1_run_jump),
  frame_121_stepping_1,
  dx(1),
  frame_122_stepping_2,
  dx(1),
  frame_123_stepping_3,
  dx(3),
  frame_124_stepping_4,
  dx(4),
  frame_125_stepping_5,
  dx(3),
  frame_126_stepping_6,
  dx(-1),
  frame_127_stepping_7,
  frame_128_stepping_8,
  frame_129_stepping_9,
  frame_130_stepping_10,
  frame_131_stepping_11,
  frame_132_stepping_12,
  jmp(stand), // goto "stand"

  LABEL(step10) // step forward 10 pixels
  act(actions_1_run_jump),
  frame_121_stepping_1,
  dx(1),
  /* "step10a "*/ LABEL(step10a) frame_122_stepping_2,
  dx(1),
  frame_123_stepping_3,
  dx(3),
  frame_124_stepping_4,
  dx(4),
  frame_125_stepping_5,
  dx(3),
  frame_126_stepping_6,
  dx(-) 2,
  frame_128_stepping_8,
  frame_129_stepping_9,
  frame_130_stepping_10,
  frame_131_stepping_11,
  frame_132_stepping_12,
  jmp(stand), // goto "stand"

  LABEL(step9) // step forward 9 pixels
  act(actions_1_run_jump),
  frame_121_stepping_1,
  jmp(step10a), // goto "step10a"

  LABEL(step8) // step forward 8 pixels
  act(actions_1_run_jump),
  frame_121_stepping_1,
  dx(1),
  frame_122_stepping_2,
  dx(1),
  frame_123_stepping_3,
  dx(3),
  frame_124_stepping_4,
  dx(4),
  frame_125_stepping_5,
  dx(-1),
  frame_127_stepping_7,
  frame_128_stepping_8,
  frame_129_stepping_9,
  frame_130_stepping_10,
  frame_131_stepping_11,
  frame_132_stepping_12,
  jmp(stand), // goto "stand"

  LABEL(step7) // step forward 7 pixels
  act(actions_1_run_jump),
  frame_121_stepping_1,
  dx(1),
  frame_122_stepping_2,
  dx(1),
  frame_123_stepping_3,
  dx(3),
  frame_124_stepping_4,
  dx(2),
  frame_129_stepping_9,
  frame_130_stepping_10,
  frame_131_stepping_11,
  frame_132_stepping_12,
  jmp(stand), // goto "stand"

  LABEL(step6) // step forward 6 pixels
  act(actions_1_run_jump),
  frame_121_stepping_1,
  dx(1),
  frame_122_stepping_2,
  dx(1),
  frame_123_stepping_3,
  dx(2),
  frame_124_stepping_4,
  dx(2),
  frame_129_stepping_9,
  frame_130_stepping_10,
  frame_131_stepping_11,
  frame_132_stepping_12,
  jmp(stand), // goto "stand"

  LABEL(step5) // step forward 5 pixels
  act(actions_1_run_jump),
  frame_121_stepping_1,
  dx(1),
  frame_122_stepping_2,
  dx(1),
  frame_123_stepping_3,
  dx(2),
  frame_124_stepping_4,
  dx(1),
  frame_129_stepping_9,
  frame_130_stepping_10,
  frame_131_stepping_11,
  frame_132_stepping_12,
  jmp(stand), // goto "stand"

  LABEL(step4) // step forward 4 pixels
  act(actions_1_run_jump),
  frame_121_stepping_1,
  dx(1),
  frame_122_stepping_2,
  dx(1),
  frame_123_stepping_3,
  dx(2),
  frame_131_stepping_11,
  frame_132_stepping_12,
  jmp(stand), // goto "stand"

  LABEL(step3) // step forward 3 pixels
  act(actions_1_run_jump),
  frame_121_stepping_1,
  dx(1),
  frame_122_stepping_2,
  dx(1),
  frame_123_stepping_3,
  dx(1),
  frame_131_stepping_11,
  frame_132_stepping_12,
  jmp(stand), // goto "stand"

  LABEL(step2) // step forward 2 pixels
  act(actions_1_run_jump),
  frame_121_stepping_1,
  dx(1),
  frame_122_stepping_2,
  dx(1),
  frame_132_stepping_12,
  jmp(stand), // goto "stand"

  LABEL(step1) // step forward 1 pixel
  act(actions_1_run_jump),
  frame_121_stepping_1,
  dx(1),
  frame_132_stepping_12,
  jmp(stand), // goto "stand"

  LABEL(stoop) // stoop
  act(actions_1_run_jump),
  dx(1),
  frame_107_fall_land_1,
  dx(2),
  frame_108_fall_land_2,
  /* ":crouch" */ LABEL(stoop_crouch) frame_109_crouch,
  jmp(stoop_crouch), // goto ":crouch

  LABEL(standup) // stand up
  act(actions_5_bumped),
  dx(1),
  frame_110_stand_up_from_crouch_1,
  frame_111_stand_up_from_crouch_2,
  dx(2),
  frame_112_stand_up_from_crouch_3,
  frame_113_stand_up_from_crouch_4,
  dx(1),
  frame_114_stand_up_from_crouch_5,
  frame_115_stand_up_from_crouch_6,
  frame_116_stand_up_from_crouch_7,
  dx(-4),
  frame_117_stand_up_from_crouch_8,
  frame_118_stand_up_from_crouch_9,
  frame_119_stand_up_from_crouch_10,
  jmp(stand), // goto "stand"

  LABEL(pickupsword) // pick up sword
  act(actions_1_run_jump),
  SEQ_GET_ITEM,
  1,
  frame_229_found_sword,
  frame_229_found_sword,
  frame_229_found_sword,
  frame_229_found_sword,
  frame_229_found_sword,
  frame_229_found_sword,
  frame_230_sheathe,
  frame_231_sheathe,
  frame_232_sheathe,
  jmp(resheathe), // goto "resheathe"

  LABEL(resheathe) // resheathe
  act(actions_1_run_jump),
  dx(-5),
  frame_233_sheathe,
  frame_234_sheathe,
  frame_235_sheathe,
  frame_236_sheathe,
  frame_237_sheathe,
  frame_238_sheathe,
  frame_239_sheathe,
  frame_240_sheathe,
  frame_133_sheathe,
  frame_133_sheathe,
  frame_134_sheathe,
  frame_134_sheathe,
  frame_134_sheathe,
  frame_48_turn,
  dx(1),
  frame_49_turn,
  dx(-2),
  act(actions_5_bumped),
  frame_50_turn,
  act(actions_1_run_jump),
  frame_51_turn,
  frame_52_turn,
  jmp(stand), // goto "stand"

  LABEL(fastsheathe) // fast sheathe
  act(actions_1_run_jump),
  dx(-5),
  frame_234_sheathe,
  frame_236_sheathe,
  frame_238_sheathe,
  frame_240_sheathe,
  frame_134_sheathe,
  dx(-1),
  jmp(stand), // goto "stand"

  LABEL(drinkpotion) // drink potion
  act(actions_1_run_jump),
  dx(4),
  frame_191_drink,
  frame_192_drink,
  frame_193_drink,
  frame_194_drink,
  frame_195_drink,
  frame_196_drink,
  frame_197_drink,
  snd(SND_DRINK),
  frame_198_drink,
  frame_199_drink,
  frame_200_drink,
  frame_201_drink,
  frame_202_drink,
  frame_203_drink,
  frame_204_drink,
  frame_205_drink,
  frame_205_drink,
  frame_205_drink,
  SEQ_GET_ITEM,
  1,
  frame_205_drink,
  frame_205_drink,
  frame_201_drink,
  frame_198_drink,
  dx(-4),
  jmp(stand), // goto "stand"

  LABEL(softland) // soft land
  act(actions_5_bumped),
  SEQ_KNOCK_DOWN,
  dx(1),
  frame_107_fall_land_1,
  dx(2),
  frame_108_fall_land_2,
  act(actions_1_run_jump),
  /* ":crouch" */ LABEL(softland_crouch) frame_109_crouch,
  jmp(softland_crouch), // goto ":crouch"

  LABEL(landrun) // land run
  act(actions_1_run_jump),
  dy(-2),
  dx(1),
  frame_107_fall_land_1,
  dx(2),
  frame_108_fall_land_2,
  frame_109_crouch,
  dx(1),
  frame_110_stand_up_from_crouch_1,
  frame_111_stand_up_from_crouch_2,
  dx(2),
  frame_112_stand_up_from_crouch_3,
  frame_113_stand_up_from_crouch_4,
  dx(1),
  dy(1),
  frame_114_stand_up_from_crouch_5,
  dy(1),
  frame_115_stand_up_from_crouch_6,
  dx(-2),
  jmp(runstt4), // goto "runstt4"

  LABEL(medland) // medium land (1.5 - 2 stories)
  act(actions_5_bumped),
  SEQ_KNOCK_DOWN,
  dy(-2),
  dx(1),
  dx(2),
  frame_108_fall_land_2,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  frame_109_crouch,
  dx(1),
  frame_110_stand_up_from_crouch_1,
  frame_110_stand_up_from_crouch_1,
  frame_110_stand_up_from_crouch_1,
  frame_111_stand_up_from_crouch_2,
  dx(2),
  frame_112_stand_up_from_crouch_3,
  frame_113_stand_up_from_crouch_4,
  dx(1),
  dy(1),
  frame_114_stand_up_from_crouch_5,
  dy(1),
  frame_115_stand_up_from_crouch_6,
  frame_116_stand_up_from_crouch_7,
  dx(-4),
  frame_117_stand_up_from_crouch_8,
  frame_118_stand_up_from_crouch_9,
  frame_119_stand_up_from_crouch_10,
  jmp(stand), // goto "stand"

  LABEL(hardland) // hard land (splat!; >2 stories)
  act(actions_5_bumped),
  SEQ_KNOCK_DOWN,
  dy(-2),
  dx(3),
  frame_185_dead,
  SEQ_DIE,
  /* ":dead" */ LABEL(hardland_dead) frame_185_dead,
  jmp(hardland_dead), // goto ":dead"

  LABEL(stabkill) // stabkill
  act(actions_5_bumped),
  jmp(dropdead), // goto "dropdead"

  LABEL(dropdead) // dropdead
  act(actions_1_run_jump),
  SEQ_DIE,
  frame_179_collapse_1,
  frame_180_collapse_2,
  frame_181_collapse_3,
  frame_182_collapse_4,
  dx(1),
  frame_183_collapse_5,
  dx(-4),
  /* ":dead" */ LABEL(dropdead_dead) frame_185_dead,
  jmp(dropdead_dead), // goto ":dead"

  LABEL(impale) // impale
  act(actions_1_run_jump),
  SEQ_KNOCK_DOWN,
  dx(4),
  frame_177_spiked,
  SEQ_DIE,
  /* ":dead" */ LABEL(impale_dead) frame_177_spiked,
  jmp(impale_dead), // goto ":dead"

  LABEL(halve) // halve
  act(actions_1_run_jump),
  frame_178_chomped,
  SEQ_DIE,
  /* ":dead" */ LABEL(halve_dead) frame_178_chomped,
  jmp(halve_dead), // goto ":dead"

  LABEL(crush)  // crush
  jmp(medland), // goto "medland"

  LABEL(deadfall) // deadfall
  set_fall(0, 0),
  act(actions_4_in_freefall),
  /* ":loop"*/ LABEL(deadfall_loop) frame_185_dead,
  jmp(deadfall_loop), // goto ":loop"

  LABEL(climbstairs) // climb stairs
  act(actions_5_bumped),
  dx(-5),
  dy(-1),
  snd(SND_FOOTSTEP),
  frame_217_exit_stairs_1,
  frame_218_exit_stairs_2,
  frame_219_exit_stairs_3,
  dx(1),
  frame_220_exit_stairs_4,
  dx(-4),
  dy(-3),
  snd(SND_FOOTSTEP),
  frame_221_exit_stairs_5,
  dx(-4),
  dy(-2),
  frame_222_exit_stairs_6,
  dx(-2),
  dy(-3),
  frame_223_exit_stairs_7,
  dx(-3),
  dy(-8),
  snd(SND_LEVEL),
  snd(SND_FOOTSTEP),
  frame_224_exit_stairs_8,
  dx(-1),
  dy(-1),
  frame_225_exit_stairs_9,
  dx(-3),
  dy(-4),
  frame_226_exit_stairs_10,
  dx(-1),
  dy(-5),
  snd(SND_FOOTSTEP),
  frame_227_exit_stairs_11,
  dx(-2),
  dy(-1),
  frame_228_exit_stairs_12,
  frame_0,
  snd(SND_FOOTSTEP),
  frame_0,
  frame_0,
  frame_0, // these footsteps are only heard when the music is off
  snd(SND_FOOTSTEP),
  frame_0,
  frame_0,
  frame_0,
  snd(SND_FOOTSTEP),
  frame_0,
  frame_0,
  frame_0,
  snd(SND_FOOTSTEP),
  SEQ_END_LEVEL,
  /* ":loop" */ LABEL(climbstairs_loop) frame_0,
  jmp(climbstairs_loop), // goto ":loop"

  LABEL(Vstand) // Vizier: stand
  alt2frame_54_Vstand,
  jmp(Vstand), // goto "Vstand"

  LABEL(Vraise) // Vizier: raise arms
  85,
  67,
  67,
  67, // numbers refer to frames in the "alternate" frame sets
  67,
  67,
  67,
  67,
  67,
  67,
  67,
  68,
  69,
  70,
  71,
  72,
  73,
  74,
  75,
  83,
  84,
  /* ":loop" */ LABEL(Vraise_loop) 76,
  jmp(Vraise_loop), // goto ":loop"

  LABEL(Vwalk) // Vizier: walk
  dx(1),
  /* "Vwalk1" */ LABEL(Vwalk1) 48,
  dx(2),
  /* "Vwalk2" */ LABEL(Vwalk2) 49,
  dx(6),
  50,
  dx(1),
  51,
  dx(-1),
  52,
  dx(1),
  53,
  dx(1),
  jmp(Vwalk1), // goto "Vwalk1"

  LABEL(Vstop) // Vizier: stop
  dx(1),
  55,
  56,
  jmp(Vstand),

  LABEL(Vexit) // Vizier: lower arms, turn & exit ("Vexit")
  77,
  78,
  79,
  80,
  81,
  82,
  dx(1),
  54,
  54,
  54,
  54,
  54,
  54,
  57,
  58,
  59,
  60,
  61,
  dx(2),
  62,
  dx(-1),
  63,
  dx(-3),
  64,
  65,
  dx(-1),
  66,
  SEQ_FLIP,
  dx(16),
  dx(3),
  jmp(Vwalk2), // goto "Vwalk2"

  // Princess: stand
  LABEL(Pstand) 11,
  jmp(Pstand), // goto "Pstand"

  LABEL(Palert) // Princess: alert
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  SEQ_FLIP,
  dx(8),
  11,
  jmp(Pstand),

  LABEL(Pstepback) // Princess: step back
  SEQ_FLIP,
  dx(11),
  12,
  dx(1),
  13,
  dx(1),
  14,
  dx(3),
  15,
  dx(1),
  16,
  /* ":loop" */ LABEL(Pstepback_loop) 17,
  jmp(Pstepback_loop), // goto ":loop"

  LABEL(Plie) // Princess lying on cushions ("Plie")
  19,
  jmp(Plie), // goto "Plie"

  LABEL(Pwaiting) // Princess: waiting
  /* ":loop" */ 20,
  jmp(Pwaiting), // goto ":loop"

  LABEL(Pembrace) // Princess: embrace
  21,
  dx(1),
  22,
  23,
  24,
  dx(1),
  25,
  dx(-3),
  26,
  dx(-2),
  27,
  dx(-4),
  28,
  dx(-3),
  29,
  dx(-2),
  30,
  dx(-3),
  31,
  dx(-1),
  32,
  /* ":loop" */ LABEL(Pembrace_loop) 33,
  jmp(Pembrace_loop), // goto ":loop"

  LABEL(Pstroke) // Princess: stroke mouse
  /* ":loop" */ 37,
  jmp(Pstroke), // goto ":loop"

  LABEL(Prise) // Princess: rise
  37,
  38,
  39,
  40,
  41,
  42,
  43,
  44,
  45,
  46,
  47,
  SEQ_FLIP,
  dx(12),
  /* ":loop" */ LABEL(Prise_loop) 11,
  jmp(Prise_loop), // goto ":loop"

  LABEL(Pcrouch) // Princess: crouch & stroke mouse
  11,
  11,
  SEQ_FLIP,
  dx(13),
  47,
  46,
  45,
  44,
  43,
  42,
  41,
  40,
  39,
  38,
  37,
  36,
  36,
  36,
  35,
  35,
  35,
  34,
  34,
  34,
  34,
  34,
  34,
  34,
  35,
  35,
  36,
  36,
  36,
  35,
  35,
  35,
  34,
  34,
  34,
  34,
  34,
  34,
  34,
  35,
  35,
  36,
  36,
  36,
  35,
  35,
  35,
  34,
  34,
  34,
  34,
  34,
  34,
  34,
  34,
  34,
  35,
  35,
  35,
  /* ":loop" */ LABEL(Pcrouch_loop) 36,
  jmp(Pcrouch_loop), // goto ":loop"

  LABEL(Pslump) // Princess: slump shoulders
  1,
  /* ":loop" */ LABEL(Pslump_loop) 18,
  jmp(Pslump_loop), // goto ":loop"

  LABEL(Mscurry) // Mouse: scurry
  act(actions_1_run_jump),
  /* "Mscurry1" */ LABEL(Mscurry1) frame_186_mouse_1,
  dx(5),
  frame_186_mouse_1,
  dx(3),
  frame_187_mouse_2,
  dx(4),
  jmp(Mscurry1), // goto "Mscurry1"

  LABEL(Mstop) // Mouse: stop
  /* ":loop" */ frame_186_mouse_1,
  jmp(Mstop), // goto ":loop"

  LABEL(Mraise) // Mouse: raise head
  /* ":loop" */ frame_188_mouse_stand,
  jmp(Mraise), // goto ":loop"

  LABEL(Mleave) // Mouse: leave
  act(actions_0_stand),
  frame_186_mouse_1,
  frame_186_mouse_1,
  frame_186_mouse_1,
  frame_188_mouse_stand,
  frame_188_mouse_stand,
  frame_188_mouse_stand,
  frame_188_mouse_stand,
  frame_188_mouse_stand,
  frame_188_mouse_stand,
  frame_188_mouse_stand,
  frame_188_mouse_stand,
  SEQ_FLIP,
  dx(8),
  jmp(Mscurry1), // goto "Mscurry1"

  LABEL(Mclimb) // Mouse: climb
  frame_186_mouse_1,
  frame_186_mouse_1,
  /* ":loop" */ LABEL(Mclimb_loop) frame_188_mouse_stand,
  jmp(Mclimb_loop) // goto ":loop"

};
extern const byte seqtbl[]; // the sequence table is defined in seqtbl.c
extern const word seqtbl_offsets[];
static const byte optgraf_min[] = {0x01, 0x1E, 0x4B, 0x4E, 0x56, 0x65, 0x7F, 0x0A};
static const byte optgraf_max[] = {0x09, 0x1F, 0x4D, 0x53, 0x5B, 0x7B, 0x8F, 0x0D};

static const char *const tbl_guard_dat[] = {"GUARD.DAT", "FAT.DAT", "SKEL.DAT", "VIZIER.DAT", "SHADOW.DAT"};
static const char *const tbl_envir_gr[] = {"", "C", "C", "E", "E", "V"};
static const char *const tbl_envir_ki[] = {"DUNGEON", "PALACE"};
static const rect_type rect_titles = {106, 24, 195, 296};

static const short y_something[] = {-1, 62, 125, 188, 25};
static const byte loose_sound[] = {0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0};
static const word y_loose_land[] = {2, 65, 128, 191, 254};
static const byte leveldoor_close_speeds[] = {0, 5, 17, 99, 0};
static const byte gate_close_speeds[] = {0, 0, 0, 20, 40, 60, 80, 100, 120};
static const byte door_delta[] = {-1, 4, 4};
static const byte door_fram_slice[] = {67, 59, 58, 57, 56, 55, 54, 53, 52};
static const word floor_left_overlay[] = {32, 151, 151, 150, 150, 151, 32, 32};
static const byte spikes_fram_fore[] = {0, 139, 140, 141, 142, 143, 142, 140, 139, 0};
static const byte chomper_fram_for[] = {106, 107, 108, 109, 110, 0};
static const byte wall_fram_main[] = {8, 10, 6, 4};
static const byte spikes_fram_left[] = {0, 128, 129, 130, 131, 132, 131, 129, 128, 0};
static const byte potion_fram_bubb[] = {0, 16, 17, 18, 19, 20, 21, 22};
static const byte chomper_fram1[] = {3, 2, 0, 1, 4, 3, 3, 0};
static const byte chomper_fram_bot[] = {101, 102, 103, 104, 105, 0};
static const byte chomper_fram_top[] = {0, 0, 111, 112, 113, 0};
static const byte chomper_fram_y[] = {0, 0, 0x25, 0x2F, 0x32};
static const byte loose_fram_left[] = {41, 69, 41, 70, 70, 41, 41, 41, 70, 70, 70, 0};
static const byte loose_fram_bottom[] = {43, 73, 43, 74, 74, 43, 43, 43, 74, 74, 74, 0};
static const byte wall_fram_bottom[] = {7, 9, 5, 3};
static const byte spikes_fram_right[] = {0, 134, 135, 136, 137, 138, 137, 135, 134, 0};
static const byte loose_fram_right[] = {42, 71, 42, 72, 72, 42, 42, 42, 72, 72, 72, 0};
static const byte blueline_fram1[] = {0, 124, 125, 126};
static const sbyte blueline_fram_y[] = {0, -20, -20, 0};
static const byte blueline_fram3[] = {44, 44, 45, 45};
static const byte doortop_fram_bot[] = {78, 80, 82, 0};
static const byte door_fram_top[] = {60, 61, 62, 63, 64, 65, 66, 67};
static const byte doortop_fram_top[] = {0, 81, 83, 0};
static const word col_xh[] = {0, 4, 8, 12, 16, 20, 24, 28, 32, 36};

static const piece tile_table[31] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},        // 0x00 empty
  {41, 1, 0, 42, 1, 2, 145, 0, 43, 0, 0, 0},   // 0x01 floor
  {127, 1, 0, 133, 1, 2, 145, 0, 43, 0, 0, 0}, // 0x02 spike
  {92, 1, 0, 93, 1, 2, 0, 94, 43, 95, 1, 0},   // 0x03 pillar
  {46, 1, 0, 47, 1, 2, 0, 48, 43, 49, 3, 0},   // 0x04 door
  {41, 1, 1, 35, 1, 3, 145, 0, 36, 0, 0, 0},   // 0x05 stuck floor
  {41, 1, 0, 42, 1, 2, 145, 0, 96, 0, 0, 0},   // 0x06 close button
  {46, 1, 0, 0, 0, 2, 0, 0, 43, 49, 3, 0},     // 0x07 door top with floor
  {86, 1, 0, 87, 1, 2, 0, 0, 43, 88, 1, 0},    // 0x08 big pillar bottom
  {0, 0, 0, 89, 0, 3, 0, 90, 0, 91, 1, 3},     // 0x09 big pillar top
  {41, 1, 0, 42, 1, 2, 145, 0, 43, 12, 2, -3}, // 0x0A potion
  {0, 1, 0, 0, 0, 0, 145, 0, 0, 0, 0, 0},      // 0x0B loose floor
  {0, 0, 0, 0, 0, 2, 0, 0, 85, 49, 3, 0},      // 0x0C door top
  {75, 1, 0, 42, 1, 2, 0, 0, 43, 77, 0, 0},    // 0x0D mirror
  {97, 1, 0, 98, 1, 2, 145, 0, 43, 100, 0, 0}, // 0x0E debris
  {147, 1, 0, 42, 1, 1, 145, 0, 149, 0, 0, 0}, // 0x0F open button
  {41, 1, 0, 37, 0, 0, 0, 38, 43, 0, 0, 0},    // 0x10 leveldoor left
  {0, 0, 0, 39, 1, 2, 0, 40, 43, 0, 0, 0},     // 0x11 leveldoor right
  {0, 0, 0, 42, 1, 2, 145, 0, 43, 0, 0, 0},    // 0x12 chomper
  {41, 1, 0, 42, 1, 2, 0, 0, 43, 0, 0, 0},     // 0x13 torch
  {0, 0, 0, 1, 1, 2, 0, 2, 0, 0, 0, 0},        // 0x14 wall
  {30, 1, 0, 31, 1, 2, 0, 0, 43, 0, 0, 0},     // 0x15 skeleton
  {41, 1, 0, 42, 1, 2, 145, 0, 43, 0, 0, 0},   // 0x16 sword
  {41, 1, 0, 10, 0, 0, 0, 11, 43, 0, 0, 0},    // 0x17 balcony left
  {0, 0, 0, 12, 1, 2, 0, 13, 43, 0, 0, 0},     // 0x18 balcony right
  {92, 1, 0, 42, 1, 2, 145, 0, 43, 95, 1, 0},  // 0x19 lattice pillar
  {1, 0, 0, 0, 0, 0, 0, 0, 2, 9, 0, -53},      // 0x1A lattice down
  {3, 0, -10, 0, 0, 0, 0, 0, 0, 9, 0, -53},    // 0x1B lattice small
  {4, 0, -10, 0, 0, 0, 0, 0, 0, 9, 0, -53},    // 0x1C lattice left
  {5, 0, -10, 0, 0, 0, 0, 0, 0, 9, 0, -53},    // 0x1D lattice right
  {97, 1, 0, 98, 1, 2, 0, 0, 43, 100, 0, 0},   // 0x1E debris with torch
};

static const byte sound_prio_table[] = {
  0x14, // sound_0_fell_to_death
  0x1E, // sound_1_falling
  0x23, // sound_2_tile_crashing
  0x66, // sound_3_button_pressed
  0x32, // sound_4_gate_closing
  0x37, // sound_5_gate_opening
  0x30, // sound_6_gate_closing_fast
  0x30, // sound_7_gate_stop
  0x4B, // sound_8_bumped
  0x50, // sound_9_grab
  0x0A, // sound_10_sword_vs_sword
  0x12, // sound_11_sword_moving
  0x0C, // sound_12_guard_hurt
  0x0B, // sound_13_kid_hurt
  0x69, // sound_14_leveldoor_closing
  0x6E, // sound_15_leveldoor_sliding
  0x73, // sound_16_medium_land
  0x78, // sound_17_soft_land
  0x7D, // sound_18_drink
  0x82, // sound_19_draw_sword
  0x91, // sound_20_loose_shake_1
  0x96, // sound_21_loose_shake_2
  0x9B, // sound_22_loose_shake_3
  0xA0, // sound_23_footstep
  0x01, // sound_24_death_regular
  0x01, // sound_25_presentation
  0x01, // sound_26_embrace
  0x01, // sound_27_cutscene_2_4_6_12
  0x01, // sound_28_death_in_fight
  0x13, // sound_29_meet_Jaffar
  0x01, // sound_30_big_potion
  0x01, // sound_31
  0x01, // sound_32_shadow_music
  0x01, // sound_33_small_potion
  0x01, // sound_34
  0x01, // sound_35_cutscene_8_9
  0x01, // sound_36_out_of_time
  0x01, // sound_37_victory
  0x01, // sound_38_blink
  0x00, // sound_39_low_weight
  0x01, // sound_40_cutscene_12_short_time
  0x01, // sound_41_end_level_music
  0x01, // sound_42
  0x01, // sound_43_victory_Jaffar
  0x87, // sound_44_skel_alive
  0x8C, // sound_45_jump_through_mirror
  0x0F, // sound_46_chomped
  0x10, // sound_47_chomper
  0x19, // sound_48_spiked
  0x16, // sound_49_spikes
  0x01, // sound_50_story_2_princess
  0x00, // sound_51_princess_door_opening
  0x01, // sound_52_story_4_Jaffar_leaves
  0x01, // sound_53_story_3_Jaffar_comes
  0x01, // sound_54_intro_music
  0x01, // sound_55_story_1_absence
  0x01, // sound_56_ending_music
  0x00};

static const byte sound_pcspeaker_exists[] = {
  1, // sound_0_fell_to_death
  0, // sound_1_falling
  1, // sound_2_tile_crashing
  1, // sound_3_button_pressed
  1, // sound_4_gate_closing
  1, // sound_5_gate_opening
  1, // sound_6_gate_closing_fast
  1, // sound_7_gate_stop
  1, // sound_8_bumped
  1, // sound_9_grab
  1, // sound_10_sword_vs_sword
  0, // sound_11_sword_moving
  1, // sound_12_guard_hurt
  1, // sound_13_kid_hurt
  1, // sound_14_leveldoor_closing
  1, // sound_15_leveldoor_sliding
  1, // sound_16_medium_land
  1, // sound_17_soft_land
  1, // sound_18_drink
  0, // sound_19_draw_sword
  0, // sound_20_loose_shake_1
  0, // sound_21_loose_shake_2
  0, // sound_22_loose_shake_3
  1, // sound_23_footstep
  1, // sound_24_death_regular
  1, // sound_25_presentation
  1, // sound_26_embrace
  1, // sound_27_cutscene_2_4_6_12
  1, // sound_28_death_in_fight
  1, // sound_29_meet_Jaffar
  1, // sound_30_big_potion
  1, // sound_31
  1, // sound_32_shadow_music
  1, // sound_33_small_potion
  1, // sound_34
  1, // sound_35_cutscene_8_9
  1, // sound_36_out_of_time
  1, // sound_37_victory
  1, // sound_38_blink
  1, // sound_39_low_weight
  1, // sound_40_cutscene_12_short_time
  1, // sound_41_end_level_music
  1, // sound_42
  1, // sound_43_victory_Jaffar
  1, // sound_44_skel_alive
  1, // sound_45_jump_through_mirror
  1, // sound_46_chomped
  1, // sound_47_chomper
  1, // sound_48_spiked
  1, // sound_49_spikes
  1, // sound_50_story_2_princess
  1, // sound_51_princess_door_opening
  1, // sound_52_story_4_Jaffar_leaves
  1, // sound_53_story_3_Jaffar_comes
  1, // sound_54_intro_music
  1, // sound_55_story_1_absence
  1, // sound_56_ending_music
  0};

static char const *const tbl_quotes[2] = {
  "\"(****/****) Incredibly realistic. . . The "
  "adventurer character actually looks human as he "
  "runs, jumps, climbs, and hangs from ledges.\"\n"
  "\n"
  "                                  Computer Entertainer\n"
  "\n"
  "\n"
  "\n"
  "\n"
  "\"A tremendous achievement. . . Mechner has crafted "
  "the smoothest animation ever seen in a game of this "
  "type.\n"
  "\n"
  "\"PRINCE OF PERSIA is the STAR WARS of its field.\"\n"
  "\n"
  "                                  Computer Gaming World",
  "\"An unmitigated delight. . . comes as close to "
  "(perfection) as any arcade game has come in a long, "
  "long time. . . what makes this game so wonderful (am "
  "I gushing?) is that the little onscreen character "
  "does not move like a little onscreen character -- he "
  "moves like a person.\"\n"
  "\n"
  "                                      Nibble"};

#ifdef USE_COPYPROT
// data:017A
static const word copyprot_word[] = {9, 1, 6, 4, 5, 3, 6, 3, 4, 4, 3, 2, 12, 5, 13, 1, 9, 2, 2, 4, 9, 4, 11, 8, 5, 4, 1, 6, 2, 4, 6, 8, 4, 2, 7, 11, 5, 4, 1, 2};
// data:012A
static const word copyprot_line[] = {2, 1, 5, 4, 3, 5, 1, 3, 7, 2, 2, 4, 6, 6, 2, 6, 3, 1, 2, 3, 2, 2, 3, 10, 5, 6, 5, 6, 3, 5, 7, 2, 2, 4, 5, 7, 2, 6, 5, 5};
// data:00DA
static const word copyprot_page[] = {5, 3, 7, 3, 3, 4, 1, 5, 12, 5, 11, 10, 1, 2, 8, 8, 2, 4, 6, 1, 4, 7, 3, 2, 1, 7, 10, 1, 4, 3, 4, 1, 4, 1, 8, 1, 1, 10, 3, 3};
#endif

static const sbyte wall_dist_from_left[] = {0, 10, 0, -1, 0, 0};
static const sbyte wall_dist_from_right[] = {0, 0, 10, 13, 0, 0};

// data:1712
static const sword_table_type sword_tbl[] = {
  {255, 0, 0},
  {0, 0, -9},
  {5, -9, -29},
  {1, 7, -25},
  {2, 17, -26},
  {6, 7, -14},
  {7, 0, -5},
  {3, 17, -16},
  {4, 16, -19},
  {30, 12, -9},
  {8, 13, -34},
  {9, 7, -25},
  {10, 10, -16},
  {11, 10, -11},
  {12, 22, -21},
  {13, 28, -23},
  {14, 13, -35},
  {15, 0, -38},
  {16, 0, -29},
  {17, 21, -19},
  {18, 14, -23},
  {19, 21, -22},
  {19, 22, -23},
  {17, 7, -13},
  {17, 15, -18},
  {7, 0, -8},
  {1, 7, -27},
  {28, 14, -28},
  {8, 7, -27},
  {4, 6, -23},
  {4, 9, -21},
  {10, 11, -18},
  {13, 24, -23},
  {13, 19, -23},
  {13, 21, -23},
  {20, 7, -32},
  {21, 14, -32},
  {22, 14, -31},
  {23, 14, -29},
  {24, 28, -28},
  {25, 28, -28},
  {26, 21, -25},
  {27, 14, -22},
  {255, 14, -25},
  {255, 21, -25},
  {29, 0, -16},
  {8, 8, -37},
  {31, 14, -24},
  {32, 14, -24},
  {33, 7, -14},
  {8, 8, -37},
};

// data:22A6
static const sbyte tile_div_tbl[256] = {
  -5, -5, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14};

// data:23A6
static const byte tile_mod_tbl[256] = {
  12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1};

// data:0FE0
static const frame_type frame_table_kid[] = {
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {0, 0x00 | 0, 1, 0, 0xC0 | 4},
  {1, 0x00 | 0, 1, 0, 0x40 | 4},
  {2, 0x00 | 0, 3, 0, 0x40 | 7},
  {3, 0x00 | 0, 4, 0, 0x40 | 8},
  {4, 0x00 | 0, 0, 0, 0xE0 | 6},
  {5, 0x00 | 0, 0, 0, 0x40 | 9},
  {6, 0x00 | 0, 0, 0, 0x40 | 10},
  {7, 0x00 | 0, 0, 0, 0xC0 | 5},
  {8, 0x00 | 0, 0, 0, 0x40 | 4},
  {9, 0x00 | 0, 0, 0, 0x40 | 7},
  {10, 0x00 | 0, 0, 0, 0x40 | 11},
  {11, 0x00 | 0, 0, 0, 0x40 | 3},
  {12, 0x00 | 0, 0, 0, 0xC0 | 3},
  {13, 0x00 | 0, 0, 0, 0x40 | 7},
  {14, 0x00 | 9, 0, 0, 0x40 | 3},
  {15, 0x00 | 0, 0, 0, 0xC0 | 3},
  {16, 0x00 | 0, 0, 0, 0x40 | 4},
  {17, 0x00 | 0, 0, 0, 0x40 | 6},
  {18, 0x00 | 0, 0, 0, 0x40 | 8},
  {19, 0x00 | 0, 0, 0, 0x80 | 9},
  {20, 0x00 | 0, 0, 0, 0x00 | 11},
  {21, 0x00 | 0, 0, 0, 0x80 | 11},
  {22, 0x00 | 0, 0, 0, 0x00 | 17},
  {23, 0x00 | 0, 0, 0, 0x00 | 7},
  {24, 0x00 | 0, 0, 0, 0x00 | 5},
  {25, 0x00 | 0, 0, 0, 0xC0 | 1},
  {26, 0x00 | 0, 0, 0, 0xC0 | 6},
  {27, 0x00 | 0, 0, 0, 0x40 | 3},
  {28, 0x00 | 0, 0, 0, 0x40 | 8},
  {29, 0x00 | 0, 0, 0, 0x40 | 2},
  {30, 0x00 | 0, 0, 0, 0x40 | 2},
  {31, 0x00 | 0, 0, 0, 0xC0 | 2},
  {32, 0x00 | 0, 0, 0, 0xC0 | 2},
  {33, 0x00 | 0, 0, 0, 0x40 | 3},
  {34, 0x00 | 0, 0, 0, 0x40 | 8},
  {35, 0x00 | 0, 0, 0, 0xC0 | 14},
  {36, 0x00 | 0, 0, 0, 0xC0 | 1},
  {37, 0x00 | 0, 0, 0, 0x40 | 5},
  {38, 0x00 | 0, 0, 0, 0x80 | 14},
  {39, 0x00 | 0, 0, 0, 0x00 | 11},
  {40, 0x00 | 0, 0, 0, 0x80 | 11},
  {41, 0x00 | 0, 0, 0, 0x80 | 10},
  {42, 0x00 | 0, 0, 0, 0x00 | 1},
  {43, 0x00 | 0, 0, 0, 0xC0 | 4},
  {44, 0x00 | 0, 0, 0, 0xC0 | 3},
  {45, 0x00 | 0, 0, 0, 0xC0 | 3},
  {46, 0x00 | 0, 0, 0, 0xA0 | 5},
  {47, 0x00 | 0, 0, 0, 0xA0 | 4},
  {48, 0x00 | 0, 0, 0, 0x60 | 6},
  {49, 0x00 | 0, 4, 0, 0x60 | 7},
  {50, 0x00 | 0, 3, 0, 0x60 | 6},
  {51, 0x00 | 0, 1, 0, 0x40 | 4},
  {64, 0x00 | 0, 0, 0, 0xC0 | 2},
  {65, 0x00 | 0, 0, 0, 0x40 | 1},
  {66, 0x00 | 0, 0, 0, 0x40 | 2},
  {67, 0x00 | 0, 0, 0, 0x00 | 0},
  {68, 0x00 | 0, 0, 0, 0x00 | 0},
  {69, 0x00 | 0, 0, 0, 0x80 | 0},
  {70, 0x00 | 0, 0, 0, 0x00 | 0},
  {71, 0x00 | 0, 0, 0, 0x80 | 0},
  {72, 0x00 | 0, 0, 0, 0x00 | 0},
  {73, 0x00 | 0, 0, 0, 0x80 | 0},
  {74, 0x00 | 0, 0, 0, 0x00 | 0},
  {75, 0x00 | 0, 0, 0, 0x00 | 0},
  {76, 0x00 | 0, 0, 0, 0x80 | 0},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {80, 0x00 | 0, -2, 0, 0x40 | 1},
  {81, 0x00 | 0, -2, 0, 0x40 | 1},
  {82, 0x00 | 0, -1, 0, 0xC0 | 2},
  {83, 0x00 | 0, -2, 0, 0x40 | 2},
  {84, 0x00 | 0, -2, 0, 0x40 | 1},
  {85, 0x00 | 0, -2, 0, 0x40 | 1},
  {86, 0x00 | 0, -2, 0, 0x40 | 1},
  {87, 0x00 | 0, -1, 0, 0x00 | 7},
  {88, 0x00 | 0, -1, 0, 0x00 | 5},
  {89, 0x00 | 0, 2, 0, 0x00 | 7},
  {90, 0x00 | 0, 2, 0, 0x00 | 7},
  {91, 0x00 | 0, 2, -3, 0x00 | 0},
  {92, 0x00 | 0, 2, -10, 0x00 | 0},
  {93, 0x00 | 0, 2, -11, 0x80 | 0},
  {94, 0x00 | 0, 3, -2, 0x40 | 3},
  {95, 0x00 | 0, 3, 0, 0xC0 | 3},
  {96, 0x00 | 0, 3, 0, 0xC0 | 3},
  {97, 0x00 | 0, 3, 0, 0x60 | 3},
  {98, 0x00 | 0, 4, 0, 0xE0 | 3},
  {28, 0x00 | 0, 0, 0, 0x00 | 0},
  {99, 0x00 | 0, 7, -14, 0x80 | 0},
  {100, 0x00 | 0, 7, -12, 0x80 | 0},
  {101, 0x00 | 0, 4, -12, 0x00 | 0},
  {102, 0x00 | 0, 3, -10, 0x80 | 0},
  {103, 0x00 | 0, 2, -10, 0x80 | 0},
  {104, 0x00 | 0, 1, -10, 0x80 | 0},
  {105, 0x00 | 0, 0, -11, 0x00 | 0},
  {106, 0x00 | 0, -1, -12, 0x00 | 0},
  {107, 0x00 | 0, -1, -14, 0x00 | 0},
  {108, 0x00 | 0, -1, -14, 0x00 | 0},
  {109, 0x00 | 0, -1, -15, 0x80 | 0},
  {110, 0x00 | 0, -1, -15, 0x80 | 0},
  {111, 0x00 | 0, 0, -15, 0x00 | 0},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {112, 0x00 | 0, 0, 0, 0xC0 | 6},
  {113, 0x00 | 0, 0, 0, 0x40 | 6},
  {114, 0x00 | 0, 0, 0, 0xC0 | 5},
  {115, 0x00 | 0, 0, 0, 0x40 | 5},
  {116, 0x00 | 0, 0, 0, 0xC0 | 2},
  {117, 0x00 | 0, 0, 0, 0xC0 | 4},
  {118, 0x00 | 0, 0, 0, 0xC0 | 5},
  {119, 0x00 | 0, 0, 0, 0x40 | 6},
  {120, 0x00 | 0, 0, 0, 0x40 | 7},
  {121, 0x00 | 0, 0, 0, 0x40 | 7},
  {122, 0x00 | 0, 0, 0, 0x40 | 9},
  {123, 0x00 | 0, 0, 0, 0xC0 | 8},
  {124, 0x00 | 0, 0, 0, 0xC0 | 9},
  {125, 0x00 | 0, 0, 0, 0x40 | 9},
  {126, 0x00 | 0, 0, 0, 0x40 | 5},
  {127, 0x00 | 0, 2, 0, 0x40 | 5},
  {128, 0x00 | 0, 2, 0, 0xC0 | 5},
  {129, 0x00 | 0, 0, 0, 0xC0 | 3},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {133, 0x00 | 0, 0, 0, 0x40 | 3},
  {134, 0x00 | 0, 0, 0, 0xC0 | 4},
  {135, 0x00 | 0, 0, 0, 0xC0 | 5},
  {136, 0x00 | 0, 0, 0, 0x40 | 8},
  {137, 0x00 | 0, 0, 0, 0x60 | 12},
  {138, 0x00 | 0, 0, 0, 0xE0 | 15},
  {139, 0x00 | 0, 0, 0, 0x60 | 3},
  {140, 0x00 | 0, 0, 0, 0xC0 | 3},
  {141, 0x00 | 0, 0, 0, 0x40 | 3},
  {142, 0x00 | 0, 0, 0, 0x40 | 3},
  {143, 0x00 | 0, 0, 0, 0x40 | 4},
  {144, 0x00 | 0, 0, 0, 0x40 | 4},
  {172, 0x00 | 0, 0, 1, 0xC0 | 1},
  {173, 0x00 | 0, 0, 1, 0xC0 | 7},
  {145, 0x00 | 0, 0, -12, 0x00 | 1},
  {146, 0x00 | 0, 0, -21, 0x00 | 0},
  {147, 0x00 | 0, 1, -26, 0x80 | 0},
  {148, 0x00 | 0, 4, -32, 0x80 | 0},
  {149, 0x00 | 0, 6, -36, 0x80 | 1},
  {150, 0x00 | 0, 7, -41, 0x80 | 2},
  {151, 0x00 | 0, 2, 17, 0x40 | 2},
  {152, 0x00 | 0, 4, 9, 0xC0 | 4},
  {153, 0x00 | 0, 4, 5, 0xC0 | 9},
  {154, 0x00 | 0, 4, 4, 0xC0 | 8},
  {155, 0x00 | 0, 5, 0, 0x60 | 9},
  {156, 0x00 | 0, 5, 0, 0xE0 | 9},
  {157, 0x00 | 0, 5, 0, 0xE0 | 8},
  {158, 0x00 | 0, 5, 0, 0x60 | 9},
  {159, 0x00 | 0, 5, 0, 0x60 | 9},
  {184, 0x00 | 16, 0, 2, 0x80 | 0},
  {174, 0x00 | 26, 0, 2, 0x80 | 0},
  {175, 0x00 | 18, 3, 2, 0x00 | 0},
  {176, 0x00 | 22, 7, 2, 0xC0 | 4},
  {177, 0x00 | 21, 10, 2, 0x00 | 0},
  {178, 0x00 | 23, 7, 2, 0x80 | 0},
  {179, 0x00 | 25, 4, 2, 0x80 | 0},
  {180, 0x00 | 24, 0, 2, 0xC0 | 14},
  {181, 0x00 | 15, 0, 2, 0xC0 | 13},
  {182, 0x00 | 20, 3, 2, 0x00 | 0},
  {183, 0x00 | 31, 3, 2, 0x00 | 0},
  {184, 0x00 | 16, 0, 2, 0x80 | 0},
  {185, 0x00 | 17, 0, 2, 0x80 | 0},
  {186, 0x00 | 32, 0, 2, 0x00 | 0},
  {187, 0x00 | 33, 0, 2, 0x80 | 0},
  {188, 0x00 | 34, 2, 2, 0xC0 | 3},
  {14, 0x00 | 0, 0, 0, 0x40 | 3},
  {189, 0x00 | 19, 7, 2, 0x80 | 0},
  {190, 0x00 | 14, 1, 2, 0x80 | 0},
  {191, 0x00 | 27, 0, 2, 0x80 | 0},
  {181, 0x00 | 15, 0, 2, 0xC0 | 13},
  {181, 0x00 | 15, 0, 2, 0xC0 | 13},
  {112, 0x00 | 43, 0, 0, 0xC0 | 6},
  {113, 0x00 | 44, 0, 0, 0x40 | 6},
  {114, 0x00 | 45, 0, 0, 0xC0 | 5},
  {115, 0x00 | 46, 0, 0, 0x40 | 5},
  {114, 0x00 | 0, 0, 0, 0xC0 | 5},
  {78, 0x00 | 0, 0, 3, 0x80 | 10},
  {77, 0x00 | 0, 4, 3, 0x80 | 7},
  {211, 0x00 | 0, 0, 1, 0x40 | 4},
  {212, 0x00 | 0, 0, 1, 0x40 | 4},
  {213, 0x00 | 0, 0, 1, 0x40 | 4},
  {214, 0x00 | 0, 0, 1, 0x40 | 7},
  {215, 0x00 | 0, 0, 7, 0x40 | 11},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {79, 0x00 | 0, 4, 7, 0x40 | 9},
  {130, 0x00 | 0, 0, 0, 0x40 | 4},
  {131, 0x00 | 0, 0, 0, 0x40 | 4},
  {132, 0x00 | 0, 0, 2, 0x40 | 4},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {192, 0x00 | 0, 0, 0, 0x00 | 0},
  {193, 0x00 | 0, 0, 1, 0x00 | 0},
  {194, 0x00 | 0, 0, 0, 0x80 | 0},
  {195, 0x00 | 0, 0, 0, 0x00 | 0},
  {196, 0x00 | 0, -1, 0, 0x00 | 0},
  {197, 0x00 | 0, -1, 0, 0x00 | 0},
  {198, 0x00 | 0, -1, 0, 0x00 | 0},
  {199, 0x00 | 0, -4, 0, 0x00 | 0},
  {200, 0x00 | 0, -4, 0, 0x80 | 0},
  {201, 0x00 | 0, -4, 0, 0x00 | 0},
  {202, 0x00 | 0, -4, 0, 0x00 | 0},
  {203, 0x00 | 0, -4, 0, 0x00 | 0},
  {204, 0x00 | 0, -4, 0, 0x00 | 0},
  {205, 0x00 | 0, -5, 0, 0x00 | 0},
  {206, 0x00 | 0, -5, 0, 0x00 | 0},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {207, 0x00 | 0, 0, 1, 0x40 | 6},
  {208, 0x00 | 0, 0, 1, 0xC0 | 6},
  {209, 0x00 | 0, 0, 1, 0xC0 | 8},
  {210, 0x00 | 0, 0, 1, 0x40 | 10},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {52, 0x00 | 0, 0, 0, 0x80 | 0},
  {53, 0x00 | 0, 0, 0, 0x00 | 0},
  {54, 0x00 | 0, 0, 0, 0x00 | 0},
  {55, 0x00 | 0, 0, 0, 0x00 | 0},
  {56, 0x00 | 0, 0, 0, 0x80 | 0},
  {57, 0x00 | 0, 0, 0, 0x00 | 0},
  {58, 0x00 | 0, 0, 0, 0x00 | 0},
  {59, 0x00 | 0, 0, 0, 0x00 | 0},
  {60, 0x00 | 0, 0, 0, 0x80 | 0},
  {61, 0x00 | 0, 0, 0, 0x00 | 0},
  {62, 0x00 | 0, 0, 0, 0x80 | 0},
  {63, 0x00 | 0, 0, 0, 0x00 | 0},
  {160, 0x00 | 35, 1, 1, 0xC0 | 3},
  {161, 0x00 | 36, 0, 1, 0x40 | 9},
  {162, 0x00 | 37, 0, 1, 0xC0 | 3},
  {163, 0x00 | 38, 0, 1, 0x40 | 9},
  {164, 0x00 | 39, 0, 1, 0xC0 | 3},
  {165, 0x00 | 40, 1, 1, 0x40 | 9},
  {166, 0x00 | 41, 1, 1, 0x40 | 3},
  {167, 0x00 | 42, 1, 1, 0xC0 | 9},
  {168, 0x00 | 0, 4, 1, 0xC0 | 6},
  {169, 0x00 | 0, 3, 1, 0xC0 | 10},
  {170, 0x00 | 0, 1, 1, 0x40 | 3},
  {171, 0x00 | 0, 1, 1, 0xC0 | 8},
};

// data:1496
static const frame_type frame_tbl_guard[] = {
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {12, 0xC0 | 13, 2, 1, 0x00 | 0},
  {2, 0xC0 | 1, 3, 1, 0x00 | 0},
  {3, 0xC0 | 2, 4, 1, 0x00 | 0},
  {4, 0xC0 | 3, 7, 1, 0x40 | 4},
  {5, 0xC0 | 4, 10, 1, 0x00 | 0},
  {6, 0xC0 | 5, 7, 1, 0x80 | 0},
  {7, 0xC0 | 6, 4, 1, 0x80 | 0},
  {8, 0xC0 | 7, 0, 1, 0x80 | 0},
  {9, 0xC0 | 8, 0, 1, 0xC0 | 13},
  {10, 0xC0 | 11, 7, 1, 0x80 | 0},
  {11, 0xC0 | 12, 3, 1, 0x00 | 0},
  {12, 0xC0 | 13, 2, 1, 0x00 | 0},
  {13, 0xC0 | 0, 2, 1, 0x00 | 0},
  {14, 0xC0 | 28, 0, 1, 0x00 | 0},
  {15, 0xC0 | 29, 0, 1, 0x80 | 0},
  {16, 0xC0 | 30, 2, 1, 0xC0 | 3},
  {17, 0xC0 | 9, -1, 1, 0x40 | 8},
  {18, 0xC0 | 10, 7, 1, 0x80 | 0},
  {19, 0xC0 | 14, 3, 1, 0x80 | 0},
  {9, 0xC0 | 8, 0, 1, 0x80 | 0},
  {20, 0xC0 | 8, 0, 1, 0xC0 | 13},
  {21, 0xC0 | 8, 0, 1, 0xC0 | 13},
  {22, 0xC0 | 47, 0, 0, 0xC0 | 6},
  {23, 0xC0 | 48, 0, 0, 0x40 | 6},
  {24, 0xC0 | 49, 0, 0, 0xC0 | 5},
  {24, 0xC0 | 49, 0, 0, 0xC0 | 5},
  {24, 0xC0 | 49, 0, 0, 0xC0 | 5},
  {26, 0xC0 | 0, 0, 3, 0x80 | 10},
  {27, 0xC0 | 0, 4, 4, 0x80 | 7},
  {28, 0xC0 | 0, -2, 1, 0x40 | 4},
  {29, 0xC0 | 0, -2, 1, 0x40 | 4},
  {30, 0xC0 | 0, -2, 1, 0x40 | 4},
  {31, 0xC0 | 0, -2, 2, 0x40 | 7},
  {32, 0xC0 | 0, -2, 2, 0x40 | 10},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {33, 0xC0 | 0, 3, 4, 0xC0 | 9},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
};

// data:1564
static const frame_type frame_tbl_cuts[] = {
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {15, 0x40 | 0, 0, 0, 0x00 | 0},
  {1, 0x40 | 0, 0, 0, 0x80 | 0},
  {2, 0x40 | 0, 0, 0, 0x80 | 0},
  {3, 0x40 | 0, 0, 0, 0x80 | 0},
  {4, 0x40 | 0, -1, 0, 0x00 | 0},
  {5, 0x40 | 0, 2, 0, 0x80 | 0},
  {6, 0x40 | 0, 2, 0, 0x00 | 0},
  {7, 0x40 | 0, 0, 0, 0x80 | 0},
  {8, 0x40 | 0, 1, 0, 0x80 | 0},
  {255, 0x00 | 0, 0, 0, 0x00 | 0},
  {0, 0x40 | 0, 0, 0, 0x80 | 0},
  {9, 0x40 | 0, 0, 0, 0x80 | 0},
  {10, 0x40 | 0, 0, 0, 0x00 | 0},
  {11, 0x40 | 0, 0, 0, 0x80 | 0},
  {12, 0x40 | 0, 0, 0, 0x80 | 0},
  {13, 0x40 | 0, 0, 0, 0x80 | 0},
  {14, 0x40 | 0, 0, 0, 0x00 | 0},
  {16, 0x40 | 0, 0, 0, 0x00 | 0},
  {0, 0x80 | 0, 0, 0, 0x00 | 0},
  {2, 0x80 | 0, 0, 0, 0x00 | 0},
  {3, 0x80 | 0, 0, 0, 0x00 | 0},
  {4, 0x80 | 0, 0, 0, 0x80 | 0},
  {5, 0x80 | 0, 0, 0, 0x00 | 0},
  {6, 0x80 | 0, 0, 0, 0x80 | 0},
  {7, 0x80 | 0, 0, 0, 0x80 | 0},
  {8, 0x80 | 0, 0, 0, 0x00 | 0},
  {9, 0x80 | 0, 0, 0, 0x00 | 0},
  {10, 0x80 | 0, 0, 0, 0x00 | 0},
  {11, 0x80 | 0, 0, 0, 0x00 | 0},
  {12, 0x80 | 0, 0, 0, 0x00 | 0},
  {13, 0x80 | 0, 0, 0, 0x00 | 0},
  {14, 0x80 | 0, 0, 0, 0x00 | 0},
  {15, 0x80 | 0, 0, 0, 0x00 | 0},
  {16, 0x80 | 0, 0, 0, 0x00 | 0},
  {17, 0x80 | 0, 0, 0, 0x00 | 0},
  {18, 0x80 | 0, 0, 0, 0x00 | 0},
  {19, 0x80 | 0, 0, 0, 0x00 | 0},
  {20, 0x80 | 0, 0, 0, 0x80 | 0},
  {21, 0x80 | 0, 0, 0, 0x80 | 0},
  {22, 0x80 | 0, 1, 0, 0x00 | 0},
  {23, 0x80 | 0, -1, 0, 0x00 | 0},
  {24, 0x80 | 0, 2, 0, 0x00 | 0},
  {25, 0x80 | 0, 1, 0, 0x80 | 0},
  {26, 0x80 | 0, 0, 0, 0x80 | 0},
  {27, 0x80 | 0, 0, 0, 0x80 | 0},
  {28, 0x80 | 0, 0, 0, 0x80 | 0},
  {29, 0x80 | 0, -1, 0, 0x00 | 0},
  {0, 0x80 | 0, 0, 0, 0x80 | 0},
  {1, 0x80 | 0, 0, 0, 0x80 | 0},
  {2, 0x80 | 0, 0, 0, 0x80 | 0},
  {3, 0x80 | 0, 0, 0, 0x00 | 0},
  {4, 0x80 | 0, 0, 0, 0x00 | 0},
  {5, 0x80 | 0, 0, 0, 0x80 | 0},
  {6, 0x80 | 0, 0, 0, 0x80 | 0},
  {7, 0x80 | 0, 0, 0, 0x80 | 0},
  {8, 0x80 | 0, 0, 0, 0x80 | 0},
  {9, 0x80 | 0, 0, 0, 0x80 | 0},
  {10, 0x80 | 0, 0, 0, 0x80 | 0},
  {11, 0x80 | 0, 0, 0, 0x80 | 0},
  {12, 0x80 | 0, 0, 0, 0x80 | 0},
  {13, 0x80 | 0, 0, 0, 0x00 | 0},
  {14, 0x80 | 0, 0, 0, 0x80 | 0},
  {15, 0x80 | 0, 0, 0, 0x00 | 0},
  {16, 0x80 | 0, 0, 0, 0x00 | 0},
  {17, 0x80 | 0, 0, 0, 0x80 | 0},
  {18, 0x80 | 0, 0, 0, 0x00 | 0},
  {19, 0x80 | 0, 3, 0, 0x00 | 0},
  {20, 0x80 | 0, 3, 0, 0x00 | 0},
  {21, 0x80 | 0, 3, 0, 0x00 | 0},
  {22, 0x80 | 0, 2, 0, 0x00 | 0},
  {23, 0x80 | 0, 3, 0, 0x80 | 0},
  {24, 0x80 | 0, 5, 0, 0x00 | 0},
  {25, 0x80 | 0, 5, 0, 0x00 | 0},
  {26, 0x80 | 0, 1, 0, 0x80 | 0},
  {27, 0x80 | 0, 2, 0, 0x80 | 0},
  {28, 0x80 | 0, 2, 0, 0x80 | 0},
  {29, 0x80 | 0, 1, 0, 0x80 | 0},
  {30, 0x80 | 0, 1, 0, 0x00 | 0},
  {31, 0x80 | 0, 2, 0, 0x00 | 0},
  {32, 0x80 | 0, 3, 0, 0x00 | 0},
  {33, 0x80 | 0, 3, 0, 0x00 | 0},
  {34, 0x80 | 0, 0, 0, 0x80 | 0},
  {35, 0x80 | 0, 2, 0, 0x80 | 0},
  {36, 0x80 | 0, 2, 0, 0x80 | 0},
  {37, 0x80 | 0, 1, 0, 0x00 | 0},
};

static const rect_type screen_rect = {0, 0, 200, 320};
//1.0//static const char copyprot_letter[] = {'A', 'A', 'B', 'B', 'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'H', 'H', 'I', 'I', 'J', 'J', 'K', 'L', 'L', 'M', 'M', 'N', 'O', 'O', 'P', 'P', 'R', 'R', 'S', 'S', 'T', 'T', 'U', 'U', 'V', 'Y', 'W', 'Y'};
static const char copyprot_letter[] = {'W', 'O', 'E', 'S', 'P', 'B', 'Y', 'S', 'K', 'J', 'T', 'B', 'C', 'F', 'E', 'S', 'K', 'M', 'M', 'T', 'P', 'Y', 'K', 'C', 'G', 'S', 'U', 'L', 'J', 'C', 'D', 'I', 'L', 'T', 'T', 'A', 'M', 'C', 'S', 'G'};
static const word tbl_line[] = {0, 10, 20};
static const byte chtab_flip_clip[10] = {1, 0, 1, 1, 1, 1, 0, 0, 0, 0};
static const byte chtab_shift[10] = {0, 1, 0, 0, 0, 0, 1, 1, 1, 0};
static const rect_type rect_top = {0, 0, 192, 320};
static const rect_type rect_bottom_text = {193, 70, 202, 250};
static const short y_land[] = {-8, 55, 118, 181, 244};
static const word copyprot_tile[] = {1, 5, 7, 9, 11, 21, 1, 3, 7, 11, 17, 21, 25, 27};
static const byte x_bump[] = {-12, 2, 16, 30, 44, 58, 72, 86, 100, 114, 128, 142, 156, 170, 184, 198, 212, 226, 240, 254};
static const short y_clip[] = {-60, 3, 66, 129, 192};
static const sbyte dir_front[] = {-1, 1};
static const sbyte dir_behind[] = {1, -1};

static custom_options_type custom_defaults = {
                                                  .start_minutes_left = 60,
                                                  .start_ticks_left = 719,
                                                  .start_hitp = 3,
                                                  .max_hitp_allowed = 10,
                                                  .saving_allowed_first_level = 3,
                                                  .saving_allowed_last_level = 13,
                                                  .start_upside_down = 0,
                                                  .start_in_blind_mode = 0,
                                                  // data:009E
                                                  .copyprot_level = 2,
                                                  .drawn_tile_top_level_edge = tiles_1_floor,
                                                  .drawn_tile_left_level_edge = tiles_20_wall,
                                                  .level_edge_hit_tile = tiles_20_wall,
                                                  .allow_triggering_any_tile = 0,
                                                  .enable_wda_in_palace = 0,
                                                  .vga_palette = VGA_PALETTE_DEFAULT,
                                                  .first_level = 1,
                                                  .skip_title = 0,
                                                  .shift_L_allowed_until_level = 4,
                                                  .shift_L_reduced_minutes = 15,
                                                  .shift_L_reduced_ticks = 719,
                                                  .demo_hitp = 4,
                                                  .demo_end_room = 24,
                                                  .intro_music_level = 1,
                                                  .intro_music_time_initial = 33,
                                                  .intro_music_time_restart = 4,
                                                  .have_sword_from_level = 2,
                                                  .checkpoint_level = 3,
                                                  .checkpoint_respawn_dir = dir_FF_left,
                                                  .checkpoint_respawn_room = 2,
                                                  .checkpoint_respawn_tilepos = 6,
                                                  .checkpoint_clear_tile_room = 7,
                                                  .checkpoint_clear_tile_col = 4,
                                                  .checkpoint_clear_tile_row = 0,
                                                  .skeleton_level = 3,
                                                  .skeleton_room = 1,
                                                  .skeleton_trigger_column_1 = 2,
                                                  .skeleton_trigger_column_2 = 3,
                                                  .skeleton_column = 5,
                                                  .skeleton_row = 1,
                                                  .skeleton_require_open_level_door = 1,
                                                  .skeleton_skill = 2,
                                                  .skeleton_reappear_room = 3,
                                                  .skeleton_reappear_x = 133,
                                                  .skeleton_reappear_row = 1,
                                                  .skeleton_reappear_dir = dir_0_right,
                                                  .mirror_level = 4,
                                                  .mirror_room = 4,
                                                  .mirror_column = 4,
                                                  .mirror_row = 0,
                                                  .mirror_tile = tiles_13_mirror,
                                                  .show_mirror_image = 1,
                                                  .falling_exit_level = 6,
                                                  .falling_exit_room = 1,
                                                  .falling_entry_level = 7,
                                                  .falling_entry_room = 17,
                                                  .mouse_level = 8,
                                                  .mouse_room = 16,
                                                  .mouse_delay = 150,
                                                  .mouse_object = 24,
                                                  .mouse_start_x = 200,
                                                  .loose_tiles_level = 13,
                                                  .loose_tiles_room_1 = 23,
                                                  .loose_tiles_room_2 = 16,
                                                  .loose_tiles_first_tile = 22,
                                                  .loose_tiles_last_tile = 27,
                                                  .jaffar_victory_level = 13,
                                                  .jaffar_victory_flash_time = 18,
                                                  .hide_level_number_from_level = 14,
                                                  .level_13_level_number = 12,
                                                  .victory_stops_time_level = 13,
                                                  .win_level = 14,
                                                  .win_room = 5,
                                                  .loose_floor_delay = 11,
                                                  // data:02B2
                                                  .tbl_level_type = {0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0},
                                                  // 1.3
                                                  .tbl_level_color = {0, 0, 0, 1, 0, 0, 0, 1, 2, 2, 0, 0, 3, 3, 4, 0},
                                                  // data:03D4
                                                  .tbl_guard_type = {0, 0, 0, 2, 0, 0, 1, 0, 0, 0, 0, 0, 4, 3, -1, -1},
                                                  // data:0EDA
                                                  .tbl_guard_hp = {4, 3, 3, 3, 3, 4, 5, 4, 4, 5, 5, 5, 4, 6, 0, 0},
                                                  .tbl_cutscenes_by_index = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                                  .tbl_entry_pose = {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0},
                                                  .tbl_seamless_exit = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 23, -1, -1, -1},

//                                                  // guard skills 1.0
//                                                  .strikeprob =    { 61, 100, 61, 61, 61, 40, 100, 220, 0, 48, 32, 48},
//                                                  .restrikeprob =  { 0, 0, 0, 5, 5, 175, 16, 8, 0, 255, 255, 150},
//                                                  .blockprob =     { 0, 150, 150, 200, 200, 255, 200, 250, 0, 255, 255, 255},
//                                                  .impblockprob =  { 0, 61, 61, 100, 100, 145, 100, 250, 0, 145, 255, 175},
//                                                  .advprob =       { 255, 200, 200, 200, 255, 255, 200, 0, 0, 255, 100, 100},
//                                                  .refractimer =   { 16, 16, 16, 16, 8, 8, 8, 8, 0, 8, 0, 0},
//                                                  .extrastrength = { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},

                                                  // guard skills 1.4
                                                  .strikeprob =    { 75, 100, 75, 75, 75, 50, 100, 220, 0, 60, 40,  60},
                                                  .restrikeprob =  { 0, 0, 0, 5, 5, 175, 20, 10, 0, 255, 255, 150},
                                                  .blockprob =     { 0, 150, 150, 200, 200, 255, 200, 250, 0, 255, 255, 255},
                                                  .impblockprob =  { 0, 75, 75, 100, 100, 145, 100, 250, 0, 145, 255, 175},
                                                  .advprob =       { 255, 200, 200, 200, 255, 255, 200, 0, 0, 255, 100, 100},
                                                  .refractimer =   { 20, 20, 20, 20, 10, 10, 10, 10, 0, 10, 0, 0},
                                                  .extrastrength = { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},

                                                  // shadow's starting positions
                                                  .init_shad_6 = {0x0F, 0x51, 0x76, 0, 0, 1, 0, 0},
                                                  .init_shad_5 = {0x0F, 0x37, 0x37, 0, 0xFF, 0, 0, 0},
                                                  .init_shad_12 = {0x0F, 0x51, 0xE8, 0, 0, 0, 0, 0},
                                                  // automatic moves
                                                  .demo_moves = {{0x00, 0}, {0x01, 1}, {0x0D, 0}, {0x1E, 1}, {0x25, 5}, {0x2F, 0}, {0x30, 1}, {0x41, 0}, {0x49, 2}, {0x4B, 0}, {0x63, 2}, {0x64, 0}, {0x73, 5}, {0x80, 6}, {0x88, 3}, {0x9D, 7}, {0x9E, 0}, {0x9F, 1}, {0xAB, 4}, {0xB1, 0}, {0xB2, 1}, {0xBC, 0}, {0xC1, 1}, {0xCD, 0}, {0xE9, -1}},
                                                  .shad_drink_move = {{0x00, 0}, {0x01, 1}, {0x0E, 0}, {0x12, 6}, {0x1D, 7}, {0x2D, 2}, {0x31, 1}, {0xFF, -2}},

                                                  // speeds
                                                  .base_speed = 5,
                                                  .fight_speed = 6,
                                                  .chomper_speed = 15,
                                                };
char *g_argv[] = {(char *)"prince"};
