    case (address)
        0: q_reg <= 1'b0; //  -- Reserved Bits = 0 (1 bit(s))
        1: q_reg <= 1'b0; //  -- Reserved Bits = 0 (1 bit(s))
        2: q_reg <= 1'b0; //  -- Loop Filter Capacitance = 0 (2 bit(s)) (Setting 0)
        3: q_reg <= 1'b0; // 
        4: q_reg <= 1'b0; //  -- Loop Filter Resistance = 8 (5 bit(s)) (Setting 8)
        5: q_reg <= 1'b1; // 
        6: q_reg <= 1'b0; // 
        7: q_reg <= 1'b0; // 
        8: q_reg <= 1'b0; // 
        9: q_reg <= 1'b0; //  -- VCO Post Scale = 0 (1 bit(s)) (VCO post-scale divider counter value = 2)
        10: q_reg <= 1'b0; //  -- Reserved Bits = 0 (5 bit(s))
        11: q_reg <= 1'b0; // 
        12: q_reg <= 1'b0; // 
        13: q_reg <= 1'b0; // 
        14: q_reg <= 1'b0; // 
        15: q_reg <= 1'b0; //  -- Charge Pump Current = 1 (3 bit(s)) (Setting 1)
        16: q_reg <= 1'b0; // 
        17: q_reg <= 1'b1; // 
        18: q_reg <= 1'b0; //  -- N counter: Bypass = 0 (1 bit(s))
        19: q_reg <= 1'b0; //  -- N counter: High Count = 7 (8 bit(s))
        20: q_reg <= 1'b0; // 
        21: q_reg <= 1'b0; // 
        22: q_reg <= 1'b0; // 
        23: q_reg <= 1'b0; // 
        24: q_reg <= 1'b1; // 
        25: q_reg <= 1'b1; // 
        26: q_reg <= 1'b1; // 
        27: q_reg <= 1'b1; //  -- N counter: Odd Division = 1 (1 bit(s))
        28: q_reg <= 1'b0; //  -- N counter: Low Count = 6 (8 bit(s))
        29: q_reg <= 1'b0; // 
        30: q_reg <= 1'b0; // 
        31: q_reg <= 1'b0; // 
        32: q_reg <= 1'b0; // 
        33: q_reg <= 1'b1; // 
        34: q_reg <= 1'b1; // 
        35: q_reg <= 1'b0; // 
        36: q_reg <= 1'b0; //  -- M counter: Bypass = 0 (1 bit(s))
        37: q_reg <= 1'b0; //  -- M counter: High Count = 52 (8 bit(s))
        38: q_reg <= 1'b0; // 
        39: q_reg <= 1'b1; // 
        40: q_reg <= 1'b1; // 
        41: q_reg <= 1'b0; // 
        42: q_reg <= 1'b1; // 
        43: q_reg <= 1'b0; // 
        44: q_reg <= 1'b0; // 
        45: q_reg <= 1'b0; //  -- M counter: Odd Division = 0 (1 bit(s))
        46: q_reg <= 1'b0; //  -- M counter: Low Count = 52 (8 bit(s))
        47: q_reg <= 1'b0; // 
        48: q_reg <= 1'b1; // 
        49: q_reg <= 1'b1; // 
        50: q_reg <= 1'b0; // 
        51: q_reg <= 1'b1; // 
        52: q_reg <= 1'b0; // 
        53: q_reg <= 1'b0; // 
        54: q_reg <= 1'b0; //  -- clk0 counter: Bypass = 0 (1 bit(s))
        55: q_reg <= 1'b0; //  -- clk0 counter: High Count = 6 (8 bit(s))
        56: q_reg <= 1'b0; // 
        57: q_reg <= 1'b0; // 
        58: q_reg <= 1'b0; // 
        59: q_reg <= 1'b0; // 
        60: q_reg <= 1'b1; // 
        61: q_reg <= 1'b1; // 
        62: q_reg <= 1'b0; // 
        63: q_reg <= 1'b1; //  -- clk0 counter: Odd Division = 1 (1 bit(s))
        64: q_reg <= 1'b0; //  -- clk0 counter: Low Count = 5 (8 bit(s))
        65: q_reg <= 1'b0; // 
        66: q_reg <= 1'b0; // 
        67: q_reg <= 1'b0; // 
        68: q_reg <= 1'b0; // 
        69: q_reg <= 1'b1; // 
        70: q_reg <= 1'b0; // 
        71: q_reg <= 1'b1; // 
        72: q_reg <= 1'b0; //  -- clk1 counter: Bypass = 0 (1 bit(s))
        73: q_reg <= 1'b0; //  -- clk1 counter: High Count = 6 (8 bit(s))
        74: q_reg <= 1'b0; // 
        75: q_reg <= 1'b0; // 
        76: q_reg <= 1'b0; // 
        77: q_reg <= 1'b0; // 
        78: q_reg <= 1'b1; // 
        79: q_reg <= 1'b1; // 
        80: q_reg <= 1'b0; // 
        81: q_reg <= 1'b1; //  -- clk1 counter: Odd Division = 1 (1 bit(s))
        82: q_reg <= 1'b0; //  -- clk1 counter: Low Count = 5 (8 bit(s))
        83: q_reg <= 1'b0; // 
        84: q_reg <= 1'b0; // 
        85: q_reg <= 1'b0; // 
        86: q_reg <= 1'b0; // 
        87: q_reg <= 1'b1; // 
        88: q_reg <= 1'b0; // 
        89: q_reg <= 1'b1; // 
        90: q_reg <= 1'b1; //  -- clk2 counter: Bypass = 1 (1 bit(s))
        91: q_reg <= 1'b0; //  -- clk2 counter: High Count = 0 (8 bit(s))
        92: q_reg <= 1'b0; // 
        93: q_reg <= 1'b0; // 
        94: q_reg <= 1'b0; // 
        95: q_reg <= 1'b0; // 
        96: q_reg <= 1'b0; // 
        97: q_reg <= 1'b0; // 
        98: q_reg <= 1'b0; // 
        99: q_reg <= 1'b0; //  -- clk2 counter: Odd Division = 0 (1 bit(s))
        100: q_reg <= 1'b0; //  -- clk2 counter: Low Count = 0 (8 bit(s))
        101: q_reg <= 1'b0; // 
        102: q_reg <= 1'b0; // 
        103: q_reg <= 1'b0; // 
        104: q_reg <= 1'b0; // 
        105: q_reg <= 1'b0; // 
        106: q_reg <= 1'b0; // 
        107: q_reg <= 1'b0; // 
        108: q_reg <= 1'b1; //  -- clk3 counter: Bypass = 1 (1 bit(s))
        109: q_reg <= 1'b0; //  -- clk3 counter: High Count = 0 (8 bit(s))
        110: q_reg <= 1'b0; // 
        111: q_reg <= 1'b0; // 
        112: q_reg <= 1'b0; // 
        113: q_reg <= 1'b0; // 
        114: q_reg <= 1'b0; // 
        115: q_reg <= 1'b0; // 
        116: q_reg <= 1'b0; // 
        117: q_reg <= 1'b0; //  -- clk3 counter: Odd Division = 0 (1 bit(s))
        118: q_reg <= 1'b0; //  -- clk3 counter: Low Count = 0 (8 bit(s))
        119: q_reg <= 1'b0; // 
        120: q_reg <= 1'b0; // 
        121: q_reg <= 1'b0; // 
        122: q_reg <= 1'b0; // 
        123: q_reg <= 1'b0; // 
        124: q_reg <= 1'b0; // 
        125: q_reg <= 1'b0; // 
        126: q_reg <= 1'b1; //  -- clk4 counter: Bypass = 1 (1 bit(s))
        127: q_reg <= 1'b0; //  -- clk4 counter: High Count = 0 (8 bit(s))
        128: q_reg <= 1'b0; // 
        129: q_reg <= 1'b0; // 
        130: q_reg <= 1'b0; // 
        131: q_reg <= 1'b0; // 
        132: q_reg <= 1'b0; // 
        133: q_reg <= 1'b0; // 
        134: q_reg <= 1'b0; // 
        135: q_reg <= 1'b0; //  -- clk4 counter: Odd Division = 0 (1 bit(s))
        136: q_reg <= 1'b0; //  -- clk4 counter: Low Count = 0 (8 bit(s))
        137: q_reg <= 1'b0; // 
        138: q_reg <= 1'b0; // 
        139: q_reg <= 1'b0; // 
        140: q_reg <= 1'b0; // 
        141: q_reg <= 1'b0; // 
        142: q_reg <= 1'b0; // 
        143: q_reg <= 1'b0; // 
    endcase
