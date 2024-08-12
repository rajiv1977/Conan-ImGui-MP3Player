#pragma once
#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <mmreg.h>
#include <msacm.h>
#include <wmsdk.h>

#pragma comment(lib, "msacm32.lib") 
#pragma comment(lib, "wmvcore.lib") 
#pragma comment(lib, "winmm.lib") 
#pragma intrinsic(memset,memcpy,memcmp)

#ifdef _DEBUG
#define mp3Assert(function) assert((function) == 0)
#else
#define mp3Assert(function) (function)
#endif

/// @brief       This is a MP3Player class: play, stop, pause, rewind and fwd music
///              Initally written by Alexandre Mutel and modifed by Rajiv Sit
class MP3Player
{
private:
	/// declaring variables
	HWAVEOUT mHandleWaveOut;
	DWORD    mBufferLength;
	double   mDurationInSecond;
	BYTE* mSoundBuffer;

public:
	/// @brief       loads a MP3 file and convert it internaly to a PCM format, ready for sound playback.
	///
	/// @param [in]  name of the input file
	/// @param [out] handle
	HRESULT openFromFile(TCHAR* inputFileName)
	{
		// Open the mp3 file
		HANDLE hFile = CreateFile(inputFileName,
			GENERIC_READ,          // desired access
			FILE_SHARE_READ,       // share for reading
			NULL,                  // no security
			OPEN_EXISTING,         // existing file only
			FILE_ATTRIBUTE_NORMAL, // normal file
			NULL);                 // no attr
		assert(hFile != INVALID_HANDLE_VALUE);

		// Get FileSize
		DWORD fileSize = GetFileSize(hFile, NULL);
		assert(fileSize != INVALID_FILE_SIZE);

		// Alloc buffer for file
		BYTE* mp3Buffer = (BYTE*)LocalAlloc(LPTR, fileSize);

		// Read file and fill mp3Buffer
		DWORD bytesRead;
		DWORD resultReadFile = ReadFile(hFile, mp3Buffer, fileSize, &bytesRead, NULL);
		assert(resultReadFile != 0);
		assert(bytesRead == fileSize);

		// Close File
		CloseHandle(hFile);

		// Open and convert MP3
		HRESULT hr = openFromMemory(mp3Buffer, fileSize);

		// Free mp3Buffer
		LocalFree(mp3Buffer);

		return hr;
	}

