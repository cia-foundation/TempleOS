U0 Main()
{
  F64 é=0,è=0,ê=0;
  CD3I32 poly[4];
  CDC *dc=DCAlias;
  DCDepthBufAlloc(dc);
  dc->flags|=DCF_TRANSFORMATION|DCF_SYMMETRY;
  try {
    while (!ScanChar) {
      DCDepthBufRst(dc);
      DCSymmetrySet(dc,Fs->pix_width/2,Fs->pix_height/2,
	    ms.pos.x-Fs->pix_left-Fs->scroll_x,
	    ms.pos.y-Fs->pix_top-Fs->scroll_y);
      dc->color=LTBLUE;
      GrLine(dc,Fs->pix_width/2,Fs->pix_height/2,
	    ms.pos.x-Fs->pix_left-Fs->scroll_x,
	    ms.pos.y-Fs->pix_top-Fs->scroll_y);

      Mat4x4IdentEqu(dc->r);
      Mat4x4RotX(dc->r,ê);
      Mat4x4RotY(dc->r,è);
      Mat4x4RotZ(dc->r,é);
      DCMat4x4Set(dc,dc->r);

      dc->x=Fs->pix_width/2;
      dc->y=Fs->pix_height/2;
      dc->z=500;

      dc->color=GREEN;
      poly[0].x=50;
      poly[0].y=150;
      poly[0].z=0;
      poly[1].x=275;
      poly[1].y=100;
      poly[1].z=0;
      poly[2].x=155;
      poly[2].y=200;
      poly[2].z=0;
      poly[3].x=100;
      poly[3].y=200;
      poly[3].z=0;
      GrFillPoly3(dc,4,poly);

      dc->color=RED;
      poly[0].x=50;
      poly[0].y=150;
      poly[0].z=50;
      poly[1].x=275;
      poly[1].y=100;
      poly[1].z=50;
      poly[2].x=155;
      poly[2].y=200;
      poly[2].z=50;
      poly[3].x=100;
      poly[3].y=200;
      poly[3].z=50;
      GrFillPoly3(dc,4,poly);

      dc->color=BLUE;
      poly[0].x=50;
      poly[0].y=150;
      poly[0].z=-50;
      poly[1].x=275;
      poly[1].y=100;
      poly[1].z=-50;
      poly[2].x=155;
      poly[2].y=200;
      poly[2].z=-50;
      poly[3].x=100;
      poly[3].y=200;
      poly[3].z=-50;
      GrFillPoly3(dc,4,poly);

      dc->color=RED+GREEN<<16+ROPF_DITHER;
      poly[0].x=50;
      poly[0].y=150;
      poly[0].z=50;
      poly[1].x=275;
      poly[1].y=100;
      poly[1].z=50;
      poly[2].x=275;
      poly[2].y=100;
      poly[2].z=0;
      poly[3].x=50;
      poly[3].y=150;
      poly[3].z=0;
      GrFillPoly3(dc,4,poly);

      dc->color=BLUE+GREEN<<16+ROPF_DITHER;
      poly[0].x=100;
      poly[0].y=200;
      poly[0].z=-50;
      poly[1].x=155;
      poly[1].y=200;
      poly[1].z=-50;
      poly[2].x=155;
      poly[2].y=200;
      poly[2].z=0;
      poly[3].x=100;
      poly[3].y=200;
      poly[3].z=0;
      GrFillPoly3(dc,4,poly);

      Refresh;
      DCFill;
      é+=2.1*ã/180.0;
      è+=1.1*ã/180.0;
      ê+=0.5*ã/180.0;
    }
  } catch
    PutExcept;

  DCFill;
  DCDel(dc);
}

Main;
