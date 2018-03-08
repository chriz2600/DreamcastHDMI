create_clock -period 54Mhz [get_ports clock54]
create_clock -period 74.25Mhz [get_ports clock74_175824]

create_clock -period 27Mhz data:inst3|raw_counterX_reg[0]

derive_pll_clocks
derive_clock_uncertainty