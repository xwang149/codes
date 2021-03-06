//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014, Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory.
//
// Written by:
//     Nikhil Jain <nikhil.jain@acm.org>
//     Abhinav Bhatele <bhatele@llnl.gov>
//     Peer-Timo Bremer <ptbremer@llnl.gov>
//
// LLNL-CODE-678961. All rights reserved.
//
// This file is part of Damselfly. For details, see:
// https://github.com/scalability-llnl/damselfly
// Please also read the LICENSE file for our notice and the LGPL.
//////////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "stdlib.h"

//Usage ./binary num_groups num_rows num_columns intra_file inter_file

int main(int argc, char **argv) {
  if(argc < 3) {
    printf("Correct usage: %s <num_g> <num_rows> <num_cols> <intra_file> <inter_file>", argv[0]);
    exit(0);
  }

  int g = atoi(argv[1]);
  int r = atoi(argv[2]);
  int c = atoi(argv[3]);

  FILE *intra = fopen(argv[4], "wb");
  FILE *inter = fopen(argv[5], "wb");

  int router = 0;
  int green = 0, black = 1;
  int groups = 0;
  for(int rows = 0; rows < r; rows++) {
    for(int cols = 0; cols < c; cols++) {
      for(int cols1 = 0; cols1 < c; cols1++) {
        if(cols1 != cols) {
          int dest = (rows * c) + cols1;
          fwrite(&router, sizeof(int), 1, intra);
          fwrite(&dest, sizeof(int), 1, intra);
          fwrite(&green, sizeof(int), 1, intra);
          // printf("INTRA %d %d %d\n", router, dest, green);
        }
      }
      for(int rows1 = 0; rows1 < r; rows1++) {
        if(rows1 != rows) {
          int dest = (rows1 * c) + cols;
          for(int link = 0; link < 3; link++) {
            fwrite(&router, sizeof(int), 1, intra);
            fwrite(&dest, sizeof(int), 1, intra);
            fwrite(&black, sizeof(int), 1, intra);
            // printf("INTRA %d %d %d\n", router, dest, black);
          }
        }
      }
      router++;
    }
  }

  for(int srcg = 0; srcg < g; srcg++) {
    for(int dstg = 0; dstg < g; dstg++) {
      if(srcg != dstg) {
        int nsrcg = srcg;
        int ndstg = dstg;
        if(srcg > dstg) {
          nsrcg--;
        } else {
          ndstg--;
        }
        int gsize = 2, gs = 16;
        for(int row = 0; row < r; row++) {
          int srcrB = srcg * r * c + row * c, srcr;
          int dstrB = dstg * r * c + row * c, dstr;
          int srcB = (ndstg % (gs/2)) * 2;
          int dstB = (nsrcg % (gs/2)) * 2;
          srcr = srcrB + srcB;
          dstr = dstrB + dstB;
          for(int r = 0; r < 2; r++) {
            for(int block = 0; block < gsize; block++) {
              fwrite(&srcr, sizeof(int), 1, inter);
              fwrite(&dstr, sizeof(int), 1, inter);
              printf("INTER %d %d\n", srcr, dstr);
            }
            srcr++;
            dstr++;
          }
        }
      }
    }
  }

  fclose(intra);
  fclose(inter);
}