	/// @brief       loads a MP3 from memory and convert it internaly to a PCM format, ready for sound playback.
	///
	/// @param [in]  mp3 input buffer
	/// @param [in]  size of the mp3 inpput buffer
	/// @param [out] handle results
	HRESULT openFromMemory(BYTE* mp3InputBuffer, DWORD mp3InputBufferSize) {
		IWMSyncReader* wmSyncReader;
		IWMHeaderInfo* wmHeaderInfo;
		IWMProfile* wmProfile;
		IWMStreamConfig* wmStreamConfig;
		IWMMediaProps* wmMediaProperties;
		WORD wmStreamNum = 0;
		WMT_ATTR_DATATYPE wmAttrDataType;
		DWORD durationInSecondInt;
		QWORD durationInNano;
		DWORD sizeMediaType;
		DWORD maxFormatSize = 0;
		HACMSTREAM acmMp3stream = NULL;
		HGLOBAL mp3HGlobal;
		IStream* mp3Stream;

		// Define output format
		static WAVEFORMATEX pcmFormat = {
		 WAVE_FORMAT_PCM, //format type
		 2,               // number of channels (i.e. mono, stereo...)
		 44100,           // sample rate
		 4 * 44100,       // for buffer estimation 
		 4,               // block size of data
		 16,              // number of bits per sample of mono data 
		 0,               // the count in bytes of the size of 
		};

		const DWORD MP3_BLOCK_SIZE = 522;

		// Define input format
		static MPEGLAYER3WAVEFORMAT mp3Format = {
		 {
		  WAVE_FORMAT_MPEGLAYER3,       // format type 
		   2,                           // number of channels (i.e. mono, stereo...) 
		   44100,                       // sample rate 
		   128 * (1024 / 8),            // average bytes per sec not really used but must be one of 64, 96, 112, 128, 160kbps
		   1,                           // block size of data 
		   0,                           // number of bits per sample of mono data 
		   MPEGLAYER3_WFX_EXTRA_BYTES,  // cbSize       
		 },
		 MPEGLAYER3_ID_MPEG,            // wID
		 MPEGLAYER3_FLAG_PADDING_OFF,   //fdwFlags
		 MP3_BLOCK_SIZE,                // nBlockSize
		 1,                             // nFramesPerBlock
		 1393,                          // nCodecDelay;
		};

		// -----------------------------------------------------------------------------------
		// Extract and verify mp3 info : duration, type = mp3, sampleRate = 44100, channels = 2
		// -----------------------------------------------------------------------------------

		// Initialize COM
		CoInitialize(0);

		// Create SyncReader
		mp3Assert(WMCreateSyncReader(NULL, WMT_RIGHT_PLAYBACK, &wmSyncReader));

		// Alloc With global and create IStream
		mp3HGlobal = GlobalAlloc(GPTR, mp3InputBufferSize);
		assert(mp3HGlobal != 0);
		void* mp3HGlobalBuffer = GlobalLock(mp3HGlobal);
		memcpy(mp3HGlobalBuffer, mp3InputBuffer, mp3InputBufferSize);
		GlobalUnlock(mp3HGlobal);
		mp3Assert(CreateStreamOnHGlobal(mp3HGlobal, FALSE, &mp3Stream));

		// Open MP3 Stream
		mp3Assert(wmSyncReader->OpenStream(mp3Stream));

		// Get HeaderInfo interface
		mp3Assert(wmSyncReader->QueryInterface(&wmHeaderInfo));

		// Retrieve mp3 song duration in seconds
		WORD lengthDataType = sizeof(QWORD);
		mp3Assert(wmHeaderInfo->GetAttributeByName(&wmStreamNum, L"Duration", &wmAttrDataType, (BYTE*)&durationInNano, &lengthDataType));
		mDurationInSecond = ((double)durationInNano) / 10000000.0;
		durationInSecondInt = (int)(durationInNano / 10000000) + 1;

		// Sequence of call to get the MediaType
		// WAVEFORMATEX for mp3 can then be extract from MediaType
		mp3Assert(wmSyncReader->QueryInterface(&wmProfile));
		mp3Assert(wmProfile->GetStream(0, &wmStreamConfig));
		mp3Assert(wmStreamConfig->QueryInterface(&wmMediaProperties));

		// Retrieve sizeof MediaType
		mp3Assert(wmMediaProperties->GetMediaType(NULL, &sizeMediaType));

		// Retrieve MediaType
		WM_MEDIA_TYPE* mediaType = (WM_MEDIA_TYPE*)LocalAlloc(LPTR, sizeMediaType);
		mp3Assert(wmMediaProperties->GetMediaType(mediaType, &sizeMediaType));

		// Check that MediaType is audio
		assert(mediaType->majortype == WMMEDIATYPE_Audio);

		// Check that input is mp3
		WAVEFORMATEX* inputFormat = (WAVEFORMATEX*)mediaType->pbFormat;
		assert(inputFormat->wFormatTag == WAVE_FORMAT_MPEGLAYER3);
		assert(inputFormat->nSamplesPerSec == 44100);
		assert(inputFormat->nChannels == 2);

		// Release COM interface
		wmMediaProperties->Release();
		wmStreamConfig->Release();
		wmProfile->Release();
		wmHeaderInfo->Release();
		wmSyncReader->Release();

		// Free allocated mem
		LocalFree(mediaType);

		// -----------------------------------------------------------------------------------
		// Convert mp3 to pcm using acm driver
		// The following code is mainly inspired from http://david.weekly.org/code/mp3acm.html
		// -----------------------------------------------------------------------------------

		// Get maximum FormatSize for all acm
		mp3Assert(acmMetrics(NULL, ACM_METRIC_MAX_SIZE_FORMAT, &maxFormatSize));

		// Allocate PCM output sound buffer
		mBufferLength = mDurationInSecond * pcmFormat.nAvgBytesPerSec;
		mSoundBuffer = (BYTE*)LocalAlloc(LPTR, mBufferLength);

		acmMp3stream = NULL;
		switch (acmStreamOpen(&acmMp3stream,    // Open an ACM conversion stream
			NULL,                       // Query all ACM drivers
			(LPWAVEFORMATEX)&mp3Format, // input format :  mp3
			&pcmFormat,                 // output format : pcm
			NULL,                       // No filters
			0,                          // No async callback
			0,                          // No data for callback
			0                           // No flags
		)
			) {
		case MMSYSERR_NOERROR:
			break; // success!
		case MMSYSERR_INVALPARAM:
			assert(!"Invalid parameters passed to acmStreamOpen");
			return E_FAIL;
		case ACMERR_NOTPOSSIBLE:
			assert(!"No ACM filter found capable of decoding MP3");
			return E_FAIL;
		default:
			assert(!"Some error opening ACM decoding stream!");
			return E_FAIL;
		}

		// Determine output decompressed buffer size
		unsigned long rawbufsize = 0;
		mp3Assert(acmStreamSize(acmMp3stream, MP3_BLOCK_SIZE, &rawbufsize, ACM_STREAMSIZEF_SOURCE));
		assert(rawbufsize > 0);

		// allocate our I/O buffers
		static BYTE mp3BlockBuffer[MP3_BLOCK_SIZE];
		LPBYTE rawbuf = (LPBYTE)LocalAlloc(LPTR, rawbufsize);

		// prepare the decoder
		static ACMSTREAMHEADER mp3streamHead;
		mp3streamHead.cbStruct = sizeof(ACMSTREAMHEADER);
		mp3streamHead.pbSrc = mp3BlockBuffer;
		mp3streamHead.cbSrcLength = MP3_BLOCK_SIZE;
		mp3streamHead.pbDst = rawbuf;
		mp3streamHead.cbDstLength = rawbufsize;
		mp3Assert(acmStreamPrepareHeader(acmMp3stream, &mp3streamHead, 0));

		BYTE* currentOutput = mSoundBuffer;
		DWORD totalDecompressedSize = 0;

		static ULARGE_INTEGER newPosition;
		static LARGE_INTEGER seekValue;
		mp3Assert(mp3Stream->Seek(seekValue, STREAM_SEEK_SET, &newPosition));

		while (1) {
			// suck in some MP3 data
			ULONG count;
			mp3Assert(mp3Stream->Read(mp3BlockBuffer, MP3_BLOCK_SIZE, &count));
			if (count != MP3_BLOCK_SIZE)
				break;

			// convert the data
			mp3Assert(acmStreamConvert(acmMp3stream, &mp3streamHead, ACM_STREAMCONVERTF_BLOCKALIGN));

			// write the decoded PCM to disk
			memcpy(currentOutput, rawbuf, mp3streamHead.cbDstLengthUsed);
			totalDecompressedSize += mp3streamHead.cbDstLengthUsed;
			currentOutput += mp3streamHead.cbDstLengthUsed;
		};

		mp3Assert(acmStreamUnprepareHeader(acmMp3stream, &mp3streamHead, 0));
		LocalFree(rawbuf);
		mp3Assert(acmStreamClose(acmMp3stream, 0));

		// Release allocated memory
		mp3Stream->Release();
		GlobalFree(mp3HGlobal);
		return S_OK;
	}

