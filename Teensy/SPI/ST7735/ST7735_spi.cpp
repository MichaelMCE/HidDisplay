// https://github.com/KurtE/GC9A01A_t3n
// This is the GC9A01A_t3 driver with the bloat removed, fixes and modified for the ST7735 controller

// TODO:  remove non Teensy 4.x code 



#if USE_SPI_ST7735

#include "ST7735_SPI.h"

static ST7735_SPI STORAGETYPE lcd = ST7735_SPI(SPI_CS, SPI_DC, SPI_RST, SPI_MOSI, SPI_SCK);
#include "../common.h"



//#define DEBUG_ASYNC_UPDATE  // Enable to print out dma info
//#define DEBUG_ASYNC_LEDS	// Enable to use // digitalWrites to Debug
#ifdef DEBUG_ASYNC_LEDS
#define DEBUG_PIN_1 2
#define DEBUG_PIN_2 3
#define DEBUG_PIN_3 4
#define DEBUG_PIN_4 5
#endif

#define ENABLE_ST7735_FRAMEBUFFER

#ifdef ENABLE_ST7735_FRAMEBUFFER
#define CBALLOC (ST7735_TFTHEIGHT * ST7735_TFTWIDTH * 2)
#define COUNT_WORDS_WRITE       ((ST7735_TFTHEIGHT * ST7735_TFTWIDTH) /  SCREEN_DMA_NUM_SETTINGS) // Note I know the divide will give whole number

#if defined(__MK66FX1M0__)
// T3.6 use Scatter/gather with chain to do transfer
DMASetting ST7735_SPI::_dmasettings[3][3];
//DMAChannel ST7735_SPI::_dmatx;
#elif defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
// DMASetting 	ST7735_SPI::_dmasettings[4];
// DMAChannel 	ST7735_SPI::_dmatx;
#elif defined(__MK64FX512__)
// T3.5 - had issues scatter/gather so do just use channels/interrupts
// and update and continue
DMAChannel ST7735_SPI::_dmatx;
DMAChannel ST7735_SPI::_dmarx;
uint16_t ST7735_SPI::_dma_count_remaining;
uint16_t ST7735_SPI::_dma_write_size_words;
volatile short _dma_dummy_rx;
#endif

ST7735_SPI *ST7735_SPI::_dmaActiveDisplay[3] = {0, 0, 0};

#if defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
// On T4 Setup the buffers to be used one per SPI buss... 
// This way we make sure it is hopefully in uncached memory
ST7735_DMA_Data_t ST7735_SPI::_dma_data[3];   // one structure for each SPI buss... 
#endif

// volatile uint8_t  	ST7735_SPI::_dma_state = 0;  // Use pointer to this
// as a way to get back to object...
// volatile uint32_t	ST7735_SPI::_dma_frame_count = 0;	// Can return a
// frame count...

void ST7735_SPI::dmaInterrupt(void) {
  if (_dmaActiveDisplay[0]) {
    _dmaActiveDisplay[0]->process_dma_interrupt();
  }
}
void ST7735_SPI::dmaInterrupt1(void) {
  if (_dmaActiveDisplay[1]) {
    _dmaActiveDisplay[1]->process_dma_interrupt();
  }
}
void ST7735_SPI::dmaInterrupt2(void) {
  if (_dmaActiveDisplay[2]) {
    _dmaActiveDisplay[2]->process_dma_interrupt();
  }
}


#ifdef DEBUG_ASYNC_UPDATE
extern void dumpDMA_TCD(DMABaseClass *dmabc, const char *psx_title);
#endif

