
const HDMIVideoConfig HDMI_VIDEO_CONFIG_480P = {
    1'b0,           // horizontal_sync_on_polarity
    12'd858,        // horizontal_pixels_per_line
    12'd720,        // horizontal_pixels_visible
    12'd62,         // horizontal_sync_width
    12'd736,        // horizontal_sync_start

    1'b0,           // vertical_sync_on_polarity
    11'd480,        // vertical_lines_visible
    11'd6,          // vertical_sync_width

    11'd525,        // vertical_lines_1
    11'd488,        // vertical_sync_start_1
    12'd736,        // vertical_sync_pixel_offset_1

    11'd525,        // vertical_lines_2
    11'd488,        // vertical_sync_start_2
    12'd736,        // vertical_sync_pixel_offset_2

    10'd0,          // horizontal_offset
    10'd0,          // vertical_offset

    12'd0,          // horizontal_capture_start
    12'd720,        // horizontal_capture_end

    12'd_722,       // horizontal_hq2x_start (horizontal_capture_end + 2)
    12'd_854,      // horizontal_hq2x_end (horizontal_pixels_per_line - 4)
    1'b0,           // is_hq2x_display_area

    11'd0,          // vertical_capture_start
    11'd480,        // vertical_capture_end

    1'b0,           // interlaceOSD

    12'd190,        // osd_bg_offset_x_start
    12'd530,        // osd_bg_offset_x_end
    11'd33,         // osd_bg_offset_y_start
    11'd447,        // osd_bg_offset_y_end

    12'd200,        // osd_text_x_start
    12'd520,        // osd_text_x_end
    11'd48,         // osd_text_y_start
    11'd432,        // osd_text_y_end

    `RAM_WIDTH'd720,        // buffer_line_length
    `RAM_WIDTH'd720,        // ram_numwords
    1'b0,           // line_doubling
    1'b0,           // pixel_repetition

    1'b0,           // pxl_rep_on
    4'd0,           // pxl_rep_h
    4'd0,           // pxl_rep_v
    4'd0,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd6_750_000   // startup_delay
};

const HDMIVideoConfig HDMI_VIDEO_CONFIG_VGA = {
    1'b0,           // horizontal_sync_on_polarity
    12'd800,        // horizontal_pixels_per_line
    12'd640,        // horizontal_pixels_visible
    12'd96,         // horizontal_sync_width
    12'd656,        // horizontal_sync_start

    1'b0,           // vertical_sync_on_polarity
    11'd480,        // vertical_lines_visible
    11'd2,          // vertical_sync_width

    11'd525,        // vertical_lines_1
    11'd489,        // vertical_sync_start_1
    12'd656,        // vertical_sync_pixel_offset_1

    11'd525,        // vertical_lines_2
    11'd489,        // vertical_sync_start_2
    12'd656,        // vertical_sync_pixel_offset_2

    10'd0,          // horizontal_offset
    10'd0,          // vertical_offset

    12'd0,          // horizontal_capture_start
    12'd640,        // horizontal_capture_end

    12'd_642,       // horizontal_hq2x_start (horizontal_capture_end + 2)
    12'd_796,      // horizontal_hq2x_end (horizontal_pixels_per_line - 4)
    1'b0,           // is_hq2x_display_area

    11'd0,          // vertical_capture_start
    11'd480,        // vertical_capture_end

    1'b0,           // interlaceOSD

    12'd150,        // osd_bg_offset_x_start
    12'd490,        // osd_bg_offset_x_end
    11'd33,         // osd_bg_offset_y_start
    11'd447,        // osd_bg_offset_y_end

    12'd160,        // osd_text_x_start
    12'd480,        // osd_text_x_end
    11'd48,         // osd_text_y_start
    11'd432,        // osd_text_y_end

    `RAM_WIDTH'd640,        // buffer_line_length
    `RAM_WIDTH'd640,       // ram_numwords
    1'b0,           // line_doubling
    1'b0,           // pixel_repetition

    1'b0,           // pxl_rep_on
    4'd0,           // pxl_rep_h
    4'd0,           // pxl_rep_v
    4'd0,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd6_300_000   // startup_delay
};

