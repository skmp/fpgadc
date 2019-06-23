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
    output reg [31:0] imm
);

always @(posedge clk) begin
    {insn_valid, insn_privileged, src1_valid, src2_valid, src3_valid, dest_valid, imm_valid} <= 'b0;
    casez (insn)
        16'h0?02, 16'h0?12, 16'h0?22, 16'h0?32, 16'h0?42: begin // STC {SR, GBR, VBR, SSR, SGR}, Rn
            {insn_valid, dest_valid} <= 2'h3;
            dest_reg <= insn[11:8];
        end
        16'h0?02, 16'h0?03: begin // BSRF Rn
            {insn_valid, src1_valid} <= 2'h3;
            src1_reg <= insn[11:8];
        end
        16'h0?23: begin // BRAF Rn
            {insn_valid, src1_valid} <= 2'h3;
            src1_reg <= insn[11:8];
        end
        16'h0?63: begin // MOVLI.L @Rm, R0
            {insn_valid, src1_valid, dest_valid} <= 3'h7;
            src1_reg <= insn[11:8];
            dest_reg <= 4'd0;
        end
        16'h0?73: begin // MOVCO.L R0, @Rn
            {insn_valid, src1_valid, src2_valid} <= 3'h7;
            src1_reg <= 4'd0;
            src2_reg <= insn[11:8];
        end
        // {PREF, OCBI, OCBP, OCBWB, PREFI, ICBI} @Rn
        16'h0?83, 16'h0?93, 16'h0?A3, 16'h0?B3, 16'h0?D3, 16'h0?E3: begin 
            {insn_valid, src1_valid} <= 2'h3;
            src1_reg <= insn[11:8];
        end
        16'h0??4, 16'h0??5, 16'h0??6: begin // MOV.[BWL] Rm, @(R0, Rn)
            {insn_valid, src1_valid, src2_valid, src3_valid} <= 4'hF;
            src1_reg <= insn[7:4];
            src2_reg <= insn[11:8];
            src3_reg <= 4'd0;
        end
        16'h0??7: begin // MUL.L Rm, Rn
            {insn_valid, src1_valid, src2_valid} <= 3'h7;
            src1_reg <= insn[7:4];
            src2_reg <= insn[11:8];
        end
        16'h0008, 16'h0018: begin // {CLRT, SETT}
            {insn_valid} <= 1'h1;
        end
        16'h0028: begin // CLRMAC
            {insn_valid} <= 1'h1;
        end
        16'h0038: begin // LDTLB
            {insn_valid} <= 1'h1;
        end
        16'h0048, 16'h0058: begin // {CLRS, SETS}
            {insn_valid} <= 1'h1;
        end
        16'h0009: begin // NOP
            {insn_valid} <= 1'h1;
        end
        16'h0019: begin // DIV0U
            {insn_valid} <= 1'h1;
        end
        16'h0?29: begin // MOVT Rn
            {insn_valid, dest_valid} <= 2'h3;
            dest_reg <= insn[11:8];
        end
        16'h000B: begin // RTS
            {insn_valid} <= 1'h1;
        end
        16'h001B: begin // SLEEP
            {insn_valid} <= 1'h1;
        end
        16'h002B: begin // RTE
            {insn_valid} <= 1'h1;
        end
        16'h00AB: begin // SYNCO
            {insn_valid} <= 1'h1;
        end
        16'h0??C, 16'h0??D, 16'h0??E: begin // MOV.[BWL] @(R0, Rm), Rn
            {insn_valid, src1_valid, src2_valid, dest_valid} <= 4'hF;
            src1_reg <= 4'd0;
            src2_reg <= insn[7:4];
            dest_reg <= insn[11:8];
        end
        16'h0??F: begin // MAC.L @Rm+, @Rn+
            {insn_valid, src1_valid, src2_valid} <= 3'h7;
            src1_reg <= insn[7:4];
            src2_reg <= insn[11:8];
        end
        16'h1???: begin // MOV.L Rm, @(disp*, Rn)
            {insn_valid, src1_valid, src2_valid, imm_valid} <= 4'hF;
            src1_reg <= insn[7:4];
            src2_reg <= insn[11:8];
            imm <= {26'b0, insn[3:0], 2'b0};
        end
        16'h2??0, 16'h2??1, 16'h2??2: begin // MOV.[BWL] Rm, @Rn
            {insn_valid, src1_valid, src2_valid} <= 3'h7;
            src1_reg <= insn[7:4];
            src2_reg <= insn[11:8];
        end
        16'h2??4, 16'h2??5, 16'h2??6: begin // MOV.[BWL] Rm, @-Rn
            {insn_valid, src1_valid, src2_valid} <= 3'h7;
            src1_reg <= insn[7:4];
            src2_reg <= insn[11:8];
        end
        16'h2??D: begin // XTRCT Rm, Rn
            {insn_valid, src1_valid, src2_valid, dest_valid} <= 4'hF;
            src1_reg <= insn[7:4];
            src2_reg <= insn[11:8];
            dest_reg <= insn[11:8];
        end
        16'hE???: begin // MOV #imm, Rn
            {insn_valid, dest_valid, imm_valid} <= 3'h7;
            dest_reg <= insn[11:8];
            imm <= {{24{insn[7]}}, insn[7:0]};
        end
    endcase
end

endmodule