void ST7735_SPI::process_dma_interrupt(void) {
#ifdef DEBUG_ASYNC_LEDS
  // digitalWriteFast(DEBUG_PIN_2, HIGH);
#endif
#if defined(__MK66FX1M0__)
  // T3.6
  _dmatx.clearInterrupt();
#ifdef DEBUG_ASYNC_UPDATE
  static uint8_t print_count;
  if (print_count < 10) {
    print_count++;
    
    Serial.printf("TCD: %x D1:%x %x%c\n", (uint32_t)_dmatx.TCD->SADDR,
                  (uint32_t)_dmasettings[_spi_num][1].TCD->SADDR,
                  (uint32_t)_dmatx.TCD->DLASTSGA,
                  (_dmatx.TCD->SADDR > _dmasettings[1].TCD->SADDR) ? '>' : '<');
  }
#endif
  if (_frame_callback_on_HalfDone &&
      (_dmatx.TCD->SADDR > _dmasettings[_spi_num][1].TCD->SADDR)) {
    _dma_sub_frame_count = 1; // set as partial frame.
  } else {
    _dma_frame_count++;

    // See if we are in continuous mode or not..
    if ((_dma_state & ST7735_DMA_CONT) == 0) {
      // We are in single refresh mode or the user has called cancel so
      // Lets try to release the CS pin
      waitFifoNotFull();
      writecommand_last(ST7735_NOP);
      endSPITransaction();
      _dma_state &= ~ST7735_DMA_ACTIVE;
      _dmaActiveDisplay[_spi_num] = 0; // We don't have a display active any more...
    }
    _dma_sub_frame_count = 0; // set as partial frame.
  }
  if (_frame_complete_callback)
    (*_frame_complete_callback)();
// See if we should do call back or not...
#elif defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
// T4
#ifdef DEBUG_ASYNC_UPDATE
  static uint8_t print_count;
  if (print_count < 10) {
    print_count++;
    
    Serial.printf("TCD: %x D1:%x %x%c\n", (uint32_t)_dma_data[_spi_num]._dmatx.TCD->SADDR,
                  (uint32_t)_dma_data[_spi_num]._dmasettings[1].TCD->SADDR,
                  (uint32_t)_dma_data[_spi_num]._dmatx.TCD->DLASTSGA,
                  (_dma_data[_spi_num]._dmatx.TCD->SADDR > _dma_data[_spi_num]._dmasettings[1].TCD->SADDR) ? '>' : '<');
  }
#endif
  _dma_data[_spi_num]._dmatx.clearInterrupt();
  if (_frame_callback_on_HalfDone && (_dma_data[_spi_num]._dmatx.TCD->SADDR > _dma_data[_spi_num]._dmasettings[1].TCD->SADDR)) {
    _dma_sub_frame_count = 1; // set as partial frame.
    if (_frame_complete_callback)
      (*_frame_complete_callback)();
    // Serial.print("-");
  } else {

    _dma_frame_count++;
    _dma_sub_frame_count = 0;
// Serial.print(".");
// if ((_dma_frame_count & 0x1f) == 0)Serial.println();
#ifdef DEBUG_ASYNC_LEDS
    // digitalWriteFast(DEBUG_PIN_3, HIGH);
#endif
    // See if we are in continuous mode or not..
    if ((_dma_state & ST7735_DMA_CONT) == 0) {
      // We are in single refresh mode or the user has called cancel so
      // Lets try to release the CS pin
      // Serial.printf("Before FSR wait: %x %x\n", _pimxrt_spi->FSR,
      // _pimxrt_spi->SR);
      while (_pimxrt_spi->FSR & 0x1f)
        ; // wait until this one is complete

      // Serial.printf("Before SR busy wait: %x\n", _pimxrt_spi->SR);
      while (_pimxrt_spi->SR & LPSPI_SR_MBF)
        ; // wait until this one is complete

      _dma_data[_spi_num]._dmatx.clearComplete();
      // Serial.println("Restore FCR");
      _pimxrt_spi->FCR = LPSPI_FCR_TXWATER(
          15);              // _spi_fcr_save;	// restore the FSR status...
      _pimxrt_spi->DER = 0; // DMA no longer doing TX (or RX)

      _pimxrt_spi->CR =
          LPSPI_CR_MEN | LPSPI_CR_RRF | LPSPI_CR_RTF; // actually clear both...
      _pimxrt_spi->SR = 0x3f00; // clear out all of the other status...

      maybeUpdateTCR(_tcr_dc_assert | LPSPI_TCR_FRAMESZ(7)); // output Command with 8 bits
      // Serial.printf("Output NOP (SR %x CR %x FSR %x FCR %x %x TCR:%x)\n",
      // _pimxrt_spi->SR, _pimxrt_spi->CR, _pimxrt_spi->FSR,
      //	_pimxrt_spi->FCR, _spi_fcr_save, _pimxrt_spi->TCR);
      writecommand_last(ST7735_NOP);
      endSPITransaction();
      _dma_state &= ~ST7735_DMA_ACTIVE;
      _dmaActiveDisplay[_spi_num] = 0; // We don't have a display active any more...
    } else {
      // Lets try to flush out memory
      if (_frame_complete_callback)
        (*_frame_complete_callback)();
      else if ((uint32_t)_pfbtft >= 0x20200000u)
        arm_dcache_flush(_pfbtft, CBALLOC);
    }
#ifdef DEBUG_ASYNC_LEDS
    // digitalWriteFast(DEBUG_PIN_3, LOW);
#endif
  }
  asm("dsb");

#elif defined(__MK64FX512__)
  //
  // T3.5...
  _dmarx.clearInterrupt();
  _dmatx.clearComplete();
  _dmarx.clearComplete();

  if (!_dma_count_remaining && !(_dma_state & ST7735_DMA_CONT)) {
    // The DMA transfers are done.
    _dma_frame_count++;
#ifdef DEBUG_ASYNC_LEDS
    // digitalWriteFast(DEBUG_PIN_3, HIGH);
#endif

    _pkinetisk_spi->RSER = 0;
    //_pkinetisk_spi->MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF |
    //SPI_MCR_PCSIS(0x1F);  // clear out the queue
    _pkinetisk_spi->SR = 0xFF0F0000;
    _pkinetisk_spi->CTAR0 &= ~(SPI_CTAR_FMSZ(8)); // Hack restore back to 8 bits

    writecommand_last(ST7735_NOP);
    endSPITransaction();
    _dma_state &= ~ST7735_DMA_ACTIVE;
    _dmaActiveDisplay[_spi_num]  = 0; // We don't have a display active any more...
    _dma_sub_frame_count = 0;
    if (_frame_complete_callback)
      (*_frame_complete_callback)();
#ifdef DEBUG_ASYNC_LEDS
    // digitalWriteFast(DEBUG_PIN_3, LOW);
#endif

  } else {
    uint16_t w;
    if (_dma_count_remaining) { // Still part of one frome.
      bool half_done = _dma_count_remaining == (CBALLOC / 4);
      _dma_count_remaining -= _dma_write_size_words;
      w = *((uint16_t *)_dmatx.TCD->SADDR);
      _dmatx.TCD->SADDR = (volatile uint8_t *)(_dmatx.TCD->SADDR) + 2;
      if (_frame_complete_callback && _frame_callback_on_HalfDone && half_done) {
        _dma_sub_frame_count = 1;
        (*_frame_complete_callback)();
      }

    } else { // start a new frame
      _dma_frame_count++;
      _dmatx.sourceBuffer(&_pfbtft[1], (_dma_write_size_words - 1) * 2);
      _dmatx.TCD->SLAST = 0; // Finish with it pointing to next location
      w = _pfbtft[0];
      _dma_count_remaining = CBALLOC / 2 - _dma_write_size_words; // how much more to transfer?
      _dma_sub_frame_count = 0;
      if (_frame_complete_callback)
        (*_frame_complete_callback)();
    }
#ifdef DEBUG_ASYNC_UPDATE
//		dumpDMA_TCD(&_dmatx);
//		dumpDMA_TCD(&_dmarx);
#endif
    _pkinetisk_spi->PUSHR = (w | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT);
    _dmarx.enable();
    _dmatx.enable();
  }

#endif
#ifdef DEBUG_ASYNC_LEDS
  // digitalWriteFast(DEBUG_PIN_2, LOW);
#endif
}

#endif
// Teensy 3.1 can only generate 30 MHz SPI when running at 120 MHz (overclock)

#define WIDTH ST7735_TFTWIDTH
#define HEIGHT ST7735_TFTHEIGHT

// Constructor when using hardware ILI9241_KINETISK__pspi->  Faster, but must
// use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
ST7735_SPI::ST7735_SPI (uint8_t cs, uint8_t dc, uint8_t rst, uint8_t mosi, uint8_t sclk)
{
  _cs = cs;
  _dc = dc;
  _rst = rst;
  _mosi = mosi;
  _sclk = sclk;
  _width = WIDTH;
  _height = HEIGHT;

  rotation = 0;
  cursor_y = cursor_x = 0;
  textsize_x = textsize_y = 1;
  textcolor = textbgcolor = 0xFFFF;
  wrap = true;


  // Added to see how much impact actually using non hardware CS pin might be
  _cspinmask = 0;
  _csport = NULL;

#ifdef ENABLE_ST7735_FRAMEBUFFER
  _pfbtft = NULL;
  _use_fbtft = 0; // Are we in frame buffer mode?
  _we_allocated_buffer = NULL;
#endif
}

//=======================================================================
// Add optinal support for using frame buffer to speed up complex outputs
//=======================================================================
void ST7735_SPI::setFrameBuffer (uint16_t *frame_buffer)
{
#ifdef ENABLE_ST7735_FRAMEBUFFER
  _pfbtft = frame_buffer;
  /*  // Maybe you don't want the memory cleared as you may be playing games
  wiht multiple buffers.
  if (_pfbtft != NULL) {
          memset(_pfbtft, 0, ST7735_TFTHEIGHT*ST7735_TFTWIDTH*2);
  }
  */
  _dma_state &= ~ST7735_DMA_INIT; // clear that we init the dma chain as our
                                   // buffer has changed...

#endif
}

#ifdef ENABLE_ST7735_FRAMEBUFFER
void ST7735_SPI::setFrameCompleteCB (void (*pcb)(), bool fCallAlsoHalfDone)
{
  _frame_complete_callback = pcb;
  _frame_callback_on_HalfDone = pcb ? fCallAlsoHalfDone : false;

  noInterrupts();
  _dma_state &=
      ~ST7735_DMA_INIT; // Lets setup  the call backs on next call out
  interrupts();
}
#endif

