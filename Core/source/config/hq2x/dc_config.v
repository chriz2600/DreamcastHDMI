const DCVideoConfig DC_VIDEO_CONFIG_1080P = {
    4'b1101,    // ICS644_settings_p
    4'b0011,    // ICS644_settings_i

    10'd40 + `OFFSET_DC_CONF,     // p_horizontal_capture_start
    10'd680 + `OFFSET_DC_CONF,    // p_horizontal_capture_end
    10'd0,      // p_vertical_capture_start
    10'd480,    // p_vertical_capture_end

    10'd40 + `OFFSET_DC_CONF,     // i_horizontal_capture_start
    10'd680 + `OFFSET_DC_CONF,    // i_horizontal_capture_end
    10'd0,      // i_vertical_capture_start
    10'd504,    // i_vertical_capture_end

    8'd23,      // buffer_size
    14'd14720,  // ram_numwords
    14'd14520,  // trigger_address (ram_numwords - 200) progressive
    14'd14320,  // trigger_address (ram_numwords - 200) interlaced
    14'd640     // buffer_line_length
};

const DCVideoConfig DC_VIDEO_CONFIG_960P = {
    4'b1100,    // ICS644_settings_p
    4'b0010,    // ICS644_settings_i

    10'd40 + `OFFSET_DC_CONF,     // p_horizontal_capture_start
    10'd680 + `OFFSET_DC_CONF,    // p_horizontal_capture_end
    10'd0,      // p_vertical_capture_start
    10'd480,    // p_vertical_capture_end

    10'd40 + `OFFSET_DC_CONF,     // i_horizontal_capture_start
    10'd680 + `OFFSET_DC_CONF,    // i_horizontal_capture_end
    10'd0,      // i_vertical_capture_start
    10'd504,    // i_vertical_capture_end

    8'd2,       // buffer_size
    14'd1280,   // ram_numwords
    14'd800,    // trigger_address progressive
    14'd800,    // trigger_address interlaced
    14'd640     // buffer_line_length
};

const DCVideoConfig DC_VIDEO_CONFIG_240P_960P = {
    4'b1100,    // ICS644_settings_p
    4'b0010,    // ICS644_settings_i

    10'd40 + `OFFSET_DC_CONF,     // p_horizontal_capture_start
    10'd680 + `OFFSET_DC_CONF,    // p_horizontal_capture_end
    10'd0,      // p_vertical_capture_start
    10'd480,    // p_vertical_capture_end

    10'd40 + `OFFSET_DC_CONF,     // i_horizontal_capture_start
    10'd680 + `OFFSET_DC_CONF,    // i_horizontal_capture_end
    10'd0,      // i_vertical_capture_start
    10'd504,    // i_vertical_capture_end

    8'd2,       // buffer_size
    14'd1280,   // ram_numwords
    14'd640,    // trigger_address progressive
    14'd640,    // trigger_address interlaced
    14'd640     // buffer_line_length
};

const DCVideoConfig DC_VIDEO_CONFIG_240P_1080P = {
    4'b1100,    // ICS644_settings_p
    4'b0010,    // ICS644_settings_i

    10'd40 + `OFFSET_DC_CONF,     // p_horizontal_capture_start
    10'd680 + `OFFSET_DC_CONF,    // p_horizontal_capture_end
    10'd0,      // p_vertical_capture_start
    10'd480,    // p_vertical_capture_end

    10'd40 + `OFFSET_DC_CONF,     // i_horizontal_capture_start
    10'd680 + `OFFSET_DC_CONF,    // i_horizontal_capture_end
    10'd0,      // i_vertical_capture_start
    10'd504,    // i_vertical_capture_end

    8'd23,      // buffer_size
    14'd14720,  // ram_numwords
    14'd14520,  // trigger_address (ram_numwords - 200) progressive
    14'd14320,  // trigger_address (ram_numwords - 200) interlaced
    14'd640     // buffer_line_length
};
