
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// hires texture funcs
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


#define GET_RESULT(A, B, C, D) ((A != C || A != D) - (B != C || B != D))

////////////////////////////////////////////////////////////////////////

#define colorMask8     0x00FEFEFE
#define lowPixelMask8  0x00010101
#define qcolorMask8    0x00FCFCFC
#define qlowpixelMask8 0x00030303


#define INTERPOLATE8_02(A, B) (((((A & colorMask8) >> 1) + ((B & colorMask8) >> 1) + (A & B & lowPixelMask8))|((((A&0xFF000000)==0x03000000)?0x03000000:(((B&0xFF000000)==0x03000000)?0x03000000:(((A&0xFF000000)==0x00000000)?0x00000000:(((B&0xFF000000)==0x00000000)?0x00000000:0xFF000000)))))))

#define Q_INTERPOLATE8_02(A, B, C, D) (((((A & qcolorMask8) >> 2) + ((B & qcolorMask8) >> 2) + ((C & qcolorMask8) >> 2) + ((D & qcolorMask8) >> 2) + ((((A & qlowpixelMask8) + (B & qlowpixelMask8) + (C & qlowpixelMask8) + (D & qlowpixelMask8)) >> 2) & qlowpixelMask8))|((((A&0xFF000000)==0x03000000)?0x03000000:(((B&0xFF000000)==0x03000000)?0x03000000:(((C&0xFF000000)==0x03000000)?0x03000000:(((D&0xFF000000)==0x03000000)?0x03000000:(((A&0xFF000000)==0x00000000)?0x00000000:(((B&0xFF000000)==0x00000000)?0x00000000:(((C&0xFF000000)==0x00000000)?0x00000000:(((D&0xFF000000)==0x00000000)?0x00000000:0xFF000000)))))))))))

#define INTERPOLATE8(A, B) (((((A & colorMask8) >> 1) + ((B & colorMask8) >> 1) + (A & B & lowPixelMask8))|((((A&0xFF000000)==0x50000000)?0x50000000:(((B&0xFF000000)==0x50000000)?0x50000000:(((A&0xFF000000)==0x00000000)?0x00000000:(((B&0xFF000000)==0x00000000)?0x00000000:0xFF000000)))))))

#define Q_INTERPOLATE8(A, B, C, D) (((((A & qcolorMask8) >> 2) + ((B & qcolorMask8) >> 2) + ((C & qcolorMask8) >> 2) + ((D & qcolorMask8) >> 2) + ((((A & qlowpixelMask8) + (B & qlowpixelMask8) + (C & qlowpixelMask8) + (D & qlowpixelMask8)) >> 2) & qlowpixelMask8))|((((A&0xFF000000)==0x50000000)?0x50000000:(((B&0xFF000000)==0x50000000)?0x50000000:(((C&0xFF000000)==0x50000000)?0x50000000:(((D&0xFF000000)==0x50000000)?0x50000000:(((A&0xFF000000)==0x00000000)?0x00000000:(((B&0xFF000000)==0x00000000)?0x00000000:(((C&0xFF000000)==0x00000000)?0x00000000:(((D&0xFF000000)==0x00000000)?0x00000000:0xFF000000)))))))))))

