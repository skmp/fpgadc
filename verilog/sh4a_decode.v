module sh4a_decode(
    input clk,
    input [15:0] insn,
    output reg insn_valid,
    output reg insn_privileged,
    output reg src1_valid,
    output reg [3:0] src1_reg,
    output reg src2_valid,
    output reg [3:0] src2_reg,
    output reg src3_valid,
    output reg [3:0] src3_reg,
    output reg dest_valid,
    output reg [3:0] dest_reg,
    output reg imm_valid,
    output reg [31:0] imm,
    output reg [5:0] op
);

localparam NO_OP = 6'd0;
localparam MULTIPLY = 6'd1;
localparam ILLEGAL = 6'd63;

always @(posedge clk) begin
    {insn_valid, insn_privileged, src1_valid, src2_valid, src3_valid, dest_valid, imm_valid} <= 'b0;
    casez (insn)
        16'h0??7: begin // mul.l Rn, Rm
            {insn_valid, src1_valid, src2_valid} <= 3'b111;
            src1_reg <= insn[11:8];
            src2_reg <= insn[7:4];
            op <= MULTIPLY;
        end
    endcase
end

endmodule
