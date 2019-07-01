module sh4a_decode(
    input clk,
    input reset,
    input [15:0] insn,
    output reg insn_valid,
    output reg insn_privileged,
    output reg src1_valid,
    output reg [5:0] src1_reg,
    output reg src2_valid,
    output reg [5:0] src2_reg,
    output reg dest_valid,
    output reg [5:0] dest_reg,
    output reg imm_valid,
    output reg [31:0] imm,
    output reg [5:0] op
);

`include "sh4a_op.vh"
`include "sh4a_registers.vh"

wire [5:0] RM = {2'b0, insn[7:4]};
wire [5:0] RN = {2'b0, insn[11:8]};

localparam STATE_DECODE = 2'b01;
localparam STATE_DECTEST = 2'b10;

reg [1:0] state;

always @(posedge clk) begin
    {insn_valid, insn_privileged, src1_valid, src2_valid, dest_valid, imm_valid} <= 6'b0;
    op <= ILLEGAL;
    
    if (reset) begin
        state <= STATE_DECODE;
    end 
    
    else if (state == STATE_DECODE) begin
        casez (insn)
            16'h0??7: begin // mul.l Rm, Rn
                {insn_valid, src1_valid, src2_valid} <= 3'b111;
                src1_reg <= RM;
                src2_reg <= RN;
                op <= MULTIPLY;
            end
            16'h0009: begin // nop - implemented as add zero, zero -> zero
                {insn_valid, src1_valid, src2_valid, dest_valid} <= 4'b1111;
                src1_reg <= REG_CONST_0;
                src2_reg <= REG_CONST_0;
                dest_reg <= REG_CONST_0;
                op <= ADD;
            end
            16'h0?1A: begin // sts MACL, Rn - implemented as add MACL, zero -> Rn
                {insn_valid, src1_valid, src2_valid, dest_valid} <= 4'b1111;
                src1_reg <= REG_MACL;
                src2_reg <= REG_CONST_0;
                dest_reg <= RN;
                op <= ADD;
            end
            16'h0?5A: begin // sts FPUL, Rn - implemented as add FPUL, zero -> Rn
                {insn_valid, src1_valid, src2_valid, dest_valid} <= 4'b1111;
                src1_reg <= REG_FPUL;
                src2_reg <= REG_CONST_0;
                dest_reg <= RN;
                op <= ADD;
            end
            16'h2??0: begin // mov.b Rm, @Rn
                {insn_valid, src1_valid, src2_valid} <= 3'b111;
                src1_reg <= RM;
                src2_reg <= RN;
                op <= STORE8;
            end
            16'h2??1: begin // mov.w Rm, @Rn
                {insn_valid, src1_valid, src2_valid} <= 3'b111;
                src1_reg <= RM;
                src2_reg <= RN;
                op <= STORE16;
            end
            16'h2??2: begin // mov.l Rm, @Rn
                {insn_valid, src1_valid, src2_valid} <= 3'b111;
                src1_reg <= RM;
                src2_reg <= RN;
                op <= STORE32;
            end
            16'h2??9: begin // and Rm, Rn
                {insn_valid, src1_valid, src2_valid, dest_valid} <= 4'b1111;
                src1_reg <= RN;
                src2_reg <= RM;
                dest_reg <= RN;
                op <= AND;
            end
            16'h2??A: begin // xor Rm, Rn
                {insn_valid, src1_valid, src2_valid, dest_valid} <= 4'b1111;
                src1_reg <= RN;
                src2_reg <= RM;
                dest_reg <= RN;
                op <= XOR;
            end
            16'h3??8: begin // sub Rm, Rn
                {insn_valid, src1_valid, src2_valid, dest_valid} <= 4'b1111;
                src1_reg <= RN;
                src2_reg <= RM;
                dest_reg <= RN;
                op <= SUBTRACT;
            end
            16'h3??C: begin // add Rm, Rn
                {insn_valid, src1_valid, src2_valid, dest_valid} <= 4'b1111;
                src1_reg <= RN;
                src2_reg <= RM;
                dest_reg <= RN;
                op <= ADD;
            end
            16'h4?10: begin // dt Rn
                {insn_valid, src1_valid, src2_valid, dest_valid} <= 4'b1111;
                src1_reg <= RN;
                src2_reg <= REG_CONST_1;
                dest_reg <= RN;
                op <= SUBTRACT;
            end
            16'h4?5A: begin // lds Rn, FPUL - implemented as add FPUL, zero -> Rn
                {insn_valid, src1_valid, src2_valid, dest_valid} <= 4'b1111;
                src1_reg <= REG_FPUL;
                src2_reg <= REG_CONST_0;
                dest_reg <= RN;
                op <= ADD;
            end
            16'h6??B: begin // neg Rm, Rn - implemented as sub zero, Rm -> Rn
                {insn_valid, src1_valid, src2_valid, dest_valid} <= 4'b1111;
                src1_reg <= REG_CONST_0;
                src2_reg <= RM;
                dest_reg <= RN;
                op <= SUBTRACT;
            end
        endcase
    end

`ifdef FORMAL
    // Valid instructions must be legal.
    if (!reset && insn_valid)
        assert(op != ILLEGAL);

    // Invalid instructions must be illegal.
    if (!reset && !insn_valid)
        assert(op == ILLEGAL);
`endif
end

endmodule
