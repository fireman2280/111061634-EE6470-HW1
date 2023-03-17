#include <cassert>
#include <cstdio>
#include <cstdlib>
using namespace std;

#include "Testbench.h"

unsigned char header[54] = {
    0x42,          // identity : B
    0x4d,          // identity : M
    0,    0, 0, 0, // file size
    0,    0,       // reserved1
    0,    0,       // reserved2
    54,   0, 0, 0, // RGB data offset
    40,   0, 0, 0, // struct BITMAPINFOHEADER size
    0,    0, 0, 0, // bmp width
    0,    0, 0, 0, // bmp height
    1,    0,       // planes
    24,   0,       // bit per pixel
    0,    0, 0, 0, // compression
    0,    0, 0, 0, // data size
    0,    0, 0, 0, // h resolution
    0,    0, 0, 0, // v resolution
    0,    0, 0, 0, // used colors
    0,    0, 0, 0  // important colors
};

Testbench::Testbench(sc_module_name n)
    : sc_module(n), output_rgb_raw_data_offset(54) {
  SC_THREAD(do_sobel);
  sensitive << i_clk.pos();
  dont_initialize();
}

int Testbench::read_bmp(string infile_name) {
  FILE *fp_s = NULL; // source file handler
  fp_s = fopen(infile_name.c_str(), "rb");
  if (fp_s == NULL) {
    printf("fopen %s error\n", infile_name.c_str());
    return -1;
  }
  // move offset to 10 to find rgb raw data offset
  fseek(fp_s, 10, SEEK_SET);
  assert(fread(&input_rgb_raw_data_offset, sizeof(unsigned int), 1, fp_s));

  // move offset to 18 to get width & height;
  fseek(fp_s, 18, SEEK_SET);
  assert(fread(&width, sizeof(unsigned int), 1, fp_s));
  assert(fread(&height, sizeof(unsigned int), 1, fp_s));

  // get bit per pixel
  fseek(fp_s, 28, SEEK_SET);
  assert(fread(&bits_per_pixel, sizeof(unsigned short), 1, fp_s));
  bytes_per_pixel = bits_per_pixel / 8;

  // move offset to input_rgb_raw_data_offset to get RGB raw data
  fseek(fp_s, input_rgb_raw_data_offset, SEEK_SET);

  source_bitmap =
      (unsigned char *)malloc((size_t)width * height * bytes_per_pixel);
  if (source_bitmap == NULL) {
    printf("malloc images_s error\n");
    return -1;
  }
  temp_bitmap =
      (unsigned char *)malloc((size_t)width * height * bytes_per_pixel);
  if (source_bitmap == NULL) {
    printf("malloc images_s error\n");
    return -1;
  }


  target_bitmap =
      (unsigned char *)malloc((size_t)width * height * bytes_per_pixel);
  if (target_bitmap == NULL) {
    printf("malloc target_bitmap error\n");
    return -1;
  }

  printf("Image width=%d, height=%d\n", width, height);
  assert(fread(source_bitmap, sizeof(unsigned char),
               (size_t)(long)width * height * bytes_per_pixel, fp_s));
  fclose(fp_s);
  return 0;
}

int Testbench::write_bmp(string outfile_name) {
  FILE *fp_t = NULL;      // target file handler
  unsigned int file_size; // file size

  fp_t = fopen(outfile_name.c_str(), "wb");
  if (fp_t == NULL) {
    printf("fopen %s error\n", outfile_name.c_str());
    return -1;
  }

  // file size
  file_size = width * height * bytes_per_pixel + output_rgb_raw_data_offset;
  header[2] = (unsigned char)(file_size & 0x000000ff);
  header[3] = (file_size >> 8) & 0x000000ff;
  header[4] = (file_size >> 16) & 0x000000ff;
  header[5] = (file_size >> 24) & 0x000000ff;

  // width
  header[18] = width & 0x000000ff;
  header[19] = (width >> 8) & 0x000000ff;
  header[20] = (width >> 16) & 0x000000ff;
  header[21] = (width >> 24) & 0x000000ff;

  // height
  header[22] = height & 0x000000ff;
  header[23] = (height >> 8) & 0x000000ff;
  header[24] = (height >> 16) & 0x000000ff;
  header[25] = (height >> 24) & 0x000000ff;

  // bit per pixel
  header[28] = bits_per_pixel;

  // write header
  fwrite(header, sizeof(unsigned char), output_rgb_raw_data_offset, fp_t);

  // write image
  fwrite(target_bitmap, sizeof(unsigned char),
         (size_t)(long)width * height * bytes_per_pixel, fp_t);

  fclose(fp_t);
  return 0;
}

