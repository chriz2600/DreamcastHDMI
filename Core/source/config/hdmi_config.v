
const HDMIVideoConfig HDMI_VIDEO_CONFIG_1080P = {
    12'd1100,       // horizontal_pixels_per_line
    12'd960,        // horizontal_pixels_visible
    12'd1004,       // horizontal_sync_start
    12'd22,         // horizontal_sync_width
    1'b1,           // horizontal_sync_on_polarity

    11'd1125,       // vertical_lines
    11'd1125,       // vertical_lines_240p
    11'd1080,       // vertical_lines_visible
    11'd1083,       // vertical_sync_start
    11'd5,          // vertical_sync_width
    1'b1,           // vertical_sync_on_polarity

    10'd160,        // horizontal_offset
    10'd60,         // vertical_offset

    12'd160,        // horizontal_capture_start
    12'd800,        // horizontal_capture_end
    11'd60,         // vertical_capture_start
    11'd1020,       // vertical_capture_end

    12'd310,        // osd_bg_offset_x_start
    12'd650,        // osd_bg_offset_x_end
    11'd130,        // osd_bg_offset_y_start
    11'd958,        // osd_bg_offset_y_end

    12'd320,        // osd_text_x_start
    12'd640,        // osd_text_x_end
    11'd160,        // osd_text_y_start
    11'd928,        // osd_text_y_end

    12'd40,         // text_offset_character_x
    11'd5,          // text_offset_character_y

    15'd640,        // buffer_line_length
    15'd21120,      // ram_numwords
    1'b1,           // pixel_repetition

    1'b1,           // pxl_rep_on
    4'd1,           // pxl_rep_h
    4'd2,           // pxl_rep_v
    4'd4,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd18_562_500  // startup_delay
};

const HDMIVideoConfig HDMI_VIDEO_CONFIG_960P = {
    12'd900,        // horizontal_pixels_per_line
    12'd640,        // horizontal_pixels_visible
    12'd688,        // horizontal_sync_start
    12'd56,         // horizontal_sync_width
    1'b1,           // horizontal_sync_on_polarity

    11'd1000,       // vertical_lines
    11'd1000,       // vertical_lines_240p
    11'd960,        // vertical_lines_visible
    11'd960,        // vertical_sync_start
    11'd3,          // vertical_sync_width
    1'b1,           // vertical_sync_on_polarity

    10'd0,          // horizontal_offset
    10'd0,          // vertical_offset

    12'd0,          // horizontal_capture_start
    12'd640,        // horizontal_capture_end
    11'd0,          // vertical_capture_start
    11'd960,        // vertical_capture_end

    12'd150,        // osd_bg_offset_x_start
    12'd490,        // osd_bg_offset_x_end
    11'd76,         // osd_bg_offset_y_start
    11'd894,        // osd_bg_offset_y_end

    12'd160,        // osd_text_x_start
    12'd480,        // osd_text_x_end
    11'd96,        // osd_text_y_start
    11'd864,        // osd_text_y_end

    12'd20,         // text_offset_character_x
    11'd3,          // text_offset_character_y

    15'd640,        // buffer_line_length
    15'd15360,      // ram_numwords
    1'b1,           // pixel_repetition

    1'b1,           // pxl_rep_on
    4'd1,           // pxl_rep_h
    4'd2,           // pxl_rep_v
    4'd4,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd13_500_000  // startup_delay
};

