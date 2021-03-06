#include "~/gitRepos/night/sandbox/tests/testHeader.hpp"

/*
/* Testing " '
*/

#define BOXIFY(x)                               \
  {                                             \
    if ((x) >= 0.5)                             \
      x -= 1.0;                                 \
    else if ((x) < -0.5)                        \
      x += 1.0;                                 \
  }

typedef int blah234;

// 0.9899*sqrt(8.0*log(10.0))/(PI*freq);
const tFloat hat_t0 = 1.3523661426929/freq; /* Testing 3 */
const tFloat &hat_t1 = hat_t0;

occaFunction tFloat dummyFunction(shared tFloat t){
  return 0;
}

occaFunction tFloat hatWavelet(tFloat t);
occaFunction tFloat hatWavelet(tFloat t){
  const tFloat pift  = PI*freq*(t - hat_t0);
  const tFloat pift2 = pift*pift;

  return (1.0 - 2.0*pift2)*exp(-pift2);
}

const int2 * const a34;

#if 1
occaKernel void fd2d(tFloat *u1,
                     tFloat *u2,
                     tFloat *u3,
                     const texture tFloat tex1[],
                     texture tFloat tex2[][],
                     texture tFloat **tex3,
                     const tFloat currentTime){

  const int bDimX = 16;
  const int bDimY = 16 + bDimX;
  const int bDimZ = 16 + bDimY;

  const int lDimX = 16 + bDimY;
  int lDimY = lDimX;
  int lDimZ = lDimX + lDimY;

  double2 s[2];

  BOXIFY(s[i].x);

  /*
    for(int n = 0; n < bDimX; ++n; tile(lDimX)){
    }

    for(int2 n = 0; n < bDimX; ++n; tile(lDimX, lDimY)){
    }

    for(int2 n(0,0); n < int2(bDimX, bDimY); n += int2(1,1); tile(lDimX,lDimY)){
    }

    for(int3 n(0,0,0);
    n < int3(bDimX, bDimY, bDimZ);
    n += int3(lDimX, lDimY, lDimZ);
    tile(lDimX,lDimY,lDimZ)){
    }
  */

  for(int by = 0; by < bDimY; by += 16; outer0){
    for(int bx = 0; bx < bDimX; bx += 16; outer1){
      shared tFloat Lu[bDimY + 2*sr][bDimX + 2*sr];
      exclusive tFloat r_u2 = 2, r_u3 = 3, r_u4[3], *r_u5, *r_u6[3];

      for(int ly = by; ly < (by + lDimY); ++ly; inner1){
        for(int lx = bx; lx < (by + lDimX); ++lx; inner0){
          const int tx = bx * lDimX + lx;
          const int ty = by * lDimY + ly;

          float2 sj, si;
          float2 s = sj - si;

          Lu[0] = WR_MIN(Lu[tx], Lu[tx+512]);

          int y1, y2;

          {
            y1 = y2 = 0;
          }

          int tmpin = *u1;

          switch(y1){
          case 0 : printf("0\n"); break;
          case 1 : {printf("1\n");}
          default: printf("default\n"); break;
          }

          switch(y2)
          case 0: printf("0\n");

          const int id = ty*w + tx;

          float *__u1 = &u1[bDimX];
          float *__u2 = (float*) (&(u1[bDimX]));

          float data = tex1[0][0];
          tex1[0][0] = data;

          r_u2 = u2[id];
          r_u3 = u3[id];

          const int nX1 = (tx - sr + w) % w;
          const int nY1 = (ty - sr + h) % h;

          const int nX2 = (tx + bDimX - sr + w) % w;
          const int nY2 = (ty + bDimY - sr + h) % h;

          Lu[ly][lx] = u2[nY1*w + nX1];

          if(lx < 2*sr){
            Lu[ly][lx + bDimX] = u2[nY1*w + nX2];

            if(ly < 2*sr)
              Lu[ly + bDimY][lx + bDimX] = u2[nY2*w + nX2];
          }

          if(ly < 2*sr)
            Lu[ly + bDimY][lx] = u2[nY2*w + nX1];

          a.b = 3;
        }
      }

      // barrier(localMemFence);

      for(int ly = 0; ly < lDimY; ++ly; inner1){
        for(int lx = 0; lx < lDimX; ++lx; inner0){
          const int tx = bx * lDimX + lx;
          const int ty = by * lDimY + ly;

          const int id = ty*w + tx;

          tFloat lap = 0.0;

          if(true)
            blah;
          else if(true)
            blah;
          else
            blah;

          for(int i = 0; i < (2*sr + 1); i++){
            lap += xStencil[i]*Lu[ly + sr][lx + i] + xStencil[i]*Lu[ly + i][lx + sr];
            if(i < 2)
              continue;
            break;
          }

          continue;

          for(int i = 0; i < (2*sr + 1); i++){
            lap += xStencil[i]*Lu[ly + sr][lx + i] + xStencil[i]*Lu[ly + i][lx + sr];
          }

          const tFloat u_n1 = (-tStencil[1]*r_u2 - tStencil[2]*r_u3 + lap)/tStencil[0];

          if((tx == mX) && (ty == mY))
            u1[id] = u_n1 + hatWavelet(currentTime)/tStencil[0];
          else
            u1[id] = u_n1;
        }
      }
    }
  }
}
#endif
