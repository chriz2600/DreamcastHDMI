// DEBUG Information
typedef struct packed {
    reg [7:0] pll_errors;
    reg [7:0] test;

    reg [9:0] frame_counter;
    reg [7:0] pll_status;
    reg [7:0] id_check_high;
    reg [7:0] id_check_low;
    reg [7:0] chip_revision;
    reg [7:0] vic_detected;
    reg [7:0] vic_to_rx;
    reg [7:0] misc_data;

    reg [7:0] cts1_status;
    reg [7:0] cts2_status;
    reg [7:0] cts3_status;

    reg [7:0] max_cts1_status;
    reg [7:0] max_cts2_status;
    reg [7:0] max_cts3_status;

    reg [7:0] summary_cts1_status;
    reg [7:0] summary_cts2_status;
    reg [7:0] summary_cts3_status;
    reg [7:0] summary_summary_cts3_status;

    reg [7:0] restart_count;
} DebugData;

