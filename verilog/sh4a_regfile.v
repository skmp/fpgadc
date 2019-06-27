module sh4a_regfile(
    input clk,
    input reset,
   
    output [31:0] program_counter,

    input [5:0] idx_read0,
    input [5:0] idx_read1,
    input [5:0] idx_write,
    input [31:0] reg_write,
    input reg_write_enable,
    output reg [31:0] reg_read0,
    output reg [31:0] reg_read1
);

`include "sh4a_registers.vh"

localparam RESET_PC     = 32'hA000_0000;

reg [31:0] program_counter_register;

reg [31:0] registers [0:63];

`ifdef FORMAL
// Always start with a high reset line.
initial begin
    restrict(reset);
end
`endif

always @(posedge clk) begin
`ifdef FORMAL
    // The clock always ticks.
    assume(clk == !$past(clk));
`endif

    if (reset) begin
        program_counter_register <= RESET_PC;
    end else begin
        if (reg_write_enable && idx_write != REG_CONST_0 && idx_write != REG_CONST_1) begin
            registers[idx_write] <= reg_write;
        end

        case (idx_read0)
            REG_CONST_0: reg_read0 <= 32'd0;
            REG_CONST_1: reg_read1 <= 32'd1;
            default: reg_read0 <= registers[idx_read0];
        endcase

        case (idx_read1)
            REG_CONST_0: reg_read1 <= 32'd0;
            REG_CONST_1: reg_read1 <= 32'd1;
            default: reg_read1 <= registers[idx_read1];
        endcase
    end

`ifdef FORMAL
    // After reset, the program counter should be at the reset PC
    if ($fell(reset))
        assert(program_counter_register == RESET_PC);

    // The zero register should always be zero.
    assert(registers[REG_ZERO] == 'd0);
`endif
end

assign program_counter = program_counter_register;

endmodule
