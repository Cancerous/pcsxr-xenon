#ifdef _IN_GPU
//#define _TR TR;printf("lGPUdataRet %08x\r\n",lGPUdataRet);

#define _TR

////////////////////////////////////////////////////////////////////////
// core read from vram
////////////////////////////////////////////////////////////////////////

void CALLBACK GPUreadDataMem(uint32_t * pMem, int iSize) {
    _TR
    int i;

    if (DataReadMode != DR_VRAMTRANSFER) return;

    GPUIsBusy;

    // adjust read ptr, if necessary
    while (VRAMRead.ImagePtr >= psxVuw_eom)
        VRAMRead.ImagePtr -= iGPUHeight * 1024;
    while (VRAMRead.ImagePtr < psxVuw)
        VRAMRead.ImagePtr += iGPUHeight * 1024;

    for (i = 0; i < iSize; i++) {
        // do 2 seperate 16bit reads for compatibility (wrap issues)
        if ((VRAMRead.ColsRemaining > 0) && (VRAMRead.RowsRemaining > 0)) {
            // lower 16 bit
            lGPUdataRet = (uint32_t) GETLE16(VRAMRead.ImagePtr);

            VRAMRead.ImagePtr++;
            if (VRAMRead.ImagePtr >= psxVuw_eom) VRAMRead.ImagePtr -= iGPUHeight * 1024;
            VRAMRead.RowsRemaining--;

            if (VRAMRead.RowsRemaining <= 0) {
                VRAMRead.RowsRemaining = VRAMRead.Width;
                VRAMRead.ColsRemaining--;
                VRAMRead.ImagePtr += 1024 - VRAMRead.Width;
                if (VRAMRead.ImagePtr >= psxVuw_eom) VRAMRead.ImagePtr -= iGPUHeight * 1024;
            }

            // higher 16 bit (always, even if it's an odd width)
            lGPUdataRet |= (uint32_t) GETLE16(VRAMRead.ImagePtr) << 16;
            PUTLE32(pMem, lGPUdataRet);
            pMem++;

            if (VRAMRead.ColsRemaining <= 0) {
                FinishedVRAMRead();
                goto ENDREAD;
            }

            VRAMRead.ImagePtr++;
            if (VRAMRead.ImagePtr >= psxVuw_eom) VRAMRead.ImagePtr -= iGPUHeight * 1024;
            VRAMRead.RowsRemaining--;
            if (VRAMRead.RowsRemaining <= 0) {
                VRAMRead.RowsRemaining = VRAMRead.Width;
                VRAMRead.ColsRemaining--;
                VRAMRead.ImagePtr += 1024 - VRAMRead.Width;
                if (VRAMRead.ImagePtr >= psxVuw_eom) VRAMRead.ImagePtr -= iGPUHeight * 1024;
            }
            if (VRAMRead.ColsRemaining <= 0) {
                FinishedVRAMRead();
                goto ENDREAD;
            }
        } else {
            FinishedVRAMRead();
            goto ENDREAD;
        }
    }

ENDREAD:
    GPUIsIdle;
}


////////////////////////////////////////////////////////////////////////

uint32_t CALLBACK GPUreadData(void) {
    _TR
    uint32_t l;
    GPUreadDataMem(&l, 1);
    return lGPUdataRet;
}