uint8_t ST7735_SPI::useFrameBuffer(
    boolean b) // use the frame buffer?  First call will allocate
{
#ifdef ENABLE_ST7735_FRAMEBUFFER

  if (b) {
    // First see if we need to allocate buffer
    if (_pfbtft == NULL) {
      // Hack to start frame buffer on 32 byte boundary
      _we_allocated_buffer = (uint16_t *)malloc(CBALLOC + 32);
      if (_we_allocated_buffer == NULL)
        return 0; // failed
      _pfbtft = (uint16_t *)(((uintptr_t)_we_allocated_buffer + 32) &
                             ~((uintptr_t)(31)));
      memset(_pfbtft, 0, CBALLOC);
    }
    _use_fbtft = 1;
    clearChangedRange(); // make sure the dirty range is updated.
  } else
    _use_fbtft = 0;

  return _use_fbtft;
#else
  return 0;
#endif
}

void ST7735_SPI::freeFrameBuffer (void) // explicit call to release the buffer
{
#ifdef ENABLE_ST7735_FRAMEBUFFER
  if (_we_allocated_buffer) {
    free(_we_allocated_buffer);
    _pfbtft = NULL;
    _use_fbtft = 0; // make sure the use is turned off
    _we_allocated_buffer = NULL;
  }
#endif
}
void ST7735_SPI::updateScreen (void) // call to say update the screen now.
{
// Not sure if better here to check flag or check existence of buffer.
// Will go by buffer as maybe can do interesting things?
#ifdef ENABLE_ST7735_FRAMEBUFFER
  if (_use_fbtft) {
    beginSPITransaction(_SPI_CLOCK);
    if (_standard && !_updateChangedAreasOnly) {
      // Doing full window.
      setAddr(0, 0, _width - 1, _height - 1);
      writecommand_cont(ST7735_RAMWR);

      // BUGBUG doing as one shot.  Not sure if should or not or do like
      // main code and break up into transactions...
      uint16_t *pfbtft_end = &_pfbtft[(ST7735_TFTWIDTH * ST7735_TFTHEIGHT) - 1]; // setup
      uint16_t *pftbft = _pfbtft;

      // Quick write out the data;
      while (pftbft < pfbtft_end) {
        writedata16_cont(*pftbft++);
      }
      writedata16_last(*pftbft);
    } else {
      // setup just to output the clip rectangle area anded with updated area if
      // enabled
      int16_t start_x = _displayclipx1;
      int16_t start_y = _displayclipy1;
      int16_t end_x = _displayclipx2 - 1;
      int16_t end_y = _displayclipy2 - 1;

      if (_updateChangedAreasOnly) {
        // maybe update range of values to update...
        if (_changed_min_x > start_x)
          start_x = _changed_min_x;
        if (_changed_min_y > start_y)
          start_y = _changed_min_y;
        if (_changed_max_x < end_x)
          end_x = _changed_max_x;
        if (_changed_max_y < end_y)
          end_y = _changed_max_y;
      }

      // Only do if actual area to update
      if ((start_x <= end_x) && (start_y <= end_y)) {
        setAddr(start_x, start_y, end_x, end_y);
        writecommand_cont(ST7735_RAMWR);

        // BUGBUG doing as one shot.  Not sure if should or not or do like
        // main code and break up into transactions...
        uint16_t *pfbPixel_row = &_pfbtft[start_y * _width + start_x];
        for (uint16_t y = start_y; y <= end_y; y++) {
          uint16_t *pfbPixel = pfbPixel_row;
          for (uint16_t x = start_x; x < end_x; x++) {
            writedata16_cont(*pfbPixel++);
          }
          if (y < (end_y))
            writedata16_cont(*pfbPixel);
          else
            writedata16_last(*pfbPixel);
          pfbPixel_row += _width; // setup for the next row.
        }
      }
    }
    endSPITransaction();
  }
  clearChangedRange(); // make sure the dirty range is updated.
#endif
}

#ifdef DEBUG_ASYNC_UPDATE
void dumpDMA_TCD (DMABaseClass *dmabc, const char *psz_title) {
  if (psz_title)
    Serial.print(psz_title);
  Serial.printf("%x %x:", (uint32_t)dmabc, (uint32_t)dmabc->TCD);

  Serial.printf(
      "SA:%x SO:%d AT:%x NB:%x SL:%d DA:%x DO: %d CI:%x DL:%x CS:%x BI:%x\n",
      (uint32_t)dmabc->TCD->SADDR, dmabc->TCD->SOFF, dmabc->TCD->ATTR,
      dmabc->TCD->NBYTES, dmabc->TCD->SLAST, (uint32_t)dmabc->TCD->DADDR,
      dmabc->TCD->DOFF, dmabc->TCD->CITER, dmabc->TCD->DLASTSGA,
      dmabc->TCD->CSR, dmabc->TCD->BITER);
}
#endif

