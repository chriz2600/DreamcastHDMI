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

    8'd33,      // buffer_size
    15'd21120,  // ram_numwords
    15'd20920,  // trigger_address (ram_numwords - 200) progressive
    15'd20600,  // trigger_address (ram_numwords - 200) interlaced
    15'd640     // buffer_line_length
};

const DCVideoConfig DC_VIDEO_CONFIG_960P = {
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

    8'd24,      // buffer_size
    15'd15360,  // ram_numwords
    15'd440,    // trigger_address progressive
    15'd760,    // trigger_address interlaced
    15'd640     // buffer_line_length
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

    8'd24,      // buffer_size
    15'd15360,  // ram_numwords
    15'd440,    // trigger_address progressive
    15'd760,    // trigger_address interlaced
    15'd640     // buffer_line_length
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

    8'd33,      // buffer_size
    15'd21120,  // ram_numwords
    15'd20920,  // trigger_address (ram_numwords - 200) progressive
    15'd20600,  // trigger_address (ram_numwords - 200) interlaced
    15'd640     // buffer_line_length
};
