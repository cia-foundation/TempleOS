//Makes a mesh man.

#define MAX_VERTICES	1024
#define MAX_TRIS	1024

class Man
{
  I32 vertex_cnt;
  I32 tri_cnt;
  CD3I32	    v[MAX_VERTICES];
  CMeshTri t[MAX_TRIS];
} *m;

class ManDefineStruct
{
  F64 head_rad	 fmtstr "$$DA-TRM,32,RT=F64,A=\"Head Radius                :%8.3f\"$$\n";

  F64 torso_len  fmtstr "$$DA-TRM,32,RT=F64,A=\"Torso Length               :%8.3f\"$$\n";
  F64 arm_len	 fmtstr "$$DA-TRM,32,RT=F64,A=\"Arm Length                 :%8.3f\"$$\n";
  F64 hand_len	 fmtstr "$$DA-TRM,32,RT=F64,A=\"Hand Length                :%8.3f\"$$\n";
  F64 leg_len	 fmtstr "$$DA-TRM,32,RT=F64,A=\"Leg Length                 :%8.3f\"$$\n";
  F64 foot_len	 fmtstr "$$DA-TRM,32,RT=F64,A=\"Foot Length                :%8.3f\"$$\n";
  F64 torso_width fmtstr "$$DA-TRM,32,RT=F64,A=\"Torso Width                :%8.3f\"$$\n";
  F64 torso_depth fmtstr "$$DA-TRM,32,RT=F64,A=\"Torso Depth                :%8.3f\"$$\n";
  F64 arm_rad	 fmtstr "$$DA-TRM,32,RT=F64,A=\"Arm Radius                 :%8.3f\"$$\n";
  F64 hand_rad	 fmtstr "$$DA-TRM,32,RT=F64,A=\"Hand Radius                :%8.3f\"$$\n";
  F64 leg_rad	 fmtstr "$$DA-TRM,32,RT=F64,A=\"Leg Radius                 :%8.3f\"$$\n";
  F64 foot_rad	 fmtstr "$$DA-TRM,32,RT=F64,A=\"Foot Radius                :%8.3f\"$$\n\n";

  F64 r_shoulder_a1	 fmtstr "$$DA-TRM,32,RT=F64,A=\"R.Shoulder Front/Back Angle:%8.3f\"$$\n";
  F64 l_shoulder_a1	 fmtstr "$$DA-TRM,32,RT=F64,A=\"L.Shoulder Front/Back Angle:%8.3f\"$$\n";
  F64 r_hip_a1	 fmtstr "$$DA-TRM,32,RT=F64,A=\"R.Hip Front/Back Angle     :%8.3f\"$$\n";
  F64 l_hip_a1	 fmtstr "$$DA-TRM,32,RT=F64,A=\"L.Hip Front/Back Angle     :%8.3f\"$$\n";

  F64 r_shoulder_a2	 fmtstr "$$DA-TRM,32,RT=F64,A=\"R.Shoulder Side Angle      :%8.3f\"$$\n";
  F64 l_shoulder_a2	 fmtstr "$$DA-TRM,32,RT=F64,A=\"L.Shoulder Side Angle      :%8.3f\"$$\n";
  F64 r_hip_a2	 fmtstr "$$DA-TRM,32,RT=F64,A=\"R.Hip Side Angle           :%8.3f\"$$\n";
  F64 l_hip_a2	 fmtstr "$$DA-TRM,32,RT=F64,A=\"L.Hip Side Angle           :%8.3f\"$$\n";

  F64 r_elbow_a	 fmtstr "$$DA-TRM,32,RT=F64,A=\"R.Elbow Angle              :%8.3f\"$$\n";
  F64 l_elbow_a	 fmtstr "$$DA-TRM,32,RT=F64,A=\"L.Elbow Angle              :%8.3f\"$$\n";
  F64 r_knee_a	 fmtstr "$$DA-TRM,32,RT=F64,A=\"R.Knee Angle               :%8.3f\"$$\n";
  F64 l_knee_a	 fmtstr "$$DA-TRM,32,RT=F64,A=\"L.Knee Angle               :%8.3f\"$$\n";