#ifdef ENABLE_ST7735_FRAMEBUFFER
//==============================================
#ifdef ENABLE_ST7735_FRAMEBUFFER
void ST7735_SPI::initDMASettings(void) {
  //Serial.printf("initDMASettings called %d\n", _dma_state);
  if (_dma_state & ST7735_DMA_INIT) { // should test for init, but...
    return;                            // we already init this.
  }

  // Serial.println("InitDMASettings");
  uint8_t dmaTXevent = _spi_hardware->tx_dma_channel;
#if defined(__MK66FX1M0__)
  // T3.6

  // BUGBUG:: check for -1 as wont work on SPI2 on T3.5
  //	uint16_t *fbtft_start_dma_addr = _pfbtft;

  // Serial.printf("CWW: %d %d %d\n", CBALLOC, SCREEN_DMA_NUM_SETTINGS,
  // count_words_write);
  // Now lets setup DMA access to this memory...
  _dmasettings[_spi_num][0].sourceBuffer(&_pfbtft[1], (COUNT_WORDS_WRITE-1)*2);
  _dmasettings[_spi_num][0].destination(_pkinetisk_spi->PUSHR);

  // Hack to reset the destination to only output 2 bytes.
  _dmasettings[_spi_num][0].TCD->ATTR_DST = 1;
  _dmasettings[_spi_num][0].replaceSettingsOnCompletion(_dmasettings[_spi_num][1]);

  _dmasettings[_spi_num][1].sourceBuffer(&_pfbtft[COUNT_WORDS_WRITE], COUNT_WORDS_WRITE*2);
  _dmasettings[_spi_num][1].destination(_pkinetisk_spi->PUSHR);
  _dmasettings[_spi_num][1].TCD->ATTR_DST = 1;
  _dmasettings[_spi_num][1].replaceSettingsOnCompletion(_dmasettings[_spi_num][2]);

  if (_frame_callback_on_HalfDone)
    _dmasettings[_spi_num][0].interruptAtCompletion();
  else
    _dmasettings[_spi_num][0].TCD->CSR &= ~(DMA_TCD_CSR_DREQ);
  // Sort of hack - but wrap around to output the first word again. 
  _dmasettings[_spi_num][2].sourceBuffer(_pfbtft, 2);
  _dmasettings[_spi_num][2].destination(_pkinetisk_spi->PUSHR);
  _dmasettings[_spi_num][2].TCD->ATTR_DST = 1;
  _dmasettings[_spi_num][2].replaceSettingsOnCompletion(_dmasettings[_spi_num][0]);


  // Setup DMA main object
  // Serial.println("Setup _dmatx");
  _dmatx.begin(true);
  _dmatx.triggerAtHardwareEvent(dmaTXevent);
  _dmatx = _dmasettings[_spi_num][0];
  if (_spi_num == 0) _dmatx.attachInterrupt(dmaInterrupt);
  else if (_spi_num == 1) _dmatx.attachInterrupt(dmaInterrupt1);
  else _dmatx.attachInterrupt(dmaInterrupt2);

#elif defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
// 240*240/2 = 28800 so fits in 2
#ifdef DEBUG_ASYNC_LEDS
  // digitalWriteFast(DEBUG_PIN_4, !digitalReadFast(DEBUG_PIN_4));
#endif
  _dma_data[_spi_num]._dmasettings[0].sourceBuffer(_pfbtft, (COUNT_WORDS_WRITE)*2);
  _dma_data[_spi_num]._dmasettings[0].destination(_pimxrt_spi->TDR);
  _dma_data[_spi_num]._dmasettings[0].TCD->ATTR_DST = 1;
  _dma_data[_spi_num]._dmasettings[0].replaceSettingsOnCompletion(_dma_data[_spi_num]._dmasettings[1]);

  _dma_data[_spi_num]._dmasettings[1].sourceBuffer(&_pfbtft[COUNT_WORDS_WRITE],
                               COUNT_WORDS_WRITE * 2);
  _dma_data[_spi_num]._dmasettings[1].destination(_pimxrt_spi->TDR);
  _dma_data[_spi_num]._dmasettings[1].TCD->ATTR_DST = 1;

  _dma_data[_spi_num]._dmasettings[1].replaceSettingsOnCompletion(_dma_data[_spi_num]._dmasettings[0]);
  _dma_data[_spi_num]._dmasettings[1].interruptAtCompletion();
  if (_frame_callback_on_HalfDone)
    _dma_data[_spi_num]._dmasettings[0].interruptAtCompletion();
  else
    _dma_data[_spi_num]._dmasettings[0].TCD->CSR &= ~(DMA_TCD_CSR_DREQ);

  // Setup DMA main object
  //Serial.println("Setup _dmatx");
  // Serial.println("DMA initDMASettings - before dmatx");
#ifdef DEBUG_ASYNC_LEDS
    // digitalWriteFast(DEBUG_PIN_4, !digitalReadFast(DEBUG_PIN_4));
#endif
  _dma_data[_spi_num]._dmatx.begin(true);
  _dma_data[_spi_num]._dmatx.triggerAtHardwareEvent(dmaTXevent);
#ifdef DEBUG_ASYNC_LEDS
    // digitalWriteFast(DEBUG_PIN_4, !digitalReadFast(DEBUG_PIN_4));
#endif
  _dma_data[_spi_num]._dmatx = _dma_data[_spi_num]._dmasettings[0];
  // probably could use const table of functions...
  if (_spi_num == 0) _dma_data[_spi_num]._dmatx.attachInterrupt(dmaInterrupt);
  else if (_spi_num == 1) _dma_data[_spi_num]._dmatx.attachInterrupt(dmaInterrupt1);
  else _dma_data[_spi_num]._dmatx.attachInterrupt(dmaInterrupt2);

#ifdef DEBUG_ASYNC_LEDS
  // digitalWriteFast(DEBUG_PIN_4, !digitalReadFast(DEBUG_PIN_4));
#endif
#elif defined(__MK64FX512__)
  // T3.5
  // Lets setup the write size.  For SPI we can use up to 32767 so same size as
  // we use on T3.6...
  // But SPI1 and SPI2 max of 511.  We will use 480 in that case as even
  // divider...

  _dmarx.disable();
  _dmarx.source(_pkinetisk_spi->POPR);
  _dmarx.TCD->ATTR_SRC = 1;
  _dmarx.destination(_dma_dummy_rx);
  _dmarx.disableOnCompletion();
  _dmarx.triggerAtHardwareEvent(_spi_hardware->rx_dma_channel);
  _dmarx.attachInterrupt(dmaInterrupt);
  _dmarx.interruptAtCompletion();

  // We may be using settings chain here so lets set it up.
  // Now lets setup TX chain.  Note if trigger TX is not set
  // we need to have the RX do it for us.
  _dmatx.disable();
  _dmatx.destination(_pkinetisk_spi->PUSHR);
  _dmatx.TCD->ATTR_DST = 1;
  _dmatx.disableOnCompletion();
  // SPI on T3.5 only SPI object can do full size...
  if (_spi_num == 0) {
    _dmatx.triggerAtHardwareEvent(dmaTXevent);
    _dma_write_size_words = COUNT_WORDS_WRITE;
  } else {
    _dma_write_size_words = 480;
    _dmatx.triggerAtTransfersOf(_dmarx);
  }
// Serial.printf("Init DMA Settings: TX:%d size:%d\n", dmaTXevent,
// _dma_write_size_words);

#endif
  _dma_state =
      ST7735_DMA_INIT | ST7735_DMA_EVER_INIT; // Should be first thing set!
  // Serial.println("DMA initDMASettings - end");
}
#endif

void ST7735_SPI::dumpDMASettings() {
#ifdef DEBUG_ASYNC_UPDATE
#if defined(__MK66FX1M0__)
  // T3.6
  Serial.printf("DMA dump TCDs %d\n", _dmatx.channel);
  dumpDMA_TCD(&_dmatx, "TX: ");
  dumpDMA_TCD(&_dmasettings[_spi_num][0], " 0: ");
  dumpDMA_TCD(&_dmasettings[_spi_num][1], " 1: ");
  dumpDMA_TCD(&_dmasettings[_spi_num][2], " 2: ");

#elif defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
  // Serial.printf("DMA dump TCDs %d\n", _dmatx.channel);
  dumpDMA_TCD(&_dma_data[_spi_num]._dmatx, " TX ");
  dumpDMA_TCD(&_dma_data[_spi_num]._dmasettings[0], " 0: ");
  dumpDMA_TCD(&_dma_data[_spi_num]._dmasettings[1], " 1: ");
#elif defined(__MK64FX512__)
  Serial.printf("DMA dump TX:%d RX:%d\n", _dmatx.channel, _dmarx.channel);
  dumpDMA_TCD(&_dmatx);
  dumpDMA_TCD(&_dmarx);
#endif
#endif
}

