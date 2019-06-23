module sh4a_regfile(
    input clk,
    input reset,
   
    output [31:0] program_counter,

    input [4:0] int_idx_read0_pipe0,
    input [4:0] int_idx_read1_pipe0,
    input [4:0] int_idx_write_pipe0,
    input [31:0] int_reg_write_pipe0,
    input int_reg_write_enable_pipe0,
    output reg [31:0] int_reg_read0_pipe0,
    output reg [31:0] int_reg_read1_pipe0,

    input [4:0] int_idx_read0_pipe1,
    input [4:0] int_idx_read1_pipe1,
    input [4:0] int_idx_write_pipe1,
    input [31:0] int_reg_write_pipe1,
    input int_reg_write_enable_pipe1,
    output reg [31:0] int_reg_read0_pipe1,
    output reg [31:0] int_reg_read1_pipe1
);

localparam REG0_BANK0   = 5'd0;
localparam REG1_BANK0   = 5'd1;
localparam REG2_BANK0   = 5'd2;
localparam REG3_BANK0   = 5'd3;
localparam REG4_BANK0   = 5'd4;
localparam REG5_BANK0   = 5'd5;
localparam REG6_BANK0   = 5'd6;
localparam REG7_BANK0   = 5'd7;
localparam REG8         = 5'd8;
localparam REG9         = 5'd9;
localparam REG10        = 5'd10;
localparam REG11        = 5'd11;
localparam REG12        = 5'd12;
localparam REG13        = 5'd13;
localparam REG14        = 5'd14;
localparam REG15        = 5'd15;
localparam REG0_BANK1   = 5'd16;
localparam REG1_BANK1   = 5'd17;
localparam REG2_BANK1   = 5'd18;
localparam REG3_BANK1   = 5'd19;
localparam REG4_BANK1   = 5'd20;
localparam REG5_BANK1   = 5'd21;
localparam REG6_BANK1   = 5'd21;
localparam REG7_BANK1   = 5'd23;

localparam RESET_PC     = 32'hA000_0000;

reg [31:0] program_counter_register;

// http://www.eecg.toronto.edu/~steffan/papers/laforest_xor_fpga12.pdf
// http://fpgacpu.ca/multiport/FPGA2010-LaForest-Slides.pdf
reg [31:0] live_value_table;

reg [31:0] int_registers_pipe0 [0:31];
reg [31:0] int_registers_pipe1 [0:31];

`ifdef FORMAL
// Always start with a high reset line.
initial @($global_clock) begin
    restrict(reset);
end
`endif

always @(posedge clk) begin
`ifdef FORMAL
    // The clock always ticks.
    assume(clk == !$past(clk));

    // We must never access an invalid register.
    assert(int_idx_read0_pipe0 >= 0 && int_idx_read0_pipe0 <= 23);
    assert(int_idx_read1_pipe0 >= 0 && int_idx_read1_pipe0 <= 23);
    assert(!int_reg_write_enable_pipe0 || (int_idx_write_pipe0 >= 0 && int_idx_write_pipe0 <= 23));

    assert(int_idx_read0_pipe1 >= 0 && int_idx_read0_pipe1 <= 23);
    assert(int_idx_read1_pipe1 >= 0 && int_idx_read1_pipe1 <= 23);
    assert(!int_reg_write_enable_pipe1 || (int_idx_write_pipe1 >= 0 && int_idx_write_pipe1 <= 23));

    // Pipes must not try to write to the same register at the same time.
    assert(int_idx_write_pipe0 != int_idx_write_pipe1);
`endif

    if (reset) begin
        program_counter_register <= RESET_PC;
    end else begin
        if (int_reg_write_enable_pipe0) begin
            int_registers_pipe0[int_idx_write_pipe0] <= int_reg_write_pipe0;
            live_value_table[int_idx_write_pipe0] <= 0;
        end

        if (int_reg_write_enable_pipe1) begin
            int_registers_pipe1[int_idx_write_pipe1] <= int_reg_write_pipe1;
            live_value_table[int_idx_write_pipe1] <= 1;
        end

        case (live_value_table[int_idx_read0_pipe0])
            0: int_reg_read0_pipe0 <= int_registers_pipe0[int_idx_read0_pipe0];
            1: int_reg_read0_pipe0 <= int_registers_pipe1[int_idx_read0_pipe0];
        endcase

        case (live_value_table[int_idx_read1_pipe0])
            0: int_reg_read1_pipe0 <= int_registers_pipe0[int_idx_read1_pipe0];
            1: int_reg_read1_pipe0 <= int_registers_pipe1[int_idx_read1_pipe0];
        endcase

        case (live_value_table[int_idx_read0_pipe1])
            0: int_reg_read0_pipe1 <= int_registers_pipe0[int_idx_read0_pipe1];
            1: int_reg_read0_pipe1 <= int_registers_pipe1[int_idx_read0_pipe1];
        endcase

        case (live_value_table[int_idx_read1_pipe1])
            0: int_reg_read1_pipe1 <= int_registers_pipe0[int_idx_read1_pipe1];
            1: int_reg_read1_pipe1 <= int_registers_pipe1[int_idx_read1_pipe1];
        endcase
    end
end

assign program_counter = program_counter_register;

endmodule
