create_clock -period 54Mhz [get_ports clock54]
create_clock -period 74.175824Mhz [get_ports clock74_175824]

derive_pll_clocks
derive_clock_uncertainty