  F64 r_wrist_a	 fmtstr "$$DA-TRM,32,RT=F64,A=\"R.Wrist Angle              :%8.3f\"$$\n";
  F64 l_wrist_a	 fmtstr "$$DA-TRM,32,RT=F64,A=\"L.Wrist Angle              :%8.3f\"$$\n";
  F64 r_ankle_a	 fmtstr "$$DA-TRM,32,RT=F64,A=\"R.Ankle Angle              :%8.3f\"$$\n";
  F64 l_ankle_a	 fmtstr "$$DA-TRM,32,RT=F64,A=\"L.Ankle Angle              :%8.3f\"$$\n";

};

U0 MDInit(ManDefineStruct *md)
{
  MemSet(md,0,sizeof(ManDefineStruct));

  md->head_rad	=5.0;

  md->torso_len	=35.0;
  md->arm_len	=30.0;
  md->hand_len	=8.0;
  md->leg_len	=32.0;
  md->foot_len	=16.0;
  md->torso_width=20.0;
  md->torso_depth=10.0;
  md->arm_rad	=3.5;
  md->hand_rad	=2.0;
  md->leg_rad	=4.0;
  md->foot_rad	=3.0;

  md->r_shoulder_a1	=30;
  md->l_shoulder_a1	=-30;
  md->r_hip_a1	=-45;
  md->l_hip_a1	=45;

  md->r_shoulder_a2	=10;
  md->l_shoulder_a2	=10;
  md->r_hip_a2	=-5;
  md->l_hip_a2	=-5;

  md->r_elbow_a	=30;
  md->l_elbow_a	=0;
  md->r_knee_a	=0;
  md->l_knee_a	=30;

  md->r_wrist_a	=0;
  md->l_wrist_a	=0;
  md->r_ankle_a	=0;
  md->l_ankle_a	=0;
}

U0 MDCorrect(ManDefineStruct *md)
{
  md->r_ankle_a    =-md->r_ankle_a;
  md->l_ankle_a    =-md->l_ankle_a;
  md->r_knee_a	  =-md->r_knee_a;
  md->l_knee_a	  =-md->l_knee_a;
  md->r_hip_a2	   =-md->r_hip_a2;
  md->r_shoulder_a2=-md->r_shoulder_a2;
  md->r_ankle_a+=90;
  md->l_ankle_a+=90;

  md->foot_len-=md->leg_rad;

  md->r_elbow_a+=md->r_shoulder_a1;
  md->l_elbow_a+=md->l_shoulder_a1;
  md->r_knee_a+=md->r_hip_a1;
  md->l_knee_a+=md->l_hip_a1;

  md->r_wrist_a+=md->r_elbow_a;
  md->l_wrist_a+=md->l_elbow_a;
  md->r_ankle_a+=md->r_knee_a;
  md->l_ankle_a+=md->l_knee_a;
}

I64 AddVertex(ManDefineStruct *md,I64 x,I64 y,I64 z)
{
  I64 result=m->vertex_cnt++;
  m->v[result].x=-x;
  m->v[result].y=-y-md->leg_len-md->foot_rad*2;
  m->v[result].z=-z;
  return result;
}

I64 AddTri(ManDefineStruct *,I64 c,I64 n0,I64 n1,I64 n2)
{
  I64 result=m->tri_cnt++;
  m->t[result].color=c;
  m->t[result].nums[0]=n1;
  m->t[result].nums[1]=n0;
  m->t[result].nums[2]=n2;
  return result;
}

