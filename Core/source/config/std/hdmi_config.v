const HDMIVideoConfig HDMI_VIDEO_CONFIG_1080P = {
    1'b1,           // horizontal_sync_on_polarity
    12'd2200,       // horizontal_pixels_per_line
    12'd1920,       // horizontal_pixels_visible
    12'd44,         // horizontal_sync_width
    12'd2008,       // horizontal_sync_start

    1'b1,           // vertical_sync_on_polarity
    11'd1080,       // vertical_lines_visible
    11'd5,          // vertical_sync_width

    11'd1125,       // vertical_lines_1
    11'd1083,       // vertical_sync_start_1
    12'd2008,       // vertical_sync_pixel_offset_1

    11'd1125,       // vertical_lines_2
    11'd1083,       // vertical_sync_start_2
    12'd2008,       // vertical_sync_pixel_offset_2

    10'd320,        // horizontal_offset
    10'd60,         // vertical_offset

    12'd320,        // horizontal_capture_start
    12'd1600,        // horizontal_capture_end

    12'd_0,       // horizontal_hq2x_start (horizontal_capture_end + 2)
    12'd_0,      // horizontal_hq2x_end (horizontal_pixels_per_line - 4)
    1'b0,           // is_hq2x_display_area

    11'd60,         // vertical_capture_start
    11'd1020,       // vertical_capture_end

    1'b0,           // interlaceOSD

    12'd620,        // osd_bg_offset_x_start
    12'd1300,       // osd_bg_offset_x_end
    11'd136,        // osd_bg_offset_y_start
    11'd954,        // osd_bg_offset_y_end

    12'd640,        // osd_text_x_start
    12'd1280,       // osd_text_x_end
    11'd156,        // osd_text_y_start
    11'd924,        // osd_text_y_end

    `RAM_WIDTH'd640,        // buffer_line_length
    `RAM_WIDTH'd21120,      // ram_numwords
    1'b1,           // line_doubling
    1'b1,           // pixel_repetition

    1'b1,           // pxl_rep_on
    4'd2,           // pxl_rep_h
    4'd2,           // pxl_rep_v
    4'd4,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd37_000_000  // startup_delay
};

const HDMIVideoConfig HDMI_VIDEO_CONFIG_960P = {
    1'b1,           // horizontal_sync_on_polarity
    12'd1800,       // horizontal_pixels_per_line
    12'd1280,       // horizontal_pixels_visible
    12'd112,        // horizontal_sync_width
    12'd1376,       // horizontal_sync_start

    1'b1,           // vertical_sync_on_polarity
    11'd960,        // vertical_lines_visible
    11'd3,          // vertical_sync_width

    11'd1000,       // vertical_lines_1
    11'd960,        // vertical_sync_start_1
    12'd1376,       // vertical_sync_pixel_offset_1

    11'd1000,       // vertical_lines_2
    11'd960,        // vertical_sync_start_2
    12'd1376,       // vertical_sync_pixel_offset_2

    10'd0,          // horizontal_offset
    10'd0,          // vertical_offset

    12'd0,          // horizontal_capture_start
    12'd1280,       // horizontal_capture_end

    12'd_0,       // horizontal_hq2x_start (horizontal_capture_end + 2)
    12'd_0,      // horizontal_hq2x_end (horizontal_pixels_per_line - 4)
    1'b0,           // is_hq2x_display_area

    11'd0,          // vertical_capture_start
    11'd960,        // vertical_capture_end

    1'b0,           // interlaceOSD

    12'd300,        // osd_bg_offset_x_start
    12'd980,        // osd_bg_offset_x_end
    11'd76,         // osd_bg_offset_y_start
    11'd894,        // osd_bg_offset_y_end

    12'd320,        // osd_text_x_start
    12'd960,        // osd_text_x_end
    11'd96,         // osd_text_y_start
    11'd864,        // osd_text_y_end

    `RAM_WIDTH'd640,        // buffer_line_length
    `RAM_WIDTH'd15360,      // ram_numwords
    1'b1,           // line_doubling
    1'b1,           // pixel_repetition

    1'b1,           // pxl_rep_on
    4'd2,           // pxl_rep_h
    4'd2,           // pxl_rep_v
    4'd4,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd27_000_000  // startup_delay
};

