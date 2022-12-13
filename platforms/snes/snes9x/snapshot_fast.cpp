uint32 S9xFreezeSizeFast()
{
    uint32 old_size = S9xFreezeSize();
    uint8* mem = (uint8*) malloc (old_size);
    memStream stream(mem, old_size);
    S9xFreezeToStreamFast(&stream);
    uint32 new_size = stream.size();
    free(mem);
    return new_size;
}

static int UnfreezeBlockCopyFast (memStream* stream, const char *name, uint8 *block, int size)
{
 int result;

 result = UnfreezeBlock(stream, name, block, size);

 return (SUCCESS);
}

void S9xFreezeToStreamFast (memStream* stream)
{
 char buffer[1024];
 uint8 *soundsnapshot = new uint8[SPC_SAVE_STATE_BLOCK_SIZE];

 sprintf(buffer, "%s:%04d\n", SNAPSHOT_MAGIC, SNAPSHOT_VERSION);
 WRITE_STREAM(buffer, strlen(buffer), stream);

 sprintf(buffer, "NAM:%06d:%s%c", (int) strlen(Memory.ROMFilename) + 1, Memory.ROMFilename, 0);
 WRITE_STREAM(buffer, strlen(buffer) + 1, stream);

 FreezeStruct(stream, "CPU", &CPU, SnapCPU, COUNT(SnapCPU));

 FreezeStruct(stream, "REG", &Registers, SnapRegisters, COUNT(SnapRegisters));

 FreezeStruct(stream, "PPU", &PPU, SnapPPU, COUNT(SnapPPU));

 struct SDMASnapshot dma_snap;
 for (int d = 0; d < 8; d++)
  dma_snap.dma[d] = DMA[d];
 FreezeStruct(stream, "DMA", &dma_snap, SnapDMA, COUNT(SnapDMA));

 FreezeBlock (stream, "RAM", Memory.RAM, 0x20000);

 FreezeBlock (stream, "FIL", Memory.FillRAM, 0x8000);

 S9xAPUSaveState(soundsnapshot);
 FreezeBlock (stream, "SND", soundsnapshot, SPC_SAVE_STATE_BLOCK_SIZE);

 struct SControlSnapshot ctl_snap;
 S9xControlPreSaveState(&ctl_snap);
 FreezeStruct(stream, "CTL", &ctl_snap, SnapControls, COUNT(SnapControls));

 FreezeStruct(stream, "TIM", &Timings, SnapTimings, COUNT(SnapTimings));

 if (Settings.SuperFX)
 {
  GSU.avRegAddr = (uint8 *) &GSU.avReg;
  FreezeStruct(stream, "SFX", &GSU, SnapFX, COUNT(SnapFX));
 }

 if (Settings.SA1)
 {
  S9xSA1PackStatus();
  FreezeStruct(stream, "SA1", &SA1, SnapSA1, COUNT(SnapSA1));
  FreezeStruct(stream, "SAR", &SA1Registers, SnapSA1Registers, COUNT(SnapSA1Registers));
 }

 if (Settings.DSP == 1)
  FreezeStruct(stream, "DP1", &DSP1, SnapDSP1, COUNT(SnapDSP1));

 if (Settings.DSP == 2)
  FreezeStruct(stream, "DP2", &DSP2, SnapDSP2, COUNT(SnapDSP2));

 if (Settings.DSP == 4)
  FreezeStruct(stream, "DP4", &DSP4, SnapDSP4, COUNT(SnapDSP4));

 if (Settings.C4)
  FreezeBlock (stream, "CX4", Memory.C4RAM, 8192);

 if (Settings.SETA == ST_010)
  FreezeStruct(stream, "ST0", &ST010, SnapST010, COUNT(SnapST010));

 if (Settings.OBC1)
 {
  FreezeStruct(stream, "OBC", &OBC1, SnapOBC1, COUNT(SnapOBC1));
  FreezeBlock (stream, "OBM", Memory.OBC1RAM, 8192);
 }

 if (Settings.SPC7110)
 {
  S9xSPC7110PreSaveState();
  FreezeStruct(stream, "S71", &s7snap, SnapSPC7110Snap, COUNT(SnapSPC7110Snap));
 }

 if (Settings.SRTC)
 {
  S9xSRTCPreSaveState();
  FreezeStruct(stream, "SRT", &srtcsnap, SnapSRTCSnap, COUNT(SnapSRTCSnap));
 }

 if (Settings.SRTC || Settings.SPC7110RTC)
  FreezeBlock (stream, "CLK", RTCData.reg, 20);

 if (Settings.BS)
  FreezeStruct(stream, "BSX", &BSX, SnapBSX, COUNT(SnapBSX));

 if (Settings.MSU1)
  FreezeStruct(stream, "MSU", &MSU1, SnapMSU1, COUNT(SnapMSU1));

 delete [] soundsnapshot;
}