void CALLBACK GPUwriteDataMem(uint32_t * pMem, int iSize) {
    _TR
    unsigned char command;
    uint32_t gdata = 0;
    int i = 0;
    GPUIsBusy;
    GPUIsNotReadyForCommands;

STARTVRAM:

    if (DataWriteMode == DR_VRAMTRANSFER) {
        BOOL bFinished = FALSE;

        // make sure we are in vram
        while (VRAMWrite.ImagePtr >= psxVuw_eom)
            VRAMWrite.ImagePtr -= iGPUHeight * 1024;
        while (VRAMWrite.ImagePtr < psxVuw)
            VRAMWrite.ImagePtr += iGPUHeight * 1024;

        // now do the loop
        while (VRAMWrite.ColsRemaining > 0) {
            while (VRAMWrite.RowsRemaining > 0) {
                if (i >= iSize) {
                    goto ENDVRAM;
                }
                i++;

                gdata = GETLE32(pMem);
                pMem++;

                PUTLE16(VRAMWrite.ImagePtr, (unsigned short) gdata);
                VRAMWrite.ImagePtr++;
                if (VRAMWrite.ImagePtr >= psxVuw_eom) VRAMWrite.ImagePtr -= iGPUHeight * 1024;
                VRAMWrite.RowsRemaining--;

                if (VRAMWrite.RowsRemaining <= 0) {
                    VRAMWrite.ColsRemaining--;
                    if (VRAMWrite.ColsRemaining <= 0) // last pixel is odd width
                    {
                        gdata = (gdata & 0xFFFF) | (((uint32_t) GETLE16(VRAMWrite.ImagePtr)) << 16);
                        FinishedVRAMWrite();
                        bDoVSyncUpdate = TRUE;
                        goto ENDVRAM;
                    }
                    VRAMWrite.RowsRemaining = VRAMWrite.Width;
                    VRAMWrite.ImagePtr += 1024 - VRAMWrite.Width;
                }

                PUTLE16(VRAMWrite.ImagePtr, (unsigned short) (gdata >> 16));
                VRAMWrite.ImagePtr++;
                if (VRAMWrite.ImagePtr >= psxVuw_eom) VRAMWrite.ImagePtr -= iGPUHeight * 1024;
                VRAMWrite.RowsRemaining--;
            }

            VRAMWrite.RowsRemaining = VRAMWrite.Width;
            VRAMWrite.ColsRemaining--;
            VRAMWrite.ImagePtr += 1024 - VRAMWrite.Width;
            bFinished = TRUE;
        }

        FinishedVRAMWrite();
        if (bFinished) bDoVSyncUpdate = TRUE;
    }

ENDVRAM:

    if (DataWriteMode == DR_NORMAL) {
        void (* *primFunc)(unsigned char *);
        if (bSkipNextFrame) primFunc = primTableSkip;
        else primFunc = primTableJ;

        for (; i < iSize;) {
            if (DataWriteMode == DR_VRAMTRANSFER) goto STARTVRAM;

            gdata = GETLE32(pMem);
            pMem++;
            i++;

            if (gpuDataC == 0) {
                command = (unsigned char) ((gdata >> 24) & 0xff);
                

                //if(command>=0xb0 && command<0xc0) auxprintf("b0 %x!!!!!!!!!\n",command);

                if (primTableCX[command]) {
                    gpuDataC = primTableCX[command];
                    gpuCommand = command;
                    //PUTLE32(&gpuDataM[0], gdata);
                    gpuDataM[0] = gdata;
                    gpuDataP = 1;
                } else continue;
            } else {
                //PUTLE32(&gpuDataM[gpuDataP], gdata);
                gpuDataM[gpuDataP] = gdata;
                if (gpuDataC > 128) {
                    if ((gpuDataC == 254 && gpuDataP >= 3) ||
                            (gpuDataC == 255 && gpuDataP >= 4 && !(gpuDataP & 1))) {
                        if ((gpuDataM[gpuDataP] & 0xF000F000) == 0x50005000)
                            gpuDataP = gpuDataC - 1;
                    }
                }
                gpuDataP++;
            }

            if (gpuDataP == gpuDataC) {
                gpuDataC = gpuDataP = 0;

                //printf("gpuCommand = %02x - %08x\r\n",gpuCommand,gdata);
                primFunc[gpuCommand]((unsigned char *) gpuDataM);
                if (dwEmuFixes & 0x0001 || dwActFixes & 0x0400) // hack for emulating "gpu busy" in some games
                    iFakePrimBusy = 4;
            }
        }
    }

    lGPUdataRet = gdata;
    
    GPUIsReadyForCommands;
    GPUIsIdle;
}


void CALLBACK GPUwriteData(uint32_t gdata) {
    _TR
    PUTLE32(&gdata, gdata);
    GPUwriteDataMem(&gdata, 1);
}


long CALLBACK GPUdmaChain(uint32_t * baseAddrL, uint32_t addr) {
    _TR
    uint32_t dmaMem;
    unsigned char * baseAddrB;
    short count;
    unsigned int DMACommandCounter = 0;

    GPUIsBusy;

    lUsedAddr[0] = lUsedAddr[1] = lUsedAddr[2] = 0xffffff;

    baseAddrB = (unsigned char*) baseAddrL;

    do {
        if (iGPUHeight == 512) addr &= 0x1FFFFC;
        if (DMACommandCounter++ > 2000000) break;
        if (CheckForEndlessLoop(addr)) break;

        count = baseAddrB[addr + 3];

        dmaMem = addr + 4;

        if (count > 0) GPUwriteDataMem(&baseAddrL[dmaMem >> 2], count);

        addr = GETLE32(&baseAddrL[addr >> 2])&0xffffff;
    } while (addr != 0xffffff);

    GPUIsIdle;

    return 0;
}

#endif