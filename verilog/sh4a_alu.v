module sh4a_alu(
    input clk,
    input [31:0] src1,
    input [31:0] src2,
    input [5:0] op,
    output reg [31:0] dest,
    output reg t
);

`include "sh4a_op.vh"

reg SIGNED_GREATER_THAN = ($signed(src1) > $signed(src2));
reg UNSIGNED_GREATER_THAN = (src1 > src2);
reg EQUAL = (src1 == src2);

always @(posedge clk) begin
    case (op)
        ADD: begin
            dest <= src1 + src2;
        end
        SUBTRACT: begin
            dest <= src1 - src2;
        end
    endcase
end 

endmodule