U0 AddBox(ManDefineStruct *md,I64 c,I64 x1,I64 y1,I64 z1,F64 w,F64 h,F64 d,F64 h2,F64 h3,F64 az,F64 ay,F64 ax,
   I64 *ox,I64 *oy,I64 *oz)
{
  I64 *r;
  I64 p1a,p2a,p3a,p4a;
  I64 p1b,p2b,p3b,p4b;
  I64 wx=w/2,wy=0,wz=0,hx=0,hy=h,hz=0,h2x=0,h2y=h2,h2z=0,h3x=0,h3y=h3,h3z=0,dx=0,dy=0,dz=d/2;

  r=GrIdentNew;
  GrRotZEqu(r,az*ã/180.0);
  GrRotYEqu(r,ay*ã/180.0);
  GrRotXEqu(r,ax*ã/180.0);
  GrRot(r,&wx,&wy,&wz);
  GrRot(r,&hx,&hy,&hz);
  GrRot(r,&h2x,&h2y,&h2z);
  GrRot(r,&h3x,&h3y,&h3z);
  GrRot(r,&dx,&dy,&dz);

  p1a=AddVertex(md,x1+dx	-wx+h2x,y1+dy	-wy+h2y,z1+dz	-wz+h2z);
  p2a=AddVertex(md,x1+dx	+wx+h2x,y1+dy	+wy+h2y,z1+dz	+wz+h2z);
  p3a=AddVertex(md,x1+dx+hx-wx	  ,y1+dy+hy-wy	  ,z1+dz+hz-wz);
  p4a=AddVertex(md,x1+dx+hx+wx	  ,y1+dy+hy+wy	  ,z1+dz+hz+wz);
  AddTri(md,c,p1a,p2a,p3a);
  AddTri(md,c,p4a,p3a,p2a);

  p1b=AddVertex(md,x1-dx	-wx+h2x,y1-dy	-wy+h2y,z1-dz	-wz+h2z);
  p2b=AddVertex(md,x1-dx	+wx+h2x,y1-dy	+wy+h2y,z1-dz	+wz+h2z);
  p3b=AddVertex(md,x1-dx+hx-wx	  ,y1-dy+hy-wy	  ,z1-dz+hz-wz);
  p4b=AddVertex(md,x1-dx+hx+wx	  ,y1-dy+hy+wy	  ,z1-dz+hz+wz);
  AddTri(md,c,p1b,p2b,p3b);
  AddTri(md,c,p4b,p3b,p2b);

  AddTri(md,c,p1a,p2a,p1b);
  AddTri(md,c,p2b,p1b,p2a);
  AddTri(md,c,p3a,p4a,p3b);
  AddTri(md,c,p4b,p3b,p4a);
  AddTri(md,c,p1a,p3a,p1b);
  AddTri(md,c,p3b,p1b,p3a);
  AddTri(md,c,p2a,p4a,p2b);
  AddTri(md,c,p4b,p2b,p4a);

  *ox=x1+hx-h3x;
  *oy=y1+hy-h3y;
  *oz=z1+hz-h3z;

  Free(r);
}

U8 *Man2CSprite()
{
//See $LK,"::/Adam/Gr/GrSpritePlot.CPP",A="FL:::/Adam/Gr/GrSpritePlot.CPP,1"$ for how CSprite are stored.
  U8 *result=MAlloc(sizeof(U8)+2*sizeof(I32)+
      m->vertex_cnt*sizeof(CD3I32)+m->tri_cnt*sizeof(CMeshTri)+sprite_elem_base_sizes[SPT_END]),
      *dst=result;
  *dst(U8 *)++ =SPT_MESH;
  *dst(I32 *)++ =m->vertex_cnt;
  *dst(I32 *)++ =m->tri_cnt;
  MemCpy(dst,&m->v,m->vertex_cnt*sizeof(CD3I32));
  dst+=m->vertex_cnt*sizeof(CD3I32);
  MemCpy(dst,&m->t,m->tri_cnt*sizeof(CMeshTri));
  dst+=m->tri_cnt*sizeof(CMeshTri);
  *dst(U8 *)++ =SPT_END;
  return result;
}

