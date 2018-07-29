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

// Maple Controller Data
typedef struct packed {
    reg a;           // 11
    reg b;           // 10
    reg x;           // 09
    reg y;           // 08

    reg up;          // 07
    reg down;        // 06
    reg left;        // 05
    reg right;       // 04

    reg start;       // 03
    reg ltrigger;    // 02
    reg rtrigger;    // 01

    // meta
    reg trigger_osd; // 00
} ControllerData;
