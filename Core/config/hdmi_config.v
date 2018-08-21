
const HDMIVideoConfig HDMI_VIDEO_CONFIG_1080P = {
    12'd1100,       // horizontal_pixels_per_line
    12'd960,        // horizontal_pixels_visible
    12'd1004,       // horizontal_sync_start
    12'd22,         // horizontal_sync_width
    1'b1,           // horizontal_sync_on_polarity

    11'd1125,       // vertical_lines
    11'd1125,       // vertical_lines_240p
    11'd1080,       // vertical_lines_visible
    11'd1084,       // vertical_sync_start
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

    8'h_02,         // adv_reg_17
    8'h_C8,         // adv_reg_3b
    8'h_10,         // adv_reg_3c

    32'd14_850_000, // startup_delay
    32'd74_250_000, // pixel_clock

    32'h_BC_8D_5E_2F // 47
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
    11'd961,        // vertical_sync_start
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

    8'h_00,         // adv_reg_17
    8'h_C8,         // adv_reg_3b
    8'h_00,         // adv_reg_3c

    32'd10_800_000, // startup_delay
    32'd54_000_000, // pixel_clock

    32'h_88_66_44_22 // 34
};

const HDMIVideoConfig HDMI_VIDEO_CONFIG_480P = { 
    12'd858,        // horizontal_pixels_per_line
    12'd720,        // horizontal_pixels_visible
    12'd746,        // horizontal_sync_start
    12'd62,         // horizontal_sync_width
    1'b0,           // horizontal_sync_on_polarity

    11'd525,        // vertical_lines
    11'd526,        // vertical_lines_240p
    11'd480,        // vertical_lines_visible
    11'd489,        // vertical_sync_start
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

    8'h_00,         // adv_reg_17
    8'h_80,         // adv_reg_3b
    8'h_02,         // adv_reg_3c

    32'd5_400_000,  // startup_delay
    32'd27_000_000, // pixel_clock

    32'h_44_33_22_11 // 17
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
    11'd490,        // vertical_sync_start
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

    8'h_00,         // adv_reg_17
    8'h_80,         // adv_reg_3b
    8'h_01,         // adv_reg_3c

    32'd5_040_000,  // startup_delay
    32'd25_200_000, // pixel_clock

    32'h_40_30_20_10 // 16
};
