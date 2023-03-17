#ifndef SOBEL_FILTER_H_
#define SOBEL_FILTER_H_
#include <systemc>
using namespace sc_core;

#include "filter_def.h"

class SobelFilter : public sc_module {
public:
  sc_in_clk i_clk;
  sc_in<bool> i_rst;
  sc_fifo_in<unsigned char> i_mean_r;
  sc_fifo_in<unsigned char> i_mean_g;
  sc_fifo_in<unsigned char> i_mean_b;
  sc_fifo_in<unsigned char> i_med_r;
  sc_fifo_in<unsigned char> i_med_g;
  sc_fifo_in<unsigned char> i_med_b;

  sc_fifo_out<int> o_mean_result_r;
  sc_fifo_out<int> o_mean_result_b;
  sc_fifo_out<int> o_mean_result_g;
  sc_fifo_out<int> o_med_result_r;
  sc_fifo_out<int> o_med_result_b;
  sc_fifo_out<int> o_med_result_g;




  SC_HAS_PROCESS(SobelFilter);
  SobelFilter(sc_module_name n);
  ~SobelFilter() = default;

private:
  void do_mean_filter();
  void do_med_filter();
  int val_mean_r,val_mean_g,val_mean_b;
  int val_med_r,val_med_g,val_med_b;

};
#endif
