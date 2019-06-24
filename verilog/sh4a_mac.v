module sh4a_mac(
    input clk,
    input [31:0] mul_src1,
    input [31:0] mul_src2,
    input [63:0] add_src1,
    input [63:0] add_src2,
    output [63:0] result
);

reg [63:0] add_src1_latch;
reg [63:0] add_src2_latch;
reg [63:0] result_stage1;

always @(posedge clk) begin
    result_stage1 <= (mul_src1 * mul_src2);
    add_src1_latch <= add_src1;
    add_src2_latch <= add_src2;
end

always @(negedge clk) begin
    result <= result_stage1 + add_src1_latch + add_src2_latch;
end

endmodule
