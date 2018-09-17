
const ADV7513Config ADV7513_CONFIG_1080P = {
    8'h_00, // adv_reg_01
    8'h_18, // adv_reg_02
    8'h_80, // adv_reg_03
    8'h_02, // adv_reg_17
    8'h_28, // adv_reg_56
    8'h_C8, // adv_reg_3b
    8'h_10  // adv_reg_3c
};

const ADV7513Config ADV7513_CONFIG_960P = {
    8'h_00, // adv_reg_01
    8'h_18, // adv_reg_02
    8'h_80, // adv_reg_03
    8'h_00, // adv_reg_17
    8'h_18, // adv_reg_56
    8'h_C8, // adv_reg_3b
    8'h_00  // adv_reg_3c
};

const ADV7513Config ADV7513_CONFIG_480P = {
    8'h_00, // adv_reg_01
    8'h_18, // adv_reg_02
    8'h_80, // adv_reg_03
    8'h_00, // adv_reg_17
    8'h_18, // adv_reg_56
    8'h_80, // adv_reg_3b
    8'h_02  // adv_reg_3c
};

const ADV7513Config ADV7513_CONFIG_VGA = {
    8'h_00, // adv_reg_01
    8'h_18, // adv_reg_02
    8'h_80, // adv_reg_03
    8'h_00, // adv_reg_17
    8'h_18, // adv_reg_56
    8'h_80, // adv_reg_3b
    8'h_01  // adv_reg_3c
};