void Super2xSaI_ex8_Ex(unsigned char *srcPtr, DWORD srcPitch,
        unsigned char *dstBitmap, int width, int height) {
    DWORD dstPitch = srcPitch * 2;
    DWORD line;
    DWORD *dP;
    DWORD *bP;
    int width2 = width * 2;
    int iXA, iXB, iXC, iYA, iYB, iYC, finish;
    DWORD color4, color5, color6;
    DWORD color1, color2, color3;
    DWORD colorA0, colorA1, colorA2, colorA3,
            colorB0, colorB1, colorB2, colorB3,
            colorS1, colorS2;
    DWORD product1a, product1b,
            product2a, product2b;

    line = 0;

    {
        for (; height; height -= 1) {
            bP = (DWORD *) srcPtr;
            dP = (DWORD *) (dstBitmap + line * dstPitch);
            for (finish = width; finish; finish -= 1) {
                //---------------------------------------    B1 B2
                //                                         4  5  6 S2
                //                                         1  2  3 S1
                //                                           A1 A2
                if (finish == width) iXA = 0;
                else iXA = 1;
                if (finish > 4) {
                    iXB = 1;
                    iXC = 2;
                } else
                    if (finish > 3) {
                    iXB = 1;
                    iXC = 1;
                } else {
                    iXB = 0;
                    iXC = 0;
                }
                if (line == 0) iYA = 0;
                else iYA = width;
                if (height > 4) {
                    iYB = width;
                    iYC = width2;
                } else
                    if (height > 3) {
                    iYB = width;
                    iYC = width;
                } else {
                    iYB = 0;
                    iYC = 0;
                }


                colorB0 = *(bP - iYA - iXA);
                colorB1 = *(bP - iYA);
                colorB2 = *(bP - iYA + iXB);
                colorB3 = *(bP - iYA + iXC);

                color4 = *(bP - iXA);
                color5 = *(bP);
                color6 = *(bP + iXB);
                colorS2 = *(bP + iXC);

                color1 = *(bP + iYB - iXA);
                color2 = *(bP + iYB);
                color3 = *(bP + iYB + iXB);
                colorS1 = *(bP + iYB + iXC);

                colorA0 = *(bP + iYC - iXA);
                colorA1 = *(bP + iYC);
                colorA2 = *(bP + iYC + iXB);
                colorA3 = *(bP + iYC + iXC);

                //--------------------------------------
                if (color2 == color6 && color5 != color3) {
                    product2b = product1b = color2;
                } else
                    if (color5 == color3 && color2 != color6) {
                    product2b = product1b = color5;
                } else
                    if (color5 == color3 && color2 == color6) {
                    register int r = 0;

                    r += GET_RESULT((color6 & 0x00ffffff), (color5 & 0x00ffffff), (color1 & 0x00ffffff), (colorA1 & 0x00ffffff));
                    r += GET_RESULT((color6 & 0x00ffffff), (color5 & 0x00ffffff), (color4 & 0x00ffffff), (colorB1 & 0x00ffffff));
                    r += GET_RESULT((color6 & 0x00ffffff), (color5 & 0x00ffffff), (colorA2 & 0x00ffffff), (colorS1 & 0x00ffffff));
                    r += GET_RESULT((color6 & 0x00ffffff), (color5 & 0x00ffffff), (colorB2 & 0x00ffffff), (colorS2 & 0x00ffffff));

                    if (r > 0)
                        product2b = product1b = color6;
                    else
                        if (r < 0)
                        product2b = product1b = color5;
                    else {
                        product2b = product1b = INTERPOLATE8_02(color5, color6);
                    }
                } else {
                    if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
                        product2b = Q_INTERPOLATE8_02(color3, color3, color3, color2);
                    else
                        if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
                        product2b = Q_INTERPOLATE8_02(color2, color2, color2, color3);
                    else
                        product2b = INTERPOLATE8_02(color2, color3);

                    if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
                        product1b = Q_INTERPOLATE8_02(color6, color6, color6, color5);
                    else
                        if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
                        product1b = Q_INTERPOLATE8_02(color6, color5, color5, color5);
                    else
                        product1b = INTERPOLATE8_02(color5, color6);
                }

                if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
                    product2a = INTERPOLATE8_02(color2, color5);
                else
                    if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
                    product2a = INTERPOLATE8_02(color2, color5);
                else
                    product2a = color2;

                if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
                    product1a = INTERPOLATE8_02(color2, color5);
                else
                    if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
                    product1a = INTERPOLATE8_02(color2, color5);
                else
                    product1a = color5;

                *dP = product1a;
                *(dP + 1) = product1b;
                *(dP + (width2)) = product2a;
                *(dP + 1 + (width2)) = product2b;

                bP += 1;
                dP += 2;
            }//end of for ( finish= width etc..)

            line += 2;
            srcPtr += srcPitch;
        }; //endof: for (; height; height--)
    }
}

