module apb_demo #(parameter  base_addr	= 32'h80000000) (
	input wire             clk    ,
	input wire             rstn   ,
    output wire            penable,
    output                 pwrite ,
    output wire [31:0]     paddr  ,
    output                 psel   ,
    output wire [31:0]     pwdata ,
    output                 pprot  ,
    output wire [3:0]      pstrb  ,
    input wire [31:0]      prdata ,
    input wire             pready ,
    input wire             pslverr
);

    parameter IDLE = 3'b000;
    parameter WRTE = 3'b001; // write
    parameter STOP = 3'b010;
    parameter READ = 3'b011;
    parameter DONE = 3'b100;
    
    reg [2:0] state;

    always @ (posedge clk) begin
        if (!rstn)
            state <= IDLE;
        else if (state == IDLE) 
            state <= WRTE;
        else if ((state == WRTE) && (count == 16))
            state <= STOP;
        else if (state == STOP)
            state <= READ;
        else if ((state == READ) && (count == 16))
            state <= DONE;
        else 
            state <= state;
    end

    reg [4:0] count;

    always @ (posedge clk) begin
        if (!rstn)
            count <= 5'b0;
        else if (state == IDLE)
            count <= 5'b0;
        else if ((state == WRTE) && (pwrite) && (penable) && (pready)) 
            count <= count + 5'b1;
        else if (state == STOP)
            count <= 5'b0;
        else if ((state == READ) && (!pwrite) && (penable) && (pready)) 
            count <= count + 5'b1;
        else
            count <= count;
    end

    reg enable;

    always @ (posedge clk) begin
        if (!rstn)
            enable <= 0;
        else if ((state == IDLE) || (!psel))
            enable <= 0;
        else if ((state == WRTE) && (penable) && (pready)) 
            enable <= 0;
        else if ((state == READ) && (penable) && (pready)) 
            enable <= 0;
        else if ((state == WRTE) && (psel)) 
            enable <= 1;
        else if ((state == READ) && (psel)) 
            enable <= 1;
        else
            enable <= 0;
    end

    assign penable = (enable) && (psel);
    assign pwrite  = ((state == WRTE) && (count < 16))? 1 : 0;
    assign paddr   = (count << 2) + base_addr;
    assign psel    = (state == WRTE) || (state == READ);
    assign pwdata  = (state == WRTE)? ((count << 2) + base_addr) : 0;
    assign pprot   = 0;
    assign pstrb   = 15;

endmodule