	/// @brief       pause audio
	void __inline setPause()
	{
		mp3Assert(waveOutPause(mHandleWaveOut));
	}

	/// @brief       unset the pause of audio
	void __inline unSetPause()
	{
		mp3Assert(waveOutRestart(mHandleWaveOut));
	}

	/// @brief       adjust volume
	void setVolume(const DWORD& value)
	{
		waveOutSetVolume(mHandleWaveOut, value);
	}

	/// @brief       close the current MP3Player, stop playback and free allocated memory
	void __inline close()
	{
		mp3Assert(waveOutReset(mHandleWaveOut));
		mp3Assert(waveOutClose(mHandleWaveOut));
		mp3Assert(LocalFree(mSoundBuffer));
	}

	/// @brief       get the total duration of audio 
	///
	/// @param [out] the music duration in seconds
	double __inline getDuration()
	{
		return mDurationInSecond;
	}

	/// @brief       get the current position from the playback
	///
	/// @param [out] current position from the sound playback (used from sync)
	double getPosition() {
		static MMTIME MMTime = { TIME_SAMPLES, 0 };
		waveOutGetPosition(mHandleWaveOut, &MMTime, sizeof(MMTIME));
		return ((double)MMTime.u.sample) / (44100.0);
	}

	/// @brief       play the previously opened mp3
	void play()
	{
		static WAVEHDR waveHDR = { (LPSTR)mSoundBuffer, mBufferLength };

		// Define output format
		static WAVEFORMATEX pcmFormat = {
			WAVE_FORMAT_PCM, // format type
			2,               // number of channels (i.e. mono, stereo...)
			44100,           // sample rate
			4 * 44100,       // for buffer estimation
			4,               // block size of data
			16,              // number of bits per sample of mono data
			0,               // the count in bytes of the size of
		};

		mp3Assert(waveOutOpen(&mHandleWaveOut, WAVE_MAPPER, &pcmFormat, NULL, 0, CALLBACK_NULL));
		mp3Assert(waveOutPrepareHeader(mHandleWaveOut, &waveHDR, sizeof(waveHDR)));
		mp3Assert(waveOutWrite(mHandleWaveOut, &waveHDR, sizeof(waveHDR)));
	}
};

#pragma function(memset, memcpy, memcmp)







