

#include <conio.h>
#include <windows.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <process.h>
#include "libHidDisplay.h"




unsigned int __stdcall renderListenerThread (void *ptr)
{
	teensyRawHidcxt_t *ctx = (teensyRawHidcxt_t*)ptr;
	winthrd_t *thrd = &ctx->async.thrd;

	const size_t frameSize = ctx->height * ctx->pitch;
	void *fbCopy = (void*)malloc(frameSize);
	if (!fbCopy) goto exittrd;
	
	ctx->async.threadState = 1;
		
	while (!thrd->exiting){
		if (WaitForSingleObject(thrd->hUpdateEvent, 500) == WAIT_OBJECT_0){
			if (thrd->exiting) break;

			memcpy(fbCopy, thrd->sourceBuffer, frameSize);
			if (libHidDisplay_WriteImage(ctx, (void*)fbCopy))
				thrd->fcount++;
		}
	}
	free(fbCopy);
	
exittrd:
	_endthreadex(1);
	return 1;
}

unsigned int renderStartListenerThread (teensyRawHidcxt_t *ctx)
{
	winthrd_t *thrd = &ctx->async.thrd;
	if (thrd->state) return 0;

	memset(thrd, 0, sizeof(*thrd));
	thrd->state = 1;
	thrd->exiting = 0;
	thrd->hUpdateEvent = CreateEvent(NULL, 0, 0, NULL);
	thrd->sourceBuffer = (void*)malloc(ctx->height * ctx->pitch);
	thrd->fcount = 0;

	unsigned int tid = 0;
	thrd->hRenderThread = (HANDLE)_beginthreadex(NULL, 0, renderListenerThread, ctx, 0, &tid);
	Sleep(10);
	return tid;
}

int libHidDisplay_WriteImageAsync (teensyRawHidcxt_t *ctx, void *buffer)
{
	winthrd_t *thrd = &ctx->async.thrd;
	if (!thrd->state){
		if (!renderStartListenerThread(ctx))
			return 0;
	}
	
	memcpy(thrd->sourceBuffer, buffer, ctx->height * ctx->pitch);
	SetEvent(thrd->hUpdateEvent);
	
	return thrd->fcount;
}

void libHidDisplay_WriteImageAsyncStop (teensyRawHidcxt_t *ctx)
{
	winthrd_t *thrd = &ctx->async.thrd;
	
	ctx->async.threadState = 0;
	thrd->exiting = 1;
	SetEvent(thrd->hUpdateEvent);
	ResumeThread(thrd->hRenderThread);
	WaitForSingleObject(thrd->hRenderThread, INFINITE);
	
	CloseHandle(thrd->hRenderThread);
	CloseHandle(thrd->hUpdateEvent);

	if (thrd->sourceBuffer)
		free(thrd->sourceBuffer);
	
	memset(thrd, 0, sizeof(*thrd));
}