const HDMIVideoConfig HDMI_VIDEO_CONFIG_240P_480P = {
    1'b0,           // horizontal_sync_on_polarity
    12'd858,        // horizontal_pixels_per_line
    12'd720,        // horizontal_pixels_visible
    12'd62,         // horizontal_sync_width
    12'd736,        // horizontal_sync_start

    1'b0,           // vertical_sync_on_polarity
    11'd480,        // vertical_lines_visible
    11'd6,          // vertical_sync_width

    11'd526,        // vertical_lines_1
    11'd488,        // vertical_sync_start_1
    12'd736,        // vertical_sync_pixel_offset_1

    11'd526,        // vertical_lines_2
    11'd488,        // vertical_sync_start_2
    12'd736,        // vertical_sync_pixel_offset_2

    10'd0,          // horizontal_offset
    10'd0,          // vertical_offset

    12'd0,          // horizontal_capture_start
    12'd720,        // horizontal_capture_end

    12'd_0,       // horizontal_hq2x_start (horizontal_capture_end + 2)
    12'd_0,      // horizontal_hq2x_end (horizontal_pixels_per_line - 4)
    1'b0,           // is_hq2x_display_area

    11'd0,          // vertical_capture_start
    11'd480,        // vertical_capture_end

    1'b0,           // interlaceOSD

    12'd190,        // osd_bg_offset_x_start
    12'd530,        // osd_bg_offset_x_end
    11'd33,         // osd_bg_offset_y_start
    11'd447,        // osd_bg_offset_y_end

    12'd200,        // osd_text_x_start
    12'd520,        // osd_text_x_end
    11'd48,         // osd_text_y_start
    11'd432,        // osd_text_y_end

    `RAM_WIDTH'd720,        // buffer_line_length
    `RAM_WIDTH'd720,        // ram_numwords
    1'b0,           // line_doubling
    1'b0,           // pixel_repetition

    1'b0,           // pxl_rep_on
    4'd0,           // pxl_rep_h
    4'd0,           // pxl_rep_v
    4'd0,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd6_750_000   // startup_delay
};

const HDMIVideoConfig HDMI_VIDEO_CONFIG_240P_VGA = {
    1'b0,           // horizontal_sync_on_polarity
    12'd800,        // horizontal_pixels_per_line
    12'd640,        // horizontal_pixels_visible
    12'd96,         // horizontal_sync_width
    12'd656,        // horizontal_sync_start

    1'b0,           // vertical_sync_on_polarity
    11'd480,        // vertical_lines_visible
    11'd2,          // vertical_sync_width

    11'd526,        // vertical_lines_1
    11'd489,        // vertical_sync_start_1
    12'd656,        // vertical_sync_pixel_offset_1

    11'd526,        // vertical_lines_2
    11'd489,        // vertical_sync_start_2
    12'd656,        // vertical_sync_pixel_offset_2

    10'd0,          // horizontal_offset
    10'd0,          // vertical_offset

    12'd0,          // horizontal_capture_start
    12'd640,        // horizontal_capture_end

    12'd_0,       // horizontal_hq2x_start (horizontal_capture_end + 2)
    12'd_0,      // horizontal_hq2x_end (horizontal_pixels_per_line - 4)
    1'b0,           // is_hq2x_display_area

    11'd0,          // vertical_capture_start
    11'd480,        // vertical_capture_end

    1'b0,           // interlaceOSD

    12'd150,        // osd_bg_offset_x_start
    12'd490,        // osd_bg_offset_x_end
    11'd33,         // osd_bg_offset_y_start
    11'd447,        // osd_bg_offset_y_end

    12'd160,        // osd_text_x_start
    12'd480,        // osd_text_x_end
    11'd48,         // osd_text_y_start
    11'd432,        // osd_text_y_end

    `RAM_WIDTH'd640,        // buffer_line_length
    `RAM_WIDTH'd640,        // ram_numwords
    1'b0,           // line_doubling
    1'b0,           // pixel_repetition

    1'b0,           // pxl_rep_on
    4'd0,           // pxl_rep_h
    4'd0,           // pxl_rep_v
    4'd0,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd6_300_000   // startup_delay
};

// ------------------------------------------------

