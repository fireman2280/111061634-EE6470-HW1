#include <cmath>

#include "SobelFilter.h"

SobelFilter::SobelFilter(sc_module_name n) : sc_module(n) {
  SC_THREAD(do_med_filter);
  sensitive << i_clk.pos();
  SC_THREAD(do_mean_filter);
  sensitive << i_clk.pos();
  dont_initialize();
  reset_signal_is(i_rst, false);
}

// sobel mask
const int mask[MASK_X][MASK_Y] = {{1, 1, 1}, {1, 2, 1}, {1, 1, 1}};

unsigned int get_med(unsigned char sort[9]){
int i, j, tmp;
unsigned char temp[9]={0,0,0,0,0,0,0,0,0};
for(int i=0;i<9;i++){

 temp[i]=sort[i];

}

for(i = 8; i > 0; i--)
{
    for(j = 0; j <= i-1; j++)
    {
        if( temp[j] > temp[j+1])
        {
            tmp = temp[j];
            temp[j] = temp[j+1];
            temp[j+1] = tmp;
        }
    }
}
return (int)(temp[4]);
}



void SobelFilter::do_med_filter() {
  unsigned char med_buffer_r[(MASK_X*MASK_Y)]={0};  //create buffer
  unsigned char med_buffer_g[(MASK_X*MASK_Y)]={0};
  unsigned char med_buffer_b[(MASK_X*MASK_Y)]={0};
  while (true) {
      for (unsigned int v = 0; v < MASK_Y; ++v) {
                                                         //read data to buffer
            med_buffer_r[v] = i_med_r.read();
            med_buffer_g[v] = i_med_g.read();
            med_buffer_b[v] = i_med_b.read();
        }
      
    int med_r=get_med(med_buffer_r);
    int med_g=get_med(med_buffer_g);
    int med_b=get_med(med_buffer_b);

    o_med_result_r.write(med_r);
    o_med_result_g.write(med_g);
    o_med_result_b.write(med_b);

    for ( int v = 8; v >=3; --v) {            /*shift buffer*/
      med_buffer_r[v]=med_buffer_r[v-3];
      med_buffer_g[v]=med_buffer_g[v-3];
      med_buffer_b[v]=med_buffer_b[v-3];
    }
    wait(4); //emulate module delay
  }
  }



void SobelFilter::do_mean_filter() {
  unsigned char mean_buffer[3][MASK_X][MASK_Y]={{{0,0,0},{0,0,0},{0,0,0}},{{0,0,0},{0,0,0},{0,0,0}},{{0,0,0},{0,0,0},{0,0,0}}}; //create buffer 
  while (true) {
      val_mean_r=0;
      val_mean_g=0;
      val_mean_b=0;
      for (unsigned int v = 0; v < MASK_Y; ++v) {
        for (unsigned int u = 0; u < MASK_X; ++u) {
          if(u==0){                                     //read data to buffer
            mean_buffer[0][0][v] = i_mean_r.read();
            mean_buffer[1][0][v] = i_mean_g.read();
            mean_buffer[2][0][v] = i_mean_b.read();
          }
          val_mean_r +=  mean_buffer[0][u][v] * mask[u][v];
          val_mean_g +=  mean_buffer[1][u][v] * mask[u][v];
          val_mean_b +=  mean_buffer[2][u][v] * mask[u][v];
          
        }
      }  
  
    int mean_result_r =(int)(val_mean_r/(MASK_Y*MASK_X));
    int mean_result_g =(int)(val_mean_r/(MASK_Y*MASK_X));
    int mean_result_b =(int)(val_mean_r/(MASK_Y*MASK_X));
    o_mean_result_r.write(mean_result_r);
    o_mean_result_g.write(mean_result_g);
    o_mean_result_b.write(mean_result_b);

    for (unsigned int v = 0; v < MASK_Y; ++v) {
        for (unsigned int u = (MASK_X-1); u > 0; --u) {      //shift col1->col2 col0->col1
              mean_buffer[0][u][v] = mean_buffer[0][u-1][v] ;
              mean_buffer[1][u][v] = mean_buffer[1][u-1][v];
              mean_buffer[2][u][v] = mean_buffer[2][u-1][v];
        }
    }


    wait(3); //emulate module delay
  }
}