bool ST7735_SPI::updateScreenAsync(
    bool update_cont) // call to say update the screen now.
{
// Not sure if better here to check flag or check existence of buffer.
// Will go by buffer as maybe can do interesting things?
// BUGBUG:: only handles full screen so bail on the rest of it...
#ifdef ENABLE_ST7735_FRAMEBUFFER
  if (!_use_fbtft)
    return false;

#if defined(__MK64FX512__) // If T3.5 only allow on SPI...
  // The T3.5 DMA to SPI has issues with preserving stuff like we want 16 bit
  // mode
  // and we want CS to stay on... So hack it.  We will turn off using CS for the
  // CS
  //	pin.
  if (!_csport) {
    pcs_data = 0;
    pcs_command = pcs_data | _pspi->setCS(_dc);
    pinMode(_cs, OUTPUT);
    _csport = portOutputRegister(digitalPinToPort(_cs));
    _cspinmask = digitalPinToBitMask(_cs);
    *_csport |= _cspinmask;
  }
#endif

#ifdef DEBUG_ASYNC_LEDS
  // digitalWriteFast(DEBUG_PIN_1, HIGH);
#endif
  // Init DMA settings.
  initDMASettings();

#ifdef DEBUG_ASYNC_LEDS
  // digitalWriteFast(DEBUG_PIN_4, !digitalReadFast(DEBUG_PIN_4));
#endif
  // Don't start one if already active.
  if (_dma_state & ST7735_DMA_ACTIVE) {
#ifdef DEBUG_ASYNC_LEDS
    // digitalWriteFast(DEBUG_PIN_1, LOW);
#endif
    return false;
  }

#if defined(__MK66FX1M0__)
  //==========================================
  // T3.6
  //==========================================
  if (update_cont) {
    // Try to link in #3 into the chain (_cnt_dma_settings)
    _dmasettings[_spi_num][SCREEN_DMA_NUM_SETTINGS-1].replaceSettingsOnCompletion(_dmasettings[_spi_num][SCREEN_DMA_NUM_SETTINGS]);
    _dmasettings[_spi_num][SCREEN_DMA_NUM_SETTINGS-1].TCD->CSR &= ~(DMA_TCD_CSR_INTMAJOR | DMA_TCD_CSR_DREQ);  // Don't interrupt on this one... 
    _dmasettings[_spi_num][SCREEN_DMA_NUM_SETTINGS].interruptAtCompletion();
    _dmasettings[_spi_num][SCREEN_DMA_NUM_SETTINGS].TCD->CSR &= ~(DMA_TCD_CSR_DREQ);  // Don't disable on this one  
    _dma_state |= ST7735_DMA_CONT;
  } else {
    // In this case we will only run through once...
    _dmasettings[_spi_num][SCREEN_DMA_NUM_SETTINGS-1].replaceSettingsOnCompletion(_dmasettings[_spi_num][0]);
    _dmasettings[_spi_num][SCREEN_DMA_NUM_SETTINGS-1].interruptAtCompletion();
    _dmasettings[_spi_num][SCREEN_DMA_NUM_SETTINGS-1].disableOnCompletion();
    _dma_state &= ~ST7735_DMA_CONT;
  }

#ifdef DEBUG_ASYNC_UPDATE
  dumpDMASettings();
#endif
  beginSPITransaction(_SPI_CLOCK);

  // Doing full window.
  setAddr(0, 0, _width - 1, _height - 1);
  writecommand_cont(ST7735_RAMWR);

  // Write the first Word out before enter DMA as to setup the proper
  // CS/DC/Continue flaugs
  writedata16_cont(*_pfbtft);
  // now lets start up the DMA
  //	volatile uint16_t  biter = _dmatx.TCD->BITER;
  // DMA_CDNE_CDNE(_dmatx.channel);
  _dmatx = _dmasettings[_spi_num][0];
  //	_dmatx.TCD->BITER = biter;
  _dma_frame_count = 0; // Set frame count back to zero.
  _dmaActiveDisplay[_spi_num]  = this;
  _dma_state |= ST7735_DMA_ACTIVE;
  _pkinetisk_spi->RSER |= SPI_RSER_TFFF_DIRS | SPI_RSER_TFFF_RE; // Set DMA Interrupt Request Select and Enable register
  _pkinetisk_spi->MCR &= ~SPI_MCR_HALT; // Start transfers.
  _dmatx.enable();
//==========================================
// T4
//==========================================
#elif defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
  /////////////////////////////
  // BUGBUG try first not worry about continueous or not.
  // Start off remove disable on completion from both...
  // it will be the ISR that disables it...
  if ((uint32_t)_pfbtft >= 0x20200000u)
    arm_dcache_flush(_pfbtft, CBALLOC);

  _dma_data[_spi_num]._dmasettings[1].TCD->CSR &= ~(DMA_TCD_CSR_DREQ);
  beginSPITransaction(_SPI_CLOCK);
// Doing full window.
#ifdef DEBUG_ASYNC_LEDS
  // digitalWriteFast(DEBUG_PIN_4, !digitalReadFast(DEBUG_PIN_4));
#endif

  setAddr(0, 0, _width - 1, _height - 1);
  writecommand_last(ST7735_RAMWR);

  // Update TCR to 16 bit mode. and output the first entry.
  _spi_fcr_save = _pimxrt_spi->FCR; // remember the FCR
  _pimxrt_spi->FCR = 0;             // clear water marks...
  maybeUpdateTCR(_tcr_dc_not_assert | LPSPI_TCR_FRAMESZ(15) | LPSPI_TCR_RXMSK /*| LPSPI_TCR_CONT*/);
  _pimxrt_spi->DER = LPSPI_DER_TDDE;
  _pimxrt_spi->SR = 0x3f00; // clear out all of the other status...

   _dma_data[_spi_num]._dmatx.triggerAtHardwareEvent(_spi_hardware->tx_dma_channel);

   _dma_data[_spi_num]._dmatx = _dma_data[_spi_num]._dmasettings[0];
#ifdef DEBUG_ASYNC_LEDS
  // digitalWriteFast(DEBUG_PIN_4, !digitalReadFast(DEBUG_PIN_4));
#endif

   _dma_data[_spi_num]._dmatx.begin(false);
   _dma_data[_spi_num]._dmatx.enable();

  _dma_frame_count = 0; // Set frame count back to zero.
  _dmaActiveDisplay[_spi_num] = this;
  if (update_cont) {
    _dma_state |= ST7735_DMA_CONT;
  } else {
     _dma_data[_spi_num]._dmasettings[1].disableOnCompletion();
    _dma_state &= ~ST7735_DMA_CONT;
  }

  _dma_state |= ST7735_DMA_ACTIVE;
#ifdef DEBUG_ASYNC_LEDS
  // digitalWriteFast(DEBUG_PIN_4, !digitalReadFast(DEBUG_PIN_4));
#endif
#ifdef DEBUG_ASYNC_UPDATE
  dumpDMASettings();
#endif

#elif defined(__MK64FX512__)
  //==========================================
  // T3.5
  //==========================================

  // lets setup the initial pointers.
  _dmatx.sourceBuffer(&_pfbtft[1], (_dma_write_size_words - 1) * 2);
  _dmatx.TCD->SLAST = 0; // Finish with it pointing to next location
  _dmarx.transferCount(_dma_write_size_words);
  _dma_count_remaining = CBALLOC / 2 - _dma_write_size_words; // how much more to transfer?
// Serial.printf("SPI1/2 - TC:%d TR:%d\n", _dma_write_size_words,
// _dma_count_remaining);

#ifdef DEBUG_ASYNC_UPDATE
  dumpDMASettings();
#endif

  beginSPITransaction(_SPI_CLOCK);
  // Doing full window.
  setAddr(0, 0, _width - 1, _height - 1);
  writecommand_cont(ST7735_RAMWR);

  // Write the first Word out before enter DMA as to setup the proper
  // CS/DC/Continue flaugs
  // On T3.5 DMA only appears to work with CTAR 0 so hack it up...
  _pkinetisk_spi->CTAR0 |= SPI_CTAR_FMSZ(8); // Hack convert from 8 bit to 16 bit...
  _pkinetisk_spi->MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_PCSIS(0x1F);
  _pkinetisk_spi->SR = 0xFF0F0000;

  // Lets try to output the first byte to make sure that we are in 16 bit
  // mode...
  _pkinetisk_spi->PUSHR = *_pfbtft | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT;

  if (_spi_num == 0) {
    // SPI - has both TX and RX so use it
    _pkinetisk_spi->RSER = SPI_RSER_RFDF_RE | SPI_RSER_RFDF_DIRS | SPI_RSER_TFFF_RE | SPI_RSER_TFFF_DIRS;

    _dmarx.enable();
    _dmatx.enable();
  } else {
    _pkinetisk_spi->RSER = SPI_RSER_RFDF_RE | SPI_RSER_RFDF_DIRS;
    _dmatx.triggerAtTransfersOf(_dmarx);
    _dmatx.enable();
    _dmarx.enable();
  }

  _dma_frame_count = 0; // Set frame count back to zero.
  _dmaActiveDisplay[_spi_num] = this;
  if (update_cont) {
    _dma_state |= ST7735_DMA_CONT;
  } else {
    _dma_state &= ~ST7735_DMA_CONT;
  }

  _dma_state |= ST7735_DMA_ACTIVE;
#endif
#ifdef DEBUG_ASYNC_LEDS
  // digitalWriteFast(DEBUG_PIN_1, LOW);
#endif
  return true;
#else
  return false;                    // no frame buffer so will never start...
#endif
}

