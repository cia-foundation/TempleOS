$TR-UL,"Adam"$
$ID,2$$TR,"SysMsgFlags"$
$ID,2$sys_msg_flags[0]=0xFF;
$ID,-2$$TR,"SysRegVer"$
$ID,2$registry_version=5.030;
$ID,-2$$TR,"InsReg"$
$ID,2$#help_index "Registry/Install Registration"
MemSet(&ins_reg,0,sizeof(CInsReg));
ins_reg.registered=1;
ins_reg.host_os=2;
ins_reg.host_vm=2;

#define INS_REG_PERSONAL_INITIALS "TAD"
StrCpy(ins_reg.initials,"TAD");
HashPublic("INS_REG_PERSONAL_INITIALS",HTT_DEFINE_STR);

#define INS_REG_MACHINE_NUM 0
ins_reg.machine_num=0;
HashPublic("INS_REG_MACHINE_NUM",HTT_DEFINE_STR);

"InsReg:%s:%d\n",INS_REG_PERSONAL_INITIALS,INS_REG_MACHINE_NUM;

#help_index ""
$ID,-2$$ID,-2$$TR-UL,"TempleOS"$
$ID,2$$TR,"EagleDive"$
$ID,2$F64 best_score=147.6932;
$ID,-2$$TR,"DiningStars"$
$ID,2$F64 best_score=9999.0000;
$ID,-2$$TR,"CircleTrace"$
$ID,2$F64 best_score=4.9425;
$ID,-2$$TR,"TOSNIST"$
$ID,2$I64 tos_nist_offset=-8049;
$ID,-2$$TR,"BlackDiamond"$
$ID,2$I64 best_score=2;
$ID,-2$$TR,"DunGen"$
$ID,2$F64 best_score=20.5295;
$ID,-2$$TR,"CastleFrankenstein"$
$ID,2$F64 best_score=33.8405;
$ID,-2$$TR,"ZoneOut"$
$ID,2$F64 best_score=8.1373;
$ID,-2$$TR,"Varoom"$
$ID,2$F64 best_score=46.2643;
$ID,-2$$TR,"FlapBat"$
$ID,2$F64 best_score=16.3077;
$ID,-2$$TR,"RawHide"$
$ID,2$F64 best_score=96.1279;
$ID,-2$$TR,"KeepAway"$
$ID,2$I64 best_score0=68,best_score1=30;
$ID,-2$$TR,"Wenceslas"$
$ID,2$F64 best_score=43.9944;
$ID,-2$$TR,"OSTestSuite"$
$ID,2$progress1_tf=0.000;progress2_tf=0.000;
progress3_tf=0.000;progress4_tf=227.554;
$ID,-2$$TR,"XCaliber"$
$ID,2$I64 best_score=90;
I64 msg_flags=2;
$ID,-2$$TR,"Talons"$
$ID,2$F64 best_score=9999.0000;
$ID,-2$$TR,"BomberGolf"$
$ID,2$I64 best_score=175;
$ID,-2$$TR,"Titanium"$
$ID,2$I64 best_score=16469;
$ID,-2$$TR,"TOSRegen"$
$ID,2$progress1_tf=0.000;progress2_tf=0.000;
progress3_tf=0.000;progress4_tf=196.886;
$ID,-2$$TR,"TempleTetris"$
$ID,2$I64 hiscore=1000;
$ID,-2$$ID,-2$$TR,"Once"$
$ID,2$$TR,"Adam"$
$ID,2$$ID,-2$$TR,"User"$
$ID,2$$ID,-2$$ID,-2$$TR,"DemoCompany"$
$ID,2$$TR,"Game1"$
$ID,2$F64 best_score=202.07117;
$TR,"SimpleVal"$
$ID,2$1239;
$ID,-2$$ID,-2$$ID,-2$