void Super2xSaI_ex8(unsigned char *srcPtr, DWORD srcPitch,
        unsigned char *dstBitmap, int width, int height) {
    DWORD dstPitch = srcPitch * 2;
    DWORD line;
    DWORD *dP;
    DWORD *bP;
    int width2 = width * 2;
    int iXA, iXB, iXC, iYA, iYB, iYC, finish;
    DWORD color4, color5, color6;
    DWORD color1, color2, color3;
    DWORD colorA0, colorA1, colorA2, colorA3,
            colorB0, colorB1, colorB2, colorB3,
            colorS1, colorS2;
    DWORD product1a, product1b,
            product2a, product2b;

    line = 0;

    {
        for (; height; height -= 1) {
            bP = (DWORD *) srcPtr;
            dP = (DWORD *) (dstBitmap + line * dstPitch);
            for (finish = width; finish; finish -= 1) {
                //---------------------------------------    B1 B2
                //                                         4  5  6 S2
                //                                         1  2  3 S1
                //                                           A1 A2
                if (finish == width) iXA = 0;
                else iXA = 1;
                if (finish > 4) {
                    iXB = 1;
                    iXC = 2;
                } else
                    if (finish > 3) {
                    iXB = 1;
                    iXC = 1;
                } else {
                    iXB = 0;
                    iXC = 0;
                }
                if (line == 0) iYA = 0;
                else iYA = width;
                if (height > 4) {
                    iYB = width;
                    iYC = width2;
                } else
                    if (height > 3) {
                    iYB = width;
                    iYC = width;
                } else {
                    iYB = 0;
                    iYC = 0;
                }


                colorB0 = *(bP - iYA - iXA);
                colorB1 = *(bP - iYA);
                colorB2 = *(bP - iYA + iXB);
                colorB3 = *(bP - iYA + iXC);

                color4 = *(bP - iXA);
                color5 = *(bP);
                color6 = *(bP + iXB);
                colorS2 = *(bP + iXC);

                color1 = *(bP + iYB - iXA);
                color2 = *(bP + iYB);
                color3 = *(bP + iYB + iXB);
                colorS1 = *(bP + iYB + iXC);

                colorA0 = *(bP + iYC - iXA);
                colorA1 = *(bP + iYC);
                colorA2 = *(bP + iYC + iXB);
                colorA3 = *(bP + iYC + iXC);

                //--------------------------------------
                if (color2 == color6 && color5 != color3) {
                    product2b = product1b = color2;
                } else
                    if (color5 == color3 && color2 != color6) {
                    product2b = product1b = color5;
                } else
                    if (color5 == color3 && color2 == color6) {
                    register int r = 0;

                    r += GET_RESULT((color6 & 0x00ffffff), (color5 & 0x00ffffff), (color1 & 0x00ffffff), (colorA1 & 0x00ffffff));
                    r += GET_RESULT((color6 & 0x00ffffff), (color5 & 0x00ffffff), (color4 & 0x00ffffff), (colorB1 & 0x00ffffff));
                    r += GET_RESULT((color6 & 0x00ffffff), (color5 & 0x00ffffff), (colorA2 & 0x00ffffff), (colorS1 & 0x00ffffff));
                    r += GET_RESULT((color6 & 0x00ffffff), (color5 & 0x00ffffff), (colorB2 & 0x00ffffff), (colorS2 & 0x00ffffff));

                    if (r > 0)
                        product2b = product1b = color6;
                    else
                        if (r < 0)
                        product2b = product1b = color5;
                    else {
                        product2b = product1b = INTERPOLATE8(color5, color6);
                    }
                } else {
                    if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
                        product2b = Q_INTERPOLATE8(color3, color3, color3, color2);
                    else
                        if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
                        product2b = Q_INTERPOLATE8(color2, color2, color2, color3);
                    else
                        product2b = INTERPOLATE8(color2, color3);

                    if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
                        product1b = Q_INTERPOLATE8(color6, color6, color6, color5);
                    else
                        if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
                        product1b = Q_INTERPOLATE8(color6, color5, color5, color5);
                    else
                        product1b = INTERPOLATE8(color5, color6);
                }

                if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
                    product2a = INTERPOLATE8(color2, color5);
                else
                    if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
                    product2a = INTERPOLATE8(color2, color5);
                else
                    product2a = color2;

                if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
                    product1a = INTERPOLATE8(color2, color5);
                else
                    if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
                    product1a = INTERPOLATE8(color2, color5);
                else
                    product1a = color5;

                *dP = product1a;
                *(dP + 1) = product1b;
                *(dP + (width2)) = product2a;
                *(dP + 1 + (width2)) = product2b;

                bP += 1;
                dP += 2;
            }//end of for ( finish= width etc..)

            line += 2;
            srcPtr += srcPitch;
        }; //endof: for (; height; height--)
    }
}
/////////////////////////////////////////////////////////////////////////////