const HDMIVideoConfig HDMI_VIDEO_CONFIG_480P = {
    12'd858,        // horizontal_pixels_per_line
    12'd720,        // horizontal_pixels_visible
    12'd736,        // horizontal_sync_start
    12'd62,         // horizontal_sync_width
    1'b0,           // horizontal_sync_on_polarity

    11'd525,        // vertical_lines
    11'd526,        // vertical_lines_240p
    11'd480,        // vertical_lines_visible
    11'd488,        // vertical_sync_start
    11'd6,          // vertical_sync_width
    1'b0,           // vertical_sync_on_polarity

    10'd0,          // horizontal_offset
    10'd0,          // vertical_offset

    12'd0,          // horizontal_capture_start
    12'd720,        // horizontal_capture_end
    11'd0,          // vertical_capture_start
    11'd480,        // vertical_capture_end

    12'd190,        // osd_bg_offset_x_start
    12'd530,        // osd_bg_offset_x_end
    11'd33,         // osd_bg_offset_y_start
    11'd447,        // osd_bg_offset_y_end

    12'd200,        // osd_text_x_start
    12'd520,        // osd_text_x_end
    11'd48,         // osd_text_y_start
    11'd432,        // osd_text_y_end

    12'd25,         // text_offset_character_x
    11'd3,          // text_offset_character_y

    15'd720,        // buffer_line_length
    15'd720,        // ram_numwords
    1'b0,           // pixel_repetition

    1'b0,           // pxl_rep_on
    4'd0,           // pxl_rep_h
    4'd0,           // pxl_rep_v
    4'd0,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd6_750_000   // startup_delay
};

const HDMIVideoConfig HDMI_VIDEO_CONFIG_VGA = {
    12'd800,        // horizontal_pixels_per_line
    12'd640,        // horizontal_pixels_visible
    12'd656,        // horizontal_sync_start
    12'd96,         // horizontal_sync_width
    1'b0,           // horizontal_sync_on_polarity

    11'd525,        // vertical_lines
    11'd526,        // vertical_lines_240p
    11'd480,        // vertical_lines_visible
    11'd489,        // vertical_sync_start
    11'd2,          // vertical_sync_width
    1'b0,           // vertical_sync_on_polarity

    10'd0,          // horizontal_offset
    10'd0,          // vertical_offset

    12'd0,          // horizontal_capture_start
    12'd640,        // horizontal_capture_end
    11'd0,          // vertical_capture_start
    11'd480,        // vertical_capture_end

    12'd150,        // osd_bg_offset_x_start
    12'd490,        // osd_bg_offset_x_end
    11'd33,         // osd_bg_offset_y_start
    11'd447,        // osd_bg_offset_y_end

    12'd160,        // osd_text_x_start
    12'd480,        // osd_text_x_end
    11'd48,         // osd_text_y_start
    11'd432,        // osd_text_y_end

    12'd20,         // text_offset_character_x
    11'd3,          // text_offset_character_y

    15'd640,        // buffer_line_length
    15'd640,        // ram_numwords
    1'b0,           // pixel_repetition

    1'b0,           // pxl_rep_on
    4'd0,           // pxl_rep_h
    4'd0,           // pxl_rep_v
    4'd0,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd6_300_000   // startup_delay
};

const HDMIVideoConfig HDMI_VIDEO_CONFIG_240Px3 = {
    12'd1716,       // horizontal_pixels_per_line
    12'd1280,       // horizontal_pixels_visible
    12'd1390,       // horizontal_sync_start
    12'd40,         // horizontal_sync_width
    1'b1,           // horizontal_sync_on_polarity

    11'd789,        // vertical_lines
    11'd789,        // vertical_lines_240p
    11'd720,        // vertical_lines_visible
    11'd724,        // vertical_sync_start
    11'd5,          // vertical_sync_width
    1'b1,           // vertical_sync_on_polarity

    10'd160,        // horizontal_offset
    10'd0,          // vertical_offset

    12'd160,        // horizontal_capture_start
    12'd1120,       // horizontal_capture_end
    11'd0,          // vertical_capture_start
    11'd720,        // vertical_capture_end

    12'd300,        // osd_bg_offset_x_start
    12'd980,        // osd_bg_offset_x_end
    11'd33,         // osd_bg_offset_y_start
    11'd447,        // osd_bg_offset_y_end

    12'd320,        // osd_text_x_start
    12'd960,        // osd_text_x_end
    11'd48,         // osd_text_y_start
    11'd432,        // osd_text_y_end

    12'd25,         // text_offset_character_x
    11'd3,          // text_offset_character_y

    15'd640,        // buffer_line_length
    15'd2560,       // ram_numwords
    1'b0,           // pixel_repetition

    1'b1,           // pxl_rep_on
    4'd3,           // pxl_rep_h
    4'd3,           // pxl_rep_v
    4'd3,           // pxl_rep_v_i
    2'd2,           // pxl_rep_addr_inr_h

    32'd20_250_000  // startup_delay
};

