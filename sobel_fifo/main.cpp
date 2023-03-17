#include <iostream>
#include <string>
using namespace std;

// Wall Clock Time Measurement
#include <sys/time.h>

#include "SobelFilter.h"
#include "Testbench.h"

// TIMEVAL STRUCT IS Defined ctime
// use start_time and end_time variables to capture
// start of simulation and end of simulation
struct timeval start_time, end_time;

// int main(int argc, char *argv[])
int sc_main(int argc, char **argv) {
  if ((argc < 3) || (argc > 4)) {
    cout << "No arguments for the executable : " << argv[0] << endl;
    cout << "Usage : >" << argv[0] << " in_image_file_name out_image_file_name"
         << endl;
    return 0;
  }

  //Create modules and signals
  Testbench tb("tb");
  SobelFilter sobel_filter("sobel_filter");
  sc_clock clk("clk", CLOCK_PERIOD, SC_NS);
  sc_signal<bool> rst("rst");

  //Create FIFO channels
  sc_fifo<unsigned char> mean_r;
  sc_fifo<unsigned char> mean_g;
  sc_fifo<unsigned char> mean_b;
  sc_fifo<unsigned char> med_r;
  sc_fifo<unsigned char> med_g;
  sc_fifo<unsigned char> med_b;
  sc_fifo<int> mean_result_r;
  sc_fifo<int> mean_result_g;
  sc_fifo<int> mean_result_b;
  sc_fifo<int> med_result_r;
  sc_fifo<int> med_result_g;
  sc_fifo<int> med_result_b;



  //Connect FIFO channels with modules
  tb.i_clk(clk);
  tb.o_rst(rst);
  sobel_filter.i_clk(clk);
  sobel_filter.i_rst(rst);
  tb.o_mean_r(mean_r);
  tb.o_mean_g(mean_g);
  tb.o_mean_b(mean_b);
  tb.o_med_r(med_r);
  tb.o_med_g(med_g);
  tb.o_med_b(med_b);

  
  tb.i_mean_result_r(mean_result_r);
  tb.i_mean_result_g(mean_result_g);
  tb.i_mean_result_b(mean_result_b);
  tb.i_med_result_r(med_result_r);
  tb.i_med_result_g(med_result_g);
  tb.i_med_result_b(med_result_b);

  sobel_filter.i_mean_r(mean_r);
  sobel_filter.i_mean_g(mean_g);
  sobel_filter.i_mean_b(mean_b);
  sobel_filter.i_med_r(med_r);
  sobel_filter.i_med_g(med_g);
  sobel_filter.i_med_b(med_b);

  sobel_filter.o_mean_result_r(mean_result_r);
  sobel_filter.o_mean_result_g(mean_result_g);
  sobel_filter.o_mean_result_b(mean_result_b);
  sobel_filter.o_med_result_r(med_result_r);
  sobel_filter.o_med_result_g(med_result_g);
  sobel_filter.o_med_result_b(med_result_b);

  tb.read_bmp(argv[1]);
  sc_start();
  std::cout << "Simulated time == " << sc_core::sc_time_stamp() << std::endl;
  tb.write_bmp(argv[2]);

  return 0;
}
