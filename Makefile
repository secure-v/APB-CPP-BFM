all:
	verilator  --cc --exe --build --trace --vpi -Wno-WIDTHEXPAND -Wno-WIDTHTRUNC apb_mem_demo.cpp apb_demo.v --top-module apb_demo  && ./obj_dir/Vapb_demo > apb_mem.log
	
display: all
	gtkwave wave.vcd &

clean:
	rm -rf obj_dir *.vcd *.log

.PHONY: clean