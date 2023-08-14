//***************************************************
// https://github.com/kurte/ILI9341_t3n
// http://forum.pjrc.com/threads/26305-Highly-optimized-ILI9341-(320x240-TFT-color-display)-library
//
// Warning this is Kurt's updated version which allows it to work on different SPI busses.
//
// This is the GC9A01A_t3 driver with the bloat removed, fixes and modified for the ST7735 controller


#ifndef _ST7735_SPI_H_
#define _ST7735_SPI_H_

#if USE_SPI_ST7735



#define ST7735_USE_DMAMEM

// Allow us to enable or disable capabilities, particully Frame Buffer and
// Clipping for speed and size
#ifndef DISABLE_ST7735_FRAMEBUFFER
#if defined(__MK66FX1M0__) // T3.6
#define ENABLE_ST7735_FRAMEBUFFER
#define SCREEN_DMA_NUM_SETTINGS       2 // see if making it a constant value makes difference...
#elif defined(__MK64FX512__) // T3.5
#define ENABLE_ST7735_FRAMEBUFFER
#define SCREEN_DMA_NUM_SETTINGS       4 // see if making it a constant value makes difference...
#elif defined(__IMXRT1052__) || defined(__IMXRT1062__)
#define ENABLE_ST7735_FRAMEBUFFER
#define SCREEN_DMA_NUM_SETTINGS       2 // see if making it a constant value makes difference...
#endif
#endif

// Allow way to override using SPI

#ifdef __cplusplus
#include <Arduino.h>
#include <DMAChannel.h>
#include <SPI.h>

#endif
#include <stdint.h>


#define ST7735_TFTWIDTH		TFT_WIDTH
#define ST7735_TFTHEIGHT	TFT_HEIGHT

// NOTE: ILI9341 registers defined (but commented out) are ones that
// *might* be compatible with the ST7735, but aren't documented in
// the device datasheet. A couple are defined (with ILI name) and NOT
// commented out because they appeared in the manufacturer example code
// as raw register addresses, no documentation in datasheet, they SEEM
// to do the same thing as their ILI equivalents but this is not 100%
// confirmed so they've not been assigned ST7735 register defines.


#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_PASET   ST7735_RASET
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_IDLEOFF 0x38
#define ST7735_IDLEON  0x39


#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define ST7735_PWCTR6  0xFC

#define ST7735_TESTM 0xf0	// test mode



#define MADCTL_RGB	0x00
#define MADCTL_MH	0x04
#define MADCTL_BGR	0x08
#define MADCTL_ML	0x10
#define MADCTL_MV	0x20
#define MADCTL_MX	0x40
#define MADCTL_MY	0x80



// Color definitions
#define BLACK 0x0000       /*   0,   0,   0 */
#define NAVY 0x000F        /*   0,   0, 128 */
#define DARKGREEN 0x03E0   /*   0, 128,   0 */
#define DARKCYAN 0x03EF    /*   0, 128, 128 */
#define MAROON 0x7800      /* 128,   0,   0 */
#define PURPLE 0x780F      /* 128,   0, 128 */
#define OLIVE 0x7BE0       /* 128, 128,   0 */
#define LIGHTGREY 0xC618   /* 192, 192, 192 */
#define DARKGREY 0x7BEF    /* 128, 128, 128 */
#define BLUE 0x001F        /*   0,   0, 255 */
#define GREEN 0x07E0       /*   0, 255,   0 */
#define CYAN 0x07FF        /*   0, 255, 255 */
#define RED 0xF800         /* 255,   0,   0 */
#define MAGENTA 0xF81F     /* 255,   0, 255 */
#define YELLOW 0xFFE0      /* 255, 255,   0 */
#define WHITE 0xFFFF       /* 255, 255, 255 */
#define ORANGE 0xFD20      /* 255, 165,   0 */
#define GREENYELLOW 0xAFE5 /* 173, 255,  47 */
#define PINK 0xF81F

