/*****************************************************************
 *
 * File name:         AUDIO_TASK.H
 * Description:       Project definitions and function prototypes for use with AUDIO_TASKS.c
 * Author:            Hab S. Collector
 * Date:              2/15/2011
 * LAST EDIT:         7/24/2012
 * Hardware:               
 * Firmware Tool:     CrossStudio for ARM
 * Notes:             This file should be written as to not be dependent 
 *                    on other includes - everything these functions need should be passed to them
*****************************************************************/ 

#ifndef _AUDIO_TASKS_DEFINES
#define _AUDIO_TASKS_DEFINES

// INCLUDES
#include "HC15C_DEFINES.H"


// WAVE AUDIO OFFSETS - COMMENTS ARE FOR 16BIT PCM WAV AUDIO
#define RIFF_CHUNCK_OFFSET      0  // EXPECTED "RIFF"
#define RIFF_TYPE_OFFSET        8  // EXPECTED "WAVE"
#define FORMAT_CHUNCK_OFFSET   12  // EXPECTED "fmt "
#define FORMAT_SIZE_OFFSET     16  // EXPECTED 16 FOR PCM
#define COMPRESSION_OFFSET     20  // EXPECTED 1 FOR PCM
#define CHANNEL_NUMBER_OFFSET  22  // 1 OR 2
#define SAMPLE_RATE_OFFSET     24  // 8000, 44100, etc.
#define BYTE_RATE_OFFSET       28  // SampleRate * NumChannels * BitsPerSample/8
#define BLOCK_ALIGN_OFFSET     32  // NumChannels * BitsPerSample/8
#define BIT_PER_SAMPLE_OFFSET  34  // 8 bits = 8, 16 bits = 16
#define DATA_CHUNCK_OFFSET     36  // EXPECTED "data"
#define DATA_SIZE_OFFSET       40  // NumSamples * NumChannels * BitsPerSample/8
#define DATA_OFFSET            44  // LEFT IS THE FIST CHANNEL READ

// WAVE AUDIO CHUNK NAMES
#define RIFF_FILE_TYPE "RIFF"
#define WAVE_RIFF_TYPE "WAVE"
#ifndef STRING_NULL
  #define STRING_NULL     ('\0')
#endif

// BUFFER SIZES
#define MAX_LENGTH_WAV_FILE    30
#define IN_COMMING_BUFFER_SIZE 2048u
#define CIRCULAR_BUFFER_SIZE   512u

// WAVE DIRECTORIES AND FILE NAMES - FILE NAME LIMIT 30 CHARS
#define ROOT_DIR                "0:"
#define HC15C_AUDIO_PATH        "0:\\HC15C_AUDIO"
#define HC15C_MUSIC_PATH        "0:\\MUSIC_FILES"
// CORE FILE NAMES               012345678901234567890123456789
#define WELCOME_WAV             "GREETINGS.WAV"
#define BUTTON_CLICK_WAV        "CLICK.WAV"
#define TIME_ALARM_WAV          "ALARM1.WAV"
#define ON_LINE_WAV             "ON_LINE.WAV"
// ERROR FILE NAMES              012345678901234567890123456789
#define INVALID_NUMBER_WAV      "INVALID_NUM.WAV"
#define VALUE_TOO_LARGE_WAV     "NUM_TOO_LARGE.WAV"
#define I_AND_I_ERROR_WAV       "I_AND_I_ERROR.WAV"
// FULL VERBSOE: REQUIRES THE FILE AND THE FILE MASK
// FULL INTERACTIVE FILE NAMES   012345678901234567890123456789
#define FIX_EXCEEDED_WAV        "FIX_EXCEEDED.WAV"
#define ANGULAR_DEGREE_WAV      "ANGULAR_DEGREE.WAV"
#define ANGULAR_RAD_WAV         "ANGULAR_RAD.WAV"
#define DEC_INPUT_WAV           "DEC_INPUT.WAV"
#define HEX_INPUT_WAV           "HEX_INPUT.WAV"
#define VERIFY_ANG_MEASURE_WAV  "VERIFY_ANGULAR_MEASURE.WAV"
// FULL INTERACTIVE MASKS
#define FIX_EXCEEDED_MASK       ((uint16_t)(1<<1))
#define ANGULAR_DEGREE_MASK     ((uint16_t)(1<<2))
#define ANGULAR_RAD_MASK        ((uint16_t)(1<<3))
#define DEC_INPUT_MASK          ((uint16_t)(1<<4))
#define HEX_INPUT_MASK          ((uint16_t)(1<<5))
#define VERIFY_ANG_MEASURE_MASK ((uint16_t)(1<<6))

// LED BAR GRAPH LEVELS
#define LEVEL_30DB   1036u          // BASED ON 32768 FULL SCALE -30dB
#define LEVEL_26DB   1642u          // BASED ON 32768 FULL SCALE -26dB
#define LEVEL_22DB   2603u          // BASED ON 32768 FULL SCALE -22dB
#define LEVEL_18DB   4125u          // BASED ON 32768 FULL SCALE -18dB
#define LEVEL_14DB   6537u          // BASED ON 32768 FULL SCALE -14dB
#define LEVEL_10DB   10361u         // BASED ON 32768 FULL SCALE -10dB

// STRUCTURES UNIONS AND ENUMS
typedef union
  {
  int32_t Int32Value;
  uint8_t ByteValue[sizeof(int32_t)];
  } Union_BytesPerSecond;

typedef union
  {
  int32_t Int32Value;
  uint8_t ByteValue[sizeof(int32_t)];
  } Union_DataChunkSize;

typedef union
  {
  int16_t Signed16Bit_Value;
  uint8_t ByteValue[sizeof(int16_t)];
  } Union_AudioValue;

typedef struct 
  {
    uint16_t Size;   // MAX NUMBER OF ELEMENTS 
    uint16_t Start;  // INDEX OF OLDEST ELEMENT 
    uint16_t End;    // INDEX AT WHICH TO WRITE NEW ELEMENT 
    uint16_t *Elems; // VECTOR OF ELEMENTS 
  } Type_CircularBuffer;

typedef struct
  {
  uint8_t FileName[MAX_LENGTH_WAV_FILE];
  uint8_t PlayLevel;
  uint16_t FullInteractiveMask;
  } Type_AudioQueueStruct;

enum ChannelDirection
  {
  NONE,
  LEFT,
  STEREO
  };

enum ByteOrder
  {
  LOW_BYTE,
  HIGH_BYTE
  };


// PROTOTYPES
void audio_taskFn(void *);
BOOLEAN call_play16Bit_WAVE(Type_AudioQueueStruct *);
uint16_t call_S16Bit_To_10Bit(int16_t);
void init_DAC(void);
void audioLED_BarGraph(uint16_t);
void init_CB(Type_CircularBuffer *, uint16_t);
void free_CB(Type_CircularBuffer *);
uint8_t isFull_CB(Type_CircularBuffer *);
uint8_t isEmpty_CB(Type_CircularBuffer *);
void write_CB(Type_CircularBuffer *, uint16_t *);
void read_CB(Type_CircularBuffer *, uint16_t *);


#endif