#define colorMask4     0x0000EEE0
#define lowPixelMask4  0x00001110
#define qcolorMask4    0x0000CCC0
#define qlowpixelMask4 0x00003330

#define INTERPOLATE4(A, B) ((((A & colorMask4) >> 1) + ((B & colorMask4) >> 1) + (A & B & lowPixelMask4))|((((A&0x0000000F)==0x00000006)?0x00000006:(((B&0x0000000F)==0x00000006)?0x00000006:(((A&0x0000000F)==0x00000000)?0x00000000:(((B&0x0000000F)==0x00000000)?0x00000000:0x0000000F))))))

#define Q_INTERPOLATE4(A, B, C, D) ((((A & qcolorMask4) >> 2) + ((B & qcolorMask4) >> 2) + ((C & qcolorMask4) >> 2) + ((D & qcolorMask4) >> 2) + ((((A & qlowpixelMask4) + (B & qlowpixelMask4) + (C & qlowpixelMask4) + (D & qlowpixelMask4)) >> 2) & qlowpixelMask4))| ((((A&0x0000000F)==0x00000006)?0x00000006:(((B&0x0000000F)==0x00000006)?0x00000006:(((C&0x0000000F)==0x00000006)?0x00000006:(((D&0x0000000F)==0x00000006)?0x00000006:(((A&0x0000000F)==0x00000000)?0x00000000:(((B&0x0000000F)==0x00000000)?0x00000000:(((C&0x0000000F)==0x00000000)?0x00000000:(((D&0x0000000F)==0x00000000)?0x00000000:0x0000000F))))))))))

void Super2xSaI_ex4(unsigned char *srcPtr, DWORD srcPitch,
        unsigned char *dstBitmap, int width, int height) {
    DWORD dstPitch = srcPitch * 2;
    DWORD line;
    unsigned short *dP;
    unsigned short *bP;
    int width2 = width * 2;
    int iXA, iXB, iXC, iYA, iYB, iYC, finish;
    DWORD color4, color5, color6;
    DWORD color1, color2, color3;
    DWORD colorA0, colorA1, colorA2, colorA3,
            colorB0, colorB1, colorB2, colorB3,
            colorS1, colorS2;
    DWORD product1a, product1b,
            product2a, product2b;

    line = 0;

    {
        for (; height; height -= 1) {
            bP = (unsigned short *) srcPtr;
            dP = (unsigned short *) (dstBitmap + line * dstPitch);
            for (finish = width; finish; finish -= 1) {
                //---------------------------------------    B1 B2
                //                                         4  5  6 S2
                //                                         1  2  3 S1
                //                                           A1 A2
                if (finish == width) iXA = 0;
                else iXA = 1;
                if (finish > 4) {
                    iXB = 1;
                    iXC = 2;
                } else
                    if (finish > 3) {
                    iXB = 1;
                    iXC = 1;
                } else {
                    iXB = 0;
                    iXC = 0;
                }
                if (line == 0) iYA = 0;
                else iYA = width;
                if (height > 4) {
                    iYB = width;
                    iYC = width2;
                } else
                    if (height > 3) {
                    iYB = width;
                    iYC = width;
                } else {
                    iYB = 0;
                    iYC = 0;
                }


                colorB0 = *(bP - iYA - iXA);
                colorB1 = *(bP - iYA);
                colorB2 = *(bP - iYA + iXB);
                colorB3 = *(bP - iYA + iXC);

                color4 = *(bP - iXA);
                color5 = *(bP);
                color6 = *(bP + iXB);
                colorS2 = *(bP + iXC);

                color1 = *(bP + iYB - iXA);
                color2 = *(bP + iYB);
                color3 = *(bP + iYB + iXB);
                colorS1 = *(bP + iYB + iXC);

                colorA0 = *(bP + iYC - iXA);
                colorA1 = *(bP + iYC);
                colorA2 = *(bP + iYC + iXB);
                colorA3 = *(bP + iYC + iXC);

                //--------------------------------------
                if (color2 == color6 && color5 != color3) {
                    product2b = product1b = color2;
                } else
                    if (color5 == color3 && color2 != color6) {
                    product2b = product1b = color5;
                } else
                    if (color5 == color3 && color2 == color6) {
                    register int r = 0;

                    r += GET_RESULT((color6 & 0xfffffff0), (color5 & 0xfffffff0), (color1 & 0xfffffff0), (colorA1 & 0xfffffff0));
                    r += GET_RESULT((color6 & 0xfffffff0), (color5 & 0xfffffff0), (color4 & 0xfffffff0), (colorB1 & 0xfffffff0));
                    r += GET_RESULT((color6 & 0xfffffff0), (color5 & 0xfffffff0), (colorA2 & 0xfffffff0), (colorS1 & 0xfffffff0));
                    r += GET_RESULT((color6 & 0xfffffff0), (color5 & 0xfffffff0), (colorB2 & 0xfffffff0), (colorS2 & 0xfffffff0));

                    if (r > 0)
                        product2b = product1b = color6;
                    else
                        if (r < 0)
                        product2b = product1b = color5;
                    else {
                        product2b = product1b = INTERPOLATE4(color5, color6);
                    }
                } else {
                    if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
                        product2b = Q_INTERPOLATE4(color3, color3, color3, color2);
                    else
                        if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
                        product2b = Q_INTERPOLATE4(color2, color2, color2, color3);
                    else
                        product2b = INTERPOLATE4(color2, color3);

                    if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
                        product1b = Q_INTERPOLATE4(color6, color6, color6, color5);
                    else
                        if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
                        product1b = Q_INTERPOLATE4(color6, color5, color5, color5);
                    else
                        product1b = INTERPOLATE4(color5, color6);
                }

                if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
                    product2a = INTERPOLATE4(color2, color5);
                else
                    if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
                    product2a = INTERPOLATE4(color2, color5);
                else
                    product2a = color2;

                if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
                    product1a = INTERPOLATE4(color2, color5);
                else
                    if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
                    product1a = INTERPOLATE4(color2, color5);
                else
                    product1a = color5;

                *dP = product1a;
                *(dP + 1) = product1b;
                *(dP + (width2)) = product2a;
                *(dP + 1 + (width2)) = product2b;

                bP += 1;
                dP += 2;
            }//end of for ( finish= width etc..)

            line += 2;
            srcPtr += srcPitch;
        }; //endof: for (; height; height--)
    }
}