#ifndef CL
#define CL(_r,_g,_b) ((((_r)&0xF8)<<8)|(((_g)&0xFC)<<3)|((_b)>>3))
#endif
#define sint16_t int16_t



#ifdef __cplusplus
// At all other speeds, _pspi->beginTransaction() will use the fastest available
// clock
#ifdef KINETISK
//#define ST7735_SPICLOCK 96000000
//#define ST7735_SPICLOCK_READ 2000000
#elif defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
#define ST7735_SPICLOCK 48		// mhz
#define ST7735_SPICLOCK_READ 20000000
#else
//#define ST7735_SPICLOCK 96000000
//#define ST7735_SPICLOCK_READ 2000000
#endif

#if defined(__IMXRT1062__)  // Teensy 4.x
// Also define these in lower memory so as to make sure they are not cached...
typedef struct {
  DMASetting      _dmasettings[2];
  DMAChannel      _dmatx;
} ST7735_DMA_Data_t;
#endif


class ST7735_SPI {
public:
  ST7735_SPI (uint8_t _CS, uint8_t _DC, uint8_t _RST = 255, uint8_t _MOSI = 11, uint8_t _SCLK = 13);
  void begin (uint32_t spi_clock = ST7735_SPICLOCK, uint32_t spi_clock_read = ST7735_SPICLOCK_READ);
  