const HDMIVideoConfig HDMI_VIDEO_CONFIG_240Px4 = {
    12'd858,        // horizontal_pixels_per_line
    12'd640,        // horizontal_pixels_visible
    12'd688,        // horizontal_sync_start
    12'd56,         // horizontal_sync_width
    1'b1,           // horizontal_sync_on_polarity

    11'd1052,       // vertical_lines
    11'd1052,       // vertical_lines_240p
    11'd960,        // vertical_lines_visible
    11'd960,        // vertical_sync_start
    11'd3,          // vertical_sync_width
    1'b1,           // vertical_sync_on_polarity

    10'd0,          // horizontal_offset
    10'd0,          // vertical_offset

    12'd0,          // horizontal_capture_start
    12'd640,        // horizontal_capture_end
    11'd0,          // vertical_capture_start
    11'd960,        // vertical_capture_end

    12'd150,        // osd_bg_offset_x_start
    12'd490,        // osd_bg_offset_x_end
    11'd76,         // osd_bg_offset_y_start
    11'd894,        // osd_bg_offset_y_end

    12'd160,        // osd_text_x_start
    12'd480,        // osd_text_x_end
    11'd96,         // osd_text_y_start
    11'd864,        // osd_text_y_end

    12'd20,         // text_offset_character_x
    11'd3,          // text_offset_character_y

    15'd640,        // buffer_line_length
    15'd15360,      // ram_numwords
    1'b1,           // pixel_repetition

    1'b1,           // pxl_rep_on
    4'd1,           // pxl_rep_h
    4'd4,           // pxl_rep_v
    4'd4,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd13_500_000  // startup_delay
};

const HDMIVideoConfig HDMI_VIDEO_CONFIG_240P_1080P = {
    12'd1052,       // horizontal_pixels_per_line
    12'd960,        // horizontal_pixels_visible
    12'd1004,       // horizontal_sync_start
    12'd22,         // horizontal_sync_width
    1'b1,           // horizontal_sync_on_polarity

    11'd1144,       // vertical_lines
    11'd1144,       // vertical_lines_240p
    11'd1080,       // vertical_lines_visible
    11'd1083,       // vertical_sync_start
    11'd5,          // vertical_sync_width
    1'b1,           // vertical_sync_on_polarity

    10'd160,        // horizontal_offset
    10'd60,         // vertical_offset

    12'd160,        // horizontal_capture_start
    12'd800,        // horizontal_capture_end
    11'd60,         // vertical_capture_start
    11'd1020,       // vertical_capture_end

    12'd310,        // osd_bg_offset_x_start
    12'd650,        // osd_bg_offset_x_end
    11'd130,        // osd_bg_offset_y_start
    11'd958,        // osd_bg_offset_y_end

    12'd320,        // osd_text_x_start
    12'd640,        // osd_text_x_end
    11'd160,        // osd_text_y_start
    11'd928,        // osd_text_y_end

    12'd40,         // text_offset_character_x
    11'd5,          // text_offset_character_y

    15'd640,        // buffer_line_length
    15'd21120,      // ram_numwords
    1'b1,           // pixel_repetition

    1'b1,           // pxl_rep_on
    4'd1,           // pxl_rep_h
    4'd4,           // pxl_rep_v
    4'd4,           // pxl_rep_v_i
    2'd1,           // pxl_rep_addr_inr_h

    32'd18_562_500  // startup_delay
};