int S9xUnfreezeFromStreamFast (memStream* stream)
{
 int  result = SUCCESS;
 int  version, len;
 char buffer[PATH_MAX + 1];

 len = strlen(SNAPSHOT_MAGIC) + 1 + 4 + 1;
 if (READ_STREAM(buffer, len, stream) != len)
  return (WRONG_FORMAT);

 if (strncmp(buffer, SNAPSHOT_MAGIC, strlen(SNAPSHOT_MAGIC)) != 0)
  return (WRONG_FORMAT);

 version = atoi(&buffer[strlen(SNAPSHOT_MAGIC) + 1]);
 if (version > SNAPSHOT_VERSION)
  return (WRONG_VERSION);

 result = UnfreezeBlock(stream, "NAM", (uint8 *) buffer, PATH_MAX);
 if (result != SUCCESS)
  return (result);

 uint8 *local_cpu           = NULL;
 uint8 *local_registers     = NULL;
 uint8 *local_ppu           = NULL;
 uint8 *local_dma           = NULL;
 uint8 *local_apu_sound     = NULL;
 uint8 *local_control_data  = NULL;
 uint8 *local_timing_data   = NULL;
 uint8 *local_superfx       = NULL;
 uint8 *local_sa1           = NULL;
 uint8 *local_sa1_registers = NULL;
 uint8 *local_dsp1          = NULL;
 uint8 *local_dsp2          = NULL;
 uint8 *local_dsp4          = NULL;
 uint8 *local_st010         = NULL;
 uint8 *local_obc1          = NULL;
 uint8 *local_obc1_data     = NULL;
 uint8 *local_spc7110       = NULL;
 uint8 *local_srtc          = NULL;
 uint8 *local_rtc_data      = NULL;
 uint8 *local_bsx_data      = NULL;
 uint8 *local_msu1_data     = NULL;

 do
 {
  result = UnfreezeStructCopy(stream, "CPU", &local_cpu, SnapCPU, COUNT(SnapCPU), version);
  if (result != SUCCESS)
   break;

  result = UnfreezeStructCopy(stream, "REG", &local_registers, SnapRegisters, COUNT(SnapRegisters), version);
  if (result != SUCCESS)
   break;

  result = UnfreezeStructCopy(stream, "PPU", &local_ppu, SnapPPU, COUNT(SnapPPU), version);
  if (result != SUCCESS)
   break;

  result = UnfreezeStructCopy(stream, "DMA", &local_dma, SnapDMA, COUNT(SnapDMA), version);
  if (result != SUCCESS)
   break;

  result = UnfreezeBlockCopyFast (stream, "RAM", Memory.RAM, 0x20000);
  if (result != SUCCESS)
   break;

  result = UnfreezeBlockCopyFast (stream, "FIL", Memory.FillRAM, 0x8000);
  if (result != SUCCESS)
   break;

  result = UnfreezeBlockCopy (stream, "SND", &local_apu_sound, SPC_SAVE_STATE_BLOCK_SIZE);
  if (result != SUCCESS)
   break;

  result = UnfreezeStructCopy(stream, "CTL", &local_control_data, SnapControls, COUNT(SnapControls), version);
  if (result != SUCCESS)
   break;

  result = UnfreezeStructCopy(stream, "TIM", &local_timing_data, SnapTimings, COUNT(SnapTimings), version);
  if (result != SUCCESS)
   break;

  result = UnfreezeStructCopy(stream, "SFX", &local_superfx, SnapFX, COUNT(SnapFX), version);
  if (result != SUCCESS && Settings.SuperFX)
   break;

  result = UnfreezeStructCopy(stream, "SA1", &local_sa1, SnapSA1, COUNT(SnapSA1), version);
  if (result != SUCCESS && Settings.SA1)
   break;

  result = UnfreezeStructCopy(stream, "SAR", &local_sa1_registers, SnapSA1Registers, COUNT(SnapSA1Registers), version);
  if (result != SUCCESS && Settings.SA1)
   break;

  result = UnfreezeStructCopy(stream, "DP1", &local_dsp1, SnapDSP1, COUNT(SnapDSP1), version);
  if (result != SUCCESS && Settings.DSP == 1)
   break;

  result = UnfreezeStructCopy(stream, "DP2", &local_dsp2, SnapDSP2, COUNT(SnapDSP2), version);
  if (result != SUCCESS && Settings.DSP == 2)
   break;

  result = UnfreezeStructCopy(stream, "DP4", &local_dsp4, SnapDSP4, COUNT(SnapDSP4), version);
  if (result != SUCCESS && Settings.DSP == 4)
   break;

  result = UnfreezeBlockCopyFast (stream, "CX4", Memory.C4RAM, 8192);
  if (result != SUCCESS && Settings.C4)
   break;

  result = UnfreezeStructCopy(stream, "ST0", &local_st010, SnapST010, COUNT(SnapST010), version);
  if (result != SUCCESS && Settings.SETA == ST_010)
   break;

  result = UnfreezeStructCopy(stream, "OBC", &local_obc1, SnapOBC1, COUNT(SnapOBC1), version);
  if (result != SUCCESS && Settings.OBC1)
   break;

  result = UnfreezeBlockCopyFast (stream, "OBM", Memory.OBC1RAM, 8192);
  if (result != SUCCESS && Settings.OBC1)
   break;

  result = UnfreezeStructCopy(stream, "S71", &local_spc7110, SnapSPC7110Snap, COUNT(SnapSPC7110Snap), version);
  if (result != SUCCESS && Settings.SPC7110)
   break;

  result = UnfreezeStructCopy(stream, "SRT", &local_srtc, SnapSRTCSnap, COUNT(SnapSRTCSnap), version);
  if (result != SUCCESS && Settings.SRTC)
   break;

  result = UnfreezeBlockCopy (stream, "CLK", &local_rtc_data, 20);
  if (result != SUCCESS && (Settings.SRTC || Settings.SPC7110RTC))
   break;

  result = UnfreezeStructCopy(stream, "BSX", &local_bsx_data, SnapBSX, COUNT(SnapBSX), version);
  if (result != SUCCESS && Settings.BS)
   break;

  result = UnfreezeStructCopy(stream, "MSU", &local_msu1_data, SnapMSU1, COUNT(SnapMSU1), version);
  if (result != SUCCESS && Settings.MSU1)
   break;

  result = SUCCESS;
 } while (false);

 if (result == SUCCESS)
 {
  uint32 old_flags     = CPU.Flags;
  uint32 sa1_old_flags = SA1.Flags;

  //S9xReset();

  UnfreezeStructFromCopy(&CPU, SnapCPU, COUNT(SnapCPU), local_cpu, version);

  UnfreezeStructFromCopy(&Registers, SnapRegisters, COUNT(SnapRegisters), local_registers, version);

  UnfreezeStructFromCopy(&PPU, SnapPPU, COUNT(SnapPPU), local_ppu, version);

  struct SDMASnapshot dma_snap;
  UnfreezeStructFromCopy(&dma_snap, SnapDMA, COUNT(SnapDMA), local_dma, version);

        if(version < SNAPSHOT_VERSION_BAPU) {
            printf("Using Blargg APU snapshot loading (snapshot version %d, current is %d)\n...", version, SNAPSHOT_VERSION);
            S9xAPULoadBlarggState(local_apu_sound);
        } else
      S9xAPULoadState(local_apu_sound);

  struct SControlSnapshot ctl_snap;
  UnfreezeStructFromCopy(&ctl_snap, SnapControls, COUNT(SnapControls), local_control_data, version);

  UnfreezeStructFromCopy(&Timings, SnapTimings, COUNT(SnapTimings), local_timing_data, version);

  if (local_superfx)
  {
   GSU.avRegAddr = (uint8 *) &GSU.avReg;
   UnfreezeStructFromCopy(&GSU, SnapFX, COUNT(SnapFX), local_superfx, version);
  }

  if (local_sa1)
   UnfreezeStructFromCopy(&SA1, SnapSA1, COUNT(SnapSA1), local_sa1, version);

  if (local_sa1_registers)
   UnfreezeStructFromCopy(&SA1Registers, SnapSA1Registers, COUNT(SnapSA1Registers), local_sa1_registers, version);

  if (local_dsp1)
   UnfreezeStructFromCopy(&DSP1, SnapDSP1, COUNT(SnapDSP1), local_dsp1, version);

  if (local_dsp2)
   UnfreezeStructFromCopy(&DSP2, SnapDSP2, COUNT(SnapDSP2), local_dsp2, version);

  if (local_dsp4)
   UnfreezeStructFromCopy(&DSP4, SnapDSP4, COUNT(SnapDSP4), local_dsp4, version);

  if (local_st010)
   UnfreezeStructFromCopy(&ST010, SnapST010, COUNT(SnapST010), local_st010, version);

  if (local_obc1)
   UnfreezeStructFromCopy(&OBC1, SnapOBC1, COUNT(SnapOBC1), local_obc1, version);

  if (local_spc7110)
   UnfreezeStructFromCopy(&s7snap, SnapSPC7110Snap, COUNT(SnapSPC7110Snap), local_spc7110, version);

  if (local_srtc)
   UnfreezeStructFromCopy(&srtcsnap, SnapSRTCSnap, COUNT(SnapSRTCSnap), local_srtc, version);

  if (local_rtc_data)
   memcpy(RTCData.reg, local_rtc_data, 20);

  if (local_bsx_data)
   UnfreezeStructFromCopy(&BSX, SnapBSX, COUNT(SnapBSX), local_bsx_data, version);

  if (local_msu1_data)
   UnfreezeStructFromCopy(&MSU1, SnapMSU1, COUNT(SnapMSU1), local_msu1_data, version);

  if (version < SNAPSHOT_VERSION_IRQ)
  {
   printf("Converting old snapshot version %d to %d\n...", version, SNAPSHOT_VERSION);

   CPU.NMILine = (CPU.Flags & (1 <<  7)) ? TRUE : FALSE;
   CPU.IRQLine = (CPU.Flags & (1 << 11)) ? TRUE : FALSE;
   CPU.IRQTransition = FALSE;
   CPU.IRQLastState = FALSE;
   CPU.IRQExternal = (Obsolete.CPU_IRQActive & ~(1 << 1)) ? TRUE : FALSE;

   switch (CPU.WhichEvent)
   {
    case 12: case   1: CPU.WhichEvent = 1; break;
    case  2: case   3: CPU.WhichEvent = 2; break;
    case  4: case   5: CPU.WhichEvent = 3; break;
    case  6: case   7: CPU.WhichEvent = 4; break;
    case  8: case   9: CPU.WhichEvent = 5; break;
    case 10: case  11: CPU.WhichEvent = 6; break;
   }

   if (local_sa1) // FIXME
   {
    SA1.Cycles = SA1.PrevCycles = 0;
    SA1.TimerIRQLastState = FALSE;
    SA1.HTimerIRQPos = Memory.FillRAM[0x2212] | (Memory.FillRAM[0x2213] << 8);
    SA1.VTimerIRQPos = Memory.FillRAM[0x2214] | (Memory.FillRAM[0x2215] << 8);
    SA1.HCounter = 0;
    SA1.VCounter = 0;
    SA1.PrevHCounter = 0;
    SA1.MemSpeed = SLOW_ONE_CYCLE;
    SA1.MemSpeedx2 = SLOW_ONE_CYCLE * 2;
   }
  }

  CPU.Flags |= old_flags & (DEBUG_MODE_FLAG | TRACE_FLAG | SINGLE_STEP_FLAG | FRAME_ADVANCE_FLAG);
  ICPU.ShiftedPB = Registers.PB << 16;
  ICPU.ShiftedDB = Registers.DB << 16;
  S9xSetPCBase(Registers.PBPC);
  S9xUnpackStatus();
  S9xFixCycles();

  for (int d = 0; d < 8; d++)
   DMA[d] = dma_snap.dma[d];
  CPU.InDMA = CPU.InHDMA = FALSE;
  CPU.InDMAorHDMA = CPU.InWRAMDMAorHDMA = FALSE;
  CPU.HDMARanInDMA = 0;

  S9xFixColourBrightness();
  IPPU.ColorsChanged = TRUE;
  IPPU.OBJChanged = TRUE;
  IPPU.RenderThisFrame = TRUE;

  uint8 hdma_byte = Memory.FillRAM[0x420c];
  S9xSetCPU(hdma_byte, 0x420c);

  S9xControlPostLoadState(&ctl_snap);

  if (local_superfx)
  {
   GSU.pfPlot = fx_PlotTable[GSU.vMode];
   GSU.pfRpix = fx_PlotTable[GSU.vMode + 5];
  }

  if (local_sa1 && local_sa1_registers)
  {
   SA1.Flags |= sa1_old_flags & TRACE_FLAG;
   S9xSA1PostLoadState();
  }

  if (Settings.SDD1)
   S9xSDD1PostLoadState();

  if (local_spc7110)
   S9xSPC7110PostLoadState(version);

  if (local_srtc)
   S9xSRTCPostLoadState(version);

  if (local_bsx_data)
   S9xBSXPostLoadState();

  if (local_msu1_data)
   S9xMSU1PostLoadState();
 }

 if (local_cpu)    delete [] local_cpu;
 if (local_registers)  delete [] local_registers;
 if (local_ppu)    delete [] local_ppu;
 if (local_dma)    delete [] local_dma;
 if (local_apu_sound)  delete [] local_apu_sound;
 if (local_control_data)  delete [] local_control_data;
 if (local_timing_data)  delete [] local_timing_data;
 if (local_superfx)   delete [] local_superfx;
 if (local_sa1)    delete [] local_sa1;
 if (local_sa1_registers) delete [] local_sa1_registers;
 if (local_dsp1)    delete [] local_dsp1;
 if (local_dsp2)    delete [] local_dsp2;
 if (local_dsp4)    delete [] local_dsp4;
 if (local_st010)   delete [] local_st010;
 if (local_obc1)    delete [] local_obc1;
 if (local_spc7110)   delete [] local_spc7110;
 if (local_srtc)    delete [] local_srtc;
 if (local_rtc_data)   delete [] local_rtc_data;
 if (local_bsx_data)   delete [] local_bsx_data;

 return (result);
}