  void sleep (bool enable);
  void setRotation (uint8_t r);
  void invertDisplay (boolean i);
  void setAddrWindow (uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
  
  void fillRect (int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void fillScreen (uint16_t color);
  
  void setBacklight (const uint8_t value);
  void pushPixels16bit (uint16_t *pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

  
  // Pass 8-bit (each) R,G,B, get back 16-bit packed color
  static uint16_t color565 (uint8_t r, uint8_t g, uint8_t b){
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  }

  // color565toRGB		- converts 565 format 16 bit color to RGB
  static void color565toRGB(uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b) {
    r = (color >> 8) & 0x00F8;
    g = (color >> 3) & 0x00FC;
    b = (color << 3) & 0x00F8;
  }

  // color565toRGB14		- converts 16 bit 565 format color to 14 bit RGB (2
  // bits clear for math and sign)
  // returns 00rrrrr000000000,00gggggg00000000,00bbbbb000000000
  // thus not overloading sign, and allowing up to double for additions for
  // fixed point delta
  static void color565toRGB14(uint16_t color, int16_t &r, int16_t &g, int16_t &b) {
    r = (color >> 2) & 0x3E00;
    g = (color << 3) & 0x3F00;
    b = (color << 9) & 0x3E00;
  }

  // RGB14tocolor565		- converts 14 bit RGB back to 16 bit 565 format
  // color
  static uint16_t RGB14tocolor565(int16_t r, int16_t g, int16_t b) {
    return (((r & 0x3E00) << 2) | ((g & 0x3F00) >> 3) | ((b & 0x3E00) >> 9));
  }

  // Device does not have MISO line so query is not valid
  // uint8_t readdata(void);
  //uint8_t readcommand8(uint8_t r//eg, uint8_t index = 0);
  //uint16_t readScanLine();
  void setFrameRateControl(uint8_t mode);

  // Added functions to read pixel data...
  // These will only work if there is a frame buffer
  // as no MISO pin on display


  void writeRect(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *pcolors);




  int16_t width (void) {
  	return _width;
  }

  int16_t height (void){ 
  	return _height;
  }
  
  uint8_t getRotation(void);
  

  
  // added support to use optional Frame buffer
  enum {
    ST7735_DMA_INIT = 0x01,
    ST7735_DMA_EVER_INIT = 0x08,
    ST7735_DMA_CONT = 0x02,
    ST7735_DMA_FINISH = 0x04,
    ST7735_DMA_ACTIVE = 0x80
  };
  void setFrameBuffer(uint16_t *frame_buffer);
  uint8_t
  useFrameBuffer(boolean b);  // use the frame buffer?  First call will allocate
  void freeFrameBuffer(void); // explicit call to release the buffer
  void updateScreen(void);    // call to say update the screen now.
  bool updateScreenAsync(bool update_cont = false); // call to say update the
                                                    // screen optinoally turn
                                                    // into continuous mode.
  void waitUpdateAsyncComplete(void);
  void endUpdateAsync(); // Turn of the continueous mode fla
  void dumpDMASettings();
#ifdef ENABLE_ST7735_FRAMEBUFFER
  uint16_t *getFrameBuffer() { return _pfbtft; }
  uint32_t frameCount() { return _dma_frame_count; }
  uint16_t subFrameCount() { return _dma_sub_frame_count; }
  boolean asyncUpdateActive(void) { return (_dma_state & ST7735_DMA_ACTIVE); }
  void initDMASettings(void);
  void setFrameCompleteCB(void (*pcb)(), bool fCallAlsoHalfDone = false);
#else
  uint32_t frameCount() { return 0; }
  uint16_t subFrameCount() { return 0; }
  uint16_t *getFrameBuffer() { return NULL; }
  boolean asyncUpdateActive(void) { return false; }
#endif
  void updateChangedAreasOnly(bool updateChangedOnly) {
#ifdef ENABLE_ST7735_FRAMEBUFFER
    _updateChangedAreasOnly = updateChangedOnly;
#endif
  }

  // Setup to send a command with parameters like ST77xx code
  void sendCommand (uint8_t commandByte, const uint8_t *dataBytes, uint8_t numDataBytes);

protected:
  SPIClass *_pspi = nullptr;
  SPIClass::SPI_Hardware_t *_spi_hardware;

  uint8_t _spi_num;         // Which buss is this spi on?
  uint32_t _SPI_CLOCK;      // #define ILI9341_SPICLOCK 30000000
  uint32_t _SPI_CLOCK_READ; //#define ILI9341_SPICLOCK_READ 2000000

#if defined(KINETISK)
  KINETISK_SPI_t *_pkinetisk_spi;
#elif defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
  IMXRT_LPSPI_t *_pimxrt_spi;

#elif defined(KINETISL)
  KINETISL_SPI_t *_pkinetisl_spi;
#endif

  int16_t _width, _height; // Display w/h as modified by current rotation
  int16_t cursor_x, cursor_y;
  bool _center_x_text = false;
  bool _center_y_text = false;
  int16_t _clipx1, _clipy1, _clipx2, _clipy2;
  int16_t _originx, _originy;
  int16_t _displayclipx1, _displayclipy1, _displayclipx2, _displayclipy2;
  bool _invisible = false;
  bool _standard = true; // no bounding rectangle or origin set.

  uint16_t _x0_last = 0xffff;
  uint16_t _x1_last = 0xffff;
  uint16_t _y0_last = 0xffff;
  uint16_t _y1_last = 0xffff;

  inline void updateDisplayClip() {
    _displayclipx1 = max(0, min(_clipx1 + _originx, width()));
    _displayclipx2 = max(0, min(_clipx2 + _originx, width()));

    _displayclipy1 = max(0, min(_clipy1 + _originy, height()));
    _displayclipy2 = max(0, min(_clipy2 + _originy, height()));
    _invisible =
        (_displayclipx1 == _displayclipx2 || _displayclipy1 == _displayclipy2);
    _standard = (_displayclipx1 == 0) && (_displayclipx2 == _width) &&
                (_displayclipy1 == 0) && (_displayclipy2 == _height);
    if (Serial) {
      // Serial.printf("UDC (%d %d)-(%d %d) %d %d\n", _displayclipx1,
      // _displayclipy1, _displayclipx2,
      //	_displayclipy2, _invisible, _standard);
    }
  }

  int16_t scroll_x, scroll_y, scroll_width, scroll_height;
  boolean scrollEnable,
      isWritingScrollArea; // If set, 'wrap' text at right edge of display

  uint16_t textcolor, textbgcolor, scrollbgcolor;
  uint32_t textcolorPrexpanded, textbgcolorPrexpanded;
  uint8_t textsize_x, textsize_y, rotation, textdatum;
  boolean wrap; // If set, 'wrap' text at right edge of display

  

  uint8_t _rst;
  uint8_t _cs, _dc;
  uint8_t pcs_data, pcs_command;
  uint8_t _mosi, _sclk;

///////////////////////////////
// BUGBUG:: reorganize this area better!
#if defined(KINETISK)
  // inline uint8_t sizeFIFO() {return _fifo_size; }
  uint32_t _fifo_full_test;
  void waitFifoNotFull(void);
  void waitFifoEmpty(void);
  void waitTransmitComplete(void);
  void waitTransmitComplete(uint32_t mcr);

#elif defined(__IMXRT1052__) || defined(__IMXRT1062__)
  uint8_t pending_rx_count = 0; // hack ...
  void waitFifoNotFull(void);
  void waitFifoEmpty(void);
  void waitTransmitComplete(void);
  uint16_t waitTransmitCompleteReturnLast();
  void waitTransmitComplete(uint32_t mcr);
#elif defined(KINETISL)
#endif
//////////////////////////////

// add support to allow only one hardware CS (used for dc)
#if defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
  uint32_t _cspinmask;
  volatile uint32_t *_csport;
  uint32_t _spi_tcr_current;
  uint32_t _dcpinmask;
  uint32_t _tcr_dc_assert;
  uint32_t _tcr_dc_not_assert;
  volatile uint32_t *_dcport;
#else
  uint8_t _cspinmask;
  volatile uint8_t *_csport;
#endif
#ifdef KINETISL
  volatile uint8_t *_dcport;
  uint8_t _dcpinmask;
#endif
#ifdef ENABLE_ST7735_FRAMEBUFFER
  // Add support for optional frame buffer
  uint16_t *_pfbtft;              // Optional Frame buffer
  uint8_t _use_fbtft;             // Are we in frame buffer mode?
  uint16_t *_we_allocated_buffer; // We allocated the buffer;
  int16_t _changed_min_x, _changed_max_x, _changed_min_y, _changed_max_y;
  bool _updateChangedAreasOnly = false; // current default off,
  void (*_frame_complete_callback)() = nullptr;
  bool _frame_callback_on_HalfDone = false;

// Add DMA support.
  static ST7735_SPI *_dmaActiveDisplay[3]; // Use pointer to this as a way to

  volatile uint8_t _dma_state = 0;            // DMA status
  volatile uint32_t _dma_frame_count = 0;     // Can return a frame count...
  volatile uint16_t _dma_sub_frame_count = 0; // Can return a frame count...
#if defined(__MK66FX1M0__)
  // T3.6 use Scatter/gather with chain to do transfer
  static DMASetting _dmasettings[3][3];
  DMAChannel _dmatx;
#elif defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
  // try work around DMA memory cached.  So have a couple of buffers we copy
  // frame buffer into
  // as to move it out of the memory that is cached...

  static const uint32_t _count_pixels = ST7735_TFTWIDTH * ST7735_TFTHEIGHT;
  static ST7735_DMA_Data_t _dma_data[3]; // one structure for each possible SPI buss
  //DMASetting _dmasettings[3];
  //DMAChannel _dmatx;
  volatile uint32_t _dma_pixel_index = 0;
  uint16_t _dma_buffer_size; // the actual size we are using <= DMA_BUFFER_SIZE;
  uint16_t _dma_cnt_sub_frames_per_frame;
  uint32_t _spi_fcr_save; // save away previous FCR register value
#elif defined(__MK64FX512__)
  // T3.5 - had issues scatter/gather so do just use channels/interrupts
  // and update and continue
  static DMAChannel _dmatx;
  static DMAChannel _dmarx;
  static uint16_t _dma_count_remaining;
  static uint16_t _dma_write_size_words;
#endif
  static void dmaInterrupt(void);
  static void dmaInterrupt1(void);
  static void dmaInterrupt2(void);
  void process_dma_interrupt(void);
#endif
  void charBounds(char c, int16_t *x, int16_t *y, int16_t *minx, int16_t *miny,
                  int16_t *maxx, int16_t *maxy);

  void setAddr(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) __attribute__((always_inline)) {
    #if 1
    writecommand_cont(ST7735_CASET); // Column addr set
    writedata16_cont(x0);             // XSTART
    writedata16_cont(x1);             // XEND
    writecommand_cont(ST7735_PASET); // Row addr set
    writedata16_cont(y0);             // YSTART
    writedata16_cont(y1);             // YEND
    #else
    if ((x0 != _x0_last) || (x1 != _x1_last)) {
      writecommand_cont(ST7735_CASET); // Column addr set
      writedata16_cont(x0);             // XSTART
      writedata16_cont(x1);             // XEND
      _x0_last = x0;
      _x1_last = x1;
    }
    if ((y0 != _y0_last) || (y1 != _y1_last)) {
      writecommand_cont(ST7735_PASET); // Row addr set
      writedata16_cont(y0);             // YSTART
      writedata16_cont(y1);             // YEND
      _y0_last = y0;
      _y1_last = y1;
    }
    #endif    
  }
//. From Onewire utility files
#if defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x

  void DIRECT_WRITE_LOW (volatile uint32_t *base, uint32_t mask) __attribute__((always_inline)) {
    *(base + 34) = mask;
  }
  void DIRECT_WRITE_HIGH (volatile uint32_t *base, uint32_t mask) __attribute__((always_inline)) {
    *(base + 33) = mask;
  }
#endif

  void beginSPITransaction(uint32_t clock) __attribute__((always_inline)) {
    _pspi->beginTransaction(SPISettings(clock, MSBFIRST, SPI_MODE0));
#if defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
    if (!_dcport)
      _spi_tcr_current = _pimxrt_spi->TCR; // Only if DC is on hardware CS
#endif
    if (_csport) {
#if defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
      DIRECT_WRITE_LOW(_csport, _cspinmask);
#else
      *_csport &= ~_cspinmask;
#endif
    }
  }
  void endSPITransaction() __attribute__((always_inline)) {
    if (_csport) {
#if defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
      DIRECT_WRITE_HIGH(_csport, _cspinmask);
#else
      *_csport |= _cspinmask;
#endif
    }
    _pspi->endTransaction();
  }
#if defined(KINETISK)
  void writecommand_cont(uint8_t c) __attribute__((always_inline)) {
    _pkinetisk_spi->PUSHR = c | (pcs_command << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT;
    waitFifoNotFull();
  }
  void writedata8_cont(uint8_t c) __attribute__((always_inline)) {
    _pkinetisk_spi->PUSHR = c | (pcs_data << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT;
    waitFifoNotFull();
  }
  void writedata16_cont(uint16_t d) __attribute__((always_inline)) {
    _pkinetisk_spi->PUSHR = d | (pcs_data << 16) | SPI_PUSHR_CTAS(1) | SPI_PUSHR_CONT;
    waitFifoNotFull();
  }
  void writecommand_last(uint8_t c) __attribute__((always_inline)) {
    uint32_t mcr = _pkinetisk_spi->MCR;
    _pkinetisk_spi->PUSHR = c | (pcs_command << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_EOQ;
    waitTransmitComplete(mcr);
  }
  void writedata8_last(uint8_t c) __attribute__((always_inline)) {
    uint32_t mcr = _pkinetisk_spi->MCR;
    _pkinetisk_spi->PUSHR = c | (pcs_data << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_EOQ;
    waitTransmitComplete(mcr);
  }
  void writedata16_last(uint16_t d) __attribute__((always_inline)) {
    uint32_t mcr = _pkinetisk_spi->MCR;
    _pkinetisk_spi->PUSHR = d | (pcs_data << 16) | SPI_PUSHR_CTAS(1) | SPI_PUSHR_EOQ;
    waitTransmitComplete(mcr);
  }
#elif defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
#ifndef TCR_MASK
#define TCR_MASK  (LPSPI_TCR_PCS(3) | LPSPI_TCR_FRAMESZ(31) | LPSPI_TCR_CONT | LPSPI_TCR_RXMSK )
#endif

  void maybeUpdateTCR (uint32_t requested_tcr_state) /*__attribute__((always_inline)) */ {
    if ((_spi_tcr_current & TCR_MASK) != requested_tcr_state) {
      bool dc_state_change = (_spi_tcr_current & LPSPI_TCR_PCS(3)) !=
                             (requested_tcr_state & LPSPI_TCR_PCS(3));
      _spi_tcr_current = (_spi_tcr_current & ~TCR_MASK) | requested_tcr_state;
      // only output when Transfer queue is empty.
      if (!dc_state_change || !_dcpinmask) {
        while ((_pimxrt_spi->FSR & 0x1f))
          ;
        _pimxrt_spi->TCR = _spi_tcr_current; // update the TCR

      } else {
        waitTransmitComplete();
        if (requested_tcr_state & LPSPI_TCR_PCS(3))
          DIRECT_WRITE_HIGH(_dcport, _dcpinmask);
        else
          DIRECT_WRITE_LOW(_dcport, _dcpinmask);
        _pimxrt_spi->TCR = _spi_tcr_current & ~(LPSPI_TCR_PCS(3) | LPSPI_TCR_CONT); // go ahead and update TCR anyway?
      }
    }
  }

  // BUGBUG:: currently assumming we only have CS_0 as valid CS
  void writecommand_cont (uint8_t c) __attribute__((always_inline)) {
    maybeUpdateTCR(_tcr_dc_assert | LPSPI_TCR_FRAMESZ(7) /*| LPSPI_TCR_CONT*/);
    _pimxrt_spi->TDR = c;
    pending_rx_count++; //
    waitFifoNotFull();
  }
  void writedata8_cont (uint8_t c) __attribute__((always_inline)) {
    maybeUpdateTCR(_tcr_dc_not_assert | LPSPI_TCR_FRAMESZ(7) | LPSPI_TCR_CONT);
    _pimxrt_spi->TDR = c;
    pending_rx_count++; //
    waitFifoNotFull();
  }
  void writedata16_cont (uint16_t d) __attribute__((always_inline)) {
    maybeUpdateTCR(_tcr_dc_not_assert | LPSPI_TCR_FRAMESZ(15) | LPSPI_TCR_CONT);
    _pimxrt_spi->TDR = d;
    pending_rx_count++; //
    waitFifoNotFull();
  }
  void writecommand_last (uint8_t c) __attribute__((always_inline)) {
    maybeUpdateTCR(_tcr_dc_assert | LPSPI_TCR_FRAMESZ(7));
    _pimxrt_spi->TDR = c;
    //		_pimxrt_spi->SR = LPSPI_SR_WCF | LPSPI_SR_FCF | LPSPI_SR_TCF;
    pending_rx_count++; //
    waitTransmitComplete();
  }
  void writedata8_last (uint8_t c) __attribute__((always_inline)) {
    maybeUpdateTCR(_tcr_dc_not_assert | LPSPI_TCR_FRAMESZ(7));
    _pimxrt_spi->TDR = c;
    //		_pimxrt_spi->SR = LPSPI_SR_WCF | LPSPI_SR_FCF | LPSPI_SR_TCF;
    pending_rx_count++; //
    waitTransmitComplete();
  }
  void writedata16_last (uint16_t d) __attribute__((always_inline)) {
    maybeUpdateTCR(_tcr_dc_not_assert | LPSPI_TCR_FRAMESZ(15));
    _pimxrt_spi->TDR = d;
    //		_pimxrt_spi->SR = LPSPI_SR_WCF | LPSPI_SR_FCF | LPSPI_SR_TCF;
    pending_rx_count++; //
    waitTransmitComplete();
  }

#elif defined(KINETISL)
  // Lets see how hard to make it work OK with T-LC
  uint8_t _dcpinAsserted;
  uint8_t _data_sent_not_completed;
  void waitTransmitComplete() {
    while (_data_sent_not_completed) {
      uint16_t timeout_count = 0xff; // hopefully enough
      while (!(_pkinetisl_spi->S & SPI_S_SPRF) && timeout_count--)
        ; // wait
      uint8_t d __attribute__((unused));
      d = _pkinetisl_spi->DL;
      d = _pkinetisl_spi->DH;
      _data_sent_not_completed--; // We hopefully received our data...
    }
  }
  uint16_t waitTransmitCompleteReturnLast() {
    uint16_t d = 0;
    while (_data_sent_not_completed) {
      uint16_t timeout_count = 0xff; // hopefully enough
      while (!(_pkinetisl_spi->S & SPI_S_SPRF) && timeout_count--)
        ; // wait
      d = (_pkinetisl_spi->DH << 8) | _pkinetisl_spi->DL;
      _data_sent_not_completed--; // We hopefully received our data...
    }
    return d;
  }

  void setCommandMode () __attribute__((always_inline)) {
    if (!_dcpinAsserted) {
      waitTransmitComplete();
      *_dcport &= ~_dcpinmask;
      _dcpinAsserted = 1;
    }
  }

  void setDataMode () __attribute__((always_inline)) {
    if (_dcpinAsserted) {
      waitTransmitComplete();
      *_dcport |= _dcpinmask;
      _dcpinAsserted = 0;
    }
  }

  void outputToSPI (uint8_t c) {
    if (_pkinetisl_spi->C2 & SPI_C2_SPIMODE) {
      // Wait to change modes until any pending output has been done.
      waitTransmitComplete();
      _pkinetisl_spi->C2 = 0; // make sure 8 bit mode.
    }
    while (!(_pkinetisl_spi->S & SPI_S_SPTEF))
      ; // wait if output buffer busy.
    // Clear out buffer if there is something there...
    if ((_pkinetisl_spi->S & SPI_S_SPRF)) {
      uint8_t d __attribute__((unused));
      d = _pkinetisl_spi->DL;
      _data_sent_not_completed--;
    }
    _pkinetisl_spi->DL = c;     // output byte
    _data_sent_not_completed++; // let system know we sent something
  }

  void outputToSPI16(uint16_t data) {
    if (!(_pkinetisl_spi->C2 & SPI_C2_SPIMODE)) {
      // Wait to change modes until any pending output has been done.
      waitTransmitComplete();
      _pkinetisl_spi->C2 = SPI_C2_SPIMODE; // make sure 8 bit mode.
    }
    uint8_t s;
    do {
      s = _pkinetisl_spi->S;
      // wait if output buffer busy.
      // Clear out buffer if there is something there...
      if ((s & SPI_S_SPRF)) {
        uint8_t d __attribute__((unused));
        d = _pkinetisl_spi->DL;
        d = _pkinetisl_spi->DH;
        _data_sent_not_completed--; // let system know we sent something
      }

    } while (!(s & SPI_S_SPTEF) || (s & SPI_S_SPRF));

    _pkinetisl_spi->DL = data;      // output low byte
    _pkinetisl_spi->DH = data >> 8; // output high byte
    _data_sent_not_completed++;     // let system know we sent something
  }

  void writecommand_cont (uint8_t c) {
    setCommandMode();
    outputToSPI(c);
  }
  void writedata8_cont (uint8_t c) {
    setDataMode();
    outputToSPI(c);
  }

  void writedata16_cont (uint16_t c) {
    setDataMode();
    outputToSPI16(c);
  }

  void writecommand_last (uint8_t c) {
    setCommandMode();
    outputToSPI(c);
    waitTransmitComplete();
  }
  void writedata8_last (uint8_t c) {
    setDataMode();
    outputToSPI(c);
    waitTransmitComplete();
  }
  void writedata16_last (uint16_t c) {
    setDataMode();
    outputToSPI16(c);
    waitTransmitComplete();
    _pkinetisl_spi->C2 = 0; // Set back to 8 bit mode...
    _pkinetisl_spi->S;      // Read in the status;
  }

#endif

#ifdef ENABLE_ST7735_FRAMEBUFFER
  void clearChangedRange() {
    _changed_min_x = 0x7fff;
    _changed_max_x = -1;
    _changed_min_y = 0x7fff;
    _changed_max_y = -1;
  }

  void updateChangedRange (int16_t x, int16_t y, int16_t w, int16_t h)
      __attribute__((always_inline)) {
    if (x < _changed_min_x)
      _changed_min_x = x;
    if (y < _changed_min_y)
      _changed_min_y = y;
    x += w - 1;
    y += h - 1;
    if (x > _changed_max_x)
      _changed_max_x = x;
    if (y > _changed_max_y)
      _changed_max_y = y;
  }

  // could combine with above, but avoids the +-...
  void updateChangedRange (int16_t x, int16_t y) __attribute__((always_inline)) {
    if (x < _changed_min_x)
      _changed_min_x = x;
    if (y < _changed_min_y)
      _changed_min_y = y;
    if (x > _changed_max_x)
      _changed_max_x = x;
    if (y > _changed_max_y)
      _changed_max_y = y;
  }
#endif


  
  /**
   * Found in a pull request for the Adafruit framebuffer library. Clever!
   * https://github.com/tricorderproject/arducordermini/pull/1/files#diff-d22a481ade4dbb4e41acc4d7c77f683d
   * Converts  0000000000000000rrrrrggggggbbbbb
   *     into  00000gggggg00000rrrrr000000bbbbb
   * with mask 00000111111000001111100000011111
   * This is useful because it makes space for a parallel fixed-point multiply
   * This implements the linear interpolation formula: result = bg * (1.0 -
   *alpha) + fg * alpha
   * This can be factorized into: result = bg + (fg - bg) * alpha
   * alpha is in Q1.5 format, so 0.0 is represented by 0, and 1.0 is represented
   *by 32
   * @param	fg		Color to draw in RGB565 (16bit)
   * @param	bg		Color to draw over in RGB565 (16bit)
   * @param	alpha	Alpha in range 0-255
   **/
  uint16_t alphaBlendRGB565 (uint32_t fg, uint32_t bg, uint8_t alpha) __attribute__((always_inline)) {
    alpha = (alpha + 4) >> 3; // from 0-255 to 0-31
    bg = (bg | (bg << 16)) & 0b00000111111000001111100000011111;
    fg = (fg | (fg << 16)) & 0b00000111111000001111100000011111;
    uint32_t result =
        ((((fg - bg) * alpha) >> 5) + bg) & 0b00000111111000001111100000011111;
    return (uint16_t)((result >> 16) | result); // contract result
  }
  /**
   * Same as above, but fg and bg are premultiplied, and alpah is already in
   * range 0-31
   */
  uint16_t alphaBlendRGB565Premultiplied(uint32_t fg, uint32_t bg, uint8_t alpha) __attribute__((always_inline)) {
    uint32_t result =
        ((((fg - bg) * alpha) >> 5) + bg) & 0b00000111111000001111100000011111;
    return (uint16_t)((result >> 16) | result); // contract result
  }
  void Pixel (int16_t x, int16_t y, uint16_t color) __attribute__((always_inline)) {
    x += _originx;
    y += _originy;

    if ((x < _displayclipx1) || (x >= _displayclipx2) || (y < _displayclipy1) ||
        (y >= _displayclipy2))
      return;

#ifdef ENABLE_ST7735_FRAMEBUFFER
    if (_use_fbtft) {
      updateChangedRange(x, y); // update the range of the screen that has been changed;
      _pfbtft[y * _width + x] = color;
      return;
    }
#endif
    setAddr(x, y, x, y);
    writecommand_cont(ST7735_RAMWR);
    writedata16_cont(color);
  }

  uint32_t fetchpixel(const uint8_t *p, uint32_t index, uint32_t x);
};

#ifndef ST7735_swap
#define ST7735_swap(a, b)                                                     \
  {                                                                            \
    typeof(a) t = a;                                                           \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif

//#endif

#endif // __cplusplus

#endif // USE_SPI_ST7735

#endif
