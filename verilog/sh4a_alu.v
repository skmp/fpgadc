module sh4a_alu(
    input reset,
    input clk,
    input [31:0] src1,
    input [31:0] src2,
    input [5:0] op,
    output reg [31:0] dest,
    output reg [31:0] status
);

`include "sh4a_op.vh"

reg SIGNED_GREATER_THAN = ($signed(src1) > $signed(src2));
reg UNSIGNED_GREATER_THAN = (src1 > src2);
reg EQUAL = (src1 == src2);

reg [31:0] status_reg;

wire T_BIT = status_reg[0];

always @(posedge clk) begin
    if (reset) begin
        // Privileged mode, register bank 1, interrupts masked
        // FPU enabled, everything else zero.
        status_reg <= 32'h7000_00F0;
    end else case (op)
        ADD: begin
            dest <= src1 + src2;
        end
        SUBTRACT: begin
            dest <= src1 - src2;
        end
    endcase

    status <= status_reg;
end 

endmodule
