/*
 * WinDrawLib
 * Copyright (c) 2016-2019 Martin Mitas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "misc.h"
#include "backend-d2d.h"
#include "backend-dwrite.h"
#include "backend-wic.h"
#include "backend-gdix.h"
#include "lock.h"


void (*wd_fn_lock)(void) = NULL;
void (*wd_fn_unlock)(void) = NULL;

static DWORD wd_preinit_flags = 0;


void
wdPreInitialize(void (*fnLock)(void), void (*fnUnlock)(void), DWORD dwFlags)
{
    wd_fn_lock = fnLock;
    wd_fn_unlock = fnUnlock;
    wd_preinit_flags = dwFlags;
}


static int
core_init(void)
{
    if(!(wd_preinit_flags & WD_DISABLE_D2D)) {
        if(d2d_init() == 0)
            return 0;
    }

    if(!(wd_preinit_flags & WD_DISABLE_GDIPLUS)) {
        if(gdix_init() == 0)
            return 0;
    }

    return -1;
}

static void
core_fini(void)
{
    if(d2d_enabled())
        d2d_fini();
    else
        gdix_fini();
}

static UINT core_counter = 0;
static UINT wic_counter = 0;
static UINT dwrite_counter = 0;

#define WD_MOD_WIC          0x0001
#define WD_MOD_DWRITE       0x0002

static void wdTerminateImpl(DWORD dwModMask);

BOOL
wdInitialize(DWORD dwFlags)
{
    int err = -1;
    DWORD undo = 0;

    wd_lock();

    /* Core module */
    if(core_counter == 0) {
        if(core_init() != 0)
            goto out;
    }
    core_counter++;

    /* WIC module */
    if(d2d_enabled()  &&  (dwFlags & WD_INIT_IMAGEAPI)) {
        if(wic_counter == 0) {
            if(wic_init() != 0)
                goto out;
        }

        wic_counter++;
        undo |= WD_MOD_WIC;
    }

    /* DWrite module */
    if(d2d_enabled()  &&  (dwFlags & (WD_INIT_STRINGAPI | WD_INIT_TEXTAPI))) {
        if(dwrite_counter == 0) {
            if(dwrite_init() != 0)
                goto out;
        }

        dwrite_counter++;
        undo |= WD_MOD_DWRITE;
    }

    if(gdix_enabled()  &&  (dwFlags & WD_INIT_TEXTAPI)) {
        WD_TRACE("wdInitialize: WD_INIT_TEXTAPI for GDI+ back-end is not (yet?) implemented.");
        goto out;
    }

    /* Success. */
    err = 0;

out:
    /* In case of an error, undo what we have done. */
    if(err != 0  &&  undo != 0)
        wdTerminateImpl(undo);

    wd_unlock();

    return (err == 0);
}

static void
wdTerminateImpl(DWORD dwModMask)
{
    /* Handle the optional modules. */
    if((dwModMask & WD_MOD_WIC)  &&  --wic_counter == 0)
        wic_fini();

    if((dwModMask & WD_MOD_DWRITE)  &&  --dwrite_counter == 0)
        dwrite_fini();

    /* Handle the core module.
     *
     * Note that if the core module counter drops to zero, we make sure no
     * optional module survives. This should only happen when  that caller
     * forgot to pass some flags into wdTerminate() so it does not match
     * those passed previously into wdInitialize().
     */
    if(--core_counter == 0) {
        if(wic_counter > 0) {
            WD_TRACE("wdTerminate: Forcefully terminating WIC module.");
            wic_fini();
            wic_counter = 0;
        }
        if(dwrite_counter > 0) {
            WD_TRACE("wdTerminate: Forcefully terminating DWrite module.");
            dwrite_fini();
            dwrite_counter = 0;
        }

        core_fini();
    }
}

void
wdTerminate(DWORD dwFlags)
{
    DWORD dwModMask = 0;

    if(d2d_enabled()  &&  (dwFlags & WD_INIT_IMAGEAPI))
        dwModMask |= WD_MOD_WIC;
    if(d2d_enabled()  &&  (dwFlags & (WD_INIT_STRINGAPI | WD_INIT_TEXTAPI)))
        dwModMask |= WD_MOD_DWRITE;

    wd_lock();
    wdTerminateImpl(dwModMask);
    wd_unlock();
}

int
wdBackend(void)
{
    if(d2d_enabled())
        return WD_BACKEND_D2D;
    
    if(gdix_enabled())
        return WD_BACKEND_GDIPLUS;
    
    return -1;
}