public U8 *ManGen()
{
  U8 *result;
  I64 x,y,z,c1,c2,c3,c4;
  ManDefineStruct md1,md2;
  MDInit(&md1);

  while (TRUE) {
    if (!DocFormDo(&md1))
      return NULL;
    MemCpy(&md2,&md1,sizeof(ManDefineStruct));
    MDCorrect(&md2);

    c1=PopUpColorLighting("$$BK,1$$Shirt$$BK,0$$\n");
    if (c1<0) return NULL;
    c2=PopUpColorLighting("$$BK,1$$Pants$$BK,0$$\n");
    if (c2<0) return NULL;
    c3=PopUpColorLighting("$$BK,1$$Skin$$BK,0$$\n");
    if (c3<0) return NULL;
    c4=PopUpColorLighting("$$BK,1$$Shoes$$BK,0$$\n");
    if (c4<0) return NULL;

    m=CAlloc(sizeof(Man));

    //Head
    AddBox(&md2,c3,0,md2.torso_len+md2.head_rad*2,0,
	md2.head_rad*2,-md2.head_rad*2,md2.head_rad*2,0,0, 0,0,0,
	&x,&y,&z);

	//Torso
    AddBox(&md2,c1,0,md2.torso_len,0,
	md2.torso_width,-md2.torso_len,md2.torso_depth,0,0, 0,0,0,
	&x,&y,&z);

	//Right Arm
    AddBox(&md2,c1,-md2.torso_width/2-md2.arm_rad,md2.torso_len-md2.arm_rad,0,
	md2.arm_rad*2,-md2.arm_len/2,md2.arm_rad*2,md2.arm_rad,0,
	md2.r_shoulder_a2,0,md2.r_shoulder_a1,&x,&y,&z);
    AddBox(&md2,c3,x,y,z,
	md2.arm_rad*2,-md2.arm_len/2,md2.arm_rad*2,0,0,
	md2.r_shoulder_a2,0,md2.r_elbow_a,&x,&y,&z);
    AddBox(&md2,c3,x,y,z,
	md2.arm_rad*2,-md2.hand_len,md2.hand_rad*2,0,0,
	md2.r_shoulder_a2,0,md2.r_wrist_a,&x,&y,&z);

	//Left Arm
    AddBox(&md2,c1,md2.torso_width/2+md2.arm_rad,md2.torso_len-md2.arm_rad,0,
	md2.arm_rad*2,-md2.arm_len/2,md2.arm_rad*2,md2.arm_rad,0,
	md2.l_shoulder_a2,0,md2.l_shoulder_a1,&x,&y,&z);
    AddBox(&md2,c3,x,y,z,
	md2.arm_rad*2,-md2.arm_len/2,md2.arm_rad*2,0,0,
	md2.l_shoulder_a2,0,md2.l_elbow_a,&x,&y,&z);
    AddBox(&md2,c3,x,y,z,
	md2.arm_rad*2,-md2.hand_len,md2.hand_rad*2,0,0,
	md2.l_shoulder_a2,0,md2.l_wrist_a,&x,&y,&z);

	//Right Leg
    AddBox(&md2,c2,-md2.torso_width/2+md2.leg_rad,0,0,
	md2.leg_rad*2,-md2.leg_len/2,md2.leg_rad*2,0,0,
	md2.r_hip_a2,0,md2.r_hip_a1,&x,&y,&z);
    AddBox(&md2,c3,x,y,z,
	md2.leg_rad*2,-md2.leg_len/2,md2.leg_rad*2,0,md2.foot_rad,
	md2.r_hip_a2,0,md2.r_knee_a,&x,&y,&z);
    AddBox(&md2,c4,x,y,z,
	md2.leg_rad*2,-md2.foot_len,md2.foot_rad*2,md2.leg_rad,0,
	md2.r_hip_a2,0,md2.r_ankle_a,&x,&y,&z);

	//Left Leg
    AddBox(&md2,c2,md2.torso_width/2-md2.leg_rad,0,0,
	md2.leg_rad*2,-md2.leg_len/2,md2.leg_rad*2,0,0,
	md2.l_hip_a2,0,md2.l_hip_a1,&x,&y,&z);
    AddBox(&md2,c3,x,y,z,
	md2.leg_rad*2,-md2.leg_len/2,md2.leg_rad*2,0,md2.foot_rad,
	md2.l_hip_a2,0,md2.l_knee_a,&x,&y,&z);
    AddBox(&md2,c4,x,y,z,
	md2.leg_rad*2,-md2.foot_len,md2.foot_rad*2,md2.leg_rad,0,
	md2.l_hip_a2,0,md2.l_ankle_a,&x,&y,&z);

    result=Man2CSprite;
    "%h7c",'\n';
    Sprite(result,"\t\t$$SP,\"<1>\",BI=%d$$");
    "%h7c",'\n';
    Free(m);
    "Do another";
    if (YorN)
      Free(result);
    else
      break;
  }
  return result;
}
