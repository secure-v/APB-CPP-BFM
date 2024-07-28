#include <cstdint>
#include<stdint.h>
#include <verilated.h>
#include "verilated_vpi.h"
#include <iostream>
#include "vpi_user.h"
#include <stdio.h>
#include <unistd.h>
#include "verilated_vcd_c.h"
#include "Vapb_demo.h"

#define APB_LOG // print log about apb
#include "apb_mem.hpp"

using namespace std;

Vapb_demo *top;
uint64_t main_time = 0;

int main(int argc, char** argv) {
	Verilated::commandArgs(argc, argv);
	top = new Vapb_demo; 

    uint8_t mem[4096] = {0};
    apb_mem<uint32_t, uint32_t> apb_ports;
    apb_ports.pclk = &top->clk;
    apb_ports.presetn = &top->rstn;
    apb_ports.penable = &top->penable;
    apb_ports.pwrite = &top->pwrite;
    apb_ports.paddr = &top->paddr;
    apb_ports.psel = &top->psel;
    apb_ports.pwdata = &top->pwdata;
    apb_ports.pprot = &top->pprot;
    apb_ports.pstrb = &top->pstrb;
    apb_ports.prdata = &top->prdata;
    apb_ports.pready = &top->pready;
    apb_ports.pslverr = &top->pslverr;

    apb_ports.init(3, 3, 0x80000000, (uint8_t*)mem, NULL, NULL);

	Verilated::traceEverOn(true);            
    VerilatedVcdC* tfp = new VerilatedVcdC();
    top->trace(tfp, 0);                      
    tfp->open("wave.vcd");                   

	while (1) {
 		if (main_time > 10)
 			top->rstn = 1;
		else
			top->rstn = 0;

		if (main_time & 1)
 			top->clk = 1;
		else
			top->clk = 0;

  		top->eval();

		tfp->dump(main_time);                

		if (top->clk) {
			apb_ports.apb_signal_update();
		}

		main_time++;

        if (main_time > 400) 
            break;
  	}

    top->final();
    delete top;

	tfp->close();
}

