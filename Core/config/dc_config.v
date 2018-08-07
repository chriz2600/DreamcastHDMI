`define ICS664_SETTINGS_P    
`define ICS664_SETTINGS_I    

`define H_CAPTURE_START_P     
`define H_CAPTURE_END_P      
`define V_CAPTURE_START_P      
`define V_CAPTURE_END_P      

`define H_CAPTURE_START_I      
`define H_CAPTURE_END_I      
`define V_CAPTURE_START_I      
`define V_CAPTURE_END_I      

`define VERTICAL_LINE_WIDTH      

`define BUFFER_SIZE
`define RAM_NUMWORDS
`define TRIGGER_ADDR

`define BUFFER_LINE_LENGTH 15'd`VERTICAL_LINE_WIDTH

const DCVideoConfig DC_VIDEO_CONFIG_1080P = {
    4'b1101,    // ICS644_settings_p
    4'b0011,    // ICS644_settings_i

    10'd44,     // p_horizontal_capture_start
    10'd684,    // p_horizontal_capture_end
    10'd0,      // p_vertical_capture_start
    10'd480,    // p_vertical_capture_end

    10'd1,      // i_horizontal_capture_start
    10'd641,    // i_horizontal_capture_end
    10'd0,      // i_vertical_capture_start
    10'd504,    // i_vertical_capture_end

    8'd33,      // buffer_size
    15'd21120,  // ram_numwords
    15'd20920,  // trigger_address (ram_numwords - 200)
    15'd640     // buffer_line_length
};

const DCVideoConfig DC_VIDEO_CONFIG_960P = {
    4'b1101,    // ICS644_settings_p
    4'b0011,    // ICS644_settings_i

    10'd44,     // p_horizontal_capture_start
    10'd684,    // p_horizontal_capture_end
    10'd0,      // p_vertical_capture_start
    10'd480,    // p_vertical_capture_end

    10'd1,      // i_horizontal_capture_start
    10'd641,    // i_horizontal_capture_end
    10'd0,      // i_vertical_capture_start
    10'd504,    // i_vertical_capture_end

    8'd24,      // buffer_size
    15'd15360,  // ram_numwords
    15'd440,    // trigger_address
    15'd640     // buffer_line_length
};

const DCVideoConfig DC_VIDEO_CONFIG_480P = {
    4'b1100,    // ICS644_settings_p
    4'b0010,    // ICS644_settings_i

    10'd0,      // p_horizontal_capture_start
    10'd720,    // p_horizontal_capture_end
    10'd0,      // p_vertical_capture_start
    10'd480,    // p_vertical_capture_end

    10'd0,      // i_horizontal_capture_start
    10'd720,    // i_horizontal_capture_end
    10'd0,      // i_vertical_capture_start
    10'd504,    // i_vertical_capture_end

    8'd1,       // buffer_size
    15'd720,    // ram_numwords
    15'd360,    // trigger_address
    15'd720     // buffer_line_length
};

const DCVideoConfig DC_VIDEO_CONFIG_VGA = {
    4'b1101,    // ICS644_settings_p
    4'b0011,    // ICS644_settings_i

    10'd44,     // p_horizontal_capture_start
    10'd684,    // p_horizontal_capture_end
    10'd0,      // p_vertical_capture_start
    10'd480,    // p_vertical_capture_end

    10'd1,      // i_horizontal_capture_start
    10'd641,    // i_horizontal_capture_end
    10'd0,      // i_vertical_capture_start
    10'd504,    // i_vertical_capture_end

    8'd1,       // buffer_size
    15'd640,    // ram_numwords
    15'd320,    // trigger_address
    15'd640     // buffer_line_length
};

