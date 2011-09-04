/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <assert.h>
#include <malloc.h>
#include <string.h>

#include "uv.h"
#include "internal.h"


int uv_utf16_to_utf8(const wchar_t* utf16Buffer, size_t utf16Size,
    char* utf8Buffer, size_t utf8Size) {
  return WideCharToMultiByte(CP_UTF8,
                             0,
                             utf16Buffer,
                             utf16Size,
                             utf8Buffer,
                             utf8Size,
                             NULL,
                             NULL);
}


int uv_utf8_to_utf16(const char* utf8Buffer, wchar_t* utf16Buffer,
    size_t utf16Size) {
  return MultiByteToWideChar(CP_UTF8,
                             0,
                             utf8Buffer,
                             -1,
                             utf16Buffer,
                             utf16Size);
}


int uv_exepath(char* buffer, size_t* size) {
  int retVal;
  size_t utf16Size;
  wchar_t* utf16Buffer;

  if (!buffer || !size) {
    return -1;
  }

  utf16Buffer = (wchar_t*)malloc(sizeof(wchar_t) * *size);
  if (!utf16Buffer) {
    retVal = -1;
    goto done;
  }

  /* Get the path as UTF-16 */
  utf16Size = GetModuleFileNameW(NULL, utf16Buffer, *size - 1);
  if (utf16Size <= 0) {
    /* uv_set_sys_error(loop, GetLastError()); */
    retVal = -1;
    goto done;
  }

  utf16Buffer[utf16Size] = L'\0';

  /* Convert to UTF-8 */
  *size = uv_utf16_to_utf8(utf16Buffer, utf16Size, buffer, *size);
  if (!*size) {
    /* uv_set_sys_error(loop, GetLastError()); */
    retVal = -1;
    goto done;
  }

  buffer[*size] = '\0';
  retVal = 0;

done:
  if (utf16Buffer) {
    free(utf16Buffer);
  }

  return retVal;
}