/////////////////////////////////////////////////////////////////////////////

#define colorMask5     0x0000F7BC
#define lowPixelMask5  0x00000842
#define qcolorMask5    0x0000E738
#define qlowpixelMask5 0x000018C6

#define INTERPOLATE5(A, B) ((((A & colorMask5) >> 1) + ((B & colorMask5) >> 1) + (A & B & lowPixelMask5))|((((A&0x00000001)==0x00000000)?0x00000000:(((B&0x00000001)==0x00000000)?0x00000000:0x00000001))))

#define Q_INTERPOLATE5(A, B, C, D) ((((A & qcolorMask5) >> 2) + ((B & qcolorMask5) >> 2) + ((C & qcolorMask5) >> 2) + ((D & qcolorMask5) >> 2) + ((((A & qlowpixelMask5) + (B & qlowpixelMask5) + (C & qlowpixelMask5) + (D & qlowpixelMask5)) >> 2) & qlowpixelMask5))| ((((A&0x00000001)==0x00000000)?0x00000000:(((B&0x00000001)==0x00000000)?0x00000000:(((C&0x00000001)==0x00000000)?0x00000000:(((D&0x00000001)==0x00000000)?0x00000000:0x00000001))))))

void Super2xSaI_ex5(unsigned char *srcPtr, DWORD srcPitch,
        unsigned char *dstBitmap, int width, int height) {
    DWORD dstPitch = srcPitch * 2;
    DWORD line;
    unsigned short *dP;
    unsigned short *bP;
    int width2 = width * 2;
    int iXA, iXB, iXC, iYA, iYB, iYC, finish;
    DWORD color4, color5, color6;
    DWORD color1, color2, color3;
    DWORD colorA0, colorA1, colorA2, colorA3,
            colorB0, colorB1, colorB2, colorB3,
            colorS1, colorS2;
    DWORD product1a, product1b,
            product2a, product2b;

    line = 0;

    {
        for (; height; height -= 1) {
            bP = (unsigned short *) srcPtr;
            dP = (unsigned short *) (dstBitmap + line * dstPitch);
            for (finish = width; finish; finish -= 1) {
                //---------------------------------------    B1 B2
                //                                         4  5  6 S2
                //                                         1  2  3 S1
                //                                           A1 A2
                if (finish == width) iXA = 0;
                else iXA = 1;
                if (finish > 4) {
                    iXB = 1;
                    iXC = 2;
                } else
                    if (finish > 3) {
                    iXB = 1;
                    iXC = 1;
                } else {
                    iXB = 0;
                    iXC = 0;
                }
                if (line == 0) iYA = 0;
                else iYA = width;
                if (height > 4) {
                    iYB = width;
                    iYC = width2;
                } else
                    if (height > 3) {
                    iYB = width;
                    iYC = width;
                } else {
                    iYB = 0;
                    iYC = 0;
                }


                colorB0 = *(bP - iYA - iXA);
                colorB1 = *(bP - iYA);
                colorB2 = *(bP - iYA + iXB);
                colorB3 = *(bP - iYA + iXC);

                color4 = *(bP - iXA);
                color5 = *(bP);
                color6 = *(bP + iXB);
                colorS2 = *(bP + iXC);

                color1 = *(bP + iYB - iXA);
                color2 = *(bP + iYB);
                color3 = *(bP + iYB + iXB);
                colorS1 = *(bP + iYB + iXC);

                colorA0 = *(bP + iYC - iXA);
                colorA1 = *(bP + iYC);
                colorA2 = *(bP + iYC + iXB);
                colorA3 = *(bP + iYC + iXC);

                //--------------------------------------
                if (color2 == color6 && color5 != color3) {
                    product2b = product1b = color2;
                } else
                    if (color5 == color3 && color2 != color6) {
                    product2b = product1b = color5;
                } else
                    if (color5 == color3 && color2 == color6) {
                    register int r = 0;

                    r += GET_RESULT((color6 & 0xfffffffe), (color5 & 0xfffffffe), (color1 & 0xfffffffe), (colorA1 & 0xfffffffe));
                    r += GET_RESULT((color6 & 0xfffffffe), (color5 & 0xfffffffe), (color4 & 0xfffffffe), (colorB1 & 0xfffffffe));
                    r += GET_RESULT((color6 & 0xfffffffe), (color5 & 0xfffffffe), (colorA2 & 0xfffffffe), (colorS1 & 0xfffffffe));
                    r += GET_RESULT((color6 & 0xfffffffe), (color5 & 0xfffffffe), (colorB2 & 0xfffffffe), (colorS2 & 0xfffffffe));

                    if (r > 0)
                        product2b = product1b = color6;
                    else
                        if (r < 0)
                        product2b = product1b = color5;
                    else {
                        product2b = product1b = INTERPOLATE5(color5, color6);
                    }
                } else {
                    if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
                        product2b = Q_INTERPOLATE5(color3, color3, color3, color2);
                    else
                        if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
                        product2b = Q_INTERPOLATE5(color2, color2, color2, color3);
                    else
                        product2b = INTERPOLATE5(color2, color3);

                    if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
                        product1b = Q_INTERPOLATE5(color6, color6, color6, color5);
                    else
                        if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
                        product1b = Q_INTERPOLATE5(color6, color5, color5, color5);
                    else
                        product1b = INTERPOLATE5(color5, color6);
                }

                if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
                    product2a = INTERPOLATE5(color2, color5);
                else
                    if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
                    product2a = INTERPOLATE5(color2, color5);
                else
                    product2a = color2;

                if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
                    product1a = INTERPOLATE5(color2, color5);
                else
                    if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
                    product1a = INTERPOLATE5(color2, color5);
                else
                    product1a = color5;

                *dP = product1a;
                *(dP + 1) = product1b;
                *(dP + (width2)) = product2a;
                *(dP + 1 + (width2)) = product2b;

                bP += 1;
                dP += 2;
            }//end of for ( finish= width etc..)

            line += 2;
            srcPtr += srcPitch;
        }; //endof: for (; height; height--)
    }
}
