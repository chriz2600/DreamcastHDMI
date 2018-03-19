create_clock -period 54Mhz -name clk54 [get_ports clock54]
create_clock -period 74.25Mhz -name clk74_175824 [get_ports clock74_175824]
create_clock -period 27Mhz -name clk27 {data:video_input|raw_counterX_reg[0]}

derive_pll_clocks -create_base_clocks

set_clock_groups -exclusive \
    -group pll54|altpll_component|auto_generated|pll1|clk[0] \
    -group pll74|altpll_component|auto_generated|pll1|clk[0]
set_clock_groups -exclusive \
    -group clk27 \
    -group pll74|altpll_component|auto_generated|pll1|clk[0]

derive_clock_uncertainty