void ST7735_SPI::endUpdateAsync ()
{
// make sure it is on
#ifdef ENABLE_ST7735_FRAMEBUFFER
  if (_dma_state & ST7735_DMA_CONT) {
    _dma_state &= ~ST7735_DMA_CONT; // Turn of the continueous mode
#if defined(__MK66FX1M0__)
    _dmasettings[_spi_num][2].disableOnCompletion();
#elif  defined(__IMXRT1062__)
    _dma_data[_spi_num]._dmasettings[1].disableOnCompletion();
#endif
  }
#endif
}

void ST7735_SPI::waitUpdateAsyncComplete (void)
{
#ifdef ENABLE_ST7735_FRAMEBUFFER
#ifdef DEBUG_ASYNC_LEDS
  // digitalWriteFast(DEBUG_PIN_3, HIGH);
#endif

  while ((_dma_state & ST7735_DMA_ACTIVE)) {
    // asm volatile("wfi");
  };
#ifdef DEBUG_ASYNC_LEDS
  // digitalWriteFast(DEBUG_PIN_3, LOW);
#endif
#endif
}
#endif
//=======================================================================

void ST7735_SPI::setAddrWindow (uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  beginSPITransaction(_SPI_CLOCK);
  setAddr(x0, y0, x1, y1);
  writecommand_last(ST7735_RAMWR); // write to RAM
  endSPITransaction();
}



void ST7735_SPI::setRotation (uint8_t m)
{
  rotation = m % 4; // can't be higher than 3
  beginSPITransaction(_SPI_CLOCK);
  writecommand_cont(ST7735_MADCTL);
  
  switch (rotation) {
  case 0:
    writedata8_last(MADCTL_MX | MADCTL_RGB);
    _width = ST7735_TFTWIDTH;
    _height = ST7735_TFTHEIGHT;
    break;
  case 1:
    writedata8_last(MADCTL_MV | MADCTL_RGB);
    _width = ST7735_TFTHEIGHT;
    _height = ST7735_TFTWIDTH;
    break;
  case 2:
    writedata8_last(/*MADCTL_MY |*/ MADCTL_RGB);
    _width = ST7735_TFTWIDTH;
    _height = ST7735_TFTHEIGHT;
    break;
  case 3:
    writedata8_last(MADCTL_MX /*| MADCTL_MY */| MADCTL_MV | MADCTL_RGB);
    _width = ST7735_TFTHEIGHT;
    _height = ST7735_TFTWIDTH;
    break;
  }
  endSPITransaction();

}

void ST7735_SPI::invertDisplay (boolean i)
{
  beginSPITransaction(_SPI_CLOCK);
  writecommand_last(i ? ST7735_INVON : ST7735_INVOFF);
  endSPITransaction();
}


void ST7735_SPI::setFrameRateControl (uint8_t mode)
{
  // Do simple version
  //beginSPITransaction(_SPI_CLOCK/4);
  //writecommand_cont(ILI9341_FRMCTR1);
  //writedata8_cont((mode >> 4) & 0x3); // Output DIVA setting (0-3)
  //writedata8_last(0x10 + (mode & 0xf)); // Output RTNA
  //endSPITransaction();
}

void ST7735_SPI::setBacklight (const uint8_t value)
{
	analogWrite(TFT_BL, value&0x7F);
}


// Now lets see if we can writemultiple pixels
void ST7735_SPI::writeRect (int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *pcolors)
{

  beginSPITransaction(_SPI_CLOCK);
  setAddr(x, y, x + w-1, y + h-1);
  writecommand_cont(ST7735_RAMWR);
  
  for (y = 0; y < h; y++){
    for (x = 0; x < w-1; x++)
      writedata16_cont(*pcolors++);

    writedata16_last(*pcolors++);
  }
  endSPITransaction();
}

void ST7735_SPI::pushPixels16bit (uint16_t *pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  const int w = (x2 - x1) + 1;
  const int h = (y2 - y1) + 1;
  int tPixels = (w * h);
  
  if (tPixels < 1) return;
  else if (tPixels > TFT_WIDTH * TFT_HEIGHT)
  	tPixels = TFT_WIDTH * TFT_HEIGHT;
  tPixels--;	// for the final write (writedata16_last())

  beginSPITransaction(_SPI_CLOCK);
  setAddr(x1, y1, x2, y2);
  writecommand_cont(ST7735_RAMWR);  
  
  while (tPixels--)
      writedata16_cont(*pcolors++);
  writedata16_last(*pcolors++);
  
  endSPITransaction();
}

void ST7735_SPI::fillRect (int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    beginSPITransaction(_SPI_CLOCK);
    setAddr(x, y, x + w-1, y + h-1);
    writecommand_cont(ST7735_RAMWR);
    
    for (y = 0; y < h; y++){
      for (x = 0; x < w-1; x++)
       	writedata16_cont(color);

      writedata16_last(color);
    }
    endSPITransaction();
}

void ST7735_SPI::fillScreen (uint16_t color)
{
#ifdef ENABLE_ST7735_FRAMEBUFFER
  if (_use_fbtft && _standard) {
    // Speed up lifted from Franks DMA code... _standard is if no offsets and
    // rects..
    updateChangedRange(0, 0, _width, _height); // update the range of the screen that has been changed;
    uint32_t color32 = (color << 16) | color;

    uint32_t *pfbPixel = (uint32_t *)_pfbtft;
    uint32_t *pfbtft_end = (uint32_t *)((uint16_t *)&_pfbtft[(ST7735_TFTWIDTH * ST7735_TFTHEIGHT)]); // setup
    
    while (pfbPixel < pfbtft_end) {
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
      *pfbPixel++ = color32;
    }

  } else
#endif
  {
    fillRect(0, 0, _width, _height, color);
  }
}

