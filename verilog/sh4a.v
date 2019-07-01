`include "sh4a_alu.v"
`include "sh4a_regfile.v"

module sh4a(
    input clk,
    input reset
);

wire [31:0] program_counter;
wire [31:0] status_register;

wire [15:0] instruction_register;

wire [5:0] alu_op;

wire [5:0] idx_read0;
wire [31:0] reg_read0;
wire [5:0] idx_read1;
wire [31:0] reg_read1;
wire [5:0] idx_write;
wire [31:0] reg_write;
wire reg_write_enable;

sha4a_decode decode(
    .clk(clk),
    .reset(reset),

    .insn(instruction_register),


sh4a_regfile regfile(
    .clk(clk),
    .reset(reset),
    
    .program_counter(program_counter),
    
    .idx_read0(idx_read0),
    .idx_read1(idx_read1),
    .idx_write(idx_write),

    .reg_read0(reg_read0),
    .reg_read1(reg_read1),
    .reg_write(reg_write),

    .reg_write_enable(reg_write_enable)
);

sh4a_alu alu(
    .reset(reset),
    .clk(clk),

    .op(alu_op),
    .src1(reg_read0),
    .src2(reg_read1),
    .dest(reg_write),

    .status(status_register)
);

endmodule
