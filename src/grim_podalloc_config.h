/*
 * Flutter
 * Lightweight LUA powered HTTP server
 *
 * Copyright (C) 2015 William Whitacre
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
 * IN THE SOFTWARE. */

#ifndef GRIM_PODALLOC_CONFIG_H
#define GRIM_PODALLOC_CONFIG_H

/* verbosity - level 0 : do not print report         *
 *             level 1 : print report on cleanup     *
 *             level 2 : print report on new record  *
 *             level 3 : print report on alloc/free  */
#ifndef GRIM_PODALLOC_VERBOSITY
#define GRIM_PODALLOC_VERBOSITY    0
#endif

/* the output file used for the reports. defaults to stderr.  *
 * not used if verbosity level is unspecified or set to zero. */
#ifndef GRIM_PODALLOC_OUTFILE
#define GRIM_PODALLOC_OUTFILE      stderr
#endif

/* the ratio of alloc/free needed to precisely double new records in size.   *
 * that is to say, if I am allocating a lot but not freeing so much, the     *
 * trend is that the user is going to want more and more memory, so we       *
 * should increase the record size for new records to accomodate this with   *
 * fewer system allocations and simultaneously improve locality of           *
 * reference. note that if there have not been any frees yet, then we double *
 * the record size every time the existing blocks are depleted.              */

/*                                    allocs / frees                         */
#ifndef GRIM_PODALLOC_GROWTH_HEURISTIC
#define GRIM_PODALLOC_GROWTH_HEURISTIC ( 3.0 / 1.0 )
#endif

#endif

