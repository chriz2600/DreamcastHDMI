/*
    adv_reg_01: // \
    adv_reg_02: //  |--> audio clock regeneration N
    adv_reg_03: // /
    adv_reg_17: // 02: 16:9, 00: 4:3
    adv_reg_56: // 28: 16:9, 18: 4:3
    adv_reg_3b: // C8: input pll x2, 80: input pll x1
    adv_reg_3c: // 00 | vic_manual
*/
const ADV7513Config ADV7513_CONFIG_1080P = {
    1'b0,   // fullrange
    8'h_00, // adv_reg_01
    8'h_22, // adv_reg_02
    8'h_D6, // adv_reg_03
    8'h_02, // adv_reg_17
    8'h_28, // adv_reg_56 // 28
    8'h_C0, // adv_reg_3b
    `ADV_1080P_REG_3C // adv_reg_3c
};

const ADV7513Config ADV7513_CONFIG_960P = {
    1'b0,   // fullrange
    8'h_00, // adv_reg_01
    8'h_18, // adv_reg_02
    8'h_80, // adv_reg_03
    8'h_00, // adv_reg_17
    8'h_18, // adv_reg_56 // 18
    8'h_C0, // adv_reg_3b
    8'h_00  // adv_reg_3c
};

const ADV7513Config ADV7513_CONFIG_480P = {
    1'b0,   // fullrange
    8'h_00, // adv_reg_01
    8'h_18, // adv_reg_02
    8'h_80, // adv_reg_03
    8'h_00, // adv_reg_17
    8'h_18, // adv_reg_56
    8'h_80, // adv_reg_3b
    8'h_02  // adv_reg_3c
};

const ADV7513Config ADV7513_CONFIG_288P = {
    1'b0,   // fullrange
    8'h_00, // adv_reg_01
    8'h_18, // adv_reg_02
    8'h_80, // adv_reg_03
    8'h_00, // adv_reg_17
    8'h_18, // adv_reg_56
    8'h_80, // adv_reg_3b
    8'h_11  // adv_reg_3c
};

const ADV7513Config ADV7513_CONFIG_576P = {
    1'b0,   // fullrange
    8'h_00, // adv_reg_01
    8'h_18, // adv_reg_02
    8'h_80, // adv_reg_03
    8'h_00, // adv_reg_17
    8'h_18, // adv_reg_56
    8'h_80, // adv_reg_3b
    8'h_11  // adv_reg_3c
};

const ADV7513Config ADV7513_CONFIG_VGA = {
    1'b1,   // fullrange
    8'h_00, // adv_reg_01
    8'h_18, // adv_reg_02
    8'h_80, // adv_reg_03
    8'h_00, // adv_reg_17
    8'h_18, // adv_reg_56
    8'h_80, // adv_reg_3b
    8'h_01  // adv_reg_3c
};

const ADV7513Config ADV7513_CONFIG_240P_1080P = {
    1'b0,   // fullrange
    8'h_00, // adv_reg_01
    8'h_18, // adv_reg_02
    8'h_80, // adv_reg_03
    8'h_02, // adv_reg_17
    8'h_28, // adv_reg_56
    8'h_C0, // adv_reg_3b
    8'h_00  // adv_reg_3c
};

const ADV7513Config ADV7513_CONFIG_240P_960P = {
    1'b0,   // fullrange
    8'h_00, // adv_reg_01
    8'h_18, // adv_reg_02
    8'h_80, // adv_reg_03
    8'h_00, // adv_reg_17
    8'h_18, // adv_reg_56
    8'h_C0, // adv_reg_3b
    8'h_00  // adv_reg_3c
};

const ADV7513Config ADV7513_CONFIG_240P_480P = {
    1'b0,   // fullrange
    8'h_00, // adv_reg_01
    8'h_18, // adv_reg_02
    8'h_80, // adv_reg_03
    8'h_00, // adv_reg_17
    8'h_18, // adv_reg_56
    8'h_80, // adv_reg_3b
    8'h_02  // adv_reg_3c
};

const ADV7513Config ADV7513_CONFIG_240P_VGA = {
    1'b1,   // fullrange
    8'h_00, // adv_reg_01
    8'h_18, // adv_reg_02
    8'h_80, // adv_reg_03
    8'h_00, // adv_reg_17
    8'h_18, // adv_reg_56
    8'h_80, // adv_reg_3b
    8'h_01  // adv_reg_3c
};

/////////////////////////////////////////////////

const ADV7513Config ADV7513_CONFIG_480I = {
    1'b0,   // fullrange
    8'h_00, // adv_reg_01 \
    8'h_18, // adv_reg_02  |--> audio config
    8'h_80, // adv_reg_03 /
    8'h_00, // adv_reg_17 sync
    8'h_18, // adv_reg_56 aspect ratio
    8'h_80, // adv_reg_3b pxl repetition
    8'h_06  // adv_reg_3c VIC
};

const ADV7513Config ADV7513_CONFIG_576I = {
    1'b0,   // fullrange
    8'h_00, // adv_reg_01 \
    8'h_18, // adv_reg_02  |--> audio config
    8'h_80, // adv_reg_03 /
    8'h_00, // adv_reg_17 sync
    8'h_18, // adv_reg_56 aspect ratio
    8'h_80, // adv_reg_3b pxl repetition
    8'h_15  // adv_reg_3c VIC
};