static const uint8_t PROGMEM init_commands[] = {
	1, ST7735_SWRESET,
	1, ST7735_SLPOUT,
	
	4, ST7735_FRMCTR1, 0x01, 0x2C, 0x2D,
	4, ST7735_FRMCTR2, 0x01, 0x2C, 0x2D,
	7, ST7735_FRMCTR3, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D,
	2, ST7735_INVCTR,  0x07, 
	4, ST7735_PWCTR1,  0xA2, 0x02, 0x84,
	2, ST7735_PWCTR2,  0xC5, 
	3, ST7735_PWCTR3,  0x0A, 0x00,
	3, ST7735_PWCTR4,  0x8A, 0x2A,
	3, ST7735_PWCTR4,  0x8A, 0xEE,
	2, ST7735_PWCTR5,  0x03,
	3, ST7735_PWCTR6,  0x11, 0x15,
	2, ST7735_VMCTR1,  0x1A,
	1, ST7735_INVOFF, 
	2, ST7735_MADCTL,  0xC0, 
	2, ST7735_COLMOD,  0x05,
	5, ST7735_CASET,   0x00, 0x00, 0x00, 0x7F,
	5, ST7735_RASET,   0x00, 0x00, 0x00, 0x9F,
    17,ST7735_GMCTRP1, 0x09, 0x16, 0x09, 0x20, 0x21, 0x1B, 0x13, 0x19, 0x17, 0x15, 0x1E, 0x2B, 0x04, 0x05, 0x02, 0x0E,
    17,ST7735_GMCTRN1, 0x0B, 0x14, 0x08, 0x1E, 0x22, 0x1D, 0x18, 0x1E, 0x1B, 0x1A, 0x24, 0x2B, 0x06, 0x06, 0x02, 0x0F,

	2, ST7735_TESTM, 0x01, 
	2, 0xF6, 0x00,
    2, 0xC7, 0xFC,	// magic 

    3, ST7735_DISSET5, 0x15, 0x02, 
    1, ST7735_IDLEOFF,
    1, ST7735_NORON,
    1, ST7735_DISPON,
	0
};


FLASHMEM void ST7735_SPI::begin (uint32_t spi_clock, uint32_t spi_clock_read)
{
  // verify SPI pins are valid;
  // allow user to say use current ones...
  
  spi_clock *= 1000 * 1000;	// convert to Mhz
  
  _SPI_CLOCK = spi_clock;           // #define ST7735_SPICLOCK 30000000
  _SPI_CLOCK_READ = spi_clock_read; //#define ST7735_SPICLOCK_READ 2000000

  //Serial.printf("::begin mosi:%d SCLK:%d CS:%d DC:%d SPI clocks:%lu %lu\n", _mosi, _sclk, _cs, _dc, _SPI_CLOCK, _SPI_CLOCK_READ);

  // Note this display does not use MISO so will ignore it.
  if (SPI.pinIsMOSI(_mosi) && SPI.pinIsSCK(_sclk)) {
    _pspi = &SPI;
    _spi_num = 0; // Which buss is this spi on?
#ifdef KINETISK
    _pkinetisk_spi = &KINETISK_SPI0; // Could hack our way to grab this from SPI
                                     // object, but...
    _fifo_full_test = (3 << 12);
#elif defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
    _pimxrt_spi = &IMXRT_LPSPI4_S; // Could hack our way to grab this from SPI
                                   // object, but...
#else
    _pkinetisl_spi = &KINETISL_SPI0;
#endif

#if defined(__MK64FX512__) || defined(__MK66FX1M0__) ||                        \
    defined(__IMXRT1062__) || defined(__MKL26Z64__)
  } else if (SPI1.pinIsMOSI(_mosi) && SPI1.pinIsSCK(_sclk)) {
    _pspi = &SPI1;
    _spi_num = 1; // Which buss is this spi on?
#ifdef KINETISK
    _pkinetisk_spi = &KINETISK_SPI1; // Could hack our way to grab this from SPI
                                     // object, but...
    _fifo_full_test = (0 << 12);
#elif defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
    _pimxrt_spi = &IMXRT_LPSPI3_S; // Could hack our way to grab this from SPI
                                   // object, but...
#else
    _pkinetisl_spi = &KINETISL_SPI1;
#endif
#if !defined(__MKL26Z64__)
  } else if (SPI2.pinIsMOSI(_mosi) && SPI2.pinIsSCK(_sclk)) {
    _pspi = &SPI2;
    _spi_num = 2; // Which buss is this spi on?
#ifdef KINETISK
    _pkinetisk_spi = &KINETISK_SPI2; // Could hack our way to grab this from SPI
                                     // object, but...
    _fifo_full_test = (0 << 12);
#elif defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
    _pimxrt_spi = &IMXRT_LPSPI1_S; // Could hack our way to grab this from SPI
                                   // object, but...
#endif
#endif
#endif
  }else{
    //Serial.println( "ST7735_SPI: The IO pins on the constructor are not valid SPI pins");
    //Serial.printf("    mosi:%d SCLK:%d CS:%d DC:%d\n", _mosi, _sclk, _cs, _dc);
    //Serial.flush();
    return; // most likely will go bomb
  }
  // Make sure we have all of the proper SPI pins selected.
  _pspi->setMOSI(_mosi);
  _pspi->setSCK(_sclk);

  // Hack to get hold of the SPI Hardware information...
  uint32_t *pa = (uint32_t *)((void *)_pspi);
  _spi_hardware = (SPIClass::SPI_Hardware_t *)(void *)pa[1];

  _pspi->begin();

 // Teensy 4.x
  // Serial.println("   T4 setup CS/DC"); Serial.flush();
  pending_rx_count = 0; // Make sure it is zero if we we do a second begin...
  _csport = portOutputRegister(_cs);
  _cspinmask = digitalPinToBitMask(_cs);
  pinMode(_cs, OUTPUT);
  DIRECT_WRITE_HIGH(_csport, _cspinmask);
  _spi_tcr_current = _pimxrt_spi->TCR; // get the current TCR value

  // TODO:  Need to setup DC to actually work.
  if (_pspi->pinIsChipSelect(_dc)) {
    uint8_t dc_cs_index = _pspi->setCS(_dc);
    // Serial.printf("    T4 hardware DC: %x\n", dc_cs_index);
    _dcport = 0;
    _dcpinmask = 0;
    // will depend on which PCS but first get this to work...
    dc_cs_index--; // convert to 0 based
    _tcr_dc_assert = LPSPI_TCR_PCS(dc_cs_index);
    _tcr_dc_not_assert = LPSPI_TCR_PCS(3);
  } else {
    // Serial.println("ST7735_SPI: DC is not valid hardware CS pin");
    _dcport = portOutputRegister(_dc);
    _dcpinmask = digitalPinToBitMask(_dc);
    pinMode(_dc, OUTPUT);
    DIRECT_WRITE_HIGH(_dcport, _dcpinmask);
    _tcr_dc_assert = LPSPI_TCR_PCS(0);
    _tcr_dc_not_assert = LPSPI_TCR_PCS(1);
  }
  maybeUpdateTCR(_tcr_dc_not_assert | LPSPI_TCR_FRAMESZ(7));

  //_pspi->beginTransaction(SPISettings(ST7735_SPICLOCK, MSBFIRST, SPI_MODE0)); // Should have our settings. 
  //_pspi->transfer(0);	// hack to see if it will actually change then...
  //_pspi->endTransaction();

  // toggle RST low to reset
  if (_rst < 255) {
    pinMode(_rst, OUTPUT);
     digitalWrite(_rst, HIGH);
    delay(20);
     digitalWrite(_rst, LOW);
    delay(20);
     digitalWrite(_rst, HIGH);
    delay(110);
  }


  beginSPITransaction(_SPI_CLOCK/4);
  
  const uint8_t *addr = init_commands;
  while (1) {
    uint8_t count = *addr++;
    if (count-- == 0)
      break;
    writecommand_cont(*addr++);
    while (count-- > 0) {
      writedata8_cont(*addr++);
    }
  }

  writecommand_cont(ST7735_SLPOUT); // Exit Sleep  
  delay(110);
  writecommand_cont(ST7735_IDLEOFF);
  delay(100);
  writecommand_last(ST7735_DISPON); // Display on
  endSPITransaction();


#ifdef DEBUG_ASYNC_LEDS
  pinMode(DEBUG_PIN_1, OUTPUT);
  pinMode(DEBUG_PIN_2, OUTPUT);
  pinMode(DEBUG_PIN_3, OUTPUT);
  pinMode(DEBUG_PIN_4, OUTPUT);
#endif
  // Serial.println("::begin - completed"); Serial.flush();
  
  pinMode(TFT_BL, OUTPUT);
  setBacklight(TFT_INTENSITY);
  setRotation(3);
}

