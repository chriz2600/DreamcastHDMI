const DCVideoConfig DC_VIDEO_CONFIG_480P = {
    4'b1100,    // ICS644_settings_p
    4'b0010,    // ICS644_settings_i

    10'd0 + `OFFSET_DC_CONF,      // p_horizontal_capture_start
    10'd720 + `OFFSET_DC_CONF,    // p_horizontal_capture_end
    10'd0,      // p_vertical_capture_start
    10'd480,    // p_vertical_capture_end

    10'd0 + `OFFSET_DC_CONF,      // i_horizontal_capture_start
    10'd720 + `OFFSET_DC_CONF,    // i_horizontal_capture_end
    10'd0,      // i_vertical_capture_start
    10'd504,    // i_vertical_capture_end

    8'd1,       // buffer_size
    `RAM_WIDTH'd720,    // ram_numwords
    `RAM_WIDTH'd360,    // trigger_address progressive
    `RAM_WIDTH'd360,    // trigger_address interlaced
    `RAM_WIDTH'd720     // buffer_line_length
};

const DCVideoConfig DC_VIDEO_CONFIG_VGA = {
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

    8'd1,       // buffer_size
    `RAM_WIDTH'd640,   // ram_numwords
    `RAM_WIDTH'd320,    // trigger_address progressive
    `RAM_WIDTH'd320,    // trigger_address interlaced
    `RAM_WIDTH'd640     // buffer_line_length
};

const DCVideoConfig DC_VIDEO_CONFIG_240P_480P = {
    4'b1100,    // ICS644_settings_p
    4'b0010,    // ICS644_settings_i

    10'd0 + `OFFSET_DC_CONF,      // p_horizontal_capture_start
    10'd720 + `OFFSET_DC_CONF,    // p_horizontal_capture_end
    10'd0,      // p_vertical_capture_start
    10'd480,    // p_vertical_capture_end

    10'd0 + `OFFSET_DC_CONF,      // i_horizontal_capture_start
    10'd720 + `OFFSET_DC_CONF,    // i_horizontal_capture_end
    10'd0,      // i_vertical_capture_start
    10'd504,    // i_vertical_capture_end

    8'd1,       // buffer_size
    `RAM_WIDTH'd720,    // ram_numwords
    `RAM_WIDTH'd360,    // trigger_address progressive
    `RAM_WIDTH'd360,    // trigger_address interlaced
    `RAM_WIDTH'd720     // buffer_line_length
};

const DCVideoConfig DC_VIDEO_CONFIG_240P_VGA = {
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

    8'd1,       // buffer_size
    `RAM_WIDTH'd640,    // ram_numwords
    `RAM_WIDTH'd320,    // trigger_address progressive
    `RAM_WIDTH'd320,    // trigger_address interlaced
    `RAM_WIDTH'd640     // buffer_line_length
};

///////////////////

const DCVideoConfig DC_VIDEO_CONFIG_480I = {
    4'b1100,    // ICS644_settings_p
    4'b0010,    // ICS644_settings_i

    10'd0 + `OFFSET_DC_CONF,      // p_horizontal_capture_start
    10'd720 + `OFFSET_DC_CONF,    // p_horizontal_capture_end
    10'd0,      // p_vertical_capture_start
    10'd480,    // p_vertical_capture_end

    10'd0 + `OFFSET_DC_CONF,      // i_horizontal_capture_start
    10'd720 + `OFFSET_DC_CONF,    // i_horizontal_capture_end
    10'd0,      // i_vertical_capture_start
    10'd504,    // i_vertical_capture_end

    8'd1,       // buffer_size
    `RAM_WIDTH'd720,    // ram_numwords
    `RAM_WIDTH'd360,    // trigger_address progressive
    `RAM_WIDTH'd360,    // trigger_address interlaced
    `RAM_WIDTH'd720     // buffer_line_length
};

const DCVideoConfig DC_VIDEO_CONFIG_576I = {
    4'b1100,    // ICS644_settings_p
    4'b0010,    // ICS644_settings_i

    10'd0 + `OFFSET_DC_CONF,      // p_horizontal_capture_start
    10'd720 + `OFFSET_DC_CONF,    // p_horizontal_capture_end
    10'd0,      // p_vertical_capture_start
    10'd576,    // p_vertical_capture_end

    10'd0 + `OFFSET_DC_CONF,      // i_horizontal_capture_start
    10'd720 + `OFFSET_DC_CONF,    // i_horizontal_capture_end
    10'd0,      // i_vertical_capture_start
    10'd600,    // i_vertical_capture_end

    8'd1,       // buffer_size
    `RAM_WIDTH'd720,    // ram_numwords
    `RAM_WIDTH'd360,    // trigger_address progressive
    `RAM_WIDTH'd360,    // trigger_address interlaced
    `RAM_WIDTH'd720     // buffer_line_length
};

const DCVideoConfig DC_VIDEO_CONFIG_288P = {
    4'b1100,    // ICS644_settings_p
    4'b0010,    // ICS644_settings_i

    10'd0 + `OFFSET_DC_CONF,      // p_horizontal_capture_start
    10'd720 + `OFFSET_DC_CONF,    // p_horizontal_capture_end
    10'd0,      // p_vertical_capture_start
    10'd576,    // p_vertical_capture_end

    10'd0 + `OFFSET_DC_CONF,      // i_horizontal_capture_start
    10'd720 + `OFFSET_DC_CONF,    // i_horizontal_capture_end
    10'd0,      // i_vertical_capture_start
    10'd600,    // i_vertical_capture_end

    8'd1,       // buffer_size
    `RAM_WIDTH'd720,    // ram_numwords
    `RAM_WIDTH'd360,    // trigger_address progressive
    `RAM_WIDTH'd360,    // trigger_address interlaced
    `RAM_WIDTH'd720     // buffer_line_length
};

const DCVideoConfig DC_VIDEO_CONFIG_576P = {
    4'b1100,    // ICS644_settings_p
    4'b0010,    // ICS644_settings_i

    10'd0 + `OFFSET_DC_CONF,      // p_horizontal_capture_start
    10'd720 + `OFFSET_DC_CONF,    // p_horizontal_capture_end
    10'd0,      // p_vertical_capture_start
    10'd576,    // p_vertical_capture_end

    10'd0 + `OFFSET_DC_CONF,      // i_horizontal_capture_start
    10'd720 + `OFFSET_DC_CONF,    // i_horizontal_capture_end
    10'd0,      // i_vertical_capture_start
    10'd600,    // i_vertical_capture_end

    8'd1,       // buffer_size
    `RAM_WIDTH'd720,    // ram_numwords
    `RAM_WIDTH'd360,    // trigger_address progressive
    `RAM_WIDTH'd360,    // trigger_address interlaced
    `RAM_WIDTH'd720     // buffer_line_length
};