const HDMIVideoConfig HDMI_VIDEO_CONFIG_480I = { 
    1'b0,           // horizontal_sync_on_polarity
    12'd1716,       // horizontal_pixels_per_line
    12'd1440,       // horizontal_pixels_visible
    12'd124,        // horizontal_sync_width
    12'd1478,       // horizontal_sync_start

    1'b0,           // vertical_sync_on_polarity
    11'd240,        // vertical_lines_visible
    11'd3,          // vertical_sync_width

    11'd263,        // vertical_lines_1
    11'd244,        // vertical_sync_start_1
    12'd620,        // vertical_sync_pixel_offset_1

    11'd262,        // vertical_lines_2
    11'd243,        // vertical_sync_start_2
    12'd1478,       // vertical_sync_pixel_offset_2

    10'd0,          // horizontal_offset
    10'd0,          // vertical_offset

    12'd0,          // horizontal_capture_start
    12'd1440,       // horizontal_capture_end

    12'd_0,       // horizontal_hq2x_start (horizontal_capture_end + 2)
    12'd_0,      // horizontal_hq2x_end (horizontal_pixels_per_line - 4)
    1'b0,           // is_hq2x_display_area

    11'd0,          // vertical_capture_start
    11'd240,        // vertical_capture_end

    1'b1,           // interlaceOSD

    12'd380,        // osd_bg_offset_x_start
    12'd1060,       // osd_bg_offset_x_end
    11'd16,         // osd_bg_offset_y_start
    11'd223,        // osd_bg_offset_y_end

    12'd400,        // osd_text_x_start
    12'd1040,       // osd_text_x_end
    11'd24,         // osd_text_y_start
    11'd216,        // osd_text_y_end

    `RAM_WIDTH'd720,        // buffer_line_length
    `RAM_WIDTH'd720,        // ram_numwords
    1'b0,           // line_doubling
    1'd1,           // pixel_repetition

    1'b1,           // pxl_rep_on
    4'd2,           // pxl_rep_h
    4'd1,           // pxl_rep_v
    4'd1,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd6_750_000   // startup_delay
};

const HDMIVideoConfig HDMI_VIDEO_CONFIG_576I = { 
    1'b0,           // horizontal_sync_on_polarity
    12'd1728,       // horizontal_pixels_per_line
    12'd1440,       // horizontal_pixels_visible
    12'd126,        // horizontal_sync_width
    12'd1464,       // horizontal_sync_start

    1'b0,           // vertical_sync_on_polarity
    11'd288,        // vertical_lines_visible
    11'd3,          // vertical_sync_width

    11'd313,        // vertical_lines_1
    11'd290,        // vertical_sync_start_1
    12'd576,        // vertical_sync_pixel_offset_1

    11'd312,        // vertical_lines_2
    11'd289,        // vertical_sync_start_2
    12'd1464,       // vertical_sync_pixel_offset_2

    10'd0,          // horizontal_offset
    10'd0,          // vertical_offset

    12'd0,          // horizontal_capture_start
    12'd1440,       // horizontal_capture_end

    12'd_0,       // horizontal_hq2x_start (horizontal_capture_end + 2)
    12'd_0,      // horizontal_hq2x_end (horizontal_pixels_per_line - 4)
    1'b0,           // is_hq2x_display_area

    11'd0,          // vertical_capture_start
    11'd288,        // vertical_capture_end

    1'b1,           // interlaceOSD

    12'd380,        // osd_bg_offset_x_start
    12'd1060,       // osd_bg_offset_x_end
    11'd40,         // osd_bg_offset_y_start
    11'd249,        // osd_bg_offset_y_end

    12'd400,        // osd_text_x_start
    12'd1040,       // osd_text_x_end
    11'd48,         // osd_text_y_start
    11'd240,        // osd_text_y_end

    `RAM_WIDTH'd720,        // buffer_line_length
    `RAM_WIDTH'd720,        // ram_numwords
    1'b0,           // line_doubling
    1'd1,           // pixel_repetition

    1'b1,           // pxl_rep_on
    4'd2,           // pxl_rep_h
    4'd1,           // pxl_rep_v
    4'd1,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd6_750_000   // startup_delay
};