void Testbench::do_sobel() {
  int x, y, v, u;        // for loop counter
  unsigned char R, G, B; // color of R, G, B
  int adjustX, adjustY, xBound, yBound;
  int v_mean_r,v_mean_g,v_mean_b,v_med_r,v_med_g,v_med_b;
  int temp_x,temp_y;
  o_rst.write(false);
  o_rst.write(true);
  for (y = 0; y != height+1; ++y) {   //pipeline two filter
    temp_y=y-1;
    for (x = 0; x != width+2; ++x) {
      temp_x=x-2;
      adjustX = (MASK_X % 2) ? 1 : 0; // 1
      adjustY = (MASK_Y % 2) ? 1 : 0; // 1
      xBound = MASK_X / 2;            // 1
      yBound = MASK_Y / 2;            // 1

      
          if(x==0){
            for (v = -yBound; v != yBound + adjustY; ++v) {   //-1, 0, 1
            for (u = -xBound; u != xBound + adjustX; ++u) { //-1, 0, 1
            if (x + u >= 0 && x + u < width && y + v >= 0 && y + v < height) {
            R = *(source_bitmap +
                  bytes_per_pixel * (width * (y + u) + (x + v)) + 2);
            G = *(source_bitmap +
                  bytes_per_pixel * (width * (y + u) + (x + v)) + 1);
            B = *(source_bitmap +
                  bytes_per_pixel * (width * (y + u) + (x + v)) + 0);
          } else {
            R = 0;
            G = 0;
            B = 0;
          }
          o_med_r.write(R);
          o_med_g.write(G);
          o_med_b.write(B);
          wait(1); //emulate channel delay
        }
      }
      //if(i_result.num_available()==0) wait(i_result.data_written_event());
      for(int count=0;count<2;count++){
        int error_r = i_med_result_r.read();
        int error_g = i_med_result_g.read();
        int error_b = i_med_result_b.read();
      }
      v_med_r = i_med_result_r.read();
      v_med_g = i_med_result_g.read();
      v_med_b = i_med_result_b.read();
      //cout << "Now at " << sc_time_stamp() << endl; //print current sc_time

      // black
      *(temp_bitmap + bytes_per_pixel * (width * y + x) + 2) = v_med_r;
      *(temp_bitmap + bytes_per_pixel * (width * y + x) + 1) = v_med_g;
      *(temp_bitmap + bytes_per_pixel * (width * y + x) + 0) = v_med_b;
          

      }else{
        for (v = -yBound; v != yBound + adjustY; ++v) { //only add new column
          if (x + 1 >= 0 && x + 1 < width && y + v >= 0 && y + v < height) {
            R = *(source_bitmap +
                  bytes_per_pixel * (width * (y + v) + (x + 1)) + 2);
            G = *(source_bitmap +
                  bytes_per_pixel * (width * (y + v) + (x + 1)) + 1);
            B = *(source_bitmap +
                  bytes_per_pixel * (width * (y + v) + (x + 1)) + 0);
          } else {
            R = 0;
            G = 0;
            B = 0;
          }
        
          o_med_r.write(R);
          o_med_g.write(G);
          o_med_b.write(B);
          wait(1); //emulate channel delay

      }
      //if(i_result.num_available()==0) wait(i_result.data_written_event());
      v_med_r = i_med_result_r.read();
      v_med_g = i_med_result_g.read();
      v_med_b = i_med_result_b.read();
      //cout << "Now at " << sc_time_stamp() << endl; //print current sc_time

        // black
      *(temp_bitmap + bytes_per_pixel * (width * y + x) + 2) = v_med_r;
      *(temp_bitmap + bytes_per_pixel * (width * y + x) + 1) = v_med_g;
      *(temp_bitmap + bytes_per_pixel * (width * y + x) + 0) = v_med_b;
      

      }
      if((temp_x>=0)&&(temp_y>=0)){                                  //mean filter start
        if(temp_x==0){
          for (v = -yBound; v != yBound + adjustY; ++v) {   //-1, 0, 1
          for (u = -xBound; u != xBound + adjustX; ++u) { //-1, 0, 1
          if (temp_x + u >= 0 && temp_x + u < width && temp_y + v >= 0 && temp_y + v < height) {
          R = *(temp_bitmap +
                  bytes_per_pixel * (width * (temp_y + v) + (temp_x + u)) + 2);
          G = *(temp_bitmap +
                  bytes_per_pixel * (width * (temp_y + v) + (temp_x + u)) + 1);
          B = *(temp_bitmap +
                  bytes_per_pixel * (width * (temp_y + v) + (temp_x + u)) + 0);
          } else {
          R = 0;
          G = 0;
          B = 0;
          }
        
          o_mean_r.write(R);
          o_mean_g.write(G);
          o_mean_b.write(B);
          wait(1); //emulate channel delay
        }
      }
      //if(i_result.num_available()==0) wait(i_result.data_written_event());
      for(int count=0;count<2;count++){
        int wrong_r = i_mean_result_r.read();
        int wrong_g = i_mean_result_g.read();
        int wrong_b = i_mean_result_b.read();
      }
      v_mean_r = i_mean_result_r.read();
      v_mean_g = i_mean_result_g.read();
      v_mean_b = i_mean_result_b.read();
      //cout << "Now at " << sc_time_stamp() << endl; //print current sc_time

      // black
      *(temp_bitmap + bytes_per_pixel * (width * temp_y + temp_x) + 2) = v_mean_r;
      *(temp_bitmap + bytes_per_pixel * (width * temp_y + temp_x) + 1) = v_mean_g;
      *(temp_bitmap + bytes_per_pixel * (width * temp_y + temp_x) + 0) = v_mean_b;
      

      }else{
        for (v = -yBound; v != yBound + adjustY; ++v) { //only add new column
          if (temp_x + 1 >= 0 && temp_x + 1 < width && temp_y + v >= 0 && temp_y + v < height) {
            R = *(temp_bitmap +
                  bytes_per_pixel * (width * (temp_y + v) + (temp_x + 1)) + 2);
            G = *(temp_bitmap +
                  bytes_per_pixel * (width * (temp_y + v) + (temp_x + 1)) + 1);
            B = *(temp_bitmap +
                  bytes_per_pixel * (width * (temp_y + v) + (temp_x + 1)) + 0);
          } else {
            R = 0;
            G = 0;
            B = 0;
          }
        
          o_mean_r.write(R);
          o_mean_g.write(G);
          o_mean_b.write(B);
          wait(1); //emulate channel delay

      }
      //if(i_result.num_available()==0) wait(i_result.data_written_event());
      v_mean_r = i_mean_result_r.read();
      v_mean_g = i_mean_result_g.read();
      v_mean_b = i_mean_result_b.read();
      //cout << "Now at " << sc_time_stamp() << endl; //print current sc_time

        // black
      *(target_bitmap + bytes_per_pixel * (width * temp_y + temp_x) + 2) = v_mean_r;
      *(target_bitmap + bytes_per_pixel * (width * temp_y + temp_x) + 1) = v_mean_g;
      *(target_bitmap + bytes_per_pixel * (width * temp_y + temp_x) + 0) = v_mean_b;
      }
         
      }
    }
    }
  
  sc_stop();
}