uint8_t ST7735_SPI::getRotation (void)
{
	return rotation;
}

void ST7735_SPI::sleep (bool enable)
{
  beginSPITransaction(_SPI_CLOCK);
  if (enable) {
    writecommand_cont(ST7735_DISPOFF);
    writecommand_last(ST7735_SLPIN);
    endSPITransaction();
  } else {
    writecommand_cont(ST7735_DISPON);
    writecommand_last(ST7735_SLPOUT);
    endSPITransaction();
    delay(5);
  }
}

void ST7735_SPI::sendCommand (uint8_t commandByte, const uint8_t *dataBytes, uint8_t numDataBytes)
{
    beginSPITransaction(_SPI_CLOCK);

    if (numDataBytes) writecommand_cont(commandByte); // Send the command byte
    else  writecommand_last(commandByte);
    while (numDataBytes > 1) {
    writedata8_cont(*dataBytes++); // Send the data bytes
    numDataBytes--;
    }
    if (numDataBytes) writedata8_last(*dataBytes);
  
    endSPITransaction();
}


//////////////////////////////////////////////////////
// From Spin:
#if defined(KINETISK)
void ST7735_SPI::waitFifoNotFull(void) {
  uint32_t sr;
  uint32_t tmp __attribute__((unused));
  do {
    sr = _pkinetisk_spi->SR;
    if (sr & 0xF0)
      tmp = _pkinetisk_spi->POPR; // drain RX FIFO
  } while ((uint32_t)(sr & (15 << 12)) > _fifo_full_test);
}
void ST7735_SPI::waitFifoEmpty(void) {
  uint32_t sr;
  uint32_t tmp __attribute__((unused));
  do {
    sr = _pkinetisk_spi->SR;
    if (sr & 0xF0)
      tmp = _pkinetisk_spi->POPR; // drain RX FIFO
  } while ((sr & 0xF0F0) > 0);    // wait both RX & TX empty
}
void ST7735_SPI::waitTransmitComplete(void) {
  uint32_t tmp __attribute__((unused));
  while (!(_pkinetisk_spi->SR & SPI_SR_TCF))
    ;                         // wait until final output done
  tmp = _pkinetisk_spi->POPR; // drain the final RX FIFO word
}
void ST7735_SPI::waitTransmitComplete(uint32_t mcr) {
  uint32_t tmp __attribute__((unused));
  while (1) {
    uint32_t sr = _pkinetisk_spi->SR;
    if (sr & SPI_SR_EOQF)
      break; // wait for last transmit
    if (sr & 0xF0)
      tmp = _pkinetisk_spi->POPR;
  }
  _pkinetisk_spi->SR = SPI_SR_EOQF;
  _pkinetisk_spi->MCR = mcr;
  while (_pkinetisk_spi->SR & 0xF0) {
    tmp = _pkinetisk_spi->POPR;
  }
}

#elif defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
void ST7735_SPI::waitFifoNotFull(void) {
  uint32_t tmp __attribute__((unused));
  do {
    if ((_pimxrt_spi->RSR & LPSPI_RSR_RXEMPTY) == 0) {
      tmp = _pimxrt_spi->RDR; // Read any pending RX bytes in
      if (pending_rx_count)
        pending_rx_count--; // decrement count of bytes still levt
    }
  } while ((_pimxrt_spi->SR & LPSPI_SR_TDF) == 0);
}
void ST7735_SPI::waitFifoEmpty(void) {
  uint32_t tmp __attribute__((unused));
  do {
    if ((_pimxrt_spi->RSR & LPSPI_RSR_RXEMPTY) == 0) {
      tmp = _pimxrt_spi->RDR; // Read any pending RX bytes in
      if (pending_rx_count)
        pending_rx_count--; // decrement count of bytes still levt
    }
  } while ((_pimxrt_spi->SR & LPSPI_SR_TCF) == 0);
}
void ST7735_SPI::waitTransmitComplete(void) {
  uint32_t tmp __attribute__((unused));
  //    // digitalWriteFast(2, HIGH);

  while (pending_rx_count) {
    if ((_pimxrt_spi->RSR & LPSPI_RSR_RXEMPTY) == 0) {
      tmp = _pimxrt_spi->RDR; // Read any pending RX bytes in
      pending_rx_count--;     // decrement count of bytes still levt
    }
  }
  _pimxrt_spi->CR = LPSPI_CR_MEN | LPSPI_CR_RRF; // Clear RX FIFO
  //    // digitalWriteFast(2, LOW);
}

uint16_t ST7735_SPI::waitTransmitCompleteReturnLast() {
  uint32_t val=0;
  //    // digitalWriteFast(2, HIGH);

  while (pending_rx_count) {
    if ((_pimxrt_spi->RSR & LPSPI_RSR_RXEMPTY) == 0) {
      val = _pimxrt_spi->RDR; // Read any pending RX bytes in
      pending_rx_count--;     // decrement count of bytes still levt
    }
  }
  _pimxrt_spi->CR = LPSPI_CR_MEN | LPSPI_CR_RRF; // Clear RX FIFO
  return val;
  //    // digitalWriteFast(2, LOW);
}

void ST7735_SPI::waitTransmitComplete(uint32_t mcr) {
  // BUGBUG:: figure out if needed...
  waitTransmitComplete();
}
#endif

#endif // USE_SPI_ST7735