const HDMIVideoConfig HDMI_VIDEO_CONFIG_288P = {
    1'b0,           // horizontal_sync_on_polarity
    12'd864,        // horizontal_pixels_per_line
    12'd720,        // horizontal_pixels_visible
    12'd64,         // horizontal_sync_width
    12'd732,        // horizontal_sync_start

    1'b0,           // vertical_sync_on_polarity
    11'd576,        // vertical_lines_visible
    11'd5,          // vertical_sync_width

    11'd626,        // vertical_lines_1
    11'd580,        // vertical_sync_start_1
    12'd732,        // vertical_sync_pixel_offset_1

    11'd626,        // vertical_lines_2
    11'd580,        // vertical_sync_start_2
    12'd732,        // vertical_sync_pixel_offset_2

    10'd0,          // horizontal_offset
    10'd0,          // vertical_offset

    12'd0,          // horizontal_capture_start
    12'd720,        // horizontal_capture_end

    12'd_0,       // horizontal_hq2x_start (horizontal_capture_end + 2)
    12'd_0,      // horizontal_hq2x_end (horizontal_pixels_per_line - 4)
    1'b0,           // is_hq2x_display_area

    11'd0,          // vertical_capture_start
    11'd576,        // vertical_capture_end

    1'b0,           // interlaceOSD

    12'd190,        // osd_bg_offset_x_start
    12'd530,        // osd_bg_offset_x_end
    11'd80,         // osd_bg_offset_y_start
    11'd498,        // osd_bg_offset_y_end

    12'd200,        // osd_text_x_start
    12'd520,        // osd_text_x_end
    11'd96,         // osd_text_y_start
    11'd480,        // osd_text_y_end

    `RAM_WIDTH'd720,        // buffer_line_length
    `RAM_WIDTH'd720,        // ram_numwords
    1'b0,           // line_doubling
    1'b0,           // pixel_repetition

    1'b0,           // pxl_rep_on
    4'd0,           // pxl_rep_h
    4'd0,           // pxl_rep_v
    4'd0,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd6_750_000   // startup_delay
};

const HDMIVideoConfig HDMI_VIDEO_CONFIG_576P = {
    1'b0,           // horizontal_sync_on_polarity
    12'd864,        // horizontal_pixels_per_line
    12'd720,        // horizontal_pixels_visible
    12'd64,         // horizontal_sync_width
    12'd732,        // horizontal_sync_start

    1'b0,           // vertical_sync_on_polarity
    11'd576,        // vertical_lines_visible
    11'd5,          // vertical_sync_width

    11'd625,        // vertical_lines_1
    11'd580,        // vertical_sync_start_1
    12'd732,        // vertical_sync_pixel_offset_1

    11'd625,        // vertical_lines_2
    11'd580,        // vertical_sync_start_2
    12'd732,        // vertical_sync_pixel_offset_2

    10'd0,          // horizontal_offset
    10'd0,          // vertical_offset

    12'd0,          // horizontal_capture_start
    12'd720,        // horizontal_capture_end

    12'd_0,       // horizontal_hq2x_start (horizontal_capture_end + 2)
    12'd_0,      // horizontal_hq2x_end (horizontal_pixels_per_line - 4)
    1'b0,           // is_hq2x_display_area

    11'd0,          // vertical_capture_start
    11'd576,        // vertical_capture_end

    1'b0,           // interlaceOSD

    12'd190,        // osd_bg_offset_x_start
    12'd530,        // osd_bg_offset_x_end
    11'd80,         // osd_bg_offset_y_start
    11'd498,        // osd_bg_offset_y_end

    12'd200,        // osd_text_x_start
    12'd520,        // osd_text_x_end
    11'd96,         // osd_text_y_start
    11'd480,        // osd_text_y_end

    `RAM_WIDTH'd720,        // buffer_line_length
    `RAM_WIDTH'd720,        // ram_numwords
    1'b0,           // line_doubling
    1'b0,           // pixel_repetition

    1'b0,           // pxl_rep_on
    4'd0,           // pxl_rep_h
    4'd0,           // pxl_rep_v
    4'd0,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd6_750_000   // startup_delay
};