const HDMIVideoConfig HDMI_VIDEO_CONFIG_240P_960P = {
    1'b1,           // horizontal_sync_on_polarity
    12'd1716,       // horizontal_pixels_per_line
    12'd1280,       // horizontal_pixels_visible
    12'd112,        // horizontal_sync_width
    12'd1376,       // horizontal_sync_start

    1'b1,           // vertical_sync_on_polarity
    11'd960,        // vertical_lines_visible
    11'd3,          // vertical_sync_width

    11'd1052,        // vertical_lines_1
    11'd960,         // vertical_sync_start_1
    12'd1376,        // vertical_sync_pixel_offset_1

    11'd1052,        // vertical_lines_2
    11'd960,         // vertical_sync_start_2
    12'd1376,        // vertical_sync_pixel_offset_2

    10'd0,          // horizontal_offset
    10'd0,          // vertical_offset

    12'd0,          // horizontal_capture_start
    12'd1280,       // horizontal_capture_end

    12'd_0,       // horizontal_hq2x_start (horizontal_capture_end + 2)
    12'd_0,      // horizontal_hq2x_end (horizontal_pixels_per_line - 4)
    1'b0,           // is_hq2x_display_area

    11'd0,          // vertical_capture_start
    11'd960,        // vertical_capture_end

    1'b0,           // interlaceOSD

    12'd300,        // osd_bg_offset_x_start
    12'd980,        // osd_bg_offset_x_end
    11'd76,         // osd_bg_offset_y_start
    11'd894,        // osd_bg_offset_y_end

    12'd320,        // osd_text_x_start
    12'd960,        // osd_text_x_end
    11'd96,         // osd_text_y_start
    11'd864,        // osd_text_y_end

    `RAM_WIDTH'd640,        // buffer_line_length
    `RAM_WIDTH'd15360,      // ram_numwords
    1'b1,           // line_doubling
    1'b1,           // pixel_repetition

    1'b1,           // pxl_rep_on
    4'd2,           // pxl_rep_h
    4'd4,           // pxl_rep_v
    4'd4,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd27_000_000  // startup_delay
};

const HDMIVideoConfig HDMI_VIDEO_CONFIG_240P_1080P = {
    1'b1,           // horizontal_sync_on_polarity
    12'd2104,       // horizontal_pixels_per_line
    12'd1920,       // horizontal_pixels_visible
    12'd44,         // horizontal_sync_width
    12'd2008,       // horizontal_sync_start

    1'b1,           // vertical_sync_on_polarity
    11'd1080,       // vertical_lines_visible
    11'd5,          // vertical_sync_width

    11'd1144,       // vertical_lines_1
    11'd1083,       // vertical_sync_start_1
    12'd2008,       // vertical_sync_pixel_offset_1

    11'd1144,       // vertical_lines_2
    11'd1083,       // vertical_sync_start_2
    12'd2008,       // vertical_sync_pixel_offset_2

    10'd320,        // horizontal_offset
    10'd60,         // vertical_offset

    12'd320,        // horizontal_capture_start
    12'd1600,       // horizontal_capture_end

    12'd_0,       // horizontal_hq2x_start (horizontal_capture_end + 2)
    12'd_0,      // horizontal_hq2x_end (horizontal_pixels_per_line - 4)
    1'b0,           // is_hq2x_display_area

    11'd60,         // vertical_capture_start
    11'd1020,       // vertical_capture_end

    1'b0,           // interlaceOSD

    12'd620,        // osd_bg_offset_x_start
    12'd1300,       // osd_bg_offset_x_end
    11'd136,        // osd_bg_offset_y_start
    11'd954,        // osd_bg_offset_y_end

    12'd640,        // osd_text_x_start
    12'd1280,       // osd_text_x_end
    11'd156,        // osd_text_y_start
    11'd924,        // osd_text_y_end

    `RAM_WIDTH'd640,        // buffer_line_length
    `RAM_WIDTH'd21120,      // ram_numwords
    1'b1,           // line_doubling
    1'b1,           // pixel_repetition

    1'b1,           // pxl_rep_on
    4'd2,           // pxl_rep_h
    4'd4,           // pxl_rep_v
    4'd4,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd37_000_000  // startup_delay